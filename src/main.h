#ifndef gess_main_h
#define gess_main_h

#include<allegro5/allegro.h>

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


#define DEFAULT_FONT_FILE "fonts/fixed_font.tga"

#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
#define EVENT_OPPONENT_MOVE (BASE_USER_EVENT_TYPE + 2)
#define EVENT_RESTART (BASE_USER_EVENT_TYPE + 3)
#define EVENT_EXIT (BASE_USER_EVENT_TYPE + 4)
#define EVENT_LOAD (BASE_USER_EVENT_TYPE + 5)
#define EVENT_SAVE (BASE_USER_EVENT_TYPE + 6)
#define EVENT_SETTINGS (BASE_USER_EVENT_TYPE + 7)


#define BF_CODEPOINT_START 0x0860

extern ALLEGRO_EVENT_SOURCE user_event_src;
extern char player_nick[32];
extern char opponent_nick[32];

void emit_event(int event_type);
void emit_data_event(int event, intptr_t d1, intptr_t d2, intptr_t d3, intptr_t d4);

void *irc_thread(ALLEGRO_THREAD *thr, void *arg);
void irc_get_next(char *opponent);
void irc_connect(char *nick);
#endif
