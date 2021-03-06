/* Koro's macros */
#ifndef koro_macros_h
#define koro_macros_h

#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h>
#include <android/log.h>

#define MOBILE 1
#define deblog(x,...) __android_log_print(ANDROID_LOG_INFO,"koro: ","%s:"x, __FILE__, ##__VA_ARGS__)
#define errlog(x,...) __android_log_print(ANDROID_LOG_INFO,"koro: ","%s:"x, __FILE__, ##__VA_ARGS__)
#else
#define MOBILE 0
#define deblog(x, ...) fprintf(stderr, "koro:%s:%u: "x"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define errlog(x, ...) fprintf(stderr, "koro ERROR:%s:%u: "x"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define SWAP(x,y) do \
{ unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
    memcpy(swap_temp,&y,sizeof(x)); \
    memcpy(&y,&x,       sizeof(x)); \
    memcpy(&x,swap_temp,sizeof(x)); \
} while(0)

#define PI 3.1415926558

#define sign(x) (((x) == 0) ? 0 : (((x) > 0) ? 1 : -1))
#define SWITCH(x) ( ((x) = ((x) ? (0):(1))) )
#undef min 
#undef max
#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))
#define iabs(x) ((x) > (-(x)) ? (x) : (-(x)))
#define nfree(x) do { free((x)); (x) = NULL; } while(0)
#define ndestroy_bitmap(x) do { al_destroy_bitmap((x)); (x) = NULL; } while(0)
#define nmod(x, n) ((((x) % (n)) + (n)) % (n))

// the sleep macro takes (float) seconds
#ifdef _WIN32
    #include <Windows.h>
    #define snprintf _snprintf
    #define vsnprintf _vsnprintf
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
    #define sleep(x) Sleep(x*1000) // neeeds <Windows.h>
#else
    #include <unistd.h>
    #define sleep(x) usleep(x*1000000)
#endif

#endif
