#include "gui.h"
#include "macros.h"
#include "draw.h"
#include "allegro_stuff.h"

#define GUI_BG_COLOR al_map_rgba_f(.4, .4, .4, 1)
#define GUI_BG_COLOR_ALT al_map_rgba_f(.7, .7, .7, 1)

#define GUI_FG_COLOR al_map_rgba_f(1, 1, 1, 1)

void init_theme(Board *b){
    static WZ_DEF_THEME theme, theme_alt;
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
    
    memset(&theme_alt, 0, sizeof(theme_alt));
    memcpy(&theme_alt, &wz_def_theme, sizeof(theme_alt));
    theme_alt.font = b->font;
    theme_alt.color1 = GUI_BG_COLOR_ALT;
    theme_alt.color2 = GUI_FG_COLOR;
    b->theme_alt = (WZ_THEME*)&theme_alt;
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
    if(theme) wz_set_theme(wgt, theme);
    return wgt;
}

WZ_WIDGET* init_gui(int x, int y, int w, int h, WZ_THEME *theme){
    WZ_WIDGET *wgt = new_widget(-1, x, y, theme);
    wgt->w = w;
    wgt->h = h;
    return wgt;
}

WZ_WIDGET* create_action_gui_2(WZ_WIDGET* parent, Board *b, int x, int y, int w){
    //xxx assume that screen is wider than taller for now
    int fsize = b->fsize;
    int butn=5;
    
    WZ_WIDGET *wgt, *gui = wz_create_widget(0, x, y, GUI_ACTION_2);
    gui->w = w; gui->h = 2.5*fsize*butn+fsize;
    //new_widget(GUI_ACTION, 0, 0, b->theme);
    
    wz_create_fill_layout(gui, 0, 0, w, gui->h, fsize, fsize, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Main menu"), 1, BUTTON_CANCEL);
//    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);

//    wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, b->irc_status_msg, 0, BUTTON_CONNECT);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Seek game"), 1, BUTTON_SEEK);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Flip board"), 1, BUTTON_FLIP);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Reset board"), 1, BUTTON_RESET);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Quit"), 1, BUTTON_QUIT);
    if(parent) wz_attach(gui, parent);
    return gui;
}

WZ_WIDGET* create_action_gui_1(WZ_WIDGET* parent, Board *b, int x, int y, int w){
    //xxx assume that screen is wider than taller for now
    int fsize = b->fsize;
    int butn=4;
    
    WZ_WIDGET *gui = wz_create_widget(0, x, y, GUI_ACTION_1);
    gui->w = w; gui->h = butn*fsize*2.5+fsize;

    wz_create_fill_layout(gui, 0, 0, w, gui->h, (w-fsize*8)/2.1, fsize, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    //xxx todo: move chat + settings to irc widget. Set it as widget.
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Show more"), 1, BUTTON_ACTION);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Chat"), 1, BUTTON_CHAT);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Settings"), 1, BUTTON_SETTINGS);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Undo"), 1, BUTTON_UNDO);
    if(parent) wz_attach(gui, parent);
    return gui;
}

WZ_WIDGET* create_info_gui(Board *b, Game *g){
    //xxx assume that screen is wider than taller for now
    int gui_w = b->panel_width;
    int gui_h = b->size;
    int fsize = b->fsize;
    WZ_WIDGET *wgt, *gui = new_widget(GUI_INFO, b->x+b->size + PANEL_SPACE*b->size, b->y, NULL);
    
    b->player2_wgt = (WZ_WIDGET*)wz_create_fill_layout(gui, 0, 0, gui_w, 3*fsize, fsize/2, fsize/3, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->player2_name,0, -1);
    
    wz_create_fill_layout(gui, 0, 3*fsize, gui_w, 5*fsize, fsize/2, fsize/3, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->server, 0, -1);
    wz_create_textbox(gui, 0, 0, al_get_text_width(b->font, "Nick:"), fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_new("Nick:"), 1, -1);
    wz_create_textbox(gui, 0, 0, gui_w-al_get_text_width(b->font, "Nick:")-fsize-fsize/2, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->nick, 0, -1);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, b->irc_status_msg, 0, BUTTON_IRC_STATUS);
    wz_create_layout_stop(gui, -1);
    
   create_action_gui_1(gui, b, 0, fsize*8, gui_w);
    
    b->player1_wgt = (WZ_WIDGET*)wz_create_fill_layout(gui, 0, gui_h-3*fsize, gui_w, 3*fsize, fsize/2, fsize/3, WZ_ALIGN_CENTRE, WZ_ALIGN_BOTTOM, -1);
    
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->player1_name,0, -1);
    wz_create_layout_stop(gui, -1);

    return gui;
}

WZ_WIDGET* create_yesno_gui(Board *b, int id, ALLEGRO_USTR *msg){
    int w = b->size/3;
    int h = get_multiline_text_lines(b->font, w, al_cstr(msg))*al_get_font_line_height(b->font);
    int fsize = b->fsize;
    WZ_WIDGET *wgt, *gui = new_widget(id, b->x+(b->size-(w+2*b->tsize))/2, b->y+(b->size - (h+5*b->tsize))/2, NULL);
    
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
    WZ_WIDGET *wgt, *gui = new_widget(id, b->x+(b->size-(w+2*b->tsize))/2, b->y+(b->size - (h+5*b->tsize))/2, NULL);
    
    wz_create_fill_layout(gui, 0, 0, w+2*b->tsize, h+2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, w, h, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, msg, 1, -1);
    
    wz_create_fill_layout(gui, 0, h+2*b->tsize, w+2*b->tsize, 2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, -1);
    
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("OK"), 1, BUTTON_OK);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}


void apply_settings_gui(Board *b, WZ_WIDGET *gui){
    WZ_WIDGET *wgt = gui->first_child;
    
    while(wgt){
        switch(wgt->id){
            case EDITBOX_SERVER:
                al_ustr_assign(b->server, ((WZ_TEXTBOX*)wgt)->text);
                break;
            case EDITBOX_PORT:
                b->port = atoi(al_cstr(((WZ_TEXTBOX*)wgt)->text));
                break;
            case EDITBOX_CHANNEL:
                al_ustr_assign(b->channel, ((WZ_TEXTBOX*)wgt)->text);
                break;
            case EDITBOX_NICK:
                al_ustr_assign(b->nick, ((WZ_TEXTBOX*)wgt)->text);
                break;
            case BUTTON_COLOR:
                if(al_ustr_has_prefix_cstr(((WZ_BUTTON*)wgt)->text, "White"))
                {
                    b->request_player = 1;
                } else if (al_ustr_has_prefix_cstr(((WZ_BUTTON*)wgt)->text, "Black"))
                {
                    b->request_player = 2;
                } else
                    b->request_player = 0;
                break;
        }
        wgt = wgt->next_sib;
    }
}

WZ_WIDGET *create_settings_gui(Board *b){
    int gui_w = b->xsize*0.7;
    int gui_h = b->ysize*0.8;
    int fsize = b->tsize*0.6;
    int lh=3;
    WZ_WIDGET *wgt, *gui = new_widget(GUI_SETTINGS,(b->xsize-gui_w)/2, (b->ysize-gui_h)/2, NULL);
    
    wgt = (WZ_WIDGET*) wz_create_fill_layout(gui, 0, 0, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, fsize*7, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("IRC Server:"),1, -1);
    wz_create_editbox(gui, 0, 0, gui_w/2.5, fsize*1.5, al_ustr_dup(b->server), 1, EDITBOX_SERVER);
    wz_create_textbox(gui, 0, 0, fsize*1, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new(":"),1, -1);
    wz_create_editbox(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_newf("%d",b->port), 1, EDITBOX_PORT);

    wz_create_fill_layout(gui, 0, fsize*lh, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, fsize*10, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("IRC Channel:"),1, -1);
    wz_create_editbox(gui, 0, 0, gui_w/2.5, fsize*1.5, al_ustr_dup(b->channel), 1, EDITBOX_CHANNEL);
    wz_create_fill_layout(gui, 0, fsize*lh*2, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, fsize*10, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("IRC Nickname:"),1, -1);
    wz_create_editbox(gui, 0, 0, gui_w/2.5, fsize*1.5, al_ustr_dup(b->nick), 1, EDITBOX_NICK);

    wz_create_fill_layout(gui, 0, fsize*lh*3, gui_w, fsize*lh, fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, fsize*10, fsize*1.5, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Color request:"),1, -1);
    wz_create_button(gui, 0, 0, fsize*4, fsize*1.5,
                     al_ustr_new(b->request_player ? ((b->request_player == 1) ? "White" : "Black") : "Any")
                     , 1, BUTTON_COLOR);
    
    wz_create_fill_layout(gui, 0, fsize*lh*4, gui_w, fsize*lh, fsize*3, fsize, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, -1);
    wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
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
    WZ_WIDGET *wgt, *gui = new_widget(GUI_CHAT, (b->size*(1+PANEL_PORTION+PANEL_SPACE)-term_w-4)/2, (b->size-term_h - 2*fh-4)/2, NULL);

    
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, term_w+4, term_h+fh*1.5+4, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    wz_create_terminal(gui, 2, 2, term_w, term_h, term, 0, -1);
    wgt = (WZ_WIDGET*) wz_create_editbox(gui, 2, term_h+2, term_w-5*fh, fh*1.5, al_ustr_new(term->buf), 1, -1);
    wgt->flags |= WZ_STATE_HAS_FOCUS;
    wgt = (WZ_WIDGET *) wz_create_button(gui, term_w-5*fh+2, term_h+2, 5*fh, fh*1.5, al_ustr_new("Close"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    
    return gui;
    
}



// xxx todo: needs to be fixed by adding a WZ_RESIZE to proc in every widget
// otherwise fill_layout space isn't properly resized
void resize_wz_widget(WZ_WIDGET *wgt, float factor){
    if(wgt->first_child)
    {
        WZ_WIDGET *child = wgt->first_child;
        do{
            resize_wz_widget(child, factor);
            child = child->next_sib;
        } while(child);
    }
    
    wgt->x *= factor;
    wgt->y *= factor;
    wgt->w *= factor;
    wgt->h *= factor;
    wgt->local_x *= factor;
    wgt->local_y *= factor;
}
