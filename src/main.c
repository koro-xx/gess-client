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

#define FOCUS_COLOR al_premul_rgba(255, 250, 250, 50)
#define LOCK_COLOR al_premul_rgba(255, 250, 250, 90)
//#define MOVE_COLOR al_premul_rgba(255, 255, 0, 100)

// this is mainly for testing, not actually used. use emit_event(EVENT_TYPE) to emit user events.
#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
ALLEGRO_EVENT_SOURCE user_event_src;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;

ALLEGRO_COLOR MOVE_COLOR[4] = {{0,0,0,0}, {0.4,0.4,0,0.3}, {0.3,0.3,0,0.3}, {0.3,0,0,0.3}};

float RESIZE_DELAY = 0.04;
float fixed_dt = 1.0/FPS;

int desktop_xsize, desktop_ysize;
int fullscreen;


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
    int lock_blk[3][3];
// distance that can be moved in direction (i-1, j-1)
    int move[3][3];
    int move_mark[20][20];
} Board;

typedef struct Game {
    int brd[20][20];
    int turn;
} Game;


#define in_board(i,j) (((i>=0) && (i<20) && (j>=0) && (j<20)) ? 1 : 0)
#define set_brd(t, i, j, k) do{ if(in_board(i,j)) t[i][j] = k; }while(0)

int brd(Game *g, int i, int j){
    if ( ((i<0) || (i>=20) || (j<0) || (j>=20)) )
        return 0;
    else
        return g->brd[i][j];
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
            g->brd[i][j] = INITIAL_POSITION[i][j];
    return;
}

void draw_board(Board *b){
    int i;
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
    
}

void paint_tile(Board *b, int i, int j, ALLEGRO_COLOR color){
    if(in_board(i,j)){
        al_draw_filled_rectangle(b->x+j*b->tsize, b->y+i*b->tsize, b->x+(j+1)*b->tsize, b->y+(i+1)*b->tsize, color);
    }
}

ALLEGRO_COLOR color_trans(ALLEGRO_COLOR c, float f){
    return (ALLEGRO_COLOR){c.r*f, c.g*f, c.b*f, c.a*f};
}

void draw_stuff(Game *g, Board *b){
    int i,j;

    draw_board(b);
    
    // focused block
    if( (b->fi>=0) && (b->fj>=0) )
    {
        if( !(b->lock && (b->move_mark[b->fi][b->fj] <2)) )
           al_draw_filled_rectangle(b->x+(b->fj-1)*b->tsize, b->y+(b->fi-1)*b->tsize, b->x+(b->fj+2)*b->tsize, b->y+(b->fi+2)*b->tsize, FOCUS_COLOR);
    }
    
    for(i=0; i<20; i++)
    {
        for(j=0; j<20; j++)
        {
            if(b->lock && b->move_mark[i][j]) // possible moves
            {
                    paint_tile(b, i, j, MOVE_COLOR[b->move_mark[i][j]]);
            }

            if(g->brd[i][j] && !(b->lock && (b->move_mark[b->fi][b->fj]==2) && (iabs(i-b->fi) <= 1) && (iabs(j-b->fj) <= 1))) // stones
            {
                al_draw_filled_circle(b->x + b->tsize*j + (float)b->tsize/2, b->y + b->tsize*i + (float)b->tsize/2, b->pr, b->pcolor[g->brd[i][j]]);
                al_draw_circle(b->x + b->tsize*j + (float)b->tsize/2, b->y + b->tsize*i + (float)b->tsize/2, b->pr, al_map_rgba(100, 100, 100, 100), 1);
            }
        }
    }
    
    if(b->lock){
        for(i=0; i<3; i++){
            for(j=0; j<3; j++){
                if(b->lock_blk[i][j]){
                    if(b->move_mark[b->fi][b->fj]<= 1)
                        al_draw_filled_circle(b->x + b->tsize*(b->lock_j+j-1) + (float)b->tsize/2, b->y + b->tsize*(b->lock_i+i-1) + (float)b->tsize/2, b->pr, b->pcolor[b->lock_blk[i][j]]);
                    al_draw_circle(b->x + b->tsize*(b->lock_j+j-1) + (float)b->tsize/2, b->y + b->tsize*(b->lock_i+i-1) + (float)b->tsize/2, b->pr, al_map_rgba(100, 100, 100, 100), 1);
                }
            }
        }

        if(b->move_mark[b->fi][b->fj]>1){
            for(i=0; i<3; i++){
                for(j=0; j<3; j++){
                    if(b->lock_blk[i][j]){
                        if((b->fj+j-1 > 0) && (b->fj+j-1 < 19) && (b->fi+i-1 > 0) && (b->fi+i-1<19)){
                            al_draw_filled_circle(b->x + b->tsize*(b->fj+j-1) + (float)b->tsize/2, b->y + b->tsize*(b->fi+i-1) + (float)b->tsize/2, b->pr, b->pcolor[b->lock_blk[i][j]]);
                            al_draw_circle(b->x + b->tsize*(b->fj+j-1) + (float)b->tsize/2, b->y + b->tsize*(b->fi+i-1) + (float)b->tsize/2, b->pr, al_map_rgba(100, 100, 100, 100), 1);
                        }
                    }
                }
            }
        }
    }
    
    // xxx todo: draw rectangle at last move source & dest
    // create draw functions for stones and board rectangles
    
}

void get_tile(Board *b, int *tx, int *ty, int x, int y){
    *ty = (x-b->x)/b->tsize;
    *tx = (y-b->y)/b->tsize;
    if( (*tx < 0) || (*tx >= 20) || (*ty < 0) || (*ty >= 20) ){
        *tx = -1;
        *ty = -1;
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
            b->move[di+1][dj+1] = k;
        }
    }
    
    for(di=-1; di<2; di++){
        for(dj=-1; dj<2; dj++){
            for(k=0; k <= b->move[di+1][dj+1]; k++){
                set_brd(b->move_mark, i+k*di, j+k*dj, 2);
            }
        }
    }
    
    set_brd(b->move_mark, i, j, 3);
}

void try_lock(Game *g, Board *b, int i, int j){
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
    
    if(!lock || !count_ring) return;
    
    b->lock_i = i;
    b->lock_j = j;
    b->lock = 1;
    get_possible_moves(g,b);
    
    // copy locked block and clean
    for(ii=0; ii<3; ii++){
        for(jj=0; jj<3; jj++){
            b->lock_blk[ii][jj] = brd(g, b->lock_i+ii-1, b->lock_j+jj-1);
            set_brd(g->brd, b->lock_i+ii-1, b->lock_j+jj-1, 0);
        }
    }
    return;
}


void try_move(Game *g, Board *b, int i, int j){
    int ii, jj;
    
    
    if(b->move_mark[i][j] != 2){
        i=b->lock_i, j=b->lock_j;
    }

    // make move
    for(ii = -1 ; ii < 2 ; ii++){
        for(jj = -1; jj < 2 ; jj++){
            if((i+ii < 19) && (i+ii > 0) && (j+jj < 19) && (j+jj > 0))
                set_brd(g->brd, i + ii, j + jj, b->lock_blk[ii+1][jj+1]);
        }
    }

    // if null move, unlock block
    if( (i != b->lock_i) || (j != b->lock_j) )
    {
        // move was made, switch turns
        if(g->turn == 2) g->turn = 1;
        else g->turn = 2;

    } // otherwise move has no effect

    b->lock = 0;
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
