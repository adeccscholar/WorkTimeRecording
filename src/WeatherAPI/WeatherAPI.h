#pragma once


#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef WEATHERAPI_BUILD_DLL
#    define WEATHERAPI_API __declspec(dllexport)
#  else
#    define WEATHERAPI_API __declspec(dllimport)
#  endif
#else
#  define WEATHERAPI_API __attribute__((visibility("default")))
#endif

