
enable_testing()

find_package(spdlog REQUIRED)

add_executable(onionrequest onionrequest.cpp)
target_link_libraries(onionrequest PUBLIC onionreq spdlog::spdlog cpr::cpr)

add_custom_target(check COMMAND onionrequest)
