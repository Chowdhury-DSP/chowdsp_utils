/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{
//================== test_preset.preset ==================
static const unsigned char temp_binary_data_0[] =
    "{\n"
    "    \"category\": \"DRUM\",\n"
    "    \"extra_info\": null,\n"
    "    \"name\": \"Name\",\n"
    "    \"plugin\": \"TestPlugin\",\n"
    "    \"preset_file\": \"/Users/jatin/test_preset.preset\",\n"
    "    \"preset_state\": {\n"
    "        \"tag\": 0.0\n"
    "    },\n"
    "    \"vendor\": \"Vendor\",\n"
    "    \"version\": \"9.9.9\"\n"
    "}\n";

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
