#ifndef gess_data_types_h
#define gess_data_types_h

#include <allegro5/allegro.h>
#include "terminal.h"
#include "widgetz.h"

#define DEFAULT_FONT_FILE "fonts/fixed_font.tga"

#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
#define EVENT_MOVE_RECEIVED (BASE_USER_EVENT_TYPE + 2)
#define EVENT_RESTART (BASE_USER_EVENT_TYPE + 3)
#define EVENT_EXIT (BASE_USER_EVENT_TYPE + 4)
#define EVENT_LOAD (BASE_USER_EVENT_TYPE + 5)
#define EVENT_SAVE (BASE_USER_EVENT_TYPE + 6)
#define EVENT_SETTINGS (BASE_USER_EVENT_TYPE + 7)
#define EVENT_IRC_CONNECT (BASE_USER_EVENT_TYPE + 8)
#define EVENT_IRC_JOIN (BASE_USER_EVENT_TYPE + 9)
#define EVENT_PRIVMSG_RECEIVED (BASE_USER_EVENT_TYPE + 10)
#define EVENT_CHANMSG_RECEIVED (BASE_USER_EVENT_TYPE + 11)
#define EVENT_IRC_DISCONNECT (BASE_USER_EVENT_TYPE + 12)

#define MIN_PANEL_PORTION 0.2
#define MAX_PANEL_PORTION 0.25

enum { // gui elements
    GUI_NULL,
    GUI_INFO,
    GUI_SETTINGS,
    GUI_CHAT,
    GUI_ACTION,
    GUI_CONFIRM_EXIT,
    GUI_CONFIRM_DISCONNECT,
    GUI_MESSAGE,
    BUTTON_OK,
    BUTTON_CANCEL,
    BUTTON_COLOR,
    BUTTON_SETTINGS,
    BUTTON_CHAT,
    BUTTON_IRC_STATUS,
    BUTTON_ACTION,
    BUTTON_SEEK,
    BUTTON_FLIP,
    BUTTON_CONNECT,
    BUTTON_UNDO
};

enum { // status
    GAME_PLAYING,
    GAME_PLAYING_IRC,
    GAME_WAITING_MOVE_ACK,
    GAME_WAITING_OPPONENT_MOVE,
    GAME_SEEKING,
};

enum {
    MODE_SAME_DEVICE,
    MODE_ONLINE,
};

typedef struct Block {
    int b[3][3];
} Block;

typedef struct Board_State{
    int s[20][20];
    char last_move[5];
    struct Board_State *parent;
    struct Board_State *child;
} Board_State;


typedef struct Game {
    Board_State *brd;
    int turn;
    int moves;
    char (*history)[5];
} Game;

typedef struct WZ_WIDGET_LIST {
    WZ_WIDGET *wgt;
    
    struct WZ_WIDGET_LIST *next;
} WZ_WIDGET_LIST;

typedef struct Board {
    // screen
    int xsize;
    int ysize;
    // real board
    int tsize;
    int size;
    int pr; // stone radius
    int x;
    int y;
    ALLEGRO_COLOR pcolor[3];
    ALLEGRO_COLOR bg_color;
    // focus
    int fi;
    int fj;
    int lock;
    int lock_i;
    int lock_j;
    Block lock_blk;
    // distance that can be moved in direction (i-1, j-1) for the locked block
    Block move;
    int move_mark[20][20];
    int pov; // player on the bottom?
    ALLEGRO_BITMAP *board_bmp;
    int draw_last; // draw last move?
    int allow_move;
    int board_input; // allow board input?
    
    // fonts
    ALLEGRO_FONT *font; // fixed with please!
    int fsize;
    
    // irc stuff
    ALLEGRO_USTR *opponent;
    int game_state;
    ALLEGRO_USTR *server;
    ALLEGRO_USTR *channel;
    int port;
    ALLEGRO_USTR *nick;
    int game_type;
    int player; // on irc who is the player
    int connected;
    int request_player;
    Terminal *chat_term;
    ALLEGRO_USTR *irc_status_msg;
    
    int focus_board;
    
    // guis | todo: make one big gui (whole screen) and destroy/create other guis as widgets
    struct WZ_WIDGET* i_gui;
    struct WZ_WIDGET* settings_gui;
    
    //  extra displayed guis (in stack order)
    WZ_WIDGET_LIST *gui;
//    WZ_WIDGET* gui[5];
    void *guiel[5];
    int gui_n;
    
    WZ_THEME* theme;
    // info gui helper
    ALLEGRO_USTR *s_player1_name;
    ALLEGRO_USTR *s_player2_name;
    
    // settings gui helper (move all settings to a signle struct later!)
    ALLEGRO_USTR *s_server;
    ALLEGRO_USTR *s_port;
    ALLEGRO_USTR *s_channel;
    ALLEGRO_USTR *s_nick;
    ALLEGRO_USTR *s_color;
    
    ALLEGRO_BITMAP *bmp_turn1;
    ALLEGRO_BITMAP *bmp_turn2;
} Board;



#endif
