#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

CHOWDSP_CHECK_HAS_MEMBER (HasName, name)
CHOWDSP_CHECK_HAS_STATIC_MEMBER (HasStaticName, name)
struct Type_With_No_Name
{
};
struct Type_With_Name
{
    std::string name;
};
struct Type_With_Name_And_Ctor
{
    Type_With_Name_And_Ctor (int, bool) {}
    std::string name;
};
struct Type_With_Static_Name
{
    static std::string name;
};

CHOWDSP_CHECK_HAS_METHOD (HasGetName, getName, std::declval<Type_With_Name_And_Ctor&>())
CHOWDSP_CHECK_HAS_STATIC_METHOD (HasStaticGetName, getName)
struct Type_With_Get_Name
{
    std::string getName (Type_With_Name_And_Ctor&) { return ""; }
};
struct Type_With_Get_Name_And_Ctor
{
    Type_With_Get_Name_And_Ctor (int, bool) {}
    std::string getName (Type_With_Name_And_Ctor&) { return ""; }
};
struct Type_With_Static_Get_Name
{
    static std::string getName (Type_With_Name_And_Ctor&) { return ""; }
};

TEST_CASE ("Type Checkers Test", "[common][types]")
{
    SECTION ("Has Member")
    {
        STATIC_REQUIRE (HasName<Type_With_No_Name> == false);
        STATIC_REQUIRE (HasName<Type_With_Name> == true);
        STATIC_REQUIRE (HasName<Type_With_Name_And_Ctor> == true);
        STATIC_REQUIRE (HasName<Type_With_Static_Name> == false);
    }

    SECTION ("Has Static Member")
    {
        STATIC_REQUIRE (HasStaticName<Type_With_No_Name> == false);
        STATIC_REQUIRE (HasStaticName<Type_With_Name> == false);
        STATIC_REQUIRE (HasStaticName<Type_With_Static_Name> == true);
    }

    SECTION ("Has Method")
    {
        STATIC_REQUIRE (HasGetName<Type_With_No_Name> == false);
        STATIC_REQUIRE (HasGetName<Type_With_Get_Name> == true);
        STATIC_REQUIRE (HasGetName<Type_With_Get_Name_And_Ctor> == true);
    }

    SECTION ("Has Static Method")
    {
        STATIC_REQUIRE (HasStaticGetName<Type_With_No_Name> == false);
        //        STATIC_REQUIRE (HasStaticGetName<Type_With_Get_Name> == false);
        //        STATIC_REQUIRE (HasStaticGetName<Type_With_Get_Name_And_Ctor> == false);
        STATIC_REQUIRE (HasStaticGetName<Type_With_Static_Get_Name> == true);
    }
}
