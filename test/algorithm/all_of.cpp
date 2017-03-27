// Range v3 library
//
//  Copyright Andrew Sutton 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3

#include <vector>
#include <range/v3/core.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include "../simple_test.hpp"

RANGES_DISABLE_WARNINGS
RANGES_DIAGNOSTIC_IGNORE_MISSING_DECLARATIONS

bool even(int n) { return n % 2 == 0; }

struct S {
  S(bool p) : test(p) { }

  bool p() const { return test; }

  bool test;
};

int main()
{
  std::vector<int> all_even { 0, 2, 4, 6 };
  std::vector<int> one_even { 1, 3, 4, 7 };
  std::vector<int> none_even { 1, 3, 5, 7 };
  CHECK(ranges::all_of(all_even.begin(), all_even.end(), even));
  CHECK(!ranges::all_of(one_even.begin(), one_even.end(), even));
  CHECK(!ranges::all_of(none_even.begin(), none_even.end(), even));

  CHECK(ranges::all_of(all_even, even));
  CHECK(!ranges::all_of(one_even, even));
  CHECK(!ranges::all_of(none_even, even));

  CHECK(ranges::all_of({0, 2, 4, 6}, [](int n) { return n % 2 == 0; }));
  CHECK(!ranges::all_of({1, 3, 4, 7}, [](int n) { return n % 2 == 0; }));
  CHECK(!ranges::all_of({1, 3, 5, 7}, [](int n) { return n % 2 == 0; }));

  std::vector<S> all_true { true, true, true };
  std::vector<S> one_true { false, false, true };
  std::vector<S> none_true { false, false, false };
  CHECK(ranges::all_of(all_true.begin(), all_true.end(), &S::p));
  CHECK(!ranges::all_of(one_true.begin(), one_true.end(), &S::p));
  CHECK(!ranges::all_of(none_true.begin(), none_true.end(), &S::p));

  CHECK(ranges::all_of(all_true, &S::p));
  CHECK(!ranges::all_of(one_true, &S::p));
  CHECK(!ranges::all_of(none_true, &S::p));

  CHECK(ranges::all_of({S(true), S(true), S(true)}, &S::p));
  CHECK(!ranges::all_of({S(false), S(true), S(false)}, &S::p));
  CHECK(!ranges::all_of({S(false), S(false), S(false)}, &S::p));

  return ::test_result();
}
