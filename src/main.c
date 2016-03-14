/*
Todo:

- deal with POV (how to draw when pov = 2?
- allow coordinates to be typed for move
- gui for nick choosing / new game / save game
- undo feature
 
 */


#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <string.h>
#include "macros.h"
#include "allegro_stuff.h"
#include "main.h"
#include <libircclient.h>

#define FPS 60.0

int INITIAL_POSITION[20][20] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0},
    {0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0},
    {0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 0, 2, 0, 0},
    {0, 2, 2, 2, 0, 2, 0, 2, 2, 2, 2, 0, 2, 0, 2, 0, 2, 2, 2, 0},
    {0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 0, 2, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


// for irc
char player_nick[32];
char opponent_nick[32];


#define FOCUS_COLOR al_premul_rgba(255, 250, 250, 50)
#define LOCK_COLOR al_premul_rgba(255, 250, 250, 90)
#define FOCUS_NOMOVE_COLOR al_premul_rgba(255, 255, 255, 30)
#define FOCUS_CENTER_COLOR al_premul_rgba(255, 255, 255, 60)
//#define MOVE_COLOR al_premul_rgba(255, 255, 0, 100)

// this is mainly for testing, not actually used. use emit_event(EVENT_TYPE) to emit user events.
#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
ALLEGRO_EVENT_SOURCE user_event_src;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;

ALLEGRO_COLOR MOVE_COLOR[4] = {{0,0,0,0}, {0.2,0.2,0,0.2}, {0.2,0,0,0.2}, {0.2,0,0,0.2}};

float RESIZE_DELAY = 0.04;
float fixed_dt = 1.0/FPS;

int desktop_xsize, desktop_ysize;
int fullscreen;

enum {
    GAME_PLAYING,
    GAME_PLAYING_IRC,
    GAME_WAITING_MOVE_ACK,
    GAME_WAITING_OPPONENT_MOVE,
    GAME_SEEKING,
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

typedef struct Board {
    int tsize;
    int size;
    int pr;
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
    int player; // on irc who is the player
    ALLEGRO_BITMAP *board_bmp;
    int draw_last; // draw last move?
    
// irc stuff
    char *opponent;
    int game_state;
    char *server;
    char *channel;
    int port;
    char *nick;
} Board;

typedef struct Game {
    Board_State *brd;
    int turn;
    int moves;
    char (*history)[5];
} Game;

#define in_board(i,j) (((i>=0) && (i<20) && (j>=0) && (j<20)) ? 1 : 0)
#define set_brd(t, i, j, k) do{ if(in_board(i,j)) t[i][j] = k; }while(0)

ALLEGRO_COLOR color_trans(ALLEGRO_COLOR c, float f){
    return (ALLEGRO_COLOR){c.r*f, c.g*f, c.b*f, c.a*f};
}

int brd(Board_State *p, int i, int j){
    if ( ((i<0) || (i>=20) || (j<0) || (j>=20)) )
        return 0;
    else
        return p->s[i][j];
}


// convert letter coordinates to board coordinates and vice-versa

char i_to_coord(int i){
    return i + 'a';
}

char j_to_coord(int j){
    return 'a'+(19-j);
}

int coord_to_i(char ci){
    return ci-'a';
}

int coord_to_j(char cj){
    return 19 - (cj-'a');
}

void get_move_coords(char *str, int i, int j, int ii, int jj){
    str[0] = i_to_coord(i);
    str[1] = j_to_coord(j);
    str[2] = i_to_coord(ii);
    str[3] = j_to_coord(jj);
    str[4] = '\0';
}

// move elsewhere
char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);   // Allocate memory
    if (d != NULL) strcpy (d,s);         // Copy string if okay
    return d;                            // Return new memory
}



void send_privmsg(char *nick, char *msg){
    irc_cmd_msg(g_irc_s, nick, msg);
    deblog("SENT: %s | %s", nick, msg);
}

void acknowledge_privmsg(char *nick, char *msg){
    char str[128];
    snprintf(str, 127, ":ACK %s", msg);
    send_privmsg(nick, str);
}

void send_move(Game *g, Board *b){
    char move[6];
    move[0] = ',';
    strcpy(move+1, g->brd->last_move);
    send_privmsg(b->opponent, move);
    b->game_state = GAME_WAITING_MOVE_ACK;
}


void emit_data_event(int event, intptr_t d1, intptr_t d2, intptr_t d3, intptr_t d4){
    static ALLEGRO_EVENT user_event = {0};
    user_event.type = event;
    user_event.user.data1 = d1;
    user_event.user.data2 = d2;
    user_event.user.data3 = d3;
    user_event.user.data4 = d4;
    al_emit_user_event(&user_event_src, &user_event, NULL);
}
    
void emit_event(int event_type){
    static ALLEGRO_EVENT user_event = {0};
    
    user_event.type = event_type;
    al_emit_user_event(&user_event_src, &user_event, NULL);
}

void init_game(Game *g){
    int i,j;
    
    g->brd = malloc(sizeof(*g->brd));
    g->brd->parent = NULL;
    g->brd->child = NULL;

    for(i=0 ; i<20 ; i++)
        for(j=0 ; j<20 ; j++)
            g->brd->s[i][j] = INITIAL_POSITION[j][i] ? ((INITIAL_POSITION[j][i] == 2) ? 1 : 2) : 0; // swap coordinates since they're in the incorrect order
    g->moves = 0;
    g->history = NULL;
    return;
}

void draw_stone(Board *b, int i, int j, int style, int player){
    if(b->pov == 2)
    {
        i = 19-i;
        j= 19-j;
    }
    
    
    if(style == 0)
        al_draw_filled_circle(b->x + b->tsize*i + (float)b->tsize/2, b->y + b->tsize*j + (float)b->tsize/2, b->pr, b->pcolor[player]);
    else if(style == 2)
        al_draw_filled_circle(b->x + b->tsize*i + (float)b->tsize/2, b->y + b->tsize*j + (float)b->tsize/2, b->pr, color_trans(b->pcolor[player],0.3));
    
    // draw outer circle
    al_draw_circle(b->x + b->tsize*i + (float)b->tsize/2, b->y + b->tsize*j + (float)b->tsize/2, b->pr, color_trans(b->pcolor[3-player], 0.5),1); // al_map_rgba(100, 100, 100, 100),1
}

#define draw_line(x0, y0, x1, y1, color, width) al_draw_line(x0+0.5, y0+0.5, x1+0.5, y1+0.5, color, width)


void draw_board(Board *b){ // todo: fix coordinates so that they're half-integers (at least for bitmap drawing)
    int i;
    char ch;
    int fsize=b->tsize/2;
    int bbx, bby, bbw, bbh;
    ALLEGRO_FONT *font;
    ALLEGRO_COLOR color = al_premul_rgba(0,0,0,80);
    
    // main board color
    al_draw_filled_rectangle(b->x, b->y, b->x + 20*b->tsize, b->y + 20*b->tsize, b->bg_color);
 
    // darken outer ring of cells
    al_draw_filled_rectangle(b->x, b->y, b->x + b->size, b->y + b->tsize, color);
    al_draw_filled_rectangle(b->x, b->y + b->tsize,  b->x + b->tsize, b->y + b->size, color);
    al_draw_filled_rectangle(b->x + b->tsize, b->y + b->size - b->tsize, b->x + b->size, b->y + b->size, color);
    al_draw_filled_rectangle(b->x + b->size - b->tsize, b->y + b->tsize,  b->x + b->size, b->y + b->size - b->tsize, color);
    
    // board lines
    for(i=0; i<=19; i++){
        draw_line(i*b->tsize, 0, i*b->tsize, b->size, DARK_GREY_COLOR, 1);
        draw_line(0, i*b->tsize, b->size, i*b->tsize, DARK_GREY_COLOR, 1);
    }
    draw_line(20*b->tsize-1, 0, i*b->tsize-1, b->size, DARK_GREY_COLOR, 1);
    draw_line(0, 20*b->tsize-1, b->size, 20*b->tsize-1, DARK_GREY_COLOR, 1);
    
    // coordinates
    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -fsize);
    al_hold_bitmap_drawing(true);
    for(i=1; i<19; i++){
        if(b->pov == 1) ch = 'a'+i;
        else ch = 'a'+19-i;
        al_get_glyph_dimensions(font, ch, &bbx, &bby, &bbw, &bbh);
        al_draw_glyph(font, WHITE_COLOR, b->x + i*b->tsize + (b->tsize-bbw)/2, b->y + 19*b->tsize + (b->tsize-fsize)/2, ch);
        al_draw_glyph(font, WHITE_COLOR,  b->x + (b->tsize-bbw)/2,  b->y + (19-i)*b->tsize + (b->tsize-fsize)/2, ch);
        al_draw_glyph(font, WHITE_COLOR, b->x + i*b->tsize + (b->tsize-bbw)/2, b->y + (b->tsize-fsize)/2, ch);
        al_draw_glyph(font, WHITE_COLOR,  b->x +  19*b->tsize + (b->tsize-bbw)/2,  b->y + (19-i)*b->tsize + (b->tsize-fsize)/2, ch);

    }
    al_hold_bitmap_drawing(false);
}

void init_board(Board *b){
    static char nick[10];
    sprintf(nick, "gess%d", rand()%10000);
   
    b->pcolor[0] = NULL_COLOR;
    b->pcolor[1] = al_map_rgb(220, 220, 220);
    b->pcolor[2] = al_map_rgb(60, 60, 60);
    b->bg_color = al_map_rgb(30, 150, 250);
    b->fi = -1;
    b->fj = -1;
    
    b->server = "irc.freenode.org";
    b->nick = nick;
    b->port = 6667;
    b->channel = "#lalala";
}

void create_board(Board *b){
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    int size = min(al_get_bitmap_width(al_get_target_bitmap()), al_get_bitmap_height(al_get_target_bitmap()));
    b->tsize = size/20;
    b->size = b->tsize*20;
    b->x=0;
    b->y=0;
    b->pr = b->tsize * 0.45;
    b->board_bmp = al_create_bitmap(b->size,b->size);
    al_set_target_bitmap(b->board_bmp);
    al_clear_to_color(NULL_COLOR);
    draw_board(b);
    al_set_target_bitmap(target);
}

void destroy_board(Board *b){
    ndestroy_bitmap(b->board_bmp);
}

void paint_tiles(Board *b, int i, int j, int w, int h, ALLEGRO_COLOR color){
    if(b->pov == 2){
        i = 19-i-w+1;
        j = 19-j-h+1;
    }
    if(i<0){
        w+=i; i=0;
    } else if(i>19) {
        w+=19-i; i=19;
    }
    
    if(j<0){
        h+=j; j=0;
    } else if(j>19) {
        h+=19-j; j=19;
    }
    
    if(i+w > 20) w = 20-i;
    if (j+h > 20) h = 20-j;
    
    if(in_board(i,j)){
        al_draw_filled_rectangle(b->x+i*b->tsize, b->y+j*b->tsize, b->x+(i+w)*b->tsize, b->y+(j+h)*b->tsize, color);
    }
}



void tile_rectangle(Board *b, int i, int j, int w, int h, ALLEGRO_COLOR color, int stroke){
    if(b->pov == 2){
        i = 19-i-w+1;
        j = 19-j-h+1;
    }
    if(i<0){
        w+=i; i=0;
    } else if(i>19) {
        w+=19-i; i=19;
    }
    
    if(j<0){
        h+=j; j=0;
    } else if(j>19) {
        h+=19-j; j=19;
    }
    
    if(i+w > 20) w = 20-i;
    if (j+h > 20) h = 20-j;
    
    if(in_board(i,j)){
        al_draw_rectangle(b->x+i*b->tsize + (float)stroke/2 + .5, b->y+j*b->tsize+ (float)stroke/2+.5, b->x+(i+w)*b->tsize - (float)stroke/2+.5, b->y+(j+h)*b->tsize - (float)stroke/2+.5, color, stroke);
    }
}



void draw_arrow(int x1, int y1, int x2, int y2, ALLEGRO_COLOR color, int stroke){
    const float hw = 3.0;
    const float hh = 5.0;
    float hyp = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    
    al_draw_line(x1, y1, x2, y2, color, stroke);
    al_draw_line(x2, y2, x2 - ((x2-x1)*hh + (y2-y1)*hw) * (float)stroke / hyp, y2 - ((y2-y1)*hh - (x2-x1)*hw) * (float)stroke / hyp, color, stroke);
    al_draw_line(x2, y2, x2 - ((x2-x1)*hh - (y2-y1)*hw) * (float) stroke / hyp, y2 - ((y2-y1)*hh + (x2-x1)*hw) * (float) stroke / hyp, color, stroke);
}

#define ARROW_COLOR al_map_rgba(150, 150, 0, 150)

//xxx todo: replace highlighting with rectangles
// add user interface
// add game_type (irc, 1v1 on same device, etc)
// add save/restore
// add resume/adjourn
// add nick collision handling / reconnect / etc
void draw_last_move(Game *g, Board *b)
{    //xxx todo: fix
    int i = coord_to_i(g->brd->last_move[0]);
    int j = coord_to_j(g->brd->last_move[1]);
    int ii = coord_to_i(g->brd->last_move[2]);
    int jj = coord_to_j(g->brd->last_move[3]);

    tile_rectangle(b, i-1, j-1, 3, 3, al_map_rgba(100,100,0,100), 3);
    tile_rectangle(b, ii-1, jj-1, 3, 3, al_map_rgba(0,100,100,100), 3);
    
    if(b->pov == 2){
        i = 19-i;
        j = 19-j;
        ii = 19-ii;
        jj = 19-jj;
    }
    
    draw_arrow(b->x+(i+0.5)*b->tsize, b->y+(j+0.5)*b->tsize, b->x+(ii+0.5)*b->tsize, b->y+(jj+0.5)*b->tsize, ARROW_COLOR, 3);
}

void draw_stuff(Game *g, Board *b){
    int i,j;

    al_clear_to_color(NULL_COLOR);
    al_draw_bitmap(b->board_bmp, b->x, b->y, 0); //xxx todo: fix draw_board to not use b->x, b->y
                   
    // focused block
    if( (b->fi>=0) && (b->fj>=0) )
    {
//        if( !(b->lock && (b->move_mark[b->fi][b->fj] <2)) ){
//            paint_tiles(b, b->fi-1, b->fj-1, 3, 3, FOCUS_COLOR);
//        }
//        else
            paint_tiles(b, b->fi-1, b->fj-1, 3, 3, FOCUS_COLOR);

        paint_tiles(b, b->fi, b->fj, 1, 1, FOCUS_CENTER_COLOR);
    }
    
    for(i=0; i<20; i++)
    {
        for(j=0; j<20; j++)
        {
            if(b->lock && (b->move_mark[i][j] >= 2)) // possible moves
            {
                paint_tiles(b, i, j, 1, 1, MOVE_COLOR[b->move_mark[i][j]]);
            }
    
            if(g->brd->s[i][j] && !(b->lock && (iabs(i-b->fi) <= 1) && (iabs(j-b->fj) <= 1))) // stones
            {
                draw_stone(b, i, j, 0, g->brd->s[i][j]);
            }
        }
    }
    
    if(b->lock){
        tile_rectangle(b, b->lock_i-1, b->lock_j-1, 3, 3, al_map_rgba(100,100,0,100), 3);
        if(b->move_mark[b->fi][b->fj]>1) tile_rectangle(b, b->fi-1, b->fj-1, 3, 3, al_map_rgba(0, 100, 100, 100), 3);
        for(i=0; i<3; i++){
            for(j=0; j<3; j++){
                if(b->lock_blk.b[i][j]){
                    if((b->fi<0)|| b->fj<0){
                        draw_stone(b, b->lock_i + i - 1, b->lock_j + j - 1, 0, b->lock_blk.b[i][j]);
                    } else {
                        draw_stone(b, b->lock_i + i - 1, b->lock_j + j - 1, 1, b->lock_blk.b[i][j]);
                        if((b->fj+j-1 > 0) && (b->fj+j-1 < 19) && (b->fi+i-1 > 0) && (b->fi+i-1<19)){
//                            if(b->move_mark[b->fi][b->fj]>1)
//                                draw_stone(b, b->fi+i-1, b->fj+j-1, 0, b->lock_blk.b[i][j]);
//                            else
//                                draw_stone(b, b->fi+i-1, b->fj+j-1, 2, b->lock_blk.b[i][j]);
draw_stone(b, b->fi+i-1, b->fj+j-1, 0, b->lock_blk.b[i][j]);
                        }
                    }
                }
            }
        }
    }
    
    //xxx todo: fix
    if((g->moves > 0) && b->draw_last) draw_last_move(g,b);
    
}
    
    // xxx todo: draw rectangle at last move source & dest
    // create draw functions for stones and board rectangles
    
void destroy_game(Game *g){
    while(g->brd->parent){
        g->brd = g->brd->parent;
        free(g->brd->child);
    }
    free(g->brd);
}

void execute_undo(Game *g, Board *b){
    if(!g->brd->parent) return;
    g->brd = g->brd->parent;
    free(g->brd->child);
    b->lock = 0;
    g->moves--;
    g->turn = (g->turn == 1) ? 2 : 1;
}


void get_tile(Board *b, int *tx, int *ty, int x, int y){
    *tx = (x-b->x)/b->tsize;
    *ty = (y-b->y)/b->tsize;
    if( (*tx < 0) || (*tx >= 20) || (*ty < 0) || (*ty >= 20) ){
        *tx = -1;
        *ty = -1;
    } else {
        if(b->pov == 2){
            *tx = 19-*tx;
            *ty = 19-*ty;
        }
    }
}

void get_possible_moves(Game *g, Board *b){
    int di, dj;
    int k;
    int i=b->lock_i, j=b->lock_j;

    memset(&b->move_mark, 0, sizeof(b->move_mark));
    
    for(di=-1; di<2; di++){
        for(dj=-1; dj<2; dj++){
            set_brd(b->move_mark, i+di, j+dj, 1);
            k=0;
            if((di || dj) && brd(g->brd, i+di, j+dj)){
                do{
                    k++;
                    if(dj){
                        set_brd(b->move_mark, i+1+k*di, j+dj+k*dj, 1);
                        set_brd(b->move_mark, i-1+k*di, j+dj+k*dj, 1);
                        set_brd(b->move_mark, i+k*di, j+dj+k*dj, 1);
                    }
                    
                    if(di){
                        set_brd(b->move_mark, i+di+k*di, j+1+k*dj, 1);
                        set_brd(b->move_mark, i+di+k*di, j+k*dj, 1);
                        set_brd(b->move_mark, i+di+k*di, j-1+k*dj, 1);
                    }
             
                    if(dj){
                        if( brd(g->brd, i + 1 + k*di, j + dj + k*dj) || brd(g->brd, i - 1 + k*di, j + dj + k*dj) || brd(g->brd, i + k*di, j + dj + k*dj) )
                            break;
                    }
                
                    if(di){
                        if( brd(g->brd, i + di + k*di, j + 1 + k*dj) || brd(g->brd, i + di + k*di, j + k*dj) || brd(g->brd, i + di + k*di, j -1 + k*dj) )
                            break;
                    }
                    
                }while(in_board(i+k*di, j+k*dj) && (g->brd->s[i][j] || (k<3)) );
            }
            b->move.b[di+1][dj+1] = k;
        }
    }
    
    for(di=-1; di<2; di++){
        for(dj=-1; dj<2; dj++){
            for(k=0; k <= b->move.b[di+1][dj+1]; k++){
                set_brd(b->move_mark, i+k*di, j+k*dj, 2);
            }
        }
    }
    
    set_brd(b->move_mark, i, j, 3);
}

int is_block_movable(Game *g, int i, int j){
    int ii, jj;
    
    int lock = 1;
    int count_ring = 0;
    
    for(ii = max(i-1, 0) ; ii < min(i+2, 20) ; ii++){
        for(jj = max(j-1, 0) ; jj < min(j+2, 20) ; jj++){
            if(g->brd->s[ii][jj]){
                if(g->brd->s[ii][jj] != g->turn){
                    lock = 0;
                    break;
                } else if((ii != i) || (jj != j)){
                    count_ring++;
                }
            }
        }
        if(!lock) break;
    }
    
    return (lock && count_ring);
}


void grab_block(Game *g, int i, int j, Block *blk){
    int ii, jj;
    for(ii=0; ii<3; ii++){
        for(jj=0; jj<3; jj++){
            blk->b[ii][jj] = brd(g->brd, i+ii-1, j+jj-1);
            set_brd(g->brd->s, i+ii-1, j+jj-1, 0);
        }
    }
}

int try_lock(Game *g, Board *b, int i, int j){
    
    if(!is_block_movable(g, i, j)) return 0;
    
    b->lock_i = i;
    b->lock_j = j;
    b->lock = 1;
    get_possible_moves(g,b);
    
    grab_block(g, i, j, &b->lock_blk);
    return 1;
}

void drop_block(Board_State *bs, int i, int j, Block *blk){
    int ii,jj;
    
    for(ii = -1 ; ii < 2 ; ii++){
        for(jj = -1; jj < 2 ; jj++){
            if((i+ii < 19) && (i+ii > 0) && (j+jj < 19) && (j+jj > 0))
                set_brd(bs->s, i + ii, j + jj, blk->b[ii+1][jj+1]);
        }
    }
}


int try_move(Game *g, Board *b, int i, int j){
    int ret = 0;
    
    if(b->move_mark[i][j] < 2){
        return 0;
//        i=b->lock_i, j=b->lock_j;
    }


    if( (i != b->lock_i) || (j != b->lock_j) )
    {
        // move was made, switch turns
        if(g->turn == 2) g->turn = 1;
        else g->turn = 2;
        g->moves++;
        g->brd->child = malloc(sizeof(*g->brd));
        memcpy(g->brd->child, g->brd, sizeof(*g->brd));
        g->brd->child->parent = g->brd;
        g->brd = g->brd->child;
        g->brd->child = NULL;
        get_move_coords(g->brd->last_move, b->lock_i, b->lock_j, i, j);
        drop_block(g->brd->parent, b->lock_i, b->lock_j, &b->lock_blk);
        b->draw_last = 1; //xxx todo: if set->draw_last
        ret = 1;
    } // else no move was made
    
    drop_block(g->brd, i, j, &b->lock_blk);
    b->lock = 0;
    return ret;
}

void focus_move(Board *b, int di, int dj){

    if(b->pov == 2) {
        di = -di;
        dj = -dj;
    }
    
    if(in_board(b->fi + di, b->fj + dj)){
        b->fi += di;
        b->fj += dj;
    }
}

int is_ring(Board_State *bs, int i, int j){
    int di, dj, p;
    int p1=1, p2=1;
    
    for(di = -1; di < 2; di++){
        for(dj = -1; dj < 2; dj++){
            if((di == 0) && (dj == 0)) continue;
            p = brd(bs, i+di, j+dj);
            if(p==0) return 0;
            else if(p==2) p1 = 0;
            else if(p==1) p2 = 0;
            if(!p1 && !p2) return 0;
        }
    }
    return p1 ? 1 : 2;
}

int check_win(Board_State *bs){
    int i, j;
    int p1_lose = 1, p2_lose = 1;
    
    for(i=0; i<20; i++){
        for(j=0; j<20; j++){
            switch(is_ring(bs, i, j)){
                case 1:
                    p1_lose=0;
                    break;
                case 2:
                    p2_lose=0;
                    break;
            }
            if( !p1_lose && !p2_lose ) return 0;
        }
    }
    
    return p1_lose + p2_lose;
}

void enter_move(Game *g, Board *b){
    
    if(b->draw_last) b->draw_last=0;
    
    if( (b->fi < 0) || (b->fj < 0) )
        return;

    if(((b->game_state == GAME_PLAYING_IRC) && (g->turn != b->player)) || b->game_state == GAME_WAITING_MOVE_ACK)
        return;
    
    if(b->lock){
        if(try_move(g, b, b->fi, b->fj))
            if(b->game_state == GAME_PLAYING_IRC) send_move(g, b);
    } else {
        try_lock(g, b, b->fi, b->fj);
    }
}

// moves should have the form "abcd" where ab = source oordinates and cd = destination
int str_is_move(char *str){
    int i;
    
    if(strlen(str) != 4) return 0;
    for(i=0;i<4;i++)
        if( (str[i]-'a' < 0) || (str[i] - 'a' > 19) )
            return 0;
    
    return 1;
}


void process_irc_event(Game *g, Board *b, int type, ALLEGRO_USER_EVENT *ev)
{
    char *origin, *msg; // we should free these
    
    origin = (char*) ev->data1;
    msg = (char *) ev->data2;

    deblog("RECEIVED: %s | %s", origin, msg);
    
    if(type == EVENT_PRIVMSG_RECEIVED)
    {
        if(b->game_state == GAME_SEEKING)
        {
            if(!strcasecmp(msg, ":ACK seek"))
            {
                b->opponent = strdup(origin);
                if(rand() % 2){ // decide who starts
                    b->player = 2;
                    b->pov = 2;
                    send_privmsg(b->opponent, ":P2"); // tell: i am player 2
                    b->game_state = GAME_PLAYING_IRC;
                    emit_event(EVENT_RESTART);
                } else {
                    b->player = 1;
                    b->pov = 1;
                    send_privmsg(b->opponent, ":P1"); // tell: i am player 1
                    b->game_state = GAME_PLAYING_IRC;
                    emit_event(EVENT_RESTART);

                }
            } else if (!strcasecmp(msg, ":P1"))
            {
                b->opponent = strdup(origin);
                b->game_state = GAME_PLAYING_IRC;
                b->pov = 2;
                b->player = 2;
                emit_event(EVENT_RESTART);
            }
            else if(!strcasecmp(msg, ":P2"))
            {
                b->opponent = strdup(origin);
                b->game_state = GAME_PLAYING_IRC;
                b->pov = 1;
                b->player = 1;
                emit_event(EVENT_RESTART);
            }
        }
        else if((b->game_state == GAME_PLAYING_IRC) && (g->turn != b->player)) // turn=2 instead?
        {
            if(!strcmp(origin, b->opponent))
            {
                if(msg[0] == ',') // means move coordinates follow
                {
                    if(!str_is_move(msg+1))
                    {
                        send_privmsg(b->opponent, "Invalid move. Send again.");
                    }
                    else
                    {
                    
                        int i = coord_to_i(msg[1]);
                        int j = coord_to_j(msg[2]);
                        int ii = coord_to_i(msg[3]);
                        int jj = coord_to_j(msg[4]);
                        if(is_block_movable(g, i, j) && try_lock(g, b, i, j) && try_move(g, b,  ii, jj))
                        {
                            acknowledge_privmsg(b->opponent, msg);
                        }
                        else
                        {
                            send_privmsg(b->opponent, "Invalid move. Send again.");
                        }
                    }
                } // else if (... other commands like undo, forefeit, adjourn, etc... )
            }
        }
        else if(b->game_state == GAME_WAITING_MOVE_ACK)
        {
            if((strstr(msg, ":ACK ,") == msg) && (!strcmp(msg+6,g->brd->last_move)))
            {
                b->game_state = GAME_PLAYING_IRC;
            }
        }
    }
    else if(type == EVENT_CHANMSG_RECEIVED)
    {
        if(b->game_state == GAME_SEEKING)
        {
            if(!strcasecmp(msg, "seek"))
            {
                send_privmsg(origin, ":ACK seek");
            }
        }
    }
    
    free(origin);
    free(msg);
}

int main(int argc, char **argv){
    ALLEGRO_EVENT ev;
    ALLEGRO_DISPLAY *display = NULL;
    double resize_time, old_time, play_time;
    int noexit, mouse_click,redraw, mouse_move,keypress, resizing, resize_update, mouse_button_down, restart;
    float max_display_factor;
    double mouse_up_time = 0, mouse_down_time = 0;
    int wait_for_double_click = 0, hold_click_check = 0;
    float DELTA_DOUBLE_CLICK = 0.2;
    float DELTA_SHORT_CLICK = 0.1;
    float DELTA_HOLD_CLICK = 0.3;
    int mbdown_x, mbdown_y;
    Board b;
    Game g;
    char move_str[64];
    char opponent[64] = "koro";
    int key_coords;
    int type_coords = 0;
    
    // seed random number generator. comment out for debug
    srand((unsigned int) time(NULL));
   
    if (init_allegro()) return -1;

#ifndef _WIN32
     // use anti-aliasing if available (seems to cause problems in windows)
     al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
     al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
#endif
    
    fullscreen = 0;
    
    // use vsync if available
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

    get_desktop_resolution(0, &desktop_xsize, &desktop_ysize);
    
    if(!MOBILE){
        if (fullscreen) {
            al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
            display = al_create_display(desktop_xsize, desktop_ysize);
        } else {
            al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE | ALLEGRO_OPENGL);
            display = al_create_display(800,600);
        }
    } else {
        al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);

        al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
        display = al_create_display(desktop_xsize, desktop_ysize);
    }
    
    if(!display) {
        fprintf(stderr, "Failed to create display!\n");
        return -1;
    }
    al_set_target_backbuffer(display);
    al_clear_to_color(BLACK_COLOR);

    
    al_set_window_title(display, "Gess");
    al_init_user_event_source(&user_event_src);

    b.pov = 1;
    b.player = 1;

    init_board(&b);

RESTART:
    init_game(&g);
    create_board(&b);
    
    if(!MOBILE && !fullscreen) {
        al_set_target_backbuffer(display);
        al_resize_display(display, b.size, b.size);
        al_set_window_position(display, (desktop_xsize-b.size)/2, (desktop_ysize-b.size)/2);
        al_acknowledge_resize(display);
        al_set_target_backbuffer(display);
    }
    
	al_convert_bitmaps(); // turn bitmaps to memory bitmaps after resize (bug in allegro doesn't autoconvert)

    
    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        return -1;
    }


    al_register_event_source(event_queue, al_get_display_event_source(display));
    if(al_is_keyboard_installed())
        al_register_event_source(event_queue, al_get_keyboard_event_source());
    if(al_is_mouse_installed())
        al_register_event_source(event_queue, al_get_mouse_event_source());
    if(al_is_touch_input_installed())
        al_register_event_source(event_queue, al_get_touch_input_event_source());

    al_register_event_source(event_queue , &user_event_src);

    al_set_target_backbuffer(display);

//  initialize flags
    redraw=1; mouse_click=0;
    noexit=1; mouse_move=0;
    restart=0; keypress=0;
    resizing=0;
    mouse_button_down = 0;
    resize_update=0; resize_time = 0;
    mbdown_x = 0;
    mbdown_y = 0;
    type_coords = 0;
    
    al_set_target_backbuffer(display);
    al_clear_to_color(BLACK_COLOR);
    al_flip_display();
    al_flush_event_queue(event_queue);
    play_time = old_time = al_get_time();

    g.turn = 1;
    b.lock = 0;
    while(noexit)
    {
        double dt = al_current_time() - old_time;
        al_rest(fixed_dt - dt); //rest at least fixed_dt
        dt = al_get_time() - old_time;
        old_time = al_get_time();
       // al_wait_for_event(event_queue, &ev);
        while(al_get_next_event(event_queue, &ev)){ // empty out the event queue
            switch(ev.type){
                case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
                    deblog("RECEIVED HALT");
                    break;
                case EVENT_RESTART:
                    destroy_game(&g);
                    destroy_board(&b);
                    restart=1;
                    goto RESTART;
                    
                case EVENT_EXIT:
                    noexit=0;
                    break;
                    
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    emit_event(EVENT_EXIT);
                    break;
                    
                case EVENT_IRC_JOIN:
                    break;
                    
                case ALLEGRO_EVENT_TOUCH_BEGIN:
                    ev.mouse.x = ev.touch.x;
                    ev.mouse.y = ev.touch.y;
                    ev.mouse.button = 1;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                {
                    enter_move(&g, &b);
                    redraw = 1;
                    break;
                }
                    
                case ALLEGRO_EVENT_MOUSE_AXES:
                    get_tile(&b, &b.fi, &b.fj, ev.mouse.x, ev.mouse.y);
                    redraw=1;
                    break;
                case EVENT_PRIVMSG_RECEIVED:
                case EVENT_CHANMSG_RECEIVED:
                    process_irc_event(&g, &b, ev.type, &ev.user);
                    redraw=1;
                    break;
                    
                case EVENT_IRC_CONNECT:
                    printf("MAIN THREAD: irc conencted.\n");
                    break;
                case ALLEGRO_EVENT_KEY_CHAR:
                    keypress=1;
                    switch(ev.keyboard.keycode){
                        case ALLEGRO_KEY_ESCAPE:
                            noexit=0;
                            break;
//                        case ALLEGRO_KEY_R:
//                            restart=1;
//                            goto RESTART;
//                            break;
                            
                        case ALLEGRO_KEY_BACKSPACE:
                            if(b.game_state == GAME_PLAYING){ // not on irc
                                execute_undo(&g, &b);
                                redraw=1;
                            } // otherwise we could request undo
                            break;
                        case ALLEGRO_KEY_SPACE:
                            //params_gui(&g, &b, event_queue);
                            //win_gui(&g, &b, event_queue);
                            break;
                        case ALLEGRO_KEY_LEFT:
                            focus_move(&b, -1, 0);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_RIGHT:
                            focus_move(&b, 1, 0);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_UP:
                            focus_move(&b, 0, -1);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_DOWN:
                            focus_move(&b, 0, 1);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_ENTER:
                            emit_event(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
                            break;
                            
                        case ALLEGRO_KEY_1:
                            IRC_connect(b.server, b.port, b.nick, b.channel);
                            break;
                        
                        case ALLEGRO_KEY_2:
                            irc_cmd_msg(g_irc_s, b.channel, "seek");
                            b.game_state = GAME_SEEKING;
                            break;
                            
                        default:
                            if((ev.keyboard.unichar>= 'a') && (ev.keyboard.unichar <= 't')){
                                if(type_coords == 0){
                                    type_coords = 1;
                                    b.fi = (b.pov == 1 ? ev.keyboard.unichar - 'a' : (19-(ev.keyboard.unichar - 'a')));
                                    key_coords = b.fi;
                                } else if (type_coords == 1){
                                    type_coords = 0;
                                    if(b.fi == key_coords){
                                        b.fj = (b.pov == 1 ? (19-(ev.keyboard.unichar - 'a')) : ev.keyboard.unichar - 'a');
                                        enter_move(&g, &b);
                                    }
                                }
                                redraw=1;
                            }
                            break;
                    }
                    break;
                case ALLEGRO_EVENT_DISPLAY_RESIZE:
                    if (fullscreen) break;
                    al_acknowledge_resize(display);
                    resizing=1; resize_time=al_get_time();
                    break;
                case EVENT_REDRAW:
                    redraw=1;
                    break;
            }
        }// while(al_get_next_event(event_queue, &ev));
	
        if(resizing){
            if(al_get_time()-resize_time > RESIZE_DELAY){
                resizing =0; resize_update=1;
            }
        }
    
        if(resize_update){
            resize_update=0;
            al_set_target_backbuffer(display);
			al_acknowledge_resize(display);
            create_board(&b);
            al_resize_display(display, b.size + 1, b.size + 1);
            al_set_target_backbuffer(display);

//            update_board(&g, &b);
//			al_convert_bitmaps(); // turn bitmaps to video bitmaps
            redraw=1;
        // android workaround, try removing:
            al_clear_to_color(BLACK_COLOR);
            al_flip_display(); 
        }
        
        if(resizing) // skip redraw and other stuff
            continue;
        
        if(keypress){
            keypress=0;
        }

//        if( old_time - play_time > 1 ){ // runs every second
//            1; // do nothing
//        }
        
        if(redraw) {
            redraw = 0;
            al_set_target_backbuffer(display);
            draw_stuff(&g, &b);
            al_flip_display();
        }

    }// end of game loop
    
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return(0);
}
