#include <JuceHeader.h>

namespace
{
// pre-computer values of the Wright-Omega function, used for reference
std::map<double, double> WO_vals {
    { -10.0, 4.539786874921544e-05 },
    { -9.5, 7.484622772024869e-05 },
    { -9.0, 0.00012339457692560975 },
    { -8.5, 0.00020342698226408345 },
    { -8.0, 0.000335350149321062 },
    { -7.5, 0.0005527787213627528 },
    { -7.0, 0.0009110515723789146 },
    { -6.5, 0.0015011839473879653 },
    { -6.0, 0.002472630709097278 },
    { -5.5, 0.004070171383753891 },
    { -5.0, 0.0066930004977309955 },
    { -4.5, 0.010987603420879434 },
    { -4.0, 0.017989102828531025 },
    { -3.5, 0.029324711813756815 },
    { -3.0, 0.04747849102486547 },
    { -2.5, 0.07607221340790257 },
    { -2.0, 0.1200282389876412 },
    { -1.5, 0.1853749184489398 },
    { -1.0, 0.27846454276107374 },
    { -0.5, 0.4046738485459385 },
    { 0.0, 0.5671432904097838 },
    { 0.5, 0.7662486081617502 },
    { 1.0, 1.0 },
    { 1.5, 1.2649597201255005 },
    { 2.0, 1.5571455989976113 },
    { 2.5, 1.8726470404165942 },
    { 3.0, 2.207940031569323 },
    { 3.5, 2.559994780412122 },
    { 4.0, 2.926271062443501 },
    { 4.5, 3.3046649181693253 },
    { 5.0, 3.6934413589606496 },
    { 5.5, 4.091169202271799 },
    { 6.0, 4.4966641730061605 },
    { 6.5, 4.908941634486258 },
    { 7.0, 5.327178301371093 },
    { 7.5, 5.750681611147114 },
    { 8.0, 6.178865346308128 },
    { 8.5, 6.611230244734983 },
    { 9.0, 7.047348546597604 },
    { 9.5, 7.486851633496902 },
    { 10.0, 7.9294200950196965 },
};
} // namespace

/** Unit tests for chowdsp::Omega. Tests accuracy for
 * omega function approximations and sub-functions.
 */
template <typename T>
class OmegaTest : public UnitTest
{
public:
    template <typename TT = T, std::enable_if_t<std::is_same_v<TT, float>, int> = 0>
    OmegaTest() : UnitTest ("Omega Test (float)")
    {
    }

    template <typename TT = T, std::enable_if_t<std::is_same_v<TT, double>, int> = 0>
    explicit OmegaTest() : UnitTest ("Omega Test (double)")
    {
    }

    using FuncType = std::function<T (T)>;
    struct FunctionTest
    {
        Range<T> range;
        FuncType testFunc;
        FuncType refFunc;
        const String& name;
        T tol;
    };

    void checkFunctionAccuracy (const FunctionTest& funcTest, size_t N = 20)
    {
        auto range = funcTest.range;
        auto step = range.getLength() / (T) N;
        for (T x = range.getStart(); x < range.getEnd(); x += step)
        {
            auto expected = funcTest.refFunc (x);
            auto actual = funcTest.testFunc (x);

            String errorMsg = funcTest.name + " incorrect at value " + String (x);
            expectWithinAbsoluteError (actual, expected, funcTest.tol, errorMsg);
        }
    }

    void checkWrightOmega (FuncType omega, const String& funcName, T tol)
    {
        for (auto vals : WO_vals)
        {
            auto expected = (T) vals.second;
            auto actual = omega ((T) vals.first);

            String errorMsg = funcName + " incorrect at value " + String (vals.first);
            expectWithinAbsoluteError (actual, expected, tol, errorMsg);
        }
    }

    void runTest() override
    {
        beginTest ("Log2 Test");
        FunctionTest log2Test {
            { (T) 1, (T) 2 },
            [] (T x) { return chowdsp::Omega::log2_approx<T> (x); },
            [] (T x) { return std::log2 (x); },
            "Log2",
            (T) 0.008
        };
        checkFunctionAccuracy (log2Test);

        beginTest ("Log Test");
        FunctionTest logTest {
            { (T) 8, (T) 12 },
            [] (T x) { return chowdsp::Omega::log_approx<T> (x); },
            [] (T x) { return std::log (x); },
            "Log",
            (T) 0.005
        };
        checkFunctionAccuracy (logTest);

        beginTest ("Pow2 Test");
        FunctionTest pow2Test {
            { (T) 0, (T) 1 },
            [] (T x) { return chowdsp::Omega::pow2_approx<T> (x); },
            [] (T x) { return std::pow ((T) 2, x); },
            "Pow2",
            (T) 0.001
        };
        checkFunctionAccuracy (pow2Test);

        beginTest ("Exp Test");
        FunctionTest expTest {
            { (T) -4, (T) 2 },
            [] (T x) { return chowdsp::Omega::exp_approx<T> (x); },
            [] (T x) { return std::exp (x); },
            "Exp",
            (T) 0.005
        };
        checkFunctionAccuracy (expTest);

        beginTest ("Omega1 Test");
        checkWrightOmega ([] (T x) { return chowdsp::Omega::omega1 (x); }, "Omega1", (T) 2.1);

        beginTest ("Omega2 Test");
        checkWrightOmega ([] (T x) { return chowdsp::Omega::omega2 (x); }, "Omega2", (T) 2.1);

        beginTest ("Omega3 Test");
        checkWrightOmega ([] (T x) { return chowdsp::Omega::omega3 (x); }, "Omega3", (T) 0.3);

        beginTest ("Omega4 Test");
        checkWrightOmega ([] (T x) { return chowdsp::Omega::omega4 (x); }, "Omega4", (T) 0.05);
    }
};

static OmegaTest<float> omegaTestFloat;
static OmegaTest<double> omegaTestDouble;
