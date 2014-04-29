//  Copyright Eric Niebler 2014.
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <range/v3/core.hpp>
#include <range/v3/numeric.hpp>
#include "../simple_test.hpp"
#include "../test_iterators.hpp"

struct S
{
    int i;
    S add(int j) const
    {
        return S{i + j};
    }
};

template <class Iter, class Sent = Iter>
void test()
{
    int ia[] = {1, 2, 3, 4, 5, 6};
    constexpr unsigned sc = ranges::size(ia);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia), 0) == 0);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia), 10) == 10);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia+1), 0) == 1);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia+1), 10) == 11);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia+2), 0) == 3);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia+2), 10) == 13);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia+sc), 0) == 21);
    CHECK(ranges::accumulate(Iter(ia), Sent(ia+sc), 10) == 31);

    using ranges::range;
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia)), 0) == 0);
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia)), 10) == 10);
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia+1)), 0) == 1);
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia+1)), 10) == 11);
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia+2)), 0) == 3);
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia+2)), 10) == 13);
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia+sc)), 0) == 21);
    CHECK(ranges::accumulate(range(Iter(ia), Sent(ia+sc)), 10) == 31);

    CHECK(ranges::accumulate({1, 2, 3, 4, 5, 6}, 10) == 31);
    CHECK(ranges::accumulate({1, 2, 3, 4, 5, 6}, S{10}, &S::add).i == 31);
}

int main()
{
    test<input_iterator<const int*> >();
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();

    test<input_iterator<const int*>, sentinel<const int*> >();
    test<forward_iterator<const int*>, sentinel<const int*> >();
    test<bidirectional_iterator<const int*>, sentinel<const int*> >();
    test<random_access_iterator<const int*>, sentinel<const int*> >();

    return ::test_result();
}