#include "draw.h"
#include "allegro_stuff.h"
#include "terminal.h"
#include <math.h>
#include "macros.h"
#include "game.h"

#define FOCUS_COLOR al_premul_rgba(255, 250, 250, 50)
#define LOCK_COLOR al_premul_rgba(255, 250, 250, 90)
#define FOCUS_NOMOVE_COLOR al_premul_rgba(255, 255, 255, 30)
#define FOCUS_CENTER_COLOR al_premul_rgba(255, 255, 255, 60)
#define ARROW_COLOR al_map_rgba(150, 150, 0, 150)
#define TURN_COLOR al_map_rgba(200, 200, 0, 255)

ALLEGRO_COLOR MOVE_COLOR[4] = {{0,0,0,0}, {0.2,0.2,0,0.2}, {0.2,0,0,0.2}, {0.2,0,0,0.2}};


void redraw_turn_buttons(Board *b, int w, int h){
    ALLEGRO_BITMAP *target =al_get_target_bitmap();
    ndestroy_bitmap(b->bmp_turn1);
    ndestroy_bitmap(b->bmp_turn2);
    b->bmp_turn1=al_create_bitmap(w,h);
    al_set_target_bitmap(b->bmp_turn1);
    al_clear_to_color(TURN_COLOR);

    b->bmp_turn2=al_create_bitmap(w,h);
    al_set_target_bitmap(b->bmp_turn2);
    al_clear_to_color(NULL_COLOR);

    al_set_target_bitmap(target);
    
}

void draw_tile_rectangle(Board *b, int i, int j, int w, int h, ALLEGRO_COLOR color, int stroke){
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

void draw_last_move(Game *g, Board *b)
{    //xxx todo: fix
    int i = coord_to_i(g->brd->last_move[0]);
    int j = coord_to_j(g->brd->last_move[1]);
    int ii = coord_to_i(g->brd->last_move[2]);
    int jj = coord_to_j(g->brd->last_move[3]);
    
    draw_tile_rectangle(b, i-1, j-1, 3, 3, al_map_rgba(100,100,0,100), 3);
    draw_tile_rectangle(b, ii-1, jj-1, 3, 3, al_map_rgba(0,100,100,100), 3);
    
    if(b->pov == 2){
        i = 19-i;
        j = 19-j;
        ii = 19-ii;
        jj = 19-jj;
    }
    
    draw_arrow(b->x+(i+0.5)*b->tsize, b->y+(j+0.5)*b->tsize, b->x+(ii+0.5)*b->tsize, b->y+(jj+0.5)*b->tsize, ARROW_COLOR, 3);
}


// main draw routine
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
        draw_tile_rectangle(b, b->lock_i-1, b->lock_j-1, 3, 3, al_map_rgba(100,100,0,100), 3);
        if(b->move_mark[b->fi][b->fj]>1) draw_tile_rectangle(b, b->fi-1, b->fj-1, 3, 3, al_map_rgba(0, 100, 100, 100), 3);
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
    
    // gui
    for(i=0; i<b->gui_n; i++)
    {
        wz_draw(b->gui[i]);
        if(b->gui[i]->id == GUI_CHAT){
            term_draw(b->chat_term, b->gui[i]->x+2, b->gui[i]->y+2, b->font, WHITE_COLOR, BLACK_COLOR);
        }
    }
    
    //    if(b->term_show){
    //        al_draw_filled_rectangle(0,0, b->chat_term->w*al_get_glyph_advance(fixed_font, '0', '0'), b->chat_term->h*al_get_font_line_height(fixed_font), BLACK_COLOR);
    //        term_draw(b->chat_term, 0, 0, fixed_font, WHITE_COLOR);
    //    }
}
