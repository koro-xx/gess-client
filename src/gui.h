#ifndef __gess__gui__
#define __gess__gui__

#include "data_types.h"

typedef struct GUI {
    WZ_WIDGET wgt;
    void (*draw)(struct GUI *gui);
    struct GUI *next;
} GUI;

GUI* create_action_gui(Board *b);
GUI* create_term_gui(Board *b, Terminal *term, int id);
GUI* create_settings_gui(Board *b);
GUI* create_yesno_gui(Board *b, int id, ALLEGRO_USTR *msg);
GUI* create_msg_gui(Board *b, int id, ALLEGRO_USTR *msg);
GUI* create_info_gui(Board *b, Game *g);

void init_theme(Board *b);


int wz_widget_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);

#endif /* defined(__gess__gui__) */
