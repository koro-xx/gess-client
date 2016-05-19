#ifndef __gess__draw__
#define __gess__draw__

#include <stdio.h>
#include "data_types.h"

void draw_stuff(Game *g, Board *b);
void draw_board(Board *b);
void redraw_turn_buttons(Board *b, int w, int h);

#endif /* defined(__gess__draw__) */
