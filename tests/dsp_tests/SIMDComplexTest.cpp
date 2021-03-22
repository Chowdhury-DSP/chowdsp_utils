#include <JuceHeader.h>

class SIMDComplexTest : public UnitTest
{
public:
    SIMDComplexTest() : UnitTest ("SIMD Complex Test") {}

    void zeroCheck()
    {
        auto a = chowdsp::SIMDComplex<float>();
        expect (a.atIndex (0) == std::complex<float> { 0, 0 }, "Zero initiatalisation incorrect!");

        auto b = a + a;
        for (size_t i = 0; i < chowdsp::SIMDComplex<float>::size; ++i)
            expect (a.atIndex (i) == b.atIndex (i), "Zero elements incorrect!");
    }

    template <typename T>
    void mathTest()
    {
        chowdsp::SIMDComplex<T> q;
        chowdsp::SIMDComplex<T> r;

        if constexpr (std::is_same<T, float>::value)
        {
            q = chowdsp::SIMDComplex<T> ({ (T) 0.f, (T) 1.f, (T) 2.f, (T) 3.f }, { (T) 1.f, (T) 0.f, (T) -1.f, (T) -2.f });
            r = chowdsp::SIMDComplex<T> ({ (T) 12.f, (T) 1.2f, (T) 2.4f, (T) 3.7f }, { (T) 1.2f, (T) 0.4f, (T) -1.2f, (T) -2.7f });
        }
        else if (std::is_same<T, double>::value)
        {
            q = chowdsp::SIMDComplex<T> ({ (T) 0.f, (T) 1.f }, { (T) 1.f, (T) 0.f });
            r = chowdsp::SIMDComplex<T> ({ (T) 12.f, (T) 1.2f }, { (T) 1.2f, (T) 0.4f });
        }

        expect (q.atIndex (0) == std::complex<T> ((T) 0.f, (T) 1.f));
        expect (q.atIndex (1) == std::complex<T> ((T) 1.f, (T) 0.f));
        if constexpr (std::is_same<T, float>::value)
        {
            expect (q.atIndex (2) == std::complex<T> ((T) 2.f, (T) -1.f));
            expect (q.atIndex (3) == std::complex<T> ((T) 3.f, (T) -2.f));
        }

        auto qpr = q + r;
        for (size_t i = 0; i < chowdsp::SIMDComplex<T>::size; ++i)
            expect (qpr.atIndex (i) == q.atIndex (i) + r.atIndex (i), "Addition incorrect!");

        auto qtr = q * r;
        for (size_t i = 0; i < chowdsp::SIMDComplex<T>::size; ++i)
            expect (qtr.atIndex (i) == q.atIndex (i) * r.atIndex (i), "Multiplication incorrect!");

        T sum = (T) 0;
        for (size_t i = 0; i < chowdsp::SIMDComplex<T>::size; ++i)
            sum += qtr.atIndex (i).real();

        T sumSIMD = qtr.real().sum();
        expectWithinAbsoluteError (sum, sumSIMD, (T) 1e-5, "Sum incorrect!");
    }

    template <typename T>
    void expTest()
    {
        T angles alignas (16)[chowdsp::SIMDComplex<T>::size];
        angles[0] = 0;
        angles[1] = MathConstants<T>::pi / (T) 2;
        if constexpr (std::is_same<T, float>::value)
        {
            angles[2] = (T) 3 * MathConstants<T>::pi / (T) 4;
            angles[3] = MathConstants<T>::pi;
        }
        auto asse = dsp::SIMDRegister<T>::fromRawArray (angles);
        auto c = chowdsp::SIMDComplex<T>::fastExp (asse);

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
        auto q = chowdsp::SIMDComplex<float> ({ 0.f, 1.f, 2.f, 3.f }, { 1.f, 0.f, -1.f, -2.f });
        auto powV = q.map ([] (const std::complex<float>& f) { return std::pow (f, 2.1f); });
        for (size_t i = 0; i < chowdsp::SIMDComplex<float>::size; ++i)
            expect (powV.atIndex (i) == std::pow (q.atIndex (i), 2.1f));
    }

    void runTest() override
    {
        beginTest ("Zero Check");
        zeroCheck();

        beginTest ("Math Test (float)");
        mathTest<float>();

        beginTest ("Math Test (double)");
        mathTest<double>();

        beginTest ("Exponential Test (float)");
        expTest<float>();

        beginTest ("Exponential Test (double)");
        expTest<double>();

        beginTest ("Map Test");
        mapTest();
    }
};

static SIMDComplexTest simdComplexTest;
