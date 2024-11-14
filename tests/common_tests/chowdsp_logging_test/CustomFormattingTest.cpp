#include <chowdsp_logging/chowdsp_logging.h>
#include <CatchUtils.h>

TEST_CASE ("Custom Formatting Test", "[common][logs]")
{
    chowdsp::ChainedArenaAllocator arena { 2048 };

    SECTION ("juce::String")
    {
        juce::String str { "This is a JUCE string!" };
        const auto format_result = chowdsp::format (arena, "{}", str);
        REQUIRE (str == chowdsp::toString (format_result));
    }

    SECTION ("std::span")
    {
        std::vector vec { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f };
        const auto format_result = chowdsp::format (arena, "{}", nonstd::span { vec });
        REQUIRE (format_result == "{0,1,2,3,4}");
    }

    SECTION ("Large String")
    {
        std::string large_string { "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam ipsum enim, bibendum venenatis sollicitudin eget, pretium at sem. Vivamus efficitur risus eget nulla tempus, eget lacinia orci dictum. Sed egestas nulla accumsan sodales finibus. Phasellus faucibus, sapien at aliquam gravida, turpis mi pellentesque mauris, et bibendum purus turpis sit amet elit. In in metus ac quam dictum sodales. Integer odio ex, mattis ac pulvinar vitae, congue vitae tortor. Duis porta vehicula consequat. Nullam pellentesque malesuada massa, id fermentum metus ultrices condimentum. Quisque felis mauris, vulputate nec luctus sed, suscipit in justo. Quisque et porta est. In in congue arcu. Donec posuere rutrum sapien, nec efficitur velit ultricies id. Vivamus eget sollicitudin leo. Donec accumsan placerat risus, eget congue enim egestas sit amet. Aliquam erat volutpat. Ut vitae dignissim elit. Integer dignissim id leo eu semper. Nullam pulvinar ac eros et hendrerit. Maecenas imperdiet, odio sit amet cursus maximus, urna augue tempus orci, fermentum porta magna sem id tortor. Sed consectetur, arcu quis placerat eleifend, augue dolor egestas tellus, in tempor eros ante quis urna. Phasellus id eleifend justo, vitae viverra tortor. Phasellus tincidunt porttitor auctor. Fusce a velit quis orci ullamcorper blandit sit amet vel arcu. Ut venenatis mauris ac augue scelerisque, sit amet malesuada lacus aliquam. Interdum et malesuada fames ac ante ipsum primis in faucibus. Sed nec magna at ipsum suscipit eleifend sit amet ac sapien. Nunc ut metus erat. Suspendisse nisl mauris, accumsan sed scelerisque et, mattis eget sem. Aliquam pulvinar metus a scelerisque vulputate. Etiam non risus risus. Sed ornare nunc fringilla consequat commodo. Nam vitae egestas enim. Duis pretium leo et felis aliquam, ullamcorper pellentesque neque congue. Vestibulum posuere massa vel purus efficitur accumsan. Integer ut metus eget elit viverra blandit. Nulla at tincidunt justo, a ornare nunc. Pellentesque scelerisque eros vitae magna posuere volutpat. Pellentesque euismod nunc vitae odio aliquam, non gravida tellus dignissim. Duis semper eu ligula at sollicitudin. Nullam euismod, diam et mollis consectetur, mauris ex facilisis dolor, nec fringilla massa nulla a dui. Curabitur lacus purus, bibendum ac justo vestibulum, tempus gravida massa. Donec quis condimentum quam. Morbi eleifend mi lectus, quis convallis ante vestibulum eget. Praesent eu metus non mauris scelerisque malesuada. Vivamus malesuada elit ac lacus fermentum sodales. Nullam blandit leo nulla, et condimentum velit bibendum pellentesque. Fusce felis risus, rhoncus eu tempus sed, euismod ut eros. Praesent sagittis placerat nisl nec aliquet. Fusce id eros diam. Nullam in tincidunt lectus. Vestibulum at finibus sem. Suspendisse eleifend mauris nec nulla ornare lacinia in vel felis. Sed finibus tempor pharetra. Aliquam efficitur aliquam suscipit. Phasellus commodo euismod posuere. Phasellus pharetra justo quam, eu auctor orci laoreet quis." };
        const auto format_result = chowdsp::format (arena, "{}", large_string);
        REQUIRE (format_result == large_string);
    }
}
