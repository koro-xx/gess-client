#include "gui.h"
#include "macros.h"
#include "draw.h"
#include "allegro_stuff.h"

#define GUI_BG_COLOR al_map_rgba_f(.4, .4, .4, 1)
#define GUI_FG_COLOR al_map_rgba_f(1, 1, 1, 1)

void init_theme(Board *b){
    static WZ_DEF_THEME theme;
    /*
     Define custom theme
     wz_def_theme is a global vtable defined by the header
     */
    
    memset(&theme, 0, sizeof(theme));
    memcpy(&theme, &wz_def_theme, sizeof(theme));
    theme.font = b->font;
    theme.color1 = GUI_BG_COLOR;
    theme.color2 = GUI_FG_COLOR;
    b->theme = (WZ_THEME*)&theme;
}

void draw_GUI(WZ_WIDGET *gui){
    wz_draw((WZ_WIDGET*) gui);
}

//// dirty hack to extend wz_widget (copying a wgt created by WidgetZ and freeing its memory by hand)
//void GUI_init(GUI *gui, int id, int x, int y, WZ_THEME *theme){
//    WZ_WIDGET *wgt;
//    wgt=wz_create_widget(0, x, y, id);
//    wz_set_theme(wgt, theme);
//    memcpy(gui, wgt, sizeof(*wgt));
//    free(wgt);
//    ((GUI*)gui)->draw = draw_GUI;
//}

WZ_WIDGET* new_widget(int id, int x, int y, WZ_THEME *theme){
    WZ_WIDGET *wgt;
    wgt=wz_create_widget(0, x, y, id);
    wz_set_theme(wgt, theme);
    return wgt;
}

WZ_WIDGET* create_info_gui(Board *b, Game *g){
    //xxx assume that screen is wider than taller for now
    int gui_w = min(b->xsize - b->x - b->size, b->xsize*MAX_PANEL_PORTION);
    int gui_h = b->size;
    int fsize = b->fsize;
    WZ_WIDGET *wgt, *gui = new_widget(GUI_INFO, b->x+b->size, b->y, b->theme);
    
    wz_create_fill_layout(gui, 0, 0, gui_w, 3*fsize, fsize/2, fsize/3, WZ_ALIGN_LEFT, WZ_ALIGN_TOP, -1);
    
    redraw_turn_buttons(b, gui_w, fsize);
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->s_player2_name,0, -1);
    wgt = (WZ_WIDGET*) wz_create_image_button(gui, 0, 0, gui_w, fsize, b->bmp_turn2, b->bmp_turn2, b->bmp_turn2, b->bmp_turn2, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    wz_create_fill_layout(gui, 0, 3*fsize, gui_w, 5*fsize, fsize/2, fsize/3, WZ_ALIGN_LEFT, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->server, 0, -1);
    wz_create_textbox(gui, 0, 0, al_get_text_width(b->font, "Nick:"), fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_new("Nick:"), 1, -1);
    wz_create_textbox(gui, 0, 0, gui_w-al_get_text_width(b->font, "Nick:")-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->nick, 0, -1);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, b->irc_status_msg, 0, BUTTON_IRC_STATUS);


    wz_create_fill_layout(gui, 0, gui_h-3.5*fsize*3-3*fsize, gui_w, 4*fsize*3, (gui_w-fsize*8)/2.1, fsize, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Action"), 1, BUTTON_ACTION);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Chat"), 1, BUTTON_CHAT);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Settings"), 1, BUTTON_SETTINGS);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Undo"), 1, BUTTON_UNDO);
    
    wz_create_fill_layout(gui, 0, gui_h-3*fsize, gui_w, 3*fsize, fsize/2, fsize/3, WZ_ALIGN_CENTRE, WZ_ALIGN_BOTTOM, -1);
    
    wgt = (WZ_WIDGET*) wz_create_image_button(gui, 0, 0, gui_w, fsize, b->bmp_turn1, b->bmp_turn1, b->bmp_turn1, b->bmp_turn1, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->s_player1_name,0, -1);
    if(g->turn == 2) swap_bitmaps(b->bmp_turn1, b->bmp_turn2);
    return gui;
}

WZ_WIDGET* create_yesno_gui(Board *b, int id, ALLEGRO_USTR *msg){
    int w = b->size/3;
    int h = get_multiline_text_lines(b->font, w, al_cstr(msg))*al_get_font_line_height(b->font);
    int fsize = b->fsize;
    WZ_WIDGET *wgt, *gui = new_widget(id, b->x+(b->size-(w+2*b->tsize))/2, b->y+(b->size - (h+5*b->tsize))/2, b->theme);
    
    wz_create_fill_layout(gui, 0, 0, w+2*b->tsize, h+2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, w, h, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, msg, 1, -1);
    
    wz_create_fill_layout(gui, 0, h+2*b->tsize, w+2*b->tsize, 2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    
    wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}

WZ_WIDGET* create_msg_gui(Board *b, int id, ALLEGRO_USTR *msg){
    int w = b->size/3;
    int h = get_multiline_text_lines(b->font, w, al_cstr(msg))*al_get_font_line_height(b->font);
    int fsize = b->fsize;
    WZ_WIDGET *wgt, *gui = new_widget(id, b->x+(b->size-(w+2*b->tsize))/2, b->y+(b->size - (h+5*b->tsize))/2, b->theme);
    
    wz_create_fill_layout(gui, 0, 0, w+2*b->tsize, h+2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, w, h, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, msg, 1, -1);
    
    wz_create_fill_layout(gui, 0, h+2*b->tsize, w+2*b->tsize, 2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, -1);
    
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("OK"), 1, BUTTON_OK);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}


//typedef struct GUI_Settings {
//    GUI gui;
//    
//    WZ_WIDGET *server;
//    WZ_WIDGET *port;
//    WZ_WIDGET *nick;
//    WZ_WIDGET *color;
//} GUI_Settings;

WZ_WIDGET *create_settings_gui(Board *b){
    int gui_w = b->xsize*0.7;
    int gui_h = b->ysize*0.8;
    int fsize = b->tsize*0.6;
    int lh=3;
    WZ_WIDGET *wgt, *gui = new_widget(GUI_SETTINGS,(b->xsize-gui_w)/2, (b->ysize-gui_h)/2, b->theme);
    
    wz_create_fill_layout(gui, 0, 0, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, fsize*7, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("IRC Server:"),1, -1);
    al_ustr_assign(b->s_server, b->server);
    wz_create_editbox(gui, 0, 0, gui_w/2.5, fsize*1.5, b->s_server, 0, -1);
    
    wz_create_textbox(gui, 0, 0, fsize*1, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new(":"),1, -1);
    
    al_ustr_free(b->s_port);
    b->s_port = al_ustr_newf("%d", b->port);
    wz_create_editbox(gui, 0, 0, fsize*4, fsize*1.5, b->s_port, 0, -1);
    
    
    wz_create_fill_layout(gui, 0, fsize*lh, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, fsize*10, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("IRC Channel:"),1, -1);
    al_ustr_assign(b->s_channel, b->channel);
    wz_create_editbox(gui, 0, 0, gui_w/2.5, fsize*1.5, b->s_channel, 0, -1);
    
    wz_create_fill_layout(gui, 0, fsize*lh*2, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    
    al_ustr_assign(b->s_nick, b->nick);
    wz_create_textbox(gui, 0, 0, fsize*10, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("IRC Nickname:"),1, -1);
    wz_create_editbox(gui, 0, 0, gui_w/2.5, fsize*1.5, b->s_nick, 0, -1);
    
    wz_create_fill_layout(gui, 0, fsize*lh*3, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, fsize*10, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Color request:"),1, -1);
    al_ustr_free(b->s_color);
    b->s_color = al_ustr_new(b->request_player ? ((b->request_player == 1) ? "White" : "Black") : "Any");
    wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, b->s_color, 0, BUTTON_COLOR);
    
    wz_create_fill_layout(gui, 0, fsize*lh*4, gui_w, fsize*lh, fsize*3, fsize, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, -1);
    wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    
    return gui;
}

WZ_WIDGET* create_action_gui(Board *b){
    //xxx assume that screen is wider than taller for now
    int gui_w = min(b->xsize - b->x - b->size, b->xsize*MAX_PANEL_PORTION);
    int gui_h = b->size;
    int fsize = b->fsize;
    WZ_WIDGET *wgt, *gui = new_widget(GUI_ACTION, b->x+b->size, b->y, b->theme);
    
    wz_create_fill_layout(gui, 0, 0, gui_w, gui_h, fsize, fsize, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, b->irc_status_msg, 0, BUTTON_CONNECT);
    wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, al_ustr_new("Seek game"), 1, BUTTON_SEEK);
    wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, al_ustr_new("Flip board"), 1, BUTTON_FLIP);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}



//void wz_terminal_proc(GUI *gui){
//    draw_GUI(gui);
//    term_draw(((GUI_Terminal *)gui)->term, ((WZ_WIDGET*)gui)->x+2, ((WZ_WIDGET*)gui)->y+2, ((WZ_WIDGET*)gui)->theme->get_font(((WZ_WIDGET*)gui)->theme, 1), WHITE_COLOR, BLACK_COLOR);
//}

int wz_terminal_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event){
    if(event->type == WZ_DESTROY){
        if(((WZ_TERMINAL *) wgt)->own){
            term_destroy(((WZ_TERMINAL *)wgt)->term);
        }
    }
    int ret = wz_widget_proc(wgt, event);
    if(event->type == WZ_DRAW){
        term_draw(((WZ_TERMINAL *)wgt)->term, wgt->x, wgt->y, wgt->theme->get_font(wgt->theme, 1), WHITE_COLOR, BLACK_COLOR);
    }
    return ret;
}

WZ_WIDGET *wz_create_terminal(WZ_WIDGET *parent, int x, int y, int w, int h, Terminal *term, int own, int id){
    WZ_WIDGET* wgt = malloc(sizeof(WZ_TERMINAL));
    
    wz_init_widget(wgt, parent, x+parent->x, y+parent->y, w, h, id);
    wgt->proc = wz_terminal_proc;

    ((WZ_TERMINAL *)wgt)->term = term;
    ((WZ_TERMINAL *)wgt)->own = own;
    
    return wgt;
}

WZ_WIDGET* create_term_gui(Board *b, Terminal *term, int id){
    int fh = b->fsize;
    int term_w = term->w*al_get_glyph_advance(b->font, '0', '0');
    int term_h = term->h*fh;
    WZ_WIDGET *wgt, *gui = new_widget(GUI_CHAT, (b->xsize-term_w-4)/2, (b->ysize-term_h - 2*fh-4)/2, b->theme);

    
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, term_w+4, term_h+fh*1.5+4, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    wz_create_terminal(gui, 2, 2, term_w, term_h, term, 0, -1);
    wgt = (WZ_WIDGET*) wz_create_editbox(gui, 2, term_h+2, term_w-5*fh, fh*1.5, al_ustr_new(term->buf), 1, -1);
    wgt->flags |= WZ_STATE_HAS_FOCUS;
    wgt = (WZ_WIDGET *) wz_create_button(gui, term_w-5*fh+2, term_h+2, 5*fh, fh*1.5, al_ustr_new("Close"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    
    return gui;
    
}