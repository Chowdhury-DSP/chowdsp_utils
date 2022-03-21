#include <TimedUnitTest.h>

class SIMDComplexTest : public TimedUnitTest
{
    template <typename T>
    using SIMDComplex = chowdsp::SIMDUtils::SIMDComplex<T>;

public:
    SIMDComplexTest() : TimedUnitTest ("SIMD Complex Test", "SIMD") {}

    void zeroCheck()
    {
        auto a = SIMDComplex<float>();
        expect (a.atIndex (0) == std::complex<float> { 0, 0 }, "Zero initiatalisation incorrect!");

        auto b = a + a;
        for (size_t i = 0; i < SIMDComplex<float>::size; ++i)
            expect (a.atIndex (i) == b.atIndex (i), "Zero elements incorrect!");
    }

    template <typename T, typename VectorOpType, typename ScalarOpType>
    void testMathOp (Random& r, int nIter, VectorOpType&& vectorOp, ScalarOpType&& scalarOp, const String& mathOpName, T maxErr)
    {
        auto checkResult = [this] (auto result_scalar, auto result_vec, const String& mathOpName, const String& opType, T maxErr)
        {
            if (std::isinf (result_scalar.real()) || std::isinf (result_scalar.imag()) || std::isnan (result_scalar.real()) || std::isnan (result_scalar.imag()))
                return;

            expectWithinAbsoluteError (result_vec.atIndex (0).real(), result_scalar.real(), maxErr, mathOpName + ": " + opType + ", real incorrect!");
            expectWithinAbsoluteError (result_vec.atIndex (0).imag(), result_scalar.imag(), maxErr, mathOpName + ": " + opType + ", imag incorrect!");
        };

        auto testComplexComplexOp = [&] ()
        {
            const auto a_scalar = std::complex<T> ((T) r.nextInt ({ -100, 100 }), (T) r.nextInt ({ -100, 100 }));
            const auto b_scalar = std::complex<T> ((T) r.nextInt ({ -100, 100 }), (T) r.nextInt ({ -100, 100 }));
            SIMDComplex<T> a_vec { a_scalar.real(), a_scalar.imag() };
            SIMDComplex<T> b_vec { b_scalar.real(), b_scalar.imag() };

            checkResult (scalarOp (a_scalar, b_scalar), vectorOp (a_vec, b_vec), mathOpName, "complex x complex", maxErr);
        };

        auto testComplexVectorOp = [&] ()
        {
            const auto a_scalar = std::complex<T> ((T) r.nextInt ({ -100, 100 }), (T) r.nextInt ({ -100, 100 }));
            const auto b_scalar = (T) r.nextInt ({ -100, 100 });
            SIMDComplex<T> a_vec { a_scalar.real(), a_scalar.imag() };
            dsp::SIMDRegister<T> b_vec { b_scalar };

            checkResult (scalarOp (a_scalar, b_scalar), vectorOp (a_vec, b_vec), mathOpName, "complex x vector", maxErr);
            checkResult (scalarOp (b_scalar, a_scalar), vectorOp (b_vec, a_vec), mathOpName, "vector x complex", maxErr);
        };

        auto testComplexScalarOp = [&] ()
        {
            const auto a_scalar = std::complex<T> ((T) r.nextInt ({ -100, 100 }), (T) r.nextInt ({ -100, 100 }));
            const auto b_scalar = (T) r.nextInt ({ -100, 100 });
            SIMDComplex<T> a_vec { a_scalar.real(), a_scalar.imag() };

            checkResult (scalarOp (a_scalar, b_scalar), vectorOp (a_vec, b_scalar), mathOpName, "complex x scalar", maxErr);
            checkResult (scalarOp (b_scalar, a_scalar), vectorOp (b_scalar, a_vec), mathOpName, "scalar x complex", maxErr);
        };


        for (int i = 0; i < nIter; ++i)
        {
            testComplexComplexOp();
            testComplexVectorOp();
            testComplexScalarOp();
        };
    }

    template <typename T, typename OpType>
    void testMathOp (Random& r, int nIter, OpType&& mathOp, const String& mathOpName, T maxErr)
    {
        testMathOp (r, nIter, std::forward<OpType> (mathOp), std::forward<OpType> (mathOp), mathOpName, maxErr);
    }

    template <typename T>
    void mathTest(Random& r, int nIter, T maxError)
    {
        using namespace chowdsp::SIMDUtils;
        testMathOp (r, nIter, [] (auto a, auto b) { return a + b; }, "Addition", maxError);
        testMathOp (r, nIter, [] (auto a, auto b) { return a - b; }, "Subtraction", maxError);
        testMathOp (r, nIter, [] (auto a, auto b) { return a * b; }, "Multiplication", maxError);
        testMathOp (r, nIter, [] (auto a, auto b) { return a / b; }, "Division", maxError * (T) 2);
    }

    template <typename T>
    void fastExpTest()
    {
        T angles alignas (16)[SIMDComplex<T>::size];
        angles[0] = 0;
        angles[1] = MathConstants<T>::pi / (T) 2;
        if constexpr (std::is_same<T, float>::value)
        {
            angles[2] = (T) 3 * MathConstants<T>::pi / (T) 4;
            angles[3] = MathConstants<T>::pi;
        }
        auto asse = dsp::SIMDRegister<T>::fromRawArray (angles);
        auto c = SIMDComplex<T>::fastExp (asse);

        auto c0 = c.atIndex (0);
        expectWithinAbsoluteError ((T) 1, c0.real(), (T) 1e-5);
        expectWithinAbsoluteError ((T) 0, c0.imag(), (T) 1e-5);

        auto c1 = c.atIndex (1);
        expectWithinAbsoluteError ((T) 0, c1.real(), (T) 1e-5);
        expectWithinAbsoluteError ((T) 1, c1.imag(), (T) 1e-5);

        if constexpr (std::is_same<T, float>::value)
        {
            auto c2 = c.atIndex (2);
            expectWithinAbsoluteError (-std::sqrt ((T) 2.0) / (T) 2, c2.real(), (T) 1e-5);
            expectWithinAbsoluteError (std::sqrt ((T) 2.0) / (T) 2, c2.imag(), (T) 1e-5);

            // At this extrema we are a touch less acrrucate
            auto c3 = c.atIndex (3);
            expectWithinAbsoluteError ((T) -1, c3.real(), (T) 1e-4);
            expectWithinAbsoluteError ((T) 0, c3.imag(), (T) 1e-4);
        }
    }

    void mapTest()
    {
        auto q = SIMDComplex<float> ({ 0.f, 1.f, 2.f, 3.f }, { 1.f, 0.f, -1.f, -2.f });
        auto powV = q.map ([] (const std::complex<float>& f) { return std::pow (f, 2.1f); });
        for (size_t i = 0; i < SIMDComplex<float>::size; ++i)
            expect (powV.atIndex (i) == std::pow (q.atIndex (i), 2.1f));
    }

    void runTestTimed() override
    {
        auto rand = getRandom();

        beginTest ("Zero Check");
        zeroCheck();

        beginTest ("Math Test (float)");
        mathTest<float> (rand, 100, 1.0e-6f);

        beginTest ("Math Test (double)");
        mathTest<double> (rand, 100, 1.0e-12);

        beginTest ("Exponential Test (float)");
        fastExpTest<float>();

        beginTest ("Exponential Test (double)");
        fastExpTest<double>();

        beginTest ("Map Test");
        mapTest();
    }
};

static SIMDComplexTest simdComplexTest;
