#ifndef _PLATFORM_H

#define _PLATFORM_H

#ifdef WIN32
    #define DLLEXPORT __declspec(dllexport)
#else
    #define DLLEXPORT
#endif

#endif
