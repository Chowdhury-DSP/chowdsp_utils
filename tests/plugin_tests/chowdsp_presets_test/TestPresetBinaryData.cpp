/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{
//================== test_preset.preset ==================
static const unsigned char temp_binary_data_0[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "\n"
    "<Preset name=\"Test Preset\" plugin=\"DummyPlugin\" vendor=\"Factory\" category=\"\" version=\"1.0.0\">\n"
    "  <Parameters>\n"
    "    <PARAM id=\"dummy\" value=\"1.0\"/>\n"
    "  </Parameters>\n"
    "</Preset>\n";

const char* test_preset_preset = (const char*) temp_binary_data_0;

const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0xfc138352:
            numBytes = 211;
            return test_preset_preset;
        default:
            break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] = {
    "test_preset_preset"
};

const char* originalFilenames[] = {
    "test_preset.preset"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
    {
        if (namedResourceList[i] == resourceNameUTF8)
            return originalFilenames[i];
    }

    return nullptr;
}

} // namespace BinaryData
