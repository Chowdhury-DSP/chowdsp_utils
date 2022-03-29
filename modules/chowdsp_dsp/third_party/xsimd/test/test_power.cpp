/***************************************************************************
 * Copyright (c) Johan Mabille, Sylvain Corlay, Wolf Vollprecht and         *
 * Martin Renou                                                             *
 * Copyright (c) QuantStack                                                 *
 * Copyright (c) Serge Guelton                                              *
 *                                                                          *
 * Distributed under the terms of the BSD 3-Clause License.                 *
 *                                                                          *
 * The full license is in the file LICENSE, distributed with this software. *
 ****************************************************************************/

#include "xsimd/xsimd.hpp"
#ifndef XSIMD_NO_SUPPORTED_ARCHITECTURE

#include "test_utils.hpp"

template <class B>
class power_test : public testing::Test
{
protected:
    using batch_type = B;
    using value_type = typename B::value_type;
    static constexpr size_t size = B::size;
    using vector_type = std::vector<value_type>;

    size_t nb_input;
    vector_type lhs_input;
    vector_type rhs_input;
    vector_type expected;
    vector_type res;

    power_test()
    {
        nb_input = size * 10000;
        lhs_input.resize(nb_input);
        rhs_input.resize(nb_input);
        for (size_t i = 0; i < nb_input; ++i)
        {
            lhs_input[i] = value_type(i) / 4 + value_type(1.2) * std::sqrt(value_type(i + 0.25));
            rhs_input[i] = value_type(10.2) / (i + 2) + value_type(0.25);
        }
        expected.resize(nb_input);
        res.resize(nb_input);
    }

    void test_power_functions()
    {
        // pow
        {
            std::transform(lhs_input.cbegin(), lhs_input.cend(), rhs_input.cbegin(), expected.begin(),
                           [](const value_type& l, const value_type& r)
                           { return std::pow(l, r); });
            batch_type lhs_in, rhs_in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(lhs_in, lhs_input, i);
                detail::load_batch(rhs_in, rhs_input, i);
                out = pow(lhs_in, rhs_in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            EXPECT_EQ(diff, 0) << print_function_name("pow");
        }
        // ipow
        {
            long k = 0;
            std::transform(lhs_input.cbegin(), lhs_input.cend(), expected.begin(),
                           [&k, this](const value_type& l)
                           { auto arg = k / size - nb_input / size / 2; ++k; return std::pow(l, arg); });
            batch_type lhs_in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(lhs_in, lhs_input, i);
                out = pow(lhs_in, i / size - nb_input / size / 2);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            EXPECT_EQ(diff, 0) << print_function_name("ipow");
        }
        // hypot
        {
            std::transform(lhs_input.cbegin(), lhs_input.cend(), rhs_input.cbegin(), expected.begin(),
                           [](const value_type& l, const value_type& r)
                           { return std::hypot(l, r); });
            batch_type lhs_in, rhs_in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(lhs_in, lhs_input, i);
                detail::load_batch(rhs_in, rhs_input, i);
                out = hypot(lhs_in, rhs_in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            EXPECT_EQ(diff, 0) << print_function_name("hypot");
        }
        // cbrt
        {
            std::transform(lhs_input.cbegin(), lhs_input.cend(), expected.begin(),
                           [](const value_type& l)
                           { return std::cbrt(l); });
            batch_type lhs_in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(lhs_in, lhs_input, i);
                out = cbrt(lhs_in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            EXPECT_EQ(diff, 0) << print_function_name("cbrt");
        }
    }
};

TYPED_TEST_SUITE(power_test, batch_float_types, simd_test_names);

TYPED_TEST(power_test, power)
{
    this->test_power_functions();
}
#endif
