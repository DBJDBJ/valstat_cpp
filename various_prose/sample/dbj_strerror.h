#ifndef DBJ_STERROR_INC_
#define DBJ_STERROR_INC_
/*
First problem is this page: https://docs.microsoft.com/en-us/cpp/c-language/strerror-function?view=msvc-160

const char * const msg_ = _sys_errlist[ec_] ;

MSVC crt error messages are in  _sys_errlist[0 .. 43]
errno outside of that message is "Unknown Error"
which is at _sys_errlist[43]

It is unclear if that applies since there is also:
https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-doserrno-sys-errlist-and-sys-nerr?view=msvc-160


Nessage: "Unknown Error" is misleading
Thus we will return: "Not a legal Windows errno: %d"
NOTE: if Windows SDK changes errno.h this will have to be changed
chances of that happening are very close to zero. But still not zero.
date of this code: 2021-03-14
*/

/*
Bounds-checked functions, 
are only guaranteed to be available if 
__STDC_LIB_EXT1__ is defined by the implementation 
and if the user defines __STDC_WANT_LIB_EXT1__ to the integer constant 1
before including
*/
#define __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1

/*
As all functions from Dynamic Memory TR, strndup is only guaranteed
to be available if __STDC_ALLOC_LIB__ is defined by the implementation
and if the user defines __STDC_WANT_LIB_EXT2__ to the integer 
constant 1 before including string.h.
*/

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    enum
    {
        dbj_strerror_max_len = 1024,
        dbj_strerror_mid_len = 512,
    };
    
    typedef struct
    {
        char data[dbj_strerror_max_len];
    } dbj_strerror_msg;

    /* 
    ec_ : the errno value 
    user_msg_ : max len 512 
    */
    inline dbj_strerror_msg dbj_strerror(
        unsigned int ec_, const char user_msg_[dbj_strerror_mid_len])
    {
        assert(strnlen_s(user_msg_, dbj_strerror_mid_len) < dbj_strerror_mid_len);
#ifdef _WIN32
        bool illegal_win_errno = false;
        switch (ec_)
        {
        case 15:
        case 26:
        case 37:
            illegal_win_errno = true;
            break;
        default:
        {
            // over 42 only 80 is legal
            if ((ec_ > 42) && (ec_ != 80))
                illegal_win_errno = true;
        }
        } // switch
        if (illegal_win_errno)
        {
            dbj_strerror_msg msg_ = {0};
            snprintf(msg_.data, dbj_strerror_max_len, "Not a legal Windows errno: %d", ec_);
            return msg_;
        }
#endif // _WIN32

        static const char *const fmt_ = "%s (%s)";
        char errmsg[dbj_strerror_mid_len]{0};

        int rezult = strerror_s(errmsg, dbj_strerror_mid_len, ec_);
        assert(rezult == 0);

        size_t size_required = 1 +
                               snprintf(nullptr, 0, fmt_, errmsg, user_msg_);

        assert((size_required > 1) && (size_required < dbj_strerror_max_len));

        dbj_strerror_msg msg_ = {0};
        snprintf(msg_.data, dbj_strerror_max_len, fmt_, errmsg, user_msg_);

        return msg_;
    }

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // DBJ_STERROR_INC_
