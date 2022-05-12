//#include <TimedUnitTest.h>
//#include <chowdsp_simd/chowdsp_simd.h>
//
//class SIMDComplexTest : public TimedUnitTest
//{
//    template <typename T>
//    using SIMDComplex = xsimd::batch<std::complex<T>>;
//    ;
//
//public:
//    SIMDComplexTest() : TimedUnitTest ("SIMD Complex Test", "SIMD") {}
//
//    template <typename T>
//    void checkResult (std::complex<T> result_scalar, SIMDComplex<T> result_vec, const juce::String& mathOpName, const juce::String& opType, T maxErr)
//    {
//        auto isNanOrInf = [] (auto x)
//        {
//            return std::isinf (x.real()) || std::isinf (x.imag()) || std::isnan (x.real()) || std::isnan (x.imag());
//        };
//
//        if (isNanOrInf (result_scalar))
//            return;
//
//        if (xsimd::any (xsimd::isinf (result_vec.real()) || xsimd::isinf (result_vec.imag()) || xsimd::isnan (result_vec)))
//        {
//            jassertfalse;
//            return;
//        }
//
//        T resultReal alignas (chowdsp::SIMDUtils::CHOWDSP_DEFAULT_SIMD_ALIGNMENT)[SIMDComplex<T>::size]{};
//        result_vec.real().store_aligned (resultReal);
//
//        T resultImag alignas (chowdsp::SIMDUtils::CHOWDSP_DEFAULT_SIMD_ALIGNMENT)[SIMDComplex<T>::size]{};
//        result_vec.imag().store_aligned (resultImag);
//
//        expectWithinAbsoluteError (resultReal[0], result_scalar.real(), maxErr, mathOpName + ": " + opType + ", real incorrect!");
//        expectWithinAbsoluteError (resultImag[0], result_scalar.imag(), maxErr, mathOpName + ": " + opType + ", imag incorrect!");
//    }
//
//    template <typename T>
//    void checkResult (T result_scalar, xsimd::batch<T> result_vec, const juce::String& mathOpName, const juce::String& opType, T maxErr)
//    {
//        if (std::isinf (result_scalar) || std::isnan (result_scalar))
//            return;
//
//        auto at0 = result_vec.get (0);
//        if (std::isinf (at0) || std::isnan (at0))
//        {
//            jassertfalse;
//            return;
//        }
//
//        expectWithinAbsoluteError (at0, result_scalar, maxErr, mathOpName + ": " + opType + ", real incorrect!");
//    }
//
//    template <typename T, typename VectorOpType, typename ScalarOpType, bool doVector = true>
//    void testMathOp (juce::Random& r, int nIter, VectorOpType&& vectorOp, ScalarOpType&& scalarOp, const juce::String& mathOpName, T maxErr, T range = (T) 100)
//    {
//        auto randVal = [&r, range]() { return (T) (2.0f * r.nextFloat() * (float) range - (float) range); };
//
//        for (int i = 0; i < nIter; ++i)
//        {
//            const auto a_scalar = std::complex<T> (randVal(), randVal());
//            const auto b_scalar = std::complex<T> (randVal(), randVal());
//            SIMDComplex<T> a_vec { a_scalar.real(), a_scalar.imag() };
//            SIMDComplex<T> b_vec { b_scalar.real(), b_scalar.imag() };
//
//            checkResult (scalarOp (a_scalar, b_scalar), vectorOp (a_vec, b_vec), mathOpName, "complex x complex", maxErr);
//        }
//
//        if constexpr (doVector)
//        {
//            for (int i = 0; i < nIter; ++i)
//            {
//                const auto a_scalar = std::complex<T> (randVal(), randVal());
//                const auto b_scalar = randVal();
//                SIMDComplex<T> a_vec { a_scalar.real(), a_scalar.imag() };
//                juce::dsp::SIMDRegister<T> b_vec { b_scalar };
//
//                checkResult (scalarOp (a_scalar, b_scalar), vectorOp (a_vec, b_vec), mathOpName, "complex x vector", maxErr);
//                checkResult (scalarOp (b_scalar, a_scalar), vectorOp (b_vec, a_vec), mathOpName, "vector x complex", maxErr);
//            }
//        }
//
//        for (int i = 0; i < nIter; ++i)
//        {
//            const auto a_scalar = std::complex<T> (randVal(), randVal());
//            const auto b_scalar = randVal();
//            SIMDComplex<T> a_vec { a_scalar.real(), a_scalar.imag() };
//
//            checkResult (scalarOp (a_scalar, b_scalar), vectorOp (a_vec, b_scalar), mathOpName, "complex x scalar", maxErr);
//            checkResult (scalarOp (b_scalar, a_scalar), vectorOp (b_scalar, a_vec), mathOpName, "scalar x complex", maxErr);
//        }
//    }
//
//    template <typename T, typename OpType, bool doVector = true>
//    void testMathOp (juce::Random& r, int nIter, OpType&& mathOp, const juce::String& mathOpName, T maxErr, T range = (T) 100)
//    {
//        testMathOp<T, OpType, OpType, doVector> (r, nIter, std::forward<OpType> (mathOp), std::forward<OpType> (mathOp), mathOpName, maxErr, range);
//    }
//
//    template <typename T>
//    void specialMathTest (juce::Random& r, int nIter, T maxError)
//    {
//        using namespace chowdsp::SIMDUtils;
//        using std::pow;
//
//        testMathOp<T> (
//            r, nIter, [] (auto a, auto b) { return pow (a, b); }, "Pow", maxError * (T) 100, (T) 1.25);
//    }
//
//    void runTestTimed() override
//    {
//        auto rand = getRandom();
//
//        beginTest ("Special Math Ops Test");
//        specialMathTest<float> (rand, 100, 1.0e-4f);
//        specialMathTest<double> (rand, 100, 1.0e-8);
//    }
//};
//
//static SIMDComplexTest simdComplexTest;
