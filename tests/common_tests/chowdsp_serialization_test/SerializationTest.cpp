#include <chowdsp_serialization/chowdsp_serialization.h>
#include <CatchUtils.h>
#include <test_utils.h>

#include "TestSerialBinaryData.h"

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

    static_assert (chowdsp::serialization_detail::HasCustomSerializer<CustomTest>);
    static_assert (chowdsp::serialization_detail::HasCustomDeserializer<CustomTest>);
};

TEMPLATE_TEST_CASE ("Serialization Test", "[common][serialization]", chowdsp::JSONSerializer, chowdsp::XMLSerializer)
{
    using Serializer = TestType;

    SECTION ("Numeric Test")
    {
        struct Test
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
        };

        Test test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        INFO (Serializer::toString (res));

        test.x = 4;
        test.y = 0.0f;
        test.z = 5.0;
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        REQUIRE_MESSAGE (pfr::eq_fields (test, Test {}), "Serialization/Deserialization is incorrect");
    }

    SECTION ("Enum Test")
    {
        enum class TestEnum
        {
            Zero,
            One,
            Two,
            Three,
        };

        struct Test
        {
            TestEnum x = TestEnum::Two;
        };

        Test test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        INFO (Serializer::toString (res));

        test.x = TestEnum::Three;
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        REQUIRE_MESSAGE (pfr::eq_fields (test, Test {}), "Serialization/Deserialization is incorrect");
    }

    SECTION ("String Test")
    {
        struct Test
        {
            std::string x = "test_1";
            juce::String y = "test_2";
        };

        Test test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        INFO (Serializer::toString (res));

        test.x = "";
        test.y = "";
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        REQUIRE_MESSAGE (pfr::eq_fields (test, Test {}), "Serialization/Deserialization is incorrect");
    }

    SECTION ("Container Test")
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
        INFO (Serializer::toString (res));

        test.x.clear();
        test.y[0].x = 40;
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        REQUIRE_MESSAGE (pfr::eq_fields (test, Test {}), "Serialization/Deserialization is incorrect");
    }

    SECTION ("Custom Serialization Test")
    {
        CustomTest test;
        auto res = chowdsp::Serialization::serialize<Serializer> (test);
        INFO (Serializer::toString (res));

        test.x.getReference (0) = "";
        chowdsp::Serialization::deserialize<Serializer> (res, test);

        REQUIRE_MESSAGE (pfr::eq_fields (test, CustomTest {}), "Serialization/Deserialization is incorrect");
    }

    SECTION ("File Serialization Test")
    {
        struct Test
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
        };

        const auto fileName = []
        {
            if constexpr (std::is_same_v<Serializer, chowdsp::XMLSerializer>)
                return "serial_test.xml";
            if constexpr (std::is_same_v<Serializer, chowdsp::JSONSerializer>)
                return "serial_test.json";
        }();

        test_utils::ScopedFile testFile { fileName };
        testFile.file.create();
        chowdsp::Serialization::serialize<Serializer> (Test {}, testFile.file);

        const auto ref = Serializer::toString (chowdsp::Serialization::serialize<Serializer> (Test {}));

        Test t2;
        chowdsp::Serialization::deserialize<Serializer> (testFile.file, t2);
        const auto actual = Serializer::toString (chowdsp::Serialization::serialize<Serializer> (t2));

        REQUIRE_MESSAGE (actual == ref, "File Serialization is Incorrect!");
    }

    SECTION ("MemoryBlock Serialization Test")
    {
        struct Test
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
        };

        juce::MemoryBlock block;
        chowdsp::Serialization::serialize<Serializer> (Test {}, block);

        const auto ref = Serializer::toString (chowdsp::Serialization::serialize<Serializer> (Test {}));

        Test t2;
        chowdsp::Serialization::deserialize<Serializer> (block, t2);
        const auto actual = Serializer::toString (chowdsp::Serialization::serialize<Serializer> (t2));

        REQUIRE_MESSAGE (actual == ref, "MemoryBlock Serialization is Incorrect!");
    }

    SECTION ("Bad Deserialization Test")
    {
        struct Test
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
            std::string str = "adsfhajklsdfhdajkl";
            juce::String jStr = "ajfhasjklfhasdklfhadksfha0";
            std::array<float, 3> arr { 1.0f, 2.0f, 3.0f };
            std::vector<size_t> vec { 2, 3, 4 };
        };

        const auto nullStruct = Test { 0, 0.0f, 0.0, "", "", {}, {} };

        {
            Test test;
            chowdsp::Serialization::deserialize<Serializer> (juce::File {}, test);

            REQUIRE_MESSAGE (pfr::eq_fields (test, nullStruct), "Bad File Deserialization is incorrect");
        }

        {
            Test test;
            chowdsp::Serialization::deserialize<Serializer> (nullptr, 0, test);

            REQUIRE_MESSAGE (pfr::eq_fields (test, nullStruct), "Bad Binary Data Deserialization is incorrect");
        }
    }

    SECTION ("Binary Data Deserialization Test")
    {
        struct Test
        {
            int x = 0;
            float y = 1.0f;
            double z = -3.0;
            std::string str;
        };

        Test expected { 40, -33.3f, 0.55, "test_data" };

        Test actual;
        if constexpr (std::is_same_v<Serializer, chowdsp::JSONSerializer>)
            chowdsp::Serialization::deserialize<Serializer> (BinaryData::test_json_json, BinaryData::test_json_jsonSize, actual);
        else
            chowdsp::Serialization::deserialize<Serializer> (BinaryData::test_xml_xml, BinaryData::test_xml_xmlSize, actual);

        REQUIRE_MESSAGE (pfr::eq_fields (expected, actual), "Binary Data Deserialization is incorrect");
    }
}
