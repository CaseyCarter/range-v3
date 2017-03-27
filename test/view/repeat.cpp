// Range v3 library
//
//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3

#include <vector>
#include <range/v3/core.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/repeat.hpp>
#include <range/v3/utility/copy.hpp>
#include "../simple_test.hpp"
#include "../test_utils.hpp"

RANGES_DISABLE_WARNINGS

int main()
{
    using namespace ranges;
    auto rng = view::repeat(9) | view::take(10);
    ::models<concepts::SizedView>(aux::copy(rng));
    ::models<concepts::View>(aux::copy(rng));
    ::models<concepts::RandomAccessIterator>(rng.begin());
    ::check_equal(rng, {9, 9, 9, 9, 9, 9, 9, 9, 9, 9});

    return test_result();
}
