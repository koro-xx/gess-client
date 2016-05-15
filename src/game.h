#ifndef __gess__game__
#define __gess__game__

#include "main.h"


// convert alphbetic coordinates to numbers and vice-versa
char i_to_coord(int i);
char j_to_coord(int j);
int coord_to_i(char ci);
int coord_to_j(char cj);

// get 4-letter string of moves for a given numeric move
void get_move_coords(char *str, int i, int j, int ii, int jj);

// grab block at coordinates i,j
void grab_block(Board_State *bs, int i, int j, Block *blk);

// drop block at coordinates i,j
void drop_block(Board_State *bs, int i, int j, Block *blk);

// is there a ring at i,j? Return ring player number if yes.
int is_ring(Board_State *bs, int i, int j);

// is there a ring in the board?
// first bit of retval is player 1, second bit is player 2
int has_ring(Board_State *bs);

// rules allow i,j block to move?
int is_block_movable(Game *g, int i, int j);

// get board value at coordinates i,j. Allows i,j out of range.
int brd(Board_State *p, int i, int j);

// check string is a valid move
// moves should have the form "abcd" where ab = source oordinates and cd = destination
int str_is_move(char *str);

// test if move breaks own player rings
int test_move(Board_State *bs, int i, int j, int ii, int jj);


void init_game(Game *g);
void destroy_game(Game *g);

#endif /* defined(__gess__game__) */
