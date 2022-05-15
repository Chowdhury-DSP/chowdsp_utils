#include <TimedUnitTest.h>
#include <chowdsp_math/chowdsp_math.h>

class MatrixOpsTest : public TimedUnitTest
{
public:
    MatrixOpsTest() : TimedUnitTest ("Matrix Ops Test") {}

    void houseHolderTest()
    {
        constexpr size_t size = 8;

        std::array<float, size> data {};
        std::array<float, size> data2 {};
        std::fill (data.begin(), data.end(), 1.0f);

        chowdsp::MatrixOps::HouseHolder<float, size>::outOfPlace (data2.data(), data.data());
        chowdsp::MatrixOps::HouseHolder<float, size>::inPlace (data.data());

        for (auto& x : data)
            expectEquals (x, -1.0f, "Householder ouput is incorrect!");

        for (auto& x : data2)
            expectEquals (x, -1.0f, "Householder out-of-place ouput is incorrect!");
    }

    void houseHolderVecTest()
    {
        using VecType = xsimd::batch<float>;
        std::array<VecType, 2> data {};
        std::fill (data.begin(), data.end(), 1.0f);
        chowdsp::MatrixOps::HouseHolder<VecType, 2>::inPlace (data.data());

        for (auto& x : data)
            expect (xsimd::all (x == -1.0f), "Householder ouput is incorrect!");
    }

    void hadamardTest()
    {
        constexpr size_t size = 8;

        std::array<float, size> data {};
        std::array<float, size> data2 {};
        std::fill (data.begin(), data.end(), 1.0f);

        chowdsp::MatrixOps::Hadamard<float, size>::outOfPlace (data2.data(), data.data());
        chowdsp::MatrixOps::Hadamard<float, size>::inPlace (data.data());

        expectEquals (data[0], (float) size / (float) sqrt (size), "Hadamard value 0 is incorrect!");
        for (size_t i = 1; i < size; ++i)
            expectEquals (data[i], 0.0f, "Hadamard ouput is incorrect!");
    }

    void hadamardVecTest()
    {
        using VecType = xsimd::batch<float>;
        constexpr size_t size = 2;
        std::array<VecType, size> data {};
        std::fill (data.begin(), data.end(), 1.0f);
        chowdsp::MatrixOps::Hadamard<VecType, size>::inPlace (data.data());

        expectEquals (data[0].get (0), float (size * VecType ::size) / (float) sqrt (size * VecType::size), "Hadamard value 0 is incorrect!");
        expectEquals (data[0].get (1), 0.0f, "Hadamard output is incorrect!");
        expectEquals (data[0].get (2), 0.0f, "Hadamard output is incorrect!");
        expectEquals (data[0].get (3), 0.0f, "Hadamard output is incorrect!");
        for (size_t i = 1; i < size; ++i)
            for (size_t j = 0; j < VecType::size; ++j)
                expectEquals (data[i].get (j), 0.0f, "Hadamard ouput is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Householder Scalar Test");
        houseHolderTest();

        beginTest ("Householder Vector Test");
        houseHolderVecTest();

        beginTest ("Hadamard Scalar Test");
        hadamardTest();

        beginTest ("Hadamard Vector Test");
        hadamardVecTest();
    }
};

static MatrixOpsTest matrixOpsTest;
