#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

static void check_poly (const chowdsp::Polynomial<float, 32, chowdsp::poly_order_ascending>& cheby_poly,
                        std::vector<float>&& actual)
{
    size_t i = 0;
    for (; i < actual.size(); ++i)
        REQUIRE (cheby_poly.coeffs[i] == Catch::Approx { actual[i] }.margin (1.0e-6));
    for (; i < cheby_poly.coeffs.size(); ++i)
        REQUIRE (cheby_poly.coeffs[i] == Catch::Approx { 0.0f }.margin (1.0e-6));
}

TEST_CASE ("Chebyshev Polynomials Test", "[dsp][math]")
{
    check_poly (chowdsp::chebyshev_polynomial<float, 2>(), { -1.0f, 0.0f, 2.0f });
    check_poly (chowdsp::chebyshev_polynomial<float, 3>(), { 0.0f, -3.0f, 0.0f, 4.0f });
    check_poly (chowdsp::chebyshev_polynomial<float, 4>(), { 1.0f, 0.0f, -8.0f, 0.0f, 8.0f });
    check_poly (chowdsp::chebyshev_polynomial<float, 5>(), { 0.0f, 5.0f, 0.0f, -20.0f, 0.0f, 16.0f });
    check_poly (chowdsp::chebyshev_polynomial<float, 6>(), { -1.0f, 0.0f, 18.0f, 0.0f, -48.0f, 0.0f, 32.0f });
    check_poly (chowdsp::chebyshev_polynomial<float, 7>(), { 0.0f, -7.0f, 0.0f, 56.0f, 0.0f, -112.0f, 0.0f, 64.0f });
    check_poly (chowdsp::chebyshev_polynomial<float, 8>(), { 1.0f, 0.0f, -32.0f, 0.0f, 160.0f, 0.0f, -256.0f, 0.0f, 128.0f });
}
