#include <onionreq/onionreq.hpp>
#include <onionreq/random.hpp>
#include <spdlog/spdlog.h>
#include <oxenmq/oxenmq.h>

#include <future>
#include <signal.h>

#include <cpr/cpr.h>

static auto
to_spdlog_level(oxenmq::LogLevel lvl)
{
  switch (lvl)
  {
    case oxenmq::LogLevel::trace:
      return spdlog::level::level_enum::trace;
    case oxenmq::LogLevel::debug:
      return spdlog::level::level_enum::debug;
    case oxenmq::LogLevel::info:
      return spdlog::level::level_enum::info;
    case oxenmq::LogLevel::warn:
      return spdlog::level::level_enum::warn;
    default:
    case oxenmq::LogLevel::error:
      return spdlog::level::level_enum::err;
  }
}

std::promise<int> _exit_promise;

void
_handle_signal(int)
{
  _exit_promise.set_value(0);
}

class Transport_CURL : public onionreq::Transport_Base
{
 public:
  void
  SendPayload(
      onionreq::OnionPayload payload,
      std::function<void(std::optional<std::string>)> responseHandler) override
  {
    const auto url = payload.path.Edge().HttpsDirect();
    spdlog::info("sending to {}", url);
    auto res = cpr::Post(cpr::Url{url}, cpr::Body{payload.ciphertext}, cpr::VerifySsl{false});
    spdlog::info("http {}", res.status_code);
    if (res.status_code == 200)
      responseHandler(std::optional<std::string>{std::move(res.text)});
    else
    {
      spdlog::error("http: {}", res.text);
      responseHandler(std::nullopt);
    }
  }
};

int
main(int argc, char* argv[])
{
  using namespace std::literals;

  signal(SIGINT, _handle_signal);
  signal(SIGTERM, _handle_signal);

  constexpr auto RefreshNodeListInterval = 5min;

  std::unordered_set<std::string> seeds = {
      {"curve://public.loki.foundation:22027/"
       "3c157ed3c675f56280dc5d8b2f00b327b5865c127bf2c6c42becc3ca73d9132b"}};

  oxenmq::OxenMQ _mq{
      [](oxenmq::LogLevel lvl, const char* file, int line, std::string msg) {
        spdlog::log(to_spdlog_level(lvl), "{}:{} || {}", file, line, msg);
      },
      oxenmq::LogLevel::debug};

  std::unique_ptr<onionreq::Consensus_Base> consensus{
      onionreq::Consensus(onionreq::direct_oxenmq{}, _mq)};

  for (const auto& seed : seeds)
    spdlog::info("using seed node: {}", seed);
  consensus->SeedNodes(std::move(seeds));

  _mq.start();

  std::shared_ptr<onionreq::NodeListFetcher_Base> fetcher{consensus->CreateNodeFetcher()};

  std::shared_ptr<onionreq::PathSelection_Base> pathselector{consensus->CreatePathSelector()};

  std::shared_ptr<onionreq::Transport_Base> transport{new Transport_CURL{}};

  onionreq::SNodeInfo remote;

  std::promise<std::optional<onionreq::SNodeInfo>> _remote;

  auto refreshNodeList = [fetcher, pathselector, &_remote]() {
    spdlog::info("fetching node list");
    fetcher->FetchAll([pathselector, &_remote](auto result) {
      spdlog::info("got node list with {} nodes", result.size());
      if (result.empty())
      {
        _remote.set_value(std::nullopt);
        return;
      }
      const auto found = pick_random_from<typename decltype(result)::value_type>(result);
      pathselector->StoreNodeList(std::move(result));
      spdlog::info("chose remote {}", found.second.HttpsDirect());
      _remote.set_value(std::optional<onionreq::SNodeInfo>{found.second});
    });
  };

  refreshNodeList();

  if (auto maybe = _remote.get_future().get())
    remote = *maybe;
  else
    return 1;

  auto handler = [pathselector](std::optional<std::string> maybePlaintext) {
    if (maybePlaintext)
    {
      const auto json = nlohmann::json::parse(*maybePlaintext);
      if (auto itr = json.find("body"); itr != json.end())
      {
        const auto inner = nlohmann::json::parse(itr->get<std::string>());
        spdlog::info("got reply: {}", inner.dump());
      }
      _exit_promise.set_value(0);
    }
    else
    {
      spdlog::error("Failed to decrypt reply");
      _exit_promise.set_value(1);
    }
  };

  auto sendOnion = [&remote,
                    transport,
                    handler,
                    pathselector,
                    onionmaker = std::shared_ptr<onionreq::OnionMaker_Base>{
                        OnionMaker(onionreq::all_xchacha20_hops{})}](auto req) {
    spdlog::info("data={}", req.dump());
    spdlog::info("selecting hop to {}", remote.DirectAddr());
    if (auto maybe = pathselector->MaybeSelectHopsTo(remote))
    {
      const auto payload = onionmaker->MakeOnion(req.dump(), *maybe);
      spdlog::info(
          "send payload of {} bytes to {} via edge {}",
          payload.ciphertext.size(),
          remote.DirectAddr(),
          maybe->Edge().DirectAddr());
      transport->SendPayload(
          payload,
          [process = payload.MakeDecrypter(handler), path = *maybe, pathselector](
              auto maybeResponse) {
            if (maybeResponse)
            {
              process(*maybeResponse);
            }
            else
            {
              spdlog::error("no response after onion request");
              _exit_promise.set_value(1);
            }
          });
    }
    else
    {
      spdlog::warn("could not select hops to {}", remote.DirectAddr());
      _exit_promise.set_value(1);
    }
  };
  std::string_view user_pubkey =
      "05fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210";

  std::map<std::string_view, std::string_view> params{{"pubkey", user_pubkey}};
  sendOnion(nlohmann::json{{"method", "get_snodes_for_pubkey"}, {"params", params}});

  auto future = _exit_promise.get_future();

  auto code = future.get();
  spdlog::info("exit code {}", code);
  return code;
}
