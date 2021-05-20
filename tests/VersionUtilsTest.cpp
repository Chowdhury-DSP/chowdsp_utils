#include <JuceHeader.h>

using namespace chowdsp::VersionUtils;

class VersionUtilsTest : public UnitTest
{
public:
    VersionUtilsTest() : UnitTest ("Version Utils Test")
    {
    }

    void versionCompareTest()
    {
        auto checkVersions = [=] (const String& v1, const String& v2, int exp)
        {
            auto actual1 = compareVersions (v1, v2);
            expectEquals (actual1, exp, "Incorrect version comparison!");

            auto actual2 = compareVersions ("v" + v1, "v" + v2);
            expectEquals (actual2, exp, "Incorrect version comparison!");
        };

        checkVersions ("1.1.1", "1.1.1", 0);
        checkVersions ("1.1.2", "1.1.1", 1);
        checkVersions ("1.1.0", "1.1.1", -1);
        checkVersions ("1.2.1", "1.1.1", 1);
        checkVersions ("1.0.1", "1.1.1", -1);
        checkVersions ("2.1.1", "1.1.1", 1);
        checkVersions ("0.1.1", "1.1.1", -1);
        checkVersions ("1.1.11", "1.1.9", 1);
        checkVersions ("1.9.1", "1.11.1", -1);
    }

    void runTest() override
    {
        beginTest("Version Compare Test");
        versionCompareTest();
    }
};

static VersionUtilsTest versionUtilsTest;
