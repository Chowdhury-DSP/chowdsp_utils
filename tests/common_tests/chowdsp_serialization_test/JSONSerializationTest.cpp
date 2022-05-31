#include <chowdsp_serialization/chowdsp_serialization.h>
#include <TimedUnitTest.h>
#include <complex>

class JSONSerializationTest : public TimedUnitTest
{
public:
    JSONSerializationTest() : TimedUnitTest ("JSON Serialization Test") {}

    void numericTest()
    {
        struct Test
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
        };

        Test t;
        auto res = chowdsp::Serialization::serialize<chowdsp::JSONSerializer> (t);

        std::cout << res << std::endl;
        std::cout << std::endl;
    }

    void stringsTest()
    {
        struct Test
        {
            std::string x = "test_1";
            juce::String y = "test_2";
        };

        Test t;
        auto res = chowdsp::Serialization::serialize<chowdsp::JSONSerializer> (t);

        std::cout << res << std::endl;
        std::cout << std::endl;
    }

    void containerTest()
    {
        struct TestInner
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
        };

        struct Test
        {
            std::vector<int> x { 3, 4, 5 };
            std::array<TestInner, 4> y {};
        };

        Test t;
        auto res = chowdsp::Serialization::serialize<chowdsp::JSONSerializer> (t);

        std::cout << res << std::endl;
        std::cout << std::endl;
    }

    void runTestTimed() override
    {
        beginTest ("Numeric Test");
        numericTest();

        beginTest ("String Test");
        stringsTest();

        beginTest ("Container Test");
        containerTest();
    }
};

static JSONSerializationTest jsonSerializationTest;
