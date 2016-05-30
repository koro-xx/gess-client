#include "terminal.h"
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include <allegro5/allegro_primitives.h>

char *my_strndup(const char *s, int len) {
    size_t n = max(len, strlen(s) + 1);
    char *d = malloc(n);   // Allocate memory
    if (d != NULL) {
        memcpy(d, s, n);         // Copy string if okay
        d[n - 1] = 0;
    }
    return d;                            // Return new memory
}

Terminal *term_create(void){
    Terminal *t = malloc(sizeof *t);
    
    memset(t, 0, sizeof(*t));
    t->pos = 0;
    t->cursor = 0;
    t->input = 0;
    return t;
}

void term_destroy(Terminal *t){
    free(t);
    t = NULL;
}

/* Add line of text to terminal, max length 512, no carriage return */
int term_add_line(Terminal *t, const char *str){
    if(!str) return -1;
    
    if(t->line[t->pos]) nfree(t->line[t->pos]);
    t->line[t->pos] = my_strndup(str, 511);
    t->pos = (t->pos + 1) % MAX_TERM_LINES;
    return 0;
}

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


char *term_print_str(Terminal *t, int tw, int th){
    int l = 1;
    int sl;
    int offset;
    char *line;
    int len;
    char *str;
    th = t->input ? th-1 : th;
    sl = th-1;
    
    str = malloc((tw+1)*th*sizeof(char));
    memset(str, 0, (tw+1)*(th - 1)*sizeof(char));
    
    do{
        line = t->line[nmod(t->pos-l, MAX_TERM_LINES)];
        if(!line){
            memset(str, 0, tw*sl);
            break;
        }
        len=strlen(line);
        offset = tw * ((len-1)/tw);
        strncpy(str+tw*sl, line + offset, tw);
        if(len % tw) str[tw*sl+len-offset] = 0; // add null terminating character if required
        
        sl--;
        offset -= tw;
        while((offset>=0) && (sl >= 0)){
            strncpy(str+tw*sl, line + offset, tw);
            sl--;
            offset -= tw;
        }
        l++;
    } while(sl>=0);
    str[th*tw] = 0; // final null character
    return str;
}

void term_draw(Terminal *t, float x, float y, float w, float h, ALLEGRO_FONT *font, ALLEGRO_COLOR fg_color, ALLEGRO_COLOR bg_color){
    int i, shift;
    int ch = al_get_font_line_height(font);
    int cw = al_get_glyph_advance(font, '0', '0');
    int tw = w/cw, th = h/ch;
    char temp[tw+1];
    char *str = term_print_str(t, tw, th);

    al_draw_filled_rectangle(x,y, x + w, y + h, bg_color);
    
    for(i=0; i<th-t->input;i++){
        strncpy(temp, str + tw*i, tw);
        temp[tw] = 0;
        al_draw_text(font, fg_color, x, y + i*ch, ALLEGRO_ALIGN_LEFT, temp);
    }
    
    if(t->input){
        shift = (t->cursor-1)/tw;
        al_draw_text(font, fg_color, x, y+(ch-1)*ch, ALLEGRO_ALIGN_LEFT, t->buf+shift*tw);
    }
}
