#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Type Traits Test", "[common][types]")
{
    using namespace chowdsp::TypeTraits;

    STATIC_REQUIRE (IsIterable<std::array<int, 4>>);
    STATIC_REQUIRE (IsIterable<std::vector<int>>);
    STATIC_REQUIRE (IsIterable<std::string>);
    STATIC_REQUIRE (IsIterable<std::map<int, int>>);
    STATIC_REQUIRE (IsIterable<chowdsp::SmallVector<int, 4>>);
    STATIC_REQUIRE (IsIterable<chowdsp::StringLiteral<4>>);
    STATIC_REQUIRE_FALSE (IsIterable<std::tuple<int>>);
    STATIC_REQUIRE_FALSE (IsIterable<chowdsp::NullType>);

    STATIC_REQUIRE (IsVectorLike<std::vector<int>>);
    STATIC_REQUIRE (IsVectorLike<chowdsp::SmallVector<int, 4>>);
    STATIC_REQUIRE_FALSE (IsVectorLike<std::array<int, 4>>);
    STATIC_REQUIRE_FALSE (IsVectorLike<chowdsp::NullType>);

    STATIC_REQUIRE (IsMapLike<std::unordered_map<int, int>>);
    STATIC_REQUIRE (IsMapLike<std::map<int, int>>);
    STATIC_REQUIRE_FALSE (IsMapLike<std::vector<int>>);

    struct A
    {
    };
    struct B;
    STATIC_REQUIRE (chowdsp::is_complete_type_v<A>);
    STATIC_REQUIRE_FALSE (chowdsp::is_complete_type_v<B>);
}
