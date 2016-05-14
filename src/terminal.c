#include "terminal.h"
#include <stdlib.h>
#include <string.h>
#include "macros.h"

Terminal *term_create(int w, int h){
    Terminal *t = malloc(sizeof *t);
    
    memset(t, 0, sizeof(*t));
    t->pos = 0;
    t->w = w;
    t->h = h;
    t->cursor = 0;
    return t;
}

void term_destroy(Terminal *t){
    free(t);
    t = NULL;
}

/* Add line of text to terminal, max length 512, no carriage return */
int term_add_line(Terminal *t, char *str){
    if(!str) return -1;
    
    if(t->line[t->pos]) nfree(t->line[t->pos]);
    t->line[t->pos] = strndup(str, 511);
    t->pos = (t->pos + 1) % MAX_TERM_LINES;
    return 0;
}


// TODO: need to fix the line width detection (it goes backwards, so for example a length 90 line turns into a length 10 followed by a length 80. Should be the opposite...
// TODO: fixed_font is not fixed width! replace by something else!
void copy_next_line(char *dest, char *str, int width, char **ptr){
    char *optr = *ptr;
    
    while( (*ptr > str) && (optr - *ptr < width) )
    {
        (*ptr)--;
        if(**ptr == '\n')
        {
            strncpy(dest, *ptr + 1, optr-*ptr);
            dest[optr-*ptr] = 0;
            return;
        }
    }
    
    strncpy(dest, *ptr, optr - *ptr);
    dest[optr-*ptr] = 0;
}

void term_input(Terminal *t, int allegro_keycode, int *c){
    switch(allegro_keycode){
        case ALLEGRO_KEY_BACKSPACE:
            if(t->cursor == 0) return;
            t->cursor--;
            t->buf[t->cursor]=0;
            return;
        case ALLEGRO_KEY_ENTER:
            term_add_line(t, t->buf);
            t->cursor = 0;
            t->buf[0] = 0;
            return;
        default:
            if(t->cursor >= TERM_LINE_SIZE) return;
            t->buf[t->cursor] = (char) c;
            t->cursor++;
            t->buf[t->cursor] = 0;
    }
}

//
//void term_append_char(Terminal *t, int *c){
//    if(t->cursor >= TERM_LINE_SIZE) return;
//    t->buf[t->cursor] = (char) c;
//    t->cursor++;
//    t->buf[t->cursor] = 0;
//}
//
//void term_backspace(Terminal *t){
//    if(t->cursor == 0) return;
//    t->cursor--;
//    t->buf[t->cursor]=0;
//}
//
//void term_enter(Terminal *t){
//    term_add_line(t, t->buf);
//    t->cursor = 0;
//    t->buf[0] = 0;
//}

/* prints displayed part of terminal to a string and returns pointer to it */
/* currently assumes terminal is at bottom */

char *term_print_str(Terminal *t){
    char *str = malloc((t->w+1)*(t->h - 1)*sizeof(char));
    memset(str, 0, (t->w+1)*(t->h - 1)*sizeof(char));
    int l = 1;
    int sl= t->h-2;
    int offset;
    char *line;
    int len;
    
    do{
        line = t->line[nmod(t->pos-l, MAX_TERM_LINES)];
        if(!line){
            memset(str, 0, t->w*sl);
            break;
        }
        len=strlen(line);
        offset = t->w * ((len-1)/t->w);
        strncpy(str+t->w*sl, line + offset, t->w);
        if(len % t->w) str[t->w*sl+len-offset] = 0; // add null terminating character if required
        
        sl--;
        offset -= t->w;
        while((offset>=0) && (sl >= 0)){
            strncpy(str+t->w*sl, line + offset, t->w);
            sl--;
            offset -= t->w;
        }
        l++;
    } while(sl>=0);
    str[(t->h-1)*t->w] = 0; // final null character
    return str;
}

void term_draw(Terminal *t, int x, int y, ALLEGRO_FONT *font, ALLEGRO_COLOR color){
    char *str = term_print_str(t);
    char temp[t->w+1];
    int i, th, shift;
    
    th = al_get_font_line_height(font);
    
    for(i=0; i<t->h-1;i++){
        strncpy(temp, str+(t->w*i), t->w);
        temp[t->w] = 0;
        al_draw_text(font, color, x, y + i*th, ALLEGRO_ALIGN_LEFT, temp);
    }
    
    shift = (t->cursor-1)/t->w;
    al_draw_text(font, color, x, y+(t->h-1)*th, ALLEGRO_ALIGN_LEFT, t->buf+shift*t->w);
}
