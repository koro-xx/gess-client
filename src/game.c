#include "game.h"
#include "macros.h"

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

void str_to_coords(char *str, int *i, int *j, int *ii, int *jj){
    *i = str[0]-'a';
    *j = 't'-str[1];
    *ii = str[2]-'a';
    *jj = 't'-str[3];
}

void coords_to_str(char *str, int i, int j, int ii, int jj){
    str[0] = i_to_coord(i);
    str[1] = j_to_coord(j);
    str[2] = i_to_coord(ii);
    str[3] = j_to_coord(jj);
    str[4] = '\0';
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

void grab_block(Board_State *bs, int i, int j, Block *blk){
    int ii, jj;
    for(ii=0; ii<3; ii++){
        for(jj=0; jj<3; jj++){
            blk->b[ii][jj] = brd(bs, i+ii-1, j+jj-1);
            set_brd(bs->s, i+ii-1, j+jj-1, 0);
        }
    }
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


int is_ring(Board_State *bs, int i, int j){
    int di, dj, p;
    int p1=1, p2=1;
    
    if(bs->s[i][j]) return 0;
    
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

int has_ring(Board_State *bs){
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
            if( !p1_lose && !p2_lose )
                return 1 + 2*1;
        }
    }
    
    return !p1_lose | ((!p2_lose)<<1);
}

// cehck this!

int test_move(Board_State *bs, int i, int j, int ii, int jj){
    Block dest, src;
    int ret = 0;
    int player = brd(bs, i + sign(ii-i), j + sign(jj-j));
    if(!player) return 0;
                  
    grab_block(bs, i, j, &src);
    grab_block(bs, ii, jj, &dest);
    drop_block(bs, ii, jj, &src);
    if(has_ring(bs) & player)
        ret = 1;
    
    drop_block(bs, ii, jj, &dest);
    drop_block(bs, i, j, &src);
    return ret;
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

void destroy_game(Game *g){
    while(g->brd->parent){
        g->brd = g->brd->parent;
        free(g->brd->child);
    }
    free(g->brd);
}