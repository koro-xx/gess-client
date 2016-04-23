#ifndef __gess__terminal__
#define __gess__terminal__

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

// create terminal type
// create_terminal()
// destroy_terminal()
// display_terminal()

#define MAX_TERM_LINES 80
#define TERM_LINE_SIZE 512

typedef struct Terminal{
/* dimensions */
    int w, h;
/* keep 128 lines, max 512 bytes each, cyclic buffer
 * initialize to null */
    char *line[MAX_TERM_LINES];
/* position of last line in buffer */
    int pos;
/* text typing buffer */
    char buf[TERM_LINE_SIZE];
/* cursor position */
    int cursor;
} Terminal;


void term_draw(Terminal *t, int x, int y, ALLEGRO_FONT *font, ALLEGRO_COLOR color);
int term_add_line(Terminal *t, char *str);
void term_destroy(Terminal *t);
Terminal *term_create(int w, int h);


#endif /* defined(__gess__terminal__) */
