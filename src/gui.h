//
//  gui.h
//  gess
//
//  Created by koro on 5/16/16.
//  Copyright (c) 2016 koro. All rights reserved.
//

#ifndef __gess__gui__
#define __gess__gui__

#include "widgetz.h"
#include "main.h"

WZ_WIDGET* create_action_gui(Board *b);
WZ_WIDGET* create_term_gui(Board *b, Terminal *term, int id);
WZ_WIDGET* create_settings_gui(Board *b);
WZ_WIDGET* create_confirm_gui(Board *b, int EVENT_TYPE, ALLEGRO_USTR *msg);
void create_info_gui(Board *b, Game *g);
void init_gui(Board *b);

#endif /* defined(__gess__gui__) */
