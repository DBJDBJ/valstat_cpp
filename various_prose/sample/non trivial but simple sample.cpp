/*
https://godbolt.org/z/7bn9r5
*/

#include "dbj_strerror.h"

// https://stackoverflow.com/a/33206814
#define VT_WHITE_BRIGHT "\033[97;1m"
#define VT_RED_BRIGHT "\033[91;1m"
#define VT_RESET "\033[0m"
#define RED_MSG(S_) VT_RED_BRIGHT S_ VT_RESET
#define WHT_MSG(S_) VT_WHITE_BRIGHT S_ VT_RESET

/*
 (c) 2020 by dbj@dbj.org,  https://dbj.org/license_dbj/
 valstat(tm) and metastate(tm) are protected trade marks

2020 Q4, one std-proposals discussion was centered arround this use-case

 std::optional<std::string> FindUsersCity() {
   std::optional<ContactsServer> contacts = GetOrOpenContactsServerConnection();
   std::optional<UserId>         uid      = contacts.GetOrReturnNullOpt()->GetUserId();
   std::optional<GeoServer>      geo      = GetOrOpenGeoServerConnection();
   std::optional<Location>       uloc     = geo.GetOrReturnNullOpt()->GetLocation(*uid);
   return uloc.GetOrReturnNullOpt()->GetCityName();
}
   For example GetOrReturnNullOpt() is a method that
   (1) either returns a value if std::optional is not empty,
   (2) or returns (exits) from FindUsersCity() if std::optional is empty.

   Bary Revzin suggested an very interesting C++20 solution using
   the co_await machinery

   bellow is my solution based around valstat
*/
#undef DBJ_CPP03
#undef DBJ_CPP11
#undef DBJ_CPP14
#undef DBJ_CPP17
#undef DBJ_CPP20

/*	199711L(until C++11) */
#if (__cplusplus == 199711)
#define DBJ_CPP03 1
#elif (__cplusplus == 201103)
#define DBJ_CPP11 1
#elif (__cplusplus == 201402)
#define DBJ_CPP14 1
#elif (__cplusplus == 201703)
#define DBJ_CPP17 1
#elif (__cplusplus == 202002)
#define DBJ_CPP20 1
#else
#error Unsuported C++ version
#endif

#include <optional>

inline auto initialize_once = []() noexcept
{
    srand((unsigned)time(0));
    return true;
}();

/* the VALSTAT structure */
template <typename T>
struct [[nodiscard]] valstat final
{
    std::optional<T> value;
    /* 
    status (if not empty) is errno message + user prompt 
    see dbj_strerror.h for details
    */
    std::optional<dbj_strerror_msg> status{};
};
/*
NOTE: one could pass some internal error code instead of messages
that is more efficient but is coupling us with the logic and 
implementation of making the error message
*/

////////////////////////////////////////////////////////////////
// Values are inside or not inside the two valstat fields
// For step one of decoding the valstat returned
// the actual value is irelevant
// only its field state of occupancy matters

// vscall actually returns new valstat on error state
// valstat error state is: "value field is empty"
#define return_on_empty_value(VS) \
    if (!VS.value)                \
        return { {}, VS.status }

#define vscall(VS, F) \
    auto VS = F;      \
    return_on_empty_value(VS)

////////////////////////////////////////////////////////////
//
// here we are emulating some busines logic  API
// using the valstat protocol
// we will be meandering between various
// types when composing various adhoc valstat's
//
// we emulate non-deterministic business system and logic
// some data may be there or not there
// random 0/1 flip-flop
// srand() is called above
#define RANDOM_0_OR_1 bool(rand() % 2)

struct UserId final
{
};
struct ContactsServer final
{
    valstat<UserId> GetUserId() noexcept
    {
        static UserId uid_;
        // random flip OK or ERROR metastate return
        if (RANDOM_0_OR_1)
            // OK = occupied value field AND empty status field
            return {uid_, {}};

        // ERROR = empty value field AND occupied status field
        // error codes are random and arbitrary
        return {{}, dbj_strerror(EFAULT, RED_MSG("while trying to obtain user id."))};
    }
};

struct Location final
{
    valstat<const char *> GetCityName() noexcept
    {
        // OK = occupied value field AND empty status field
        return {"Valhala", {}};
    }
};

struct GeoServer final
{
    valstat<Location> GetLocation(UserId *uid) noexcept
    {
        static Location loc_;
        if (RANDOM_0_OR_1)
            return {loc_, {}};
        // ERROR = empty value field AND occupied status field
        return {{}, dbj_strerror(EBUSY, RED_MSG("upon getting the location."))};
    }
};

inline valstat<ContactsServer>
GetOrOpenContactsServerConnection() noexcept
{
    static ContactsServer cs{};
    if (RANDOM_0_OR_1)
        return {cs, {}};
    // ERROR = empty value field AND occupied status field
    return {{}, dbj_strerror(EINVAL, RED_MSG("while trying to get or open the server connection"))};
}

inline valstat<GeoServer>
GetOrOpenGeoServerConnection() noexcept
{
    static GeoServer gs_;
    if (RANDOM_0_OR_1)
        return {gs_, {}};
    // ERROR = empty value field AND occupied status field
    return {{}, dbj_strerror(ESPIPE, RED_MSG("Could not open geo server connection"))};
}

///////////////////////////////////////////////////////////////////
// return type is record of two fields aka valstat<T>
// vscall macro only checks the occupancy of the value field
// T is irrelevant for the step one of valstat decoding
inline valstat<const char *> FindUsersCity() noexcept
{
    vscall(contacts, GetOrOpenContactsServerConnection());
    vscall(uid, contacts.value->GetUserId());
    vscall(geo, GetOrOpenGeoServerConnection());
    vscall(uloc, geo.value->GetLocation(&*uid.value));
    vscall(cityname, (uloc.value)->GetCityName());
    // all the vscall's above do return_on_empty_value
    // thus the value field must be "occupied" here
    return {*cityname.value, {}};
}

static int test_()
{
    int id_ = 0;
    // business logic: loop until city is found
    do
    {
        id_ += 1;
        auto [city_name, status] = FindUsersCity();

        // notice the two step decoding
        // step one: is field "occupied"
        // we do not need the type information
        // of the value inside the field
        if (city_name)
        {
            /*
        step two: use the value field is holding
        FindUsersCity() returns valstat<const char*>
        thus city_name type is std::optional<const char *>
        */
            printf("\n\n" WHT_MSG("City name is: %s") "\n", *city_name);
            break;
        }
        // if present the status means error
        // step one: is field "occupied"
        if (status)
            // step two: use the value field is holding
            printf("\n" RED_MSG("ERROR: ") "%s ", status->data);

    } while (true);
    return id_;
}

int main()
{
    // test "tries" until city is found
    int tries = test_();
    printf("\n" WHT_MSG("Found after %4d tries"), tries - 1);
    printf("\n" WHT_MSG("---------------------------------"));
}


#undef vscall
#undef return_on_empty_value

#undef VT_WHITE_BRIGHT
#undef VT_RED_BRIGHT
#undef VT_RESET
#undef RED_MSG
#undef WHT_MSG

/*
    VALSTAT variant for above but inside 
    tight runtimes depends on two facts:

    if(!value) yields true for value == null
    if(!status) yields true if status == 0
	
    template<typename T> struct valstat final {
		T* value;
 		int status;
	};
*/