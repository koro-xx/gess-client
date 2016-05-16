/*
Todo:

- allow coordinates to be typed for move
- gui for nick choosing / new game / save game
- add request undo for irc
- add store/adjourn/resume
- ADD RING CHECK + RING INFO + RING HIGHLIGHT
 */


#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <string.h>
#include "macros.h"
#include "allegro_stuff.h"
#include "main.h"
#include <libircclient.h>
#include "widgetz.h"
#include "terminal.h"
#include "draw.h"
#include "game.h"

#define FPS 60.0


// for irc
char player_nick[32];
char opponent_nick[32];

//#define MOVE_COLOR al_premul_rgba(255, 255, 0, 100)

// this is mainly for testing, not actually used. use emit_event(EVENT_TYPE) to emit user events.
ALLEGRO_EVENT_SOURCE user_event_src;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;


float RESIZE_DELAY = 0.04;
float fixed_dt = 1.0/FPS;

int desktop_xsize, desktop_ysize;
int fullscreen;

// move elsewhere
char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);   // Allocate memory
    if (d != NULL) strcpy (d,s);         // Copy string if okay
    return d;                            // Return new memory
}

void chat_term_add_line(Terminal *t, char *origin, char *msg){
    char str[512];
    snprintf(str, 512, "<%s> %s", origin, msg);
    term_add_line(t, str);
}

void send_privmsg(Board *b, char *nick, char *msg){
    if(nick) irc_cmd_msg(g_irc_s, nick, msg);
    deblog("SENT: %s | %s", nick, msg);
    chat_term_add_line(b->chat_term, al_cstr(b->nick), msg);
}

void acknowledge_privmsg(Board *b, char *nick, char *msg){
    char str[128];
    snprintf(str, 127, ":ACK %s", msg);
    send_privmsg(b, nick, str);
}

void send_move(Game *g, Board *b){
    char move[32];
    snprintf(move, 32, ":%d,%s", g->moves, g->brd->last_move);
//    strcpy(move+1, g->brd->last_move);
    send_privmsg(b, al_cstr(b->opponent), move);
//    b->game_state = GAME_WAITING_MOVE_ACK;
    b->allow_move = 0;
}

void emit_data_event(int event, intptr_t d1, intptr_t d2, intptr_t d3, intptr_t d4){
    static ALLEGRO_EVENT user_event = {0};
    user_event.type = event;
    user_event.user.data1 = d1;
    user_event.user.data2 = d2;
    user_event.user.data3 = d3;
    user_event.user.data4 = d4;
    al_emit_user_event(&user_event_src, &user_event, NULL);
}
    
void emit_event(int event_type){
    static ALLEGRO_EVENT user_event = {0};
    
    user_event.type = event_type;
    al_emit_user_event(&user_event_src, &user_event, NULL);
}

void try_irc_connect(Board *b){
    IRC_connect(al_cstr(b->server), b->port, al_cstr(b->nick), al_cstr(b->channel));
    al_ustr_assign_cstr(b->irc_status_msg, "Connecting...");
    b->connected = -1; //connecting
    emit_event(EVENT_REDRAW);
}

void init_board(Board *b){

    b->pcolor[0] = NULL_COLOR;
    b->pcolor[1] = al_map_rgb(220, 220, 220);
    b->pcolor[2] = al_map_rgb(60, 60, 60);
    b->bg_color = al_map_rgb(30, 150, 250);
    b->fi = -1;
    b->fj = -1;
    
    b->game_state = GAME_PLAYING;
    
    b->server = al_ustr_new("irc.freenode.org");
    b->nick = al_ustr_newf("gess%d", rand()%10000);
    b->port = 6667;
    b->channel = al_ustr_new("#lalala");
    b->connected = 0;
    b->allow_move = 1;
    b->chat_term = term_create(80, 24);
    b->opponent = al_ustr_new("");
    b->irc_status_msg = al_ustr_new("Disconnected");
    b->request_player = 0;
    
    // xxx todo: fix this shit (don't create them, let the gui create them)
    b->s_server = al_ustr_new("");
    b->s_port = al_ustr_new("");
    b->s_channel = al_ustr_new("");
    b->s_nick = al_ustr_new("");
    b->s_color = al_ustr_new("");
    b->s_player1_name = al_ustr_new("Player 1");
    b->s_player2_name = al_ustr_new("Player 2");
    
    b->gui_confirm_n = 0;
    
    b->bmp_turn1=NULL;
    b->bmp_turn2=NULL;
    
   // b->game_type = ?
}


void create_info_gui(Board *b, Game *g){
    //xxx assume that screen is wider than taller for now
    int gui_w = b->xsize - b->x - b->size;
    int gui_h = b->size;
    int fsize = b->tsize*0.5;
    WZ_WIDGET *gui;
    static WZ_DEF_THEME theme;
    b->font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -fsize);
    
    /*
     Define custom theme
     wz_def_theme is a global vtable defined by the header
     */
    memset(&theme, 0, sizeof(theme));
    memcpy(&theme, &wz_def_theme, sizeof(theme));
    theme.font = b->font;
    theme.color1 = al_map_rgba_f(.5, .5, .5, 1);
    theme.color2 = al_map_rgba_f(1, 1, 1,1);
    
    gui = wz_create_widget(0, b->x + b->size, b->y, GUI_INFO);
    wz_set_theme(gui, (WZ_THEME*)&theme);
    
    wz_create_fill_layout(gui, 0, 0, gui_w, gui_h/3, fsize/2, fsize/3, WZ_ALIGN_LEFT, WZ_ALIGN_TOP, -1);
    
    redraw_turn_buttons(b, gui_w, fsize);

    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->s_player2_name,0, -1);
    wz_create_image_button(gui, 0, 0, gui_w, fsize, b->bmp_turn2, b->bmp_turn2, b->bmp_turn2, b->bmp_turn2, -1);
    
    wz_create_fill_layout(gui, 0, gui_h/3, gui_w, gui_h/3, fsize/2, fsize/3, WZ_ALIGN_LEFT, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->server, 0, -1);
    wz_create_textbox(gui, 0, 0, al_get_text_width(b->font, "Nick:"), fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_new("Nick:"), 1, -1);
    wz_create_textbox(gui, 0, 0, gui_w-al_get_text_width(b->font, "Nick:")-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->nick, 0, -1);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, b->irc_status_msg, 0, BUTTON_IRC_STATUS);
//    wz_create_box(gui, 0, 0, gui_w, fsize, -1); //xxx must hide box
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Action"), 1, BUTTON_ACTION);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Chat"), 1, BUTTON_CHAT);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Settings"), 1, BUTTON_SETTINGS);
    wz_create_button(gui, 0, 0, fsize*8, fsize*1.5, al_ustr_new("Undo"), 1, BUTTON_UNDO);

    wz_create_fill_layout(gui, 0, 2*gui_h/3, gui_w, gui_h/3, fsize/2, fsize/3, WZ_ALIGN_CENTRE, WZ_ALIGN_BOTTOM, -1);
    
    wz_create_image_button(gui, 0, 0, gui_w, fsize, b->bmp_turn1, b->bmp_turn1, b->bmp_turn1, b->bmp_turn1, -1);
    wz_create_textbox(gui, 0, 0, gui_w-fsize, fsize, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, b->s_player1_name,0, -1);
    b->i_gui = gui;
    if(g->turn == 2) swap_bitmaps(b->bmp_turn1, b->bmp_turn2);
}

WZ_WIDGET* create_confirm_gui(Board *b, int EVENT_TYPE, ALLEGRO_USTR *msg){
    int w = b->size/3;
    int h = get_multiline_text_lines(b->font, w, al_cstr(msg))*al_get_font_line_height(b->font);
    int fsize = b->tsize*0.5;

    WZ_WIDGET *gui, *wgt;
    static WZ_DEF_THEME theme;
    memset(&theme, 0, sizeof(theme));
    memcpy(&theme, &wz_def_theme, sizeof(theme));
    theme.font = b->font;
    theme.color1 = al_map_rgba_f(.5, .5, .5, 1);
    theme.color2 = al_map_rgba_f(1, 1, 1,1);
    
    b->gui_confirm_event[b->gui_confirm_n] = EVENT_TYPE;
    b->gui_confirm_n++;
    
    gui = wz_create_widget(0, b->x+(b->size-(w+2*b->tsize))/2, b->y+(b->size - (h+5*b->tsize))/2, GUI_CONFIRM);
    wz_set_theme(gui, (WZ_THEME*)&theme);
    
    wz_create_fill_layout(gui, 0, 0, w+2*b->tsize, h+2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, w, h, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, msg, 1, -1);
    
    wz_create_fill_layout(gui, 0, h+2*b->tsize, w+2*b->tsize, 2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    
    wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*4, fsize*1.5, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}

WZ_WIDGET* create_settings_gui(Board *b){
    //xxx assume that screen is wider than taller for now
    int gui_w = b->xsize*0.7;
    int gui_h = b->ysize*0.8;
    int fsize = b->tsize*0.6;
    int lh=3;
    WZ_WIDGET *gui, *wgt;
    static WZ_DEF_THEME theme;
    
    
    /*
     Define custom theme
     wz_def_theme is a global vtable defined by the header
     */
    memset(&theme, 0, sizeof(theme));
    memcpy(&theme, &wz_def_theme, sizeof(theme));
    theme.font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -fsize);
    theme.color1 = al_map_rgba_f(.5, .5, .5, 1);
    theme.color2 = al_map_rgba_f(1, 1, 1,1);
    gui = wz_create_widget(0, (b->xsize-gui_w)/2, (b->ysize-gui_h)/2, GUI_SETTINGS);
    wz_set_theme(gui, (WZ_THEME*)&theme);
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
    int gui_w = b->xsize - b->size - b->x;
    int gui_h = b->size;
    int fsize = b->tsize*0.6;
    WZ_WIDGET *gui, *wgt;
    static WZ_DEF_THEME theme;
    
    /*
     Define custom theme
     wz_def_theme is a global vtable defined by the header
     */
    memset(&theme, 0, sizeof(theme));
    memcpy(&theme, &wz_def_theme, sizeof(theme));
    theme.font = b->font;
    theme.color1 = al_map_rgba_f(.5, .5, .5, 1);
    theme.color2 = al_map_rgba_f(1, 1, 1,1);
    
    gui = wz_create_widget(0, b->x + b->size, b->y, GUI_ACTION);
    wz_set_theme(gui, (WZ_THEME*)&theme);
    wz_create_fill_layout(gui, 0, 0, gui_w, gui_h, fsize, fsize*3, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, al_ustr_new("Connect"), 1, BUTTON_CONNECT);
    wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, al_ustr_new("Seek game"), 1, BUTTON_SEEK);
    wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, al_ustr_new("Flip board"), 1, BUTTON_FLIP);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, fsize*7, fsize*1.5, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}


WZ_WIDGET* create_term_gui(Board *b, Terminal *term, int id){
    int fh = al_get_font_line_height(b->font);
    int term_w = term->w*al_get_glyph_advance(b->font, '0', '0');
    int term_h = term->h*fh;
    WZ_WIDGET *gui, *wgt;
    static WZ_DEF_THEME theme;
    /*
     Define custom theme
     wz_def_theme is a global vtable defined by the header
     */
    memset(&theme, 0, sizeof(theme));
    memcpy(&theme, &wz_def_theme, sizeof(theme));
    theme.font = b->font;
    theme.color1 = al_map_rgba_f(.3, .3, .3, 1);
    theme.color2 = al_map_rgba_f(1, 1, 1,1);
    gui = wz_create_widget(0, (b->xsize-term_w-4)/2, (b->ysize-term_h - 2*fh-4)/2, id);
    wz_set_theme(gui, (WZ_THEME*)&theme);
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, term_w+4, term_h+fh*1.5+4, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    wgt = (WZ_WIDGET*) wz_create_editbox(gui, 2, term_h+2, term_w-5*fh, fh*1.5, al_ustr_new(term->buf), 1, -1);
    wgt->flags |= WZ_STATE_HAS_FOCUS;
    wgt = (WZ_WIDGET *) wz_create_button(gui, term_w-5*fh+2, term_h+2, 5*fh, fh*1.5, al_ustr_new("Close"), 1, BUTTON_CANCEL);
    
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}
                     
void add_gui(Board *b, ALLEGRO_EVENT_QUEUE *queue, WZ_WIDGET *gui){
    wz_register_sources(gui, queue);
    b->gui[b->gui_n] = gui;
    b->gui_n++;
    wz_update(gui, 0);
}

void remove_gui(Board *b){
    if(!b->gui_n) return;
    b->gui_n--;
    wz_destroy(b->gui[b->gui_n]);
    b->gui[b->gui_n] = NULL;
}

//xxx todo: add struct for settings
void settings_apply(Board *b, Game *g, ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE *queue){
    al_ustr_assign(b->server, b->s_server);
    b->port = atoi(al_cstr(b->s_port));
    al_ustr_assign(b->channel, b->s_channel);
    al_ustr_assign(b->nick, b->s_nick);

    if(al_ustr_has_prefix_cstr(b->s_color, "White"))
    {
        b->request_player = 1;
    } else if (al_ustr_has_prefix_cstr(b->s_color, "Black"))
    {
        b->request_player = 2;
    } else
        b->request_player = 0;
    
    remove_gui(b);
}

void seek_game(Game *g, Board *b)
{
    char foo[10];
    //    destroy_game(g);
    //    init_game(g);
    if(b->connected < 1)
        return;
    sprintf(foo, "seek %1d", b->request_player);
    irc_cmd_msg(g_irc_s, al_cstr(b->channel), foo);
    b->game_state = GAME_SEEKING;
}

void flip_board(Board *b){
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    ALLEGRO_USTR *tmp;
    b->pov = 3-b->pov;
    al_set_target_bitmap(b->board_bmp);
    al_clear_to_color(NULL_COLOR);
    draw_board(b);
    
    tmp=al_ustr_dup(b->s_player1_name);
    al_ustr_assign(b->s_player1_name, b->s_player2_name);
    al_ustr_assign(b->s_player2_name, tmp);
    al_set_target_bitmap(target);
}


void execute_undo(Game *g, Board *b){
    if(b->game_state != GAME_PLAYING) return;
    if(!g->brd->parent) return;
    g->brd = g->brd->parent;
    free(g->brd->child);
    b->lock = 0;
    g->moves--;
    g->turn = (g->turn == 1) ? 2 : 1;
    swap_bitmaps(b->bmp_turn1, b->bmp_turn2);

}

void gui_handler(Board *b, Game *g, ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE *queue){
    WZ_WIDGET* wgt = (WZ_WIDGET *)ev->user.data2;
    if(!wgt->parent) return; // just in case
    
    switch(wgt->parent->id)
    {
        case GUI_SETTINGS:
        {
            if(ev->type == WZ_BUTTON_PRESSED){
                switch(ev->user.data1){
                    case BUTTON_OK:
                        settings_apply(b, g, ev, queue);
                        break;
                    case BUTTON_CANCEL:
                        remove_gui(b);
                        break;
                    case BUTTON_COLOR:
                        if(al_ustr_has_prefix_cstr(b->s_color, "White"))
                            al_ustr_assign_cstr(b->s_color, "Black");
                        else if(al_ustr_has_prefix_cstr(b->s_color, "Black"))
                            al_ustr_assign_cstr(b->s_color, "Any");
                        else if(al_ustr_has_prefix_cstr(b->s_color, "Any"))
                            al_ustr_assign_cstr(b->s_color, "White");
                        break;
                }
            }
            break;
        }
        case GUI_INFO:
        {
            if(ev->type == WZ_BUTTON_PRESSED){
                switch(ev->user.data1){
                    case BUTTON_SETTINGS:
                        add_gui(b, queue, create_settings_gui(b));
                        break;
                    case BUTTON_CHAT:
                        add_gui(b, event_queue, create_term_gui(b, b->chat_term, GUI_CHAT));
                        break;
                    case BUTTON_IRC_STATUS:
                        if(b->connected)
                        {
                            irc_disconnect(g_irc_s);
                            b->connected = 0;
                            al_ustr_assign_cstr(b->irc_status_msg, "Disconnected");
//                            emit_event(EVENT_IRC_DISCONNECT);
                        }
                        else
                        {
                            try_irc_connect(b);
                        }
                        break;
                    case BUTTON_ACTION:
                        add_gui(b, event_queue, create_action_gui(b));
                        break;
                    case BUTTON_UNDO:
                        execute_undo(g, b);
                        break;
                }
            }
            break;
        }
        case GUI_CHAT:
        {
            if(ev->type == WZ_TEXT_CHANGED){
             // xxx todo: check that satus is playing on irc and opponent exists!
                if(b->connected <= 0 || b->game_state != GAME_PLAYING_IRC)
                    break;
                send_privmsg(b, al_cstr(b->opponent), (char *) al_cstr(((WZ_EDITBOX*)wgt)->text));
                wz_set_text(wgt, USTR_NULL);
            } else if(ev->type == WZ_BUTTON_PRESSED){
                if(ev->user.data1 == BUTTON_CANCEL){
                    remove_gui(b);
                }
            }
            break;
        }
        case GUI_ACTION:
        {
            if(ev->type == WZ_BUTTON_PRESSED){
                switch(ev->user.data1){
                    case BUTTON_SEEK:
                        seek_game(g, b);
                        remove_gui(b);
                        break;
                    case BUTTON_FLIP:
                        flip_board(b);
                        remove_gui(b);
                        break;
                    case BUTTON_CONNECT:
                        if(b->connected == 0)
                            try_irc_connect(b);
                        remove_gui(b);
                        break;
                    case BUTTON_CANCEL:
                        remove_gui(b);
                        break;
                }
            }
            break;
        }
        case GUI_CONFIRM:
        {
            if(ev->type == WZ_BUTTON_PRESSED){
                switch(ev->user.data1){
                    case BUTTON_OK:
                        emit_event(b->gui_confirm_event[b->gui_confirm_n-1]);
                    case BUTTON_CANCEL:
                        b->gui_confirm_n--;
                        remove_gui(b);
                        break;
                }
            }
        }
    }
}



void create_board(Board *b, Game *g){
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    int size;
    b->xsize = al_get_bitmap_width(al_get_target_bitmap());
    b->ysize = al_get_bitmap_height(al_get_target_bitmap());
    size = min(b->xsize*(1.0-MIN_PANEL_PORTION), b->ysize);
    b->tsize = size/20;
    b->size = b->tsize*20;
    b->x=0;
    b->y=0;
    b->pr = b->tsize * 0.45;
    b->board_bmp = al_create_bitmap(b->size,b->size);
    al_set_target_bitmap(b->board_bmp);
    al_clear_to_color(NULL_COLOR);
    draw_board(b);
    al_set_target_bitmap(target);
    b->gui_n = 0;
    b->gui[0] = NULL;
    b->board_input = 1;
    create_info_gui(b, g);
}



void destroy_board(Board *b){
    if(b->font){
        al_destroy_font(b->font);
        b->font = NULL;
    }
    
    while(b->gui_n) remove_gui(b);
    ndestroy_bitmap(b->board_bmp);
}


//xxx todo: replace highlighting with rectangles
// add user interface
// add game_type (irc, 1v1 on same device, etc)
// add save/restore
// add resume/adjourn
// add nick collision handling / reconnect / etc
    
    // xxx todo: draw rectangle at last move source & dest
    // create draw functions for stones and board rectangles




void get_tile(Board *b, int *tx, int *ty, int x, int y){
    *tx = (x-b->x)/b->tsize;
    *ty = (y-b->y)/b->tsize;
    if( (*tx < 0) || (*tx >= 20) || (*ty < 0) || (*ty >= 20) ){
        *tx = -1;
        *ty = -1;
    } else {
        if(b->pov == 2){
            *tx = 19-*tx;
            *ty = 19-*ty;
        }
    }
}

// xxx todo: update to account for rings
void get_possible_moves(Game *g, Board *b){
    int di, dj;
    int k;
    int i=b->lock_i, j=b->lock_j;

    memset(&b->move_mark, 0, sizeof(b->move_mark));
    
    for(di=-1; di<2; di++){
        for(dj=-1; dj<2; dj++){
            set_brd(b->move_mark, i+di, j+dj, 1);
            k=0;
            if((di || dj) && brd(g->brd, i+di, j+dj)){
                do{
                    k++;
                    if(dj){
                        set_brd(b->move_mark, i+1+k*di, j+dj+k*dj, 1);
                        set_brd(b->move_mark, i-1+k*di, j+dj+k*dj, 1);
                        set_brd(b->move_mark, i+k*di, j+dj+k*dj, 1);
                    }
                    
                    if(di){
                        set_brd(b->move_mark, i+di+k*di, j+1+k*dj, 1);
                        set_brd(b->move_mark, i+di+k*di, j+k*dj, 1);
                        set_brd(b->move_mark, i+di+k*di, j-1+k*dj, 1);
                    }
             
                    if(dj){
                        if( brd(g->brd, i + 1 + k*di, j + dj + k*dj) || brd(g->brd, i - 1 + k*di, j + dj + k*dj) || brd(g->brd, i + k*di, j + dj + k*dj) )
                            break;
                    }
                
                    if(di){
                        if( brd(g->brd, i + di + k*di, j + 1 + k*dj) || brd(g->brd, i + di + k*di, j + k*dj) || brd(g->brd, i + di + k*di, j -1 + k*dj) )
                            break;
                    }
                    
                }while(in_board(i+k*di, j+k*dj) && (g->brd->s[i][j] || (k<3)) );
            }
            b->move.b[di+1][dj+1] = k;
        }
    }
    
    for(di=-1; di<2; di++){
        for(dj=-1; dj<2; dj++){
            for(k=1; k <= b->move.b[di+1][dj+1]; k++){
                if(test_move(g->brd, i, j, i+k*di, j+k*dj))
                    set_brd(b->move_mark, i+k*di, j+k*dj, 2);
            }
        }
    }
    
    set_brd(b->move_mark, i, j, 3);
}


int try_lock(Game *g, Board *b, int i, int j){
    
    if(!is_block_movable(g, i, j)) return 0;
    
    b->lock_i = i;
    b->lock_j = j;
    b->lock = 1;
    get_possible_moves(g,b);
    
    grab_block(g->brd, i, j, &b->lock_blk);
    return 1;
}


void focus_move(Board *b, int di, int dj){

    if(b->pov == 2) {
        di = -di;
        dj = -dj;
    }
    
    if(in_board(b->fi + di, b->fj + dj)){
        b->fi += di;
        b->fj += dj;
    }
}



int try_move(Game *g, Board *b, int i, int j){
    int ret = 0;
    
    if(b->move_mark[i][j] < 2){
        return 0;
        //        i=b->lock_i, j=b->lock_j;
    }
    
    
    if( (i != b->lock_i) || (j != b->lock_j) )
    {
        g->brd->child = malloc(sizeof(*g->brd));
        memcpy(g->brd->child, g->brd, sizeof(*g->brd));
        g->brd->child->parent = g->brd;
        drop_block(g->brd->child, i, j, &b->lock_blk);
        if(!(has_ring(g->brd->child) & g->turn)){
            nfree(g->brd->child);
            return 0;
        }
        
        // move was made, switch turns
        g->brd = g->brd->child;
        g->turn = (g->turn == 1) ? 2 : 1;
        g->moves++;
        g->brd->child = NULL;
        coords_to_str(g->brd->last_move, b->lock_i, b->lock_j, i, j);
        drop_block(g->brd->parent, b->lock_i, b->lock_j, &b->lock_blk);
        b->draw_last = 1; //xxx todo: if set->draw_last
        ret = 1;
        swap_bitmaps(b->bmp_turn1, b->bmp_turn2);
    } // else no move was made
    
    drop_block(g->brd, i, j, &b->lock_blk);
    b->lock = 0;
    return ret;
}

void enter_move(Game *g, Board *b){
    
    if(b->draw_last) b->draw_last=0;
    
    if( (b->fi < 0) || (b->fj < 0) )
        return;
    
    if(!b->allow_move) return;
    
    //    if(((b->game_state == GAME_PLAYING_IRC) && (g->turn != b->player)) || b->game_state == GAME_WAITING_MOVE_ACK)
    //        return;
    
    if(b->lock){
        if(try_move(g, b, b->fi, b->fj))
            if(b->game_state == GAME_PLAYING_IRC){
                send_move(g, b);
            }
    } else {
        try_lock(g, b, b->fi, b->fj);
    }
}

void unlock_block(Game *g, Board *b){
    drop_block(g->brd, b->lock_i, b->lock_j, &b->lock_blk);
    b->lock = 0;
}

void process_irc_event(Game *g, Board *b, int type, ALLEGRO_USER_EVENT *ev)
{
    char *origin, *msg; // we should free these
    
    origin = (char*) ev->data1;
    msg = (char *) ev->data2;

    deblog("RECEIVED: %s | %s", origin, msg);
    chat_term_add_line(b->chat_term, origin, msg);
    
    if(type == EVENT_PRIVMSG_RECEIVED)
    {
        if(b->game_state == GAME_SEEKING)
        {
            if(!strcasecmp(msg, ":ACK seek"))
            {
                b->game_state = GAME_PLAYING_IRC;
                b->player =  b->request_player ? b->request_player : ((rand()%2)+1);
            }
            else if (!strcasecmp(msg, ":P1"))
            {
                b->game_state = GAME_PLAYING_IRC;
                b->player = 2;
            }
            else if(!strcasecmp(msg, ":P2"))
            {
                b->game_state = GAME_PLAYING_IRC;
                b->player = 1;
            }
            
            if(b->game_state == GAME_PLAYING_IRC)
            {
                al_ustr_assign_cstr(b->opponent, origin);
                if(b->player == 1){
                    al_ustr_assign(b->s_player1_name, b->nick);
                    al_ustr_assign(b->s_player2_name, b->opponent);
                } else {
                    al_ustr_assign(b->s_player1_name, b->opponent);
                    al_ustr_assign(b->s_player2_name, b->nick);
                }
                
                b->pov = b->player;
                b->game_state = GAME_PLAYING_IRC;
                b->allow_move = (b->player == 1) ? 1 : 0;
                send_privmsg(b, al_cstr(b->opponent), (b->player == 1) ? ":P1" : ":P2"); // tell: i am player 2
                emit_event(EVENT_RESTART);
            }
            
// xxx todo: log chat message into chat console.
//            else
//            {
//                chat_msg_add(origin, msg);
//            }
        }
        else if((b->game_state == GAME_PLAYING_IRC) && (g->turn != b->player)) // turn=2 instead?
        {
            if(!strcmp(origin, al_cstr(b->opponent)))
            {
                char op_move_str[5];
                int op_moves;
                if(sscanf(msg, ":%d,%4s", &op_moves, op_move_str) == 2) // move was made
                {
                    if(op_moves!= g->moves + 1){
                        send_privmsg(b, al_cstr(b->opponent), "SYNC problem. This is not the move I'm waiting.");
                    }
                    else if(!str_is_move(op_move_str))
                    {
                        send_privmsg(b, al_cstr(b->opponent), "Invalid move. Send again.");
                    }
                    else
                    {
                        int i, j, ii, jj;
                        str_to_coords(op_move_str, &i, &j, &ii, &jj);
                        
                        if(is_block_movable(g, i, j) && try_lock(g, b, i, j) && try_move(g, b,  ii, jj))
                        {
                            acknowledge_privmsg(b, al_cstr(b->opponent), msg);
                            b->allow_move = 1;
                        }
                        else
                        {
                            unlock_block(g, b);
                            send_privmsg(b, al_cstr(b->opponent), "Invalid move. Send again.");
                        }
                    }
                } // else if (... other commands like undo, forefeit, adjourn, etc... )
            }
        }// xxx todo: remove this:
        else if(b->game_state == GAME_WAITING_MOVE_ACK)
        {
            if((strstr(msg, ":ACK ,") == msg) && (!strcmp(msg+6,g->brd->last_move)))
            {
                b->game_state = GAME_PLAYING_IRC;
            }
        }
    }
    else if(type == EVENT_CHANMSG_RECEIVED)
    {
        if(b->game_state == GAME_SEEKING)
        {
            if(strncmp(msg, ":seek", 5)>0)
            {
                send_privmsg(b, origin, ":ACK seek");
            }
        }
    }
    
        
    free(origin);
    free(msg);
}


int main(int argc, char **argv){
    ALLEGRO_EVENT ev;
    ALLEGRO_DISPLAY *display = NULL;
    double resize_time, old_time, play_time;
    int noexit, mouse_click,redraw, mouse_move,keypress, resizing, resize_update, mouse_button_down, restart;
    float max_display_factor;
    double mouse_up_time = 0, mouse_down_time = 0;
    int wait_for_double_click = 0, hold_click_check = 0;
    float DELTA_DOUBLE_CLICK = 0.2;
    float DELTA_SHORT_CLICK = 0.1;
    float DELTA_HOLD_CLICK = 0.3;
    int mbdown_x, mbdown_y;
    Board b;
    Game g;
    char move_str[64];
    char opponent[64] = "koro";
    int key_coords;
    int type_coords = 0;
    int i;
    
    // seed random number generator. comment out for debug
    srand((unsigned int) time(NULL));
   
    if (init_allegro()) return -1;

#ifndef _WIN32
     // use anti-aliasing if available (seems to cause problems in windows)
     al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
     al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
#endif
    
    fullscreen = 0;
    
    // use vsync if available
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

    get_desktop_resolution(0, &desktop_xsize, &desktop_ysize);
    
    if(!MOBILE){
        if (fullscreen) {
            al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
            display = al_create_display(desktop_xsize, desktop_ysize);
        } else {
            al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE | ALLEGRO_OPENGL);
            display = al_create_display(800,600);
        }
    } else {
        al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);

        al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
        display = al_create_display(desktop_xsize, desktop_ysize);
    }
    
    if(!display) {
        fprintf(stderr, "Failed to create display!\n");
        return -1;
    }
    al_set_target_backbuffer(display);
    al_clear_to_color(BLACK_COLOR);

    
    al_set_window_title(display, "Gess");
    al_init_user_event_source(&user_event_src);

    b.pov = 1;
    b.player = 1;

    init_board(&b);

    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        return -1;
    }
    
    
    al_register_event_source(event_queue, al_get_display_event_source(display));
    if(al_is_keyboard_installed())
        al_register_event_source(event_queue, al_get_keyboard_event_source());
    if(al_is_mouse_installed())
        al_register_event_source(event_queue, al_get_mouse_event_source());
    if(al_is_touch_input_installed())
        al_register_event_source(event_queue, al_get_touch_input_event_source());
    
    al_register_event_source(event_queue , &user_event_src);

RESTART:
    init_game(&g);
    create_board(&b, &g);
    
//    if(!MOBILE && !fullscreen) {
//        al_set_target_backbuffer(display);
//        al_resize_display(display, b.size, b.size);
//        al_set_window_position(display, (desktop_xsize-b.size)/2, (desktop_ysize-b.size)/2);
//        al_acknowledge_resize(display);
//        al_set_target_backbuffer(display);
//    }
//    
//	al_convert_bitmaps(); // turn bitmaps to memory bitmaps after resize (bug in allegro doesn't autoconvert)

    
    /// need to move this before restart!!!

    al_set_target_backbuffer(display);

//  initialize flags
    redraw=1; mouse_click=0;
    noexit=1; mouse_move=0;
    restart=0; keypress=0;
    resizing=0;
    mouse_button_down = 0;
    resize_update=0; resize_time = 0;
    mbdown_x = 0;
    mbdown_y = 0;
    type_coords = 0;
    
    al_set_target_backbuffer(display);
    al_clear_to_color(BLACK_COLOR);
    al_flip_display();
    al_flush_event_queue(event_queue);
    play_time = old_time = al_get_time();
    
    g.turn = 1;
    b.lock = 0;

    // temp
    add_gui(&b, event_queue, b.i_gui);
//    wz_register_sources(b.i_gui, event_queue);
    
    while(noexit)
    {
        double dt = al_current_time() - old_time;
        al_rest(fixed_dt - dt); //rest at least fixed_dt
        dt = al_get_time() - old_time;
        old_time = al_get_time();

        // temp
        for(i=0; i<b.gui_n; i++){
            wz_update(b.gui[i], fixed_dt); // we may not be refreshing changes in the gui!
        }
        
        if(b.gui_n>1) redraw=1;
        
        while(al_get_next_event(event_queue, &ev)){ // empty out the event queue
            if(ev.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING){
                // first thing to process
                deblog("RECEIVED HALT");
                break;
            }
            
            // send event to topmost gui
            if(b.gui_n) // ignore mouse events if focus is in gui
                wz_send_event(b.gui[b.gui_n-1], &ev);

            switch(ev.type){
                case EVENT_RESTART:
                    destroy_game(&g);
                    destroy_board(&b);
                    restart=1;
                    goto RESTART;
                    
                case EVENT_EXIT:
                    noexit=0;
                    break;
                    
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    emit_event(EVENT_EXIT);
                    break;
                    
                case WZ_BUTTON_PRESSED:
                case WZ_TEXT_CHANGED:
                    gui_handler(&b, &g, &ev, event_queue);
                    redraw=1;
                    break;
                    
                case EVENT_IRC_JOIN:
                    break;
                    
                case ALLEGRO_EVENT_TOUCH_BEGIN:
                    if(!b.board_input || b.gui_n > 1) break;
                    ev.mouse.x = ev.touch.x;
                    ev.mouse.y = ev.touch.y;
                    ev.mouse.button = 1;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    if(!b.board_input || b.gui_n > 1) break;
                {
                    enter_move(&g, &b);
                    redraw = 1;
                    break;
                }
                    
                case ALLEGRO_EVENT_MOUSE_AXES:
                    if(!b.board_input || b.gui_n > 1) break;
                    get_tile(&b, &b.fi, &b.fj, ev.mouse.x, ev.mouse.y);
                    redraw=1;
                    break;
                case EVENT_PRIVMSG_RECEIVED:
                case EVENT_CHANMSG_RECEIVED:
                    process_irc_event(&g, &b, ev.type, &ev.user);
                    redraw=1;
                    break;
                    
                case EVENT_IRC_CONNECT:
                    printf("MAIN THREAD: irc conencted.\n");
                    b.connected = 1;
                    al_ustr_assign_cstr(b.irc_status_msg, "Connected");
                    redraw=1;
                    break;
                    
                case ALLEGRO_EVENT_KEY_CHAR:
                    keypress=1;
                    
//                    if(b.gui_n>1)
//                    {
//                        if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
//                        {
//                            remove_gui(&b);
//                            redraw=1;
//                        }
//                    }
//                    else
                    if (b.board_input && b.gui_n <= 1)
                    {
                        switch(ev.keyboard.keycode){
                            case ALLEGRO_KEY_ESCAPE:
                                add_gui(&b, event_queue, create_confirm_gui(&b, EVENT_EXIT, al_ustr_new("Exit application?")));
//                                noexit=0;
                                break;
    //                        case ALLEGRO_KEY_R:
    //                            restart=1;
    //                            goto RESTART;
    //                            break;
                                
                            case ALLEGRO_KEY_BACKSPACE:
                                execute_undo(&g, &b);
                                redraw=1;
                                break;
                            case ALLEGRO_KEY_SPACE:
                                //params_gui(&g, &b, event_queue);
                                //win_gui(&g, &b, event_queue);
                                break;
                            case ALLEGRO_KEY_LEFT:
                                focus_move(&b, -1, 0);
                                redraw=1;
                                break;
                            case ALLEGRO_KEY_RIGHT:
                                focus_move(&b, 1, 0);
                                redraw=1;
                                break;
                            case ALLEGRO_KEY_UP:
                                focus_move(&b, 0, -1);
                                redraw=1;
                                break;
                            case ALLEGRO_KEY_DOWN:
                                focus_move(&b, 0, 1);
                                redraw=1;
                                break;
                            case ALLEGRO_KEY_ENTER:
                                emit_event(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
                                break;
                                
                            case ALLEGRO_KEY_1:
                                try_irc_connect(&b);
                                break;
                            
                            case ALLEGRO_KEY_2:
                                seek_game(&g, &b);
                                break;
                                
                            default:
                                if((ev.keyboard.unichar>= 'a') && (ev.keyboard.unichar <= 't')){
                                    if(type_coords == 0){
                                        type_coords = 1;
                                        b.fi = (b.pov == 1 ? ev.keyboard.unichar - 'a' : (19-(ev.keyboard.unichar - 'a')));
                                        key_coords = b.fi;
                                    } else if (type_coords == 1){
                                        type_coords = 0;
                                        if(b.fi == key_coords){
                                            b.fj = (b.pov == 1 ? (19-(ev.keyboard.unichar - 'a')) : ev.keyboard.unichar - 'a');
                                            enter_move(&g, &b);
                                        }
                                    }
                                    redraw=1;
                                }
                                break;
                        }
                    }
                    break;
                case ALLEGRO_EVENT_DISPLAY_RESIZE:
                    if (fullscreen) break;
                    al_acknowledge_resize(display);
                    resizing=1; resize_time=al_get_time();
                    break;
                case EVENT_REDRAW:
                    redraw=1;
                    break;
            }
        }// while(al_get_next_event(event_queue, &ev));
	
        if(resizing){
            if(al_get_time()-resize_time > RESIZE_DELAY){
                resizing =0; resize_update=1;
            }
        }
    
        if(resize_update){
            resize_update=0;
            al_set_target_backbuffer(display);
			al_acknowledge_resize(display);
            destroy_board(&b); // fix this
            create_board(&b, &g);
            add_gui(&b, event_queue, b.i_gui);
//            wz_register_sources(b.i_gui, event_queue);

//            al_resize_display(display, b.size + 1, b.size + 1);
            al_set_target_backbuffer(display);

//            update_board(&g, &b);
//			al_convert_bitmaps(); // turn bitmaps to video bitmaps
            redraw=1;
        // android workaround, try removing:
//            al_clear_to_color(BLACK_COLOR);
//            al_flip_display();
            continue;
        }
        
        if(resizing) // skip redraw and other stuff
            continue;
        
        if(keypress){
            keypress=0;
        }

//        if( old_time - play_time > 1 ){ // runs every second
//            1; // do nothing
//        }
        
        if(redraw) {
            redraw = 0;
            al_set_target_backbuffer(display);
            draw_stuff(&g, &b);
            al_flip_display();
        }

    }// end of game loop
    
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return(0);
}
