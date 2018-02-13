/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-present
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_ACTION_CONCEPTS_HPP
#define RANGES_V3_ACTION_CONCEPTS_HPP

#include <utility>
#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/utility/functional.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \cond
        namespace detail
        {
            template<typename T>
            struct movable_input_iterator
            {
                using iterator_category = std::input_iterator_tag;
                using value_type = T;
                using difference_type = std::ptrdiff_t;
                using pointer = T *;
                using reference = T &&;

                movable_input_iterator() = default;
                movable_input_iterator &operator++();
                movable_input_iterator operator++(int);
                bool operator==(movable_input_iterator const &) const;
                bool operator!=(movable_input_iterator const &) const;
                T && operator*() const;
            };
        }
        /// \endcond

        /// \addtogroup group-concepts
        /// @{

        // std::array is a SemiContainer, native arrays are not.
        CONCEPT_def
        (
            template(typename T)
            concept SemiContainer,
                ForwardRange<T>() && DefaultConstructible<uncvref_t<T>>() &&
                Movable<uncvref_t<T>>() &&
                !View<T>()
        );

        // std::vector is a Container, std::array is not
        CONCEPT_def
        (
            template(typename T)
            concept Container,
                SemiContainer<T>() &&
                Constructible<
                    uncvref_t<T>,
                    detail::movable_input_iterator<range_value_type_t<T>>,
                    detail::movable_input_iterator<range_value_type_t<T>>>()
        );

        CONCEPT_def
        (
            template(typename C)
            concept Reservable,
                requires (C &c, C const &cc, range_size_type_t<C> s)
                {
                    cc.capacity() ->* Same<_&&, range_size_type_t<C>>(),
                    cc.max_size() ->* Same<_&&, range_size_type_t<C>>(),
                    ((void)c.reserve(s), 42)
                } &&
                Container<C>() && SizedRange<C>()
        );

        CONCEPT_def
        (
            template(typename C, typename I)
            concept ReserveAndAssignable,
                requires (C &c, I i)
                {
                    ((void) c.assign(i, i), 42)
                } &&
                Reservable<C>() && InputIterator<I>()
        );

        CONCEPT_def
        (
            template(typename C)
            concept RandomAccessReservable,
                Reservable<C>() && RandomAccessRange<C>()
        );

        /// \cond
        namespace detail
        {
            CONCEPT_template(typename T)(
                requires Container<T>())
            (std::true_type) is_lvalue_container_like(T &) noexcept
            {
                return {};
            }

            CONCEPT_template(typename T)(
                requires Container<T>())
            (meta::not_<std::is_rvalue_reference<T>>)
            is_lvalue_container_like(reference_wrapper<T>) noexcept
            {
                return {};
            }

            CONCEPT_template(typename T)(
                requires Container<T>())
            (std::true_type) is_lvalue_container_like(std::reference_wrapper<T>) noexcept
            {
                return {};
            }
        }
        /// \endcond

        CONCEPT_def
        (
            template(typename T)
            concept LvalueContainerLike,
                ForwardRange<T>() &&
                True(detail::is_lvalue_container_like(std::declval<T>()))
        );
        /// @}
    }
}

#endif
