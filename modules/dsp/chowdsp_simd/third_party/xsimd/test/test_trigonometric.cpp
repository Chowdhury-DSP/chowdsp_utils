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
struct trigonometric_test
{
    using batch_type = B;
    using value_type = typename B::value_type;
    static constexpr size_t size = B::size;
    using vector_type = std::vector<value_type>;

    size_t nb_input;
    vector_type input;
    vector_type ainput;
    vector_type atan_input;
    vector_type expected;
    vector_type res;

    trigonometric_test()
    {
        nb_input = size * 10000;
        input.resize(nb_input);
        ainput.resize(nb_input);
        atan_input.resize(nb_input);
        for (size_t i = 0; i < nb_input; ++i)
        {
            input[i] = value_type(0.) + i * value_type(80.) / nb_input;
            ainput[i] = value_type(-1.) + value_type(2.) * i / nb_input;
            atan_input[i] = value_type(-10.) + i * value_type(20.) / nb_input;
        }
        expected.resize(nb_input);
        res.resize(nb_input);
    }

    void test_trigonometric_functions()
    {
        // sin
        {
            std::transform(input.cbegin(), input.cend(), expected.begin(),
                           [](const value_type& v)
                           { return std::sin(v); });
            batch_type in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, input, i);
                out = sin(in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("sin");
            CHECK_EQ(diff, 0);
        }
        // cos
        {
            std::transform(input.cbegin(), input.cend(), expected.begin(),
                           [](const value_type& v)
                           { return std::cos(v); });
            batch_type in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, input, i);
                out = cos(in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("cos");
            CHECK_EQ(diff, 0);
        }
        // sincos
        {
            vector_type expected2(nb_input), res2(nb_input);
            std::transform(input.cbegin(), input.cend(), expected.begin(),
                           [](const value_type& v)
                           { return std::sin(v); });
            std::transform(input.cbegin(), input.cend(), expected2.begin(),
                           [](const value_type& v)
                           { return std::cos(v); });
            batch_type in, out1, out2;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, input, i);
                std::tie(out1, out2) = sincos(in);
                detail::store_batch(out1, res, i);
                detail::store_batch(out2, res2, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("sincos(sin)");
            CHECK_EQ(diff, 0);
            diff = detail::get_nb_diff(res2, expected2);
            INFO("sincos(cos)");
            CHECK_EQ(diff, 0);
        }
        // tan
        {
            std::transform(input.cbegin(), input.cend(), expected.begin(),
                           [](const value_type& v)
                           { return std::tan(v); });
            batch_type in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, input, i);
                out = tan(in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("tan");
            CHECK_EQ(diff, 0);
        }
    }

    void test_reciprocal_functions()
    {

        // asin
        {
            std::transform(ainput.cbegin(), ainput.cend(), expected.begin(),
                           [](const value_type& v)
                           { return std::asin(v); });
            batch_type in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, ainput, i);
                out = asin(in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("asin");
            CHECK_EQ(diff, 0);
        }
        // acos
        {
            std::transform(ainput.cbegin(), ainput.cend(), expected.begin(),
                           [](const value_type& v)
                           { return std::acos(v); });
            batch_type in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, ainput, i);
                out = acos(in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("acos");
            CHECK_EQ(diff, 0);
        }
        // atan
        {
            std::transform(atan_input.cbegin(), atan_input.cend(), expected.begin(),
                           [](const value_type& v)
                           { return std::atan(v); });
            batch_type in, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, atan_input, i);
                out = atan(in);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("atan");
            CHECK_EQ(diff, 0);
        }
        // atan2
        {
            std::transform(atan_input.cbegin(), atan_input.cend(), input.cbegin(), expected.begin(),
                           [](const value_type& v, const value_type& r)
                           { return std::atan2(v, r); });
            batch_type in, rhs, out;
            for (size_t i = 0; i < nb_input; i += size)
            {
                detail::load_batch(in, atan_input, i);
                detail::load_batch(rhs, input, i);
                out = atan2(in, rhs);
                detail::store_batch(out, res, i);
            }
            size_t diff = detail::get_nb_diff(res, expected);
            INFO("atan2");
            CHECK_EQ(diff, 0);
        }
    }
};

TEST_CASE_TEMPLATE("[trigonometric]", B, BATCH_FLOAT_TYPES)
{
    trigonometric_test<B> Test;
    SUBCASE("trigonometric")
    {
        Test.test_trigonometric_functions();
    }

    SUBCASE("reciprocal")
    {
        Test.test_reciprocal_functions();
    }
}
#endif
