#ifndef gess_main_h
#define gess_main_h

/* Order of includes is important! */
#include <libircclient.h>
#include <allegro5/allegro.h>
#include "widgetz.h"
#include "terminal.h"
#include "data_types.h"
#include "gui.h"


// board macros
#define in_board(i,j) (((i>=0) && (i<20) && (j>=0) && (j<20)) ? 1 : 0)
#define set_brd(t, i, j, k) do{ if(in_board(i,j)) t[i][j] = k; }while(0)

// global variables
extern ALLEGRO_EVENT_SOURCE user_event_src;
extern char player_nick[32];
extern char opponent_nick[32];
irc_session_t *g_irc_s;

// function prototypes
void emit_event(int event_type);
void emit_data_event(int event, intptr_t d1, intptr_t d2, intptr_t d3, intptr_t d4);
int IRC_connect(const char *server, int port, const char *nick, const char *channel);
char *strdup(const char *s);

#endif
