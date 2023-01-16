#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

TEST_CASE ("Jacobi Elliptic Test", "[dsp][math]")
{
    auto checkValues = [] (double x, double k, double sn, double cn, double dn)
    {
        auto [sn_actual, cn_actual, dn_actual] = chowdsp::jacobi::jacobi_elliptic (x, k);
        REQUIRE_MESSAGE (sn_actual == Catch::Approx (sn).margin (1.0e-12), "sn value is incorrect!");
        REQUIRE_MESSAGE (cn_actual == Catch::Approx (cn).margin (1.0e-12), "cn value is incorrect!");
        REQUIRE_MESSAGE (dn_actual == Catch::Approx (dn).margin (1.0e-12), "dn value is incorrect!");
    };

    SECTION ("Zero x")
    {
        checkValues (0.0, 1.0, 0.0, 1.0, 1.0);
    }

    SECTION ("Zero k")
    {
        checkValues (1.0, 0.0, 0.8414709848078965, 0.5403023058681398, 1.0);
    }

    SECTION ("One k")
    {
        checkValues (0.5, 1.0, 0.46211715726000974, 0.886818883970074, 0.886818883970074);
    }

    SECTION ("Small k")
    {
        checkValues (0.5, 1.0e-15, 0.479425538604203, 0.8775825618903728, 0.9999999999999999);
    }

    SECTION ("Regular Range")
    {
        checkValues (0.5, 0.5, 0.47075047365565736, 0.8822663948904402, 0.9429724257773857);
        checkValues (0.1, 0.75, 0.09970933501024688, 0.9950166071537773, 0.9962647923036629);
        checkValues (0.9, 0.25, 0.7670852375827275, 0.6415451958222823, 0.92352317760339);
    }

    SECTION ("k > 1")
    {
        checkValues (0.5, 2.0, 0.40150091244782193, 0.5959765676721407, 0.8231610016315962);
        checkValues (0.1, 4.0 / 3.0, 0.09948463864830816, 0.9911634525349069, 0.9933799586415499);
        checkValues (0.9, 4.0, -0.056509300172441254, -0.9741186703396743, 0.9935928723456523);
    }
}
