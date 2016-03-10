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
#define FOCUS_NOMOVE_COLOR al_premul_rgba(255, 255, 255, 20)
#define FOCUS_CENTER_COLOR al_premul_rgba(255, 0, 0, 40)
//#define MOVE_COLOR al_premul_rgba(255, 255, 0, 100)

// this is mainly for testing, not actually used. use emit_event(EVENT_TYPE) to emit user events.
#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
ALLEGRO_EVENT_SOURCE user_event_src;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;

ALLEGRO_COLOR MOVE_COLOR[4] = {{0,0,0,0}, {0.2,0.2,0,0.2}, {0.15,0,0,0.15}, {0.3,0,0,0.3}};

float RESIZE_DELAY = 0.04;
float fixed_dt = 1.0/FPS;

int desktop_xsize, desktop_ysize;
int fullscreen;

typedef struct Block {
    int b[3][3];
} Block;

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
    char nick[10]; // irc nickname
} Board;

typedef struct Game {
    int brd[20][20];
    int turn;
} Game;


#define in_board(i,j) (((i>=0) && (i<20) && (j>=0) && (j<20)) ? 1 : 0)
#define set_brd(t, i, j, k) do{ if(in_board(i,j)) t[i][j] = k; }while(0)

ALLEGRO_COLOR color_trans(ALLEGRO_COLOR c, float f){
    return (ALLEGRO_COLOR){c.r*f, c.g*f, c.b*f, c.a*f};
}

int brd(Game *g, int i, int j){
    if ( ((i<0) || (i>=20) || (j<0) || (j>=20)) )
        return 0;
    else
        return g->brd[i][j];
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

void create_board(Game *g, Board *b){
    int size = min(al_get_bitmap_width(al_get_target_bitmap()), al_get_bitmap_height(al_get_target_bitmap()));
    b->tsize = size/20;
    b->size = b->tsize*20;
    b->x=0;
    b->y=0;
    b->pr = b->tsize * 0.45;
    b->pcolor[0] = NULL_COLOR;
    b->pcolor[1] = al_map_rgb(220, 220, 220);
    b->pcolor[2] = al_map_rgb(60, 60, 60);
    b->bg_color = al_map_rgb(30, 150, 250);
    b->fi = -1;
    b->fj = -1;
}

void init_game(Game *g){
    int i,j;
    for(i=0 ; i<20 ; i++)
        for(j=0 ; j<20 ; j++)
            g->brd[i][j] = INITIAL_POSITION[j][i]; // swap coordinates since they're in the incorrect order
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

void draw_board(Board *b){
    int i;
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
    for(i=0; i<=20; i++){
        al_draw_line(b->x+i*b->tsize, b->y, b->x+i*b->tsize, b->y+ b->size, DARK_GREY_COLOR, 1);
        al_draw_line(b->x,b->y+i*b->tsize, b->x+b->size, b->y+i*b->tsize, DARK_GREY_COLOR, 1);
    }
    
    // coordinates
    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -fsize);
    al_hold_bitmap_drawing(true);
    for(i=1; i<19; i++){
        al_get_glyph_dimensions(font, 'A'+i, &bbx, &bby, &bbw, &bbh);
        al_draw_glyph(font, WHITE_COLOR, b->x + i*b->tsize + (b->tsize-bbw)/2, b->y + 19*b->tsize + (b->tsize-fsize)/2, 'a'+i);
        al_draw_glyph(font, WHITE_COLOR,  b->x + (b->tsize-bbw)/2,  b->y + i*b->tsize + (b->tsize-fsize)/2, 'a'+19-i);
        al_draw_glyph(font, WHITE_COLOR, b->x + i*b->tsize + (b->tsize-bbw)/2, b->y + (b->tsize-fsize)/2, 'a'+i);
        al_draw_glyph(font, WHITE_COLOR,  b->x +  19*b->tsize + (b->tsize-bbw)/2,  b->y + i*b->tsize + (b->tsize-fsize)/2, 'a'+19-i);

    }
    al_hold_bitmap_drawing(false);
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



// draw_block
// create type block (3x3)

void draw_stuff(Game *g, Board *b){
    int i,j;

    draw_board(b);
    
    // focused block
    if( (b->fi>=0) && (b->fj>=0) )
    {
        if( !(b->lock && (b->move_mark[b->fi][b->fj] <2)) ){
            paint_tiles(b, b->fi-1, b->fj-1, 3, 3, FOCUS_COLOR);
        }
        else
            paint_tiles(b, b->fi-1, b->fj-1, 3, 3, FOCUS_NOMOVE_COLOR);

        paint_tiles(b, b->fi, b->fj, 1, 1, FOCUS_CENTER_COLOR);
    }
    
    for(i=0; i<20; i++)
    {
        for(j=0; j<20; j++)
        {
            if(b->lock && b->move_mark[i][j]) // possible moves
            {
                    paint_tiles(b, i, j, 1, 1, MOVE_COLOR[b->move_mark[i][j]]);
            }

            if(g->brd[i][j] && !(b->lock && (b->move_mark[b->fi][b->fj]==2) && (iabs(i-b->fi) <= 1) && (iabs(j-b->fj) <= 1))) // stones
            {
                draw_stone(b, i, j, 0, g->brd[i][j]);
            }
        }
    }
    
    if(b->lock){
        for(i=0; i<3; i++){
            for(j=0; j<3; j++){
                if(b->lock_blk.b[i][j]){
                    if((b->fi<0)|| b->fj<0){
                        draw_stone(b, b->lock_i + i - 1, b->lock_j + j - 1, 0, b->lock_blk.b[i][j]);
                    } else {
                        draw_stone(b, b->lock_i + i - 1, b->lock_j + j - 1, 1, b->lock_blk.b[i][j]);
                        if((b->fj+j-1 > 0) && (b->fj+j-1 < 19) && (b->fi+i-1 > 0) && (b->fi+i-1<19)){
                            if(b->move_mark[b->fi][b->fj]>1)
                                draw_stone(b, b->fi+i-1, b->fj+j-1, 0, b->lock_blk.b[i][j]);
                            else
                                draw_stone(b, b->fi+i-1, b->fj+j-1, 2, b->lock_blk.b[i][j]);
                        }
                    }
                }
            }
        }
    }
}
    
    // xxx todo: draw rectangle at last move source & dest
    // create draw functions for stones and board rectangles
    


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
            if((di || dj) && brd(g, i+di, j+dj)){
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
                        if( brd(g, i + 1 + k*di, j + dj + k*dj) || brd(g, i - 1 + k*di, j + dj + k*dj) || brd(g, i + k*di, j + dj + k*dj) )
                            break;
                    }
                
                    if(di){
                        if( brd(g, i + di + k*di, j + 1 + k*dj) || brd(g, i + di + k*di, j + k*dj) || brd(g, i + di + k*di, j -1 + k*dj) )
                            break;
                    }
                    
                }while(in_board(i+k*di, j+k*dj) && (g->brd[i][j] || (k<3)) );
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
            if(g->brd[ii][jj]){
                if(g->brd[ii][jj] != g->turn){
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
            blk->b[ii][jj] = brd(g, i+ii-1, j+jj-1);
            set_brd(g->brd, i+ii-1, j+jj-1, 0);
        }
    }
}

void try_lock(Game *g, Board *b, int i, int j){
    
    if(!is_block_movable(g, i, j)) return;
    
    b->lock_i = i;
    b->lock_j = j;
    b->lock = 1;
    get_possible_moves(g,b);
    
    grab_block(g, i, j, &b->lock_blk);
}

void drop_block(Game *g, int i, int j, Block *blk){
    int ii,jj;
    
    for(ii = -1 ; ii < 2 ; ii++){
        for(jj = -1; jj < 2 ; jj++){
            if((i+ii < 19) && (i+ii > 0) && (j+jj < 19) && (j+jj > 0))
                set_brd(g->brd, i + ii, j + jj, blk->b[ii+1][jj+1]);
        }
    }
}

void try_move(Game *g, Board *b, int i, int j){
    
    if(b->move_mark[i][j] != 2){
        i=b->lock_i, j=b->lock_j;
    }

    // make move
    drop_block(g, i, j, &b->lock_blk);

    // if null move, unlock block
    if( (i != b->lock_i) || (j != b->lock_j) )
    {
        // move was made, switch turns
        if(g->turn == 2) g->turn = 1;
        else g->turn = 2;

    } // otherwise move has no effect

    b->lock = 0;
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

int is_ring(Game *g, int i, int j){
    int di, dj, p;
    int p1=1, p2=1;
    
    for(di = -1; di < 2; di++){
        for(dj = -1; dj < 2; dj++){
            if((di == 0) && (dj == 0)) continue;
            p = brd(g, i+di, j+dj);
            if(p==0) return 0;
            else if(p==2) p1 = 0;
            else if(p==1) p2 = 0;
            if(!p1 && !p2) return 0;
        }
    }
    return p1 ? 1 : 2;
}

int check_win(Game *g){
    int i, j;
    int p1_lose = 1, p2_lose = 1;
    
    for(i=0; i<20; i++){
        for(j=0; j<20; j++){
            switch(is_ring(g, i, j)){
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
    ALLEGRO_THREAD *comm_thread;
    char opponent[64] = "koro";
    
//    irc_connect("gess-test");
//    comm_thread = al_create_thread(irc_thread, (void *) opponent);
//    al_start_thread(comm_thread);
//    
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

    init_game(&g);
    create_board(&g, &b);
    b.pov = 2;
    b.player = 1;
    
    al_set_window_title(display, "Gess");
    al_init_user_event_source(&user_event_src);
    
RESTART:
    
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
                    restart=1;
                    goto RESTART;
                    
                case EVENT_EXIT:
                    noexit=0;
                    break;
                    
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    emit_event(EVENT_EXIT);
                    break;
                case ALLEGRO_EVENT_TOUCH_BEGIN:
                    ev.mouse.x = ev.touch.x;
                    ev.mouse.y = ev.touch.y;
                    ev.mouse.button = 1;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                {
                    int ci, cj;
                    if( (b.fi < 0) || (b.fj < 0) )
                        break;
                    
                    get_tile(&b, &ci, &cj, ev.mouse.x, ev.mouse.y);
                    if(b.lock)
                    {
                        try_move(&g, &b, ci, cj);
                        redraw=1;
                        break;
                    }
                    
                    if( (ci != b.fi) || (cj != b.fj) )
                    {
                        b.fi = ci;
                        b.fj = cj;
                        redraw = 1;
                        break;
                    }
                    try_lock(&g, &b, ci, cj);
                    redraw = 1;
                    break;
                }
                    
                case ALLEGRO_EVENT_MOUSE_AXES:
                    get_tile(&b, &b.fi, &b.fj, ev.mouse.x, ev.mouse.y);
                    redraw=1;
                    break;
                    
                case EVENT_OPPONENT_MOVE:
                    printf("Moving %d,%d - %d,%d\n", (int)ev.user.data1, (int)ev.user.data2, (int)ev.user.data3, (int)ev.user.data4);
                    break;
                case ALLEGRO_EVENT_KEY_CHAR:
                    keypress=1;
                    switch(ev.keyboard.keycode){
                        case ALLEGRO_KEY_ESCAPE:
                            noexit=0;
                            break;
                        case ALLEGRO_KEY_R:
                            restart=1;
                            goto RESTART;
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
            create_board(&g, &b);
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

        if( old_time - play_time > 1 ){ // runs every second
            1; // do nothing
        }
        
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
