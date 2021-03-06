#ifndef gess_data_types_h
#define gess_data_types_h

#include <allegro5/allegro.h>
#include "terminal.h"
#include "widgetz/widgetz.h"

//#define DEFAULT_FONT_FILE "fonts/fixed_font.tga"

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
#define EVENT_IRC_NICK (BASE_USER_EVENT_TYPE + 13)
#define EVENT_IRC_QUIT (BASE_USER_EVENT_TYPE + 14)
#define EVENT_GAME_OVER (BASE_USER_EVENT_TYPE + 15)

#define PANEL_PORTION 0.30
#define PANEL_SPACE 0.01
//#define MAX_PANEL_PORTION 0.25

enum { // gui elements
    GUI_NULL,
    GUI_INFO,
    GUI_IRC,
    GUI_SETTINGS,
    GUI_CHAT,
    GUI_ACTION_1,
    GUI_ACTION_2,
    GUI_CONFIRM_EXIT,
    GUI_CONFIRM_DISCONNECT,
    GUI_MESSAGE,
    GUI_MATCH,
    GUI_MATCH_INCOMING,
    GUI_UNDO_INCOMING,
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
    BUTTON_UNDO,
    BUTTON_QUIT,
    BUTTON_RESET,
    BUTTON_MATCH,
    EDITBOX_SERVER,
    EDITBOX_PORT,
    EDITBOX_CHANNEL,
    EDITBOX_NICK,
    EDITBOX_MATCH,
};

enum { // status
    GAME_PLAYING,
    GAME_PLAYING_IRC,
    GAME_WAITING_MOVE_ACK,
    GAME_WAITING_OPPONENT_MOVE,
    GAME_SEEKING,
    GAME_OVER,
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
    int game_state;

    // fonts
    ALLEGRO_FONT *font; // fixed with please!
    int fsize;
    
    // irc stuff
    ALLEGRO_USTR *opponent;
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
    WZ_WIDGET* i_gui;
    WZ_WIDGET* chat_button;
    int chat_waiting;
    int panel_width;
    
    //  extra displayed guis (in stack order)
    WZ_WIDGET *gui;
    
    WZ_THEME* theme;
    WZ_THEME* theme_alt;
    // info gui helper
    
    ALLEGRO_USTR *player1_name;
    ALLEGRO_USTR *player2_name;
    WZ_WIDGET *player1_wgt;
    WZ_WIDGET *player2_wgt;
    
    int new_player; // what player I will be in next game
    int new_game_type; // game mode
    ALLEGRO_USTR *new_opponent; // opponent name in next game
    
    int request_undo;
} Board;



#endif
