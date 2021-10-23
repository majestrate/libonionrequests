#pragma once
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <exception>
#include <sodium/randombytes.h>

namespace
{
  /// @brief psuedo random number generator
  class PRNG
  {
   public:
    using result_type = uint64_t;

    static constexpr result_type
    min()
    {
      return std::numeric_limits<result_type>::min();
    };

    static constexpr result_type
    max()
    {
      return std::numeric_limits<result_type>::max();
    };

    result_type
    operator()()
    {
      result_type r{};
      ::randombytes(reinterpret_cast<uint8_t*>(&r), sizeof(r));
      return r;
    };
  };

  template <typename Out_Type, typename Container>
  constexpr Out_Type
  pick_random(typename Container::const_iterator begin, typename Container::const_iterator end)
  {
    std::vector<typename Container::value_type> _vec;
    std::sample(begin, end, std::back_inserter(_vec), std::distance(begin, end), PRNG{});

    if (_vec.empty())
      throw std::invalid_argument{"selection range empty"};

    return Out_Type{_vec[0]};
  }

  template <typename Out_Type, typename Container>
  constexpr Out_Type
  pick_random_from(const Container& _in)
  {
    return pick_random<Out_Type, Container>(_in.begin(), _in.end());
  }

}  // namespace
