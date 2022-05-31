#include <chowdsp_serialization/chowdsp_serialization.h>
#include <TimedUnitTest.h>

struct CustomTest
{
    juce::StringArray x { "one", "two", "three" };

    template <typename Serializer>
    static typename Serializer::SerializedType serialize (const CustomTest& object)
    {
        auto serial = Serializer::createBaseElement();
        for (auto& val : object.x)
            Serializer::addChildElement (serial, Serializer::template serialize<Serializer> (val));

        return serial;
    }

    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType serial, CustomTest& object)
    {
        object.x.clear();

        for (int i = 0; i < Serializer::getNumChildElements (serial); ++i)
        {
            juce::String newStr {};
            Serializer::template deserialize<Serializer> (Serializer::getChildElement (serial, i), newStr);
            object.x.add (std::move (newStr));
        }
    }

    static_assert (chowdsp::serialization_detail::HasCustomSerializer<CustomTest>::custom_serializer_value);
    static_assert (chowdsp::serialization_detail::HasCustomSerializer<CustomTest>::custom_deserializer_value);
};

template <typename Serializer>
class SerializationTest : public TimedUnitTest
{
public:
    SerializationTest() : TimedUnitTest (getSerializerTypeName() + " Serialization Test") {}

    static juce::String getSerializerTypeName()
    {
        if constexpr (std::is_same_v<Serializer, chowdsp::JSONSerializer>)
            return "JSON";
        else
            return "XML";
    }

    void numericTest()
    {
        struct Test
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
        };

        Test test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        std::cout << Serializer::toString (res) << std::endl;

        test.x = 4;
        test.y = 0.0f;
        test.z = 5.0;
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        expect (pfr::eq_fields (test, Test {}), "Serialization/Deserialization is incorrect");
    }

    void stringsTest()
    {
        struct Test
        {
            std::string x = "test_1";
            juce::String y = "test_2";
        };

        Test test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        std::cout << Serializer::toString (res) << std::endl;

        test.x = "";
        test.y = "";
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        expect (pfr::eq_fields (test, Test {}), "Serialization/Deserialization is incorrect");
    }

    void containerTest()
    {
        struct TestInner
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;

            bool operator== (const TestInner& other) const
            {
                return pfr::eq_fields (*this, other);
            }
        };

        struct Test
        {
            std::vector<int> x { 3, 4, 5 };
            std::array<TestInner, 4> y {};
            std::unordered_map<size_t, float> z { { 0, 1.0f }, { 1, -1.0f } };
        };

        Test test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        std::cout << Serializer::toString (res) << std::endl;

        test.x.clear();
        test.y[0].x = 40;
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        expect (pfr::eq_fields (test, Test {}), "Serialization/Deserialization is incorrect");
    }

    void customSerializationTest()
    {
        CustomTest test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        std::cout << Serializer::toString (res) << std::endl;

        test.x.getReference (0) = "";
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        expect (pfr::eq_fields (test, CustomTest {}), "Serialization/Deserialization is incorrect");
    }

    void runTestTimed() override
    {
        beginTest ("Numeric Test");
        numericTest();

        beginTest ("String Test");
        stringsTest();

        beginTest ("Container Test");
        containerTest();

        beginTest ("Custom Serialization Test");
        customSerializationTest();
    }
};

static SerializationTest<chowdsp::JSONSerializer> jsonSerializationTest;
static SerializationTest<chowdsp::XMLSerializer> xmlSerializationTest;
