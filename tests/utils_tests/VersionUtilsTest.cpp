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
        auto checkVersions = [=] (const String& strV1, const String& strV2, const std::array<bool, 6>& exp) {
            const Version v1 (strV1);
            const Version v2 (strV2);

            const Version vv1 ("v" + strV1);
            const Version vv2 ("v" + strV2);

            expectEquals (int (v1 == v2), (int) exp[0], "Incorrect version comparison (equals)!");
            expectEquals (int (vv1 == vv2), (int) exp[0], "Incorrect version comparison (equals)!");

            expectEquals (int (v1 != v2), (int) exp[1], "Incorrect version comparison (not equals)!");
            expectEquals (int (vv1 != vv2), (int) exp[1], "Incorrect version comparison (not equals)!");

            expectEquals (int (v1 > v2), (int) exp[2], "Incorrect version comparison (greater than)!");
            expectEquals (int (vv1 > vv2), (int) exp[2], "Incorrect version comparison (greater than)!");

            expectEquals (int (v1 < v2), (int) exp[3], "Incorrect version comparison (less than)!");
            expectEquals (int (vv1 < vv2), (int) exp[3], "Incorrect version comparison (less than)!");

            expectEquals (int (v1 >= v2), (int) exp[4], "Incorrect version comparison (greater than or equals)!");
            expectEquals (int (vv1 >= vv2), (int) exp[4], "Incorrect version comparison (greater than or equals)!");

            expectEquals (int (v1 <= v2), (int) exp[5], "Incorrect version comparison (less than or equals)!");
            expectEquals (int (vv1 <= vv2), (int) exp[5], "Incorrect version comparison (less than or equals)!");
        };

        checkVersions ("1.1.1", "1.1.1", { true, false, false, false, true, true });
        checkVersions ("1.1.2", "1.1.1", { false, true, true, false, true, false });
        checkVersions ("1.1.0", "1.1.1", { false, true, false, true, false, true });
        checkVersions ("1.2.1", "1.1.1", { false, true, true, false, true, false });
        checkVersions ("1.0.1", "1.1.1", { false, true, false, true, false, true });
        checkVersions ("2.1.1", "1.1.1", { false, true, true, false, true, false });
        checkVersions ("0.1.1", "1.1.1", { false, true, false, true, false, true });
        checkVersions ("1.1.11", "1.1.9", { false, true, true, false, true, false });
        checkVersions ("1.9.1", "1.11.1", { false, true, false, true, false, true });
    }

    void versionStringTest()
    {
        Version v1 ("1.2.3");
        expectEquals (v1.getVersionString(), String ("1.2.3"), "Incorrect version string!");

        Version v2 ("v1.2.3");
        expectEquals (v2.getVersionString(), String ("1.2.3"), "Incorrect version string!");
    }

    void runTest() override
    {
        beginTest ("Version Compare Test");
        versionCompareTest();

        beginTest ("Version String Test");
        versionStringTest();
    }
};

static VersionUtilsTest versionUtilsTest;
