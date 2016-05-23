#ifndef __gess__gui__
#define __gess__gui__

#include "data_types.h"

typedef struct WZ_TERMINAL {
    WZ_WIDGET wgt;
    
    Terminal *term;
    int own;
} WZ_TERMINAL;

//WZ_WIDGET* create_action_gui(Board *b);
WZ_WIDGET* create_term_gui(Board *b, Terminal *term, int id);
WZ_WIDGET* create_settings_gui(Board *b);
WZ_WIDGET* create_yesno_gui(Board *b, int id, ALLEGRO_USTR *msg);
WZ_WIDGET* create_msg_gui(Board *b, int id, ALLEGRO_USTR *msg);
WZ_WIDGET* create_info_gui(Board *b, Game *g);
WZ_WIDGET* create_action_gui_1(WZ_WIDGET* parent, Board *b, int x, int y, int w);
WZ_WIDGET* create_action_gui_2(WZ_WIDGET* parent, Board *b, int x, int y, int w);

void init_theme(Board *b);
void apply_settings_gui(Board *b, WZ_WIDGET *gui);

int wz_widget_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
void wz_init_widget(WZ_WIDGET* wgt, WZ_WIDGET* parent, float x, float y, float w, float h, int id);

WZ_WIDGET* init_gui(int x, int y, int w, int h, WZ_THEME *theme);

void resize_wz_widget(WZ_WIDGET *wgt, float factor);

#endif /* defined(__gess__gui__) */
