// Range v3 library
//
//  Copyright Casey Carter 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//
#include <range/v3/detail/config.hpp>
#if RANGES_CXX_COROUTINES >= RANGES_CXX_COROUTINES_TS1
#include <iostream>
#include <vector>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_for.hpp>
#include <range/v3/algorithm/copy.hpp>
#include <range/v3/algorithm/count.hpp>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/utility/swap.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/take_exactly.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/experimental/utility/generator.hpp>
#include "../../simple_test.hpp"
#include "../../test_utils.hpp"

template<bool Condition>
using maybe_sized_generator = meta::if_c<Condition,
    meta::quote<ranges::experimental::sized_generator>,
    meta::quote<ranges::experimental::generator>>;

struct coro_fn
{
private:
    template<typename Rng>
    CONCEPT_alias(Constraint,
        ranges::InputRange<Rng>() &&
        (True<std::is_reference<ranges::range_reference_t<Rng>>>() ||
            ranges::CopyConstructible<ranges::range_reference_t<Rng>>()));

    template<typename V>
    using generator_for = meta::invoke<
        maybe_sized_generator<(bool) ranges::SizedRange<V>()>,
        ranges::range_reference_t<V>,
        ranges::range_value_type_t<V>>;

    CONCEPT_template(typename V)(
        requires Constraint<V>() && ranges::View<V>())
    static generator_for<V> impl(V v)
    {
        if /* constexpr */ (ranges::SizedRange<V>())
            co_await static_cast<ranges::experimental::generator_size>(ranges::distance(v));
        auto first = ranges::begin(v);
        auto const last = ranges::end(v);
        for (; first != last; ++first)
            co_yield *first;
    }
public:
    CONCEPT_template(typename Rng)(
        requires meta::and_<
            meta::not_<meta::is<ranges::uncvref_t<Rng>, ranges::experimental::generator>>,
            meta::not_<meta::is<ranges::uncvref_t<Rng>, ranges::experimental::sized_generator>>,
            Constraint<Rng>>::value)
    generator_for<ranges::view::all_t<Rng>> operator()(Rng &&rng) const
    {
        return impl(ranges::view::all(static_cast<Rng &&>(rng)));
    }
    template<typename R, typename V>
    ranges::experimental::generator<R, V>
    operator()(ranges::experimental::generator<R, V> g) const noexcept
    {
        return g;
    }
    template<typename R, typename V>
    ranges::experimental::sized_generator<R, V>
    operator()(ranges::experimental::sized_generator<R, V> g) const noexcept
    {
        return g;
    }
};

RANGES_INLINE_VARIABLE(coro_fn, coro)

auto f(int const n)
RANGES_DECLTYPE_AUTO_RETURN
(
    ::coro(ranges::view::iota(0, n))
)

ranges::experimental::sized_generator<int> g(int const n)
{
    co_await static_cast<ranges::experimental::generator_size>(n > 0 ? n : 0);
    for (int i = 0; i < n; ++i)
        co_yield i;
}

ranges::experimental::sized_generator<int &> h(int const n)
{
    co_await static_cast<ranges::experimental::generator_size>(n > 0 ? n : 0);
    for (int i = 0; i < n; ++i)
        co_yield i;
}

CONCEPT_template(class T)(

    requires ranges::WeaklyIncrementable<T>())
ranges::experimental::generator<T> iota_generator(T t)
{
    for (;; ++t)
        co_yield t;
}

CONCEPT_template(class T, class S)(
    requires ranges::WeaklyIncrementable<T>() &&
        ranges::WeaklyEqualityComparableWith<T, S>() &&
        !ranges::SizedIncrementableSentinel<S, T>())
ranges::experimental::generator<T> iota_generator(T t, S const s)
{
    for (; t != s; ++t)
        co_yield t;
}

CONCEPT_template(class T, class S)(
    requires ranges::SizedIncrementableSentinel<S, T>())
ranges::experimental::sized_generator<T> iota_generator(T t, S const s)
{
    co_await static_cast<ranges::experimental::generator_size>(s - t);
    for (; t != s; ++t)
        co_yield t;
}

CONCEPT_template(class V, class F)(
    requires ranges::InputView<V>() &&
        ranges::IndirectPredicate<F, ranges::iterator_t<V>>())
ranges::experimental::generator<ranges::range_reference_t<V>, ranges::range_value_type_t<V>>
filter(V view, F f)
{
    RANGES_FOR(auto &&i, view)
    {
        if (ranges::invoke(f, i))
            co_yield i;
    }
}

CONCEPT_template(class V, class F)(
    requires ranges::InputView<V>() &&
        ranges::IndirectInvocable<F, ranges::iterator_t<V>>())
meta::invoke<
    maybe_sized_generator<(bool) ranges::SizedRange<V>()>,
    ranges::indirect_result_of_t<F &(ranges::iterator_t<V>)>>
transform(V view, F f)
{
    if /* constexpr */ (ranges::SizedRange<V>())
        co_await static_cast<ranges::experimental::generator_size>(ranges::distance(view));
    RANGES_FOR(auto &&i, view)
        co_yield ranges::invoke(f, i);
}

struct MoveInt
{
    int i_;

    MoveInt(int i = 42) : i_{i}
    {}
    MoveInt(MoveInt &&that) noexcept
      : i_{ranges::exchange(that.i_, 0)}
    {}
    MoveInt &operator=(MoveInt &&that) noexcept
    {
        i_ = ranges::exchange(that.i_, 0);
        return *this;
    }

    friend bool operator==(MoveInt const &x, MoveInt const &y)
    {
        return x.i_ == y.i_;
    }
    friend bool operator!=(MoveInt const &x, MoveInt const &y)
    {
        return !(x == y);
    }

    friend std::ostream &operator<<(std::ostream &os, MoveInt const &mi)
    {
        return os << mi.i_;
    }
};

int main()
{
    using namespace ranges;

    auto even = [](int i){ return i % 2 == 0; };

    {
        auto rng = ::iota_generator(0, 10);
        ::models<SizedRangeConcept>(rng);
        CHECK(size(rng) == 10u);
        ::check_equal(rng, {0,1,2,3,4,5,6,7,8,9});
    }
    {
        auto rng = ::coro(::coro(::coro(::iota_generator(0, 10))));
        ::has_type<decltype(::iota_generator(0, 10)) &>(rng);
        ::models<SizedRangeConcept>(rng);
        CHECK(size(rng) == 10u);
        ::check_equal(rng, {0,1,2,3,4,5,6,7,8,9});
    }
    {
        auto rng = ::coro(view::ints | view::filter(even) | view::take_exactly(10));
        ::models<SizedRangeConcept>(rng);
        CHECK(size(rng) == 10u);
        ::check_equal(rng, {0,2,4,6,8,10,12,14,16,18});
    }
    {
        auto const control = {1, 2, 3};
        MoveInt a[] = {{1}, {2}, {3}};
        MoveInt b[3];
        CHECK(equal(a, control, std::equal_to<int>{}, &MoveInt::i_));
        CHECK(count(b, 42, &MoveInt::i_) == 3);
        auto rng = ::coro(view::move(a));
        ::models<SizedRangeConcept>(rng);
        CHECK(size(rng) == 3u);
        copy(rng, b);
        CHECK(equal(b, control, std::equal_to<int>{}, &MoveInt::i_));
        CHECK(count(a, 0, &MoveInt::i_) == 3);
    }
    {
        int some_ints[] = {0,1,2};
        auto rng = ::coro(some_ints);
        ::models<SizedRangeConcept>(rng);
        CHECK(size(rng) == 3u);
        auto i = begin(rng);
        auto e = end(rng);
        CHECK(i != e);
        CHECK(&*i == &some_ints[0]);
        ++i;
        CHECK(i != e);
        CHECK(&*i == &some_ints[1]);
        ++i;
        CHECK(i != e);
        CHECK(&*i == &some_ints[2]);
        ++i;
        CHECK(i == e);
    }
    {
        std::vector<bool> vec(3, false);
        auto rng = ::coro(vec);
        ::models<SizedRangeConcept>(rng);
        CHECK(size(rng) == 3u);
        ::check_equal(rng, {false,false,false});
    }

    ::check_equal(f(42), g(42));
    ::check_equal(f(42), h(42));

    {
        auto rng = h(20) | view::transform([](int &x) { return ++x; });
        ::check_equal(rng, {1,3,5,7,9,11,13,15,17,19});
    }

    {
        auto rng = f(20) | view::filter(even);
        ::check_equal(rng, {0,2,4,6,8,10,12,14,16,18});
    }

    {
        auto square = [](int i) { return i * i; };

        int const some_ints[] = {0,1,2,3,4,5,6,7};
        auto rng = ::transform(::filter(debug_input_view<int const>{some_ints}, even), square);
        ::check_equal(rng, {0,4,16,36});
    }

    std::cout << f(8) << '\n';

    return ::test_result();
}
#else // RANGES_CXX_COROUTINES >= RANGES_CXX_COROUTINES_TS1
int main() {}
#endif // RANGES_CXX_COROUTINES >= RANGES_CXX_COROUTINES_TS1
