/*
A Gess game client
Read more about the game here: http://www.archim.org.uk/eureka/53/gess.html
Licensed under the GNU GPLv3
 
Todo:

- add some feedback when typing coordinates
- add request undo for irc
- add store/adjourn/resume
- add RING HIGHLIGHT
 */

// add game_type (irc, 1v1 on same device, etc)
// add restart option
// add save/restore
// add nick collision handling / reconnect / etc


#ifdef _WIN32 // should define these things globally
    #define _CRT_SECURE_NO_WARNINGS
// for libirccclient
    #define WIN32
    #define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
//#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
//#include <allegro5/allegro_color.h>
#include <string.h>
#include "macros.h"
#include "allegro_stuff.h"
#include "main.h"
#include <libircclient.h>
#include "terminal.h"
#include "draw.h"
#include "game.h"
#include "gui.h"

#define FPS 60.0


ALLEGRO_EVENT_SOURCE user_event_src;

float RESIZE_DELAY = 1.0/FPS;
float fixed_dt = 1.0/FPS;

int desktop_xsize, desktop_ysize;
int fullscreen;

void update_irc_nick(char *new_nick)
{
    irc_cmd_nick(g_irc_s, new_nick);
}

void chat_term_add_line(Terminal *t, const char *origin, const char *msg){
    char str[512];
    snprintf(str, 512, "<%s> %s", origin, msg);
    term_add_line(t, str);
}

void send_privmsg(Board *b, const char *nick, const char *msg){
    if(nick) irc_cmd_msg(g_irc_s, nick, msg);
    deblog("SENT: %s | %s", nick, msg);
    chat_term_add_line(b->chat_term, al_cstr(b->nick), msg);
}

void send_chanmsg(Board *b, const char *channel, const char *msg){
    char str[512];
    
    if(channel) irc_cmd_msg(g_irc_s, channel, msg);
    deblog("SENT: %s | %s", channel, msg);
    snprintf(str, 511, "<%s|%s> %s", channel, al_cstr(b->nick), msg);
    term_add_line(b->chat_term, str);
}

void acknowledge_privmsg(Board *b, const char *nick, const char *msg){
    char str[128];
    snprintf(str, 127, ":ACK %s", msg);
    send_privmsg(b, nick, str);
}

void send_move(Game *g, Board *b){
    char move[32];
    snprintf(move, 32, ":%d,%s", g->moves, g->brd->last_move);
//    strcpy(move+1, g->brd->last_move);
    send_privmsg(b, al_cstr(b->opponent), move);
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

void irc_disconnect_request(Board *b){
    irc_disconnect(g_irc_s);
    b->connected = 0;
    b->game_type = MODE_SAME_DEVICE;
    b->game_state = GAME_PLAYING;
    al_ustr_assign_cstr(b->opponent, "Player 2");
    al_ustr_assign_cstr(b->irc_status_msg, "Connect");
}

void hilight_chat_button(Board *b, int state)
{
    if(state){
        b->chat_waiting = 1;
        wz_set_theme(b->chat_button, b->theme_alt);
    } else
    {
        b->chat_waiting = 0;
        wz_set_theme(b->chat_button, b->theme);
    }
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
    b->channel = al_ustr_new("#gess");
    b->connected = 0;
    b->allow_move = 1;
    b->chat_term = term_create();
    b->opponent = NULL;
    b->irc_status_msg = al_ustr_new("Connect");
    b->request_player = 0;
    b->new_opponent = NULL;
    
    b->player1_name = al_ustr_new("Player 1");
    b->player2_name = al_ustr_new("Player 2");
    b->gui = NULL;
    b->chat_button = NULL;
    b->chat_waiting = 0;
    b->game_type = MODE_SAME_DEVICE;
    b->focus_board=1;
    b->request_undo=0;
}

void add_gui(WZ_WIDGET *base, ALLEGRO_EVENT_QUEUE *queue, WZ_WIDGET *gui, int stack){
    wz_register_sources(gui, queue);
    if(base){
        if(stack && base->last_child) wz_enable(base->last_child, 0);
        wz_attach(gui, base);
        wz_update(base, 0);
    } else
        wz_update(gui, 0);
}

void remove_gui(WZ_WIDGET* wgt, int stack){
    if(stack && wgt->prev_sib) wz_enable(wgt->prev_sib,1);
    wz_destroy(wgt);
}

void swap_turn(Game *g, Board *b)
{
    g->turn = (g->turn == 1) ? 2 : 1;
    SWAP(b->player1_wgt->theme, b->player2_wgt->theme);
    if(b->game_state == GAME_PLAYING_IRC && g->turn != b->player)
        b->allow_move = 0;
    else
        b->allow_move = 1;
}


void seek_game(Game *g, Board *b, ALLEGRO_EVENT_QUEUE *queue)
{
    char foo[10];
    //    destroy_game(g);
    //    init_game(g);
    if(b->connected < 1){
        add_gui(b->gui, queue, create_msg_gui(b, GUI_MESSAGE, al_ustr_new("Cannot seek game while disconnected.")), 1);
        return;
    }
    sprintf(foo, "seek %1d", b->request_player);
    irc_cmd_msg(g_irc_s, al_cstr(b->channel), foo);
    b->game_state = GAME_SEEKING;
    if(b->new_opponent) al_ustr_free(b->new_opponent);
    b->new_opponent = NULL;
}

void flip_board(Board *b){
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    ALLEGRO_USTR *tmp;
    b->pov = 3-b->pov;
    al_set_target_bitmap(b->board_bmp);
    al_clear_to_color(NULL_COLOR);
    draw_board(b);
    
    SWAP(b->player1_wgt->theme, b->player2_wgt->theme);
 
    tmp=al_ustr_dup(b->player1_name);
    al_ustr_assign(b->player1_name, b->player2_name);
    al_ustr_assign(b->player2_name, tmp);
    al_ustr_free(tmp);
    
    al_set_target_bitmap(target);
}

void set_pov(Board *b, int pov){
    if(pov == b->pov)
        return;
    flip_board(b);
}

void execute_undo(Game *g, Board *b){
    if((b->game_state != GAME_PLAYING) && (b->game_state != GAME_PLAYING_IRC)) return;
    if(!g->brd->parent) return;
    g->brd = g->brd->parent;
    free(g->brd->child);
    b->lock = 0;
    g->moves--;
    swap_turn(g, b);
}

void request_undo(Game *g, Board *b){
    char msg[20];
    
    if(b->game_state != GAME_PLAYING_IRC)
    {
        execute_undo(g,b);
        return;
    }
    
    if(b->request_undo == g->moves || b->request_undo == g->moves - 1) return;
    
    if(g->moves == 0 || (g->moves == 1 && b->player == 2)) return;
    if(g->turn == b->player)
        b->request_undo = g->moves-1;
    else
        b->request_undo = g->moves;
    
    sprintf(msg, ":UNDO %d", b->request_undo);
    send_privmsg(b, al_cstr(b->opponent), msg);
}

void reject_match(Board *b){
    al_ustr_free(b->opponent);
    b->opponent = NULL;
    send_privmsg(b, al_cstr(b->new_opponent), ":REJECT");
}

void request_match(Board *b){
    //xxx todo: check for existing opponent
    char msg[10];
    sprintf(msg, ":seek %1d", b->request_player);
    send_privmsg(b, al_cstr(b->new_opponent), msg);
    
    b->game_state = GAME_SEEKING;
    b->new_player = b->request_player;
}

void ack_seek(Board *b){
    send_privmsg(b, al_cstr(b->new_opponent), ":ACK seek");
}

void gui_handler(Board *b, Game *g, ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE *queue){
    WZ_WIDGET* wgt = (WZ_WIDGET *)ev->user.data2;
    if(!wgt->parent) return; // just in case
    
    if(ev->type == WZ_BUTTON_PRESSED)
    {
        switch(wgt->id)
        {
            case BUTTON_SEEK:
                seek_game(g, b, queue);
                break;

            case BUTTON_MATCH:
                if(b->connected <= 0)
                    add_gui(b->gui, queue, create_msg_gui(b, GUI_MESSAGE, al_ustr_new("Must be connected.")), 1);
                else
                    add_gui(b->gui, queue, create_match_gui(b), 1);
                break;

            case BUTTON_FLIP:
                flip_board(b);
                break;

            case BUTTON_RESET:
                if(b->game_state == GAME_PLAYING_IRC)
                    add_gui(b->gui, queue, create_msg_gui(b, GUI_MESSAGE, al_ustr_new("Cannot do that while playing online")),1);
                else
                    emit_event(EVENT_RESTART);
                break;

            case BUTTON_QUIT:
                add_gui(b->gui, queue, create_yesno_gui(b, GUI_CONFIRM_EXIT, al_ustr_new("Exit application?")),1);
                break;
                
            case BUTTON_IRC_STATUS:
            case BUTTON_CONNECT:
                if(b->connected)
                    add_gui(b->gui, queue, create_yesno_gui(b, GUI_CONFIRM_DISCONNECT, al_ustr_new("Disconnect from IRC server?")), 1);
                else
                    try_irc_connect(b);
                break;
            
            case BUTTON_CHAT:
                add_gui(b->gui, queue, create_term_gui(b, b->chat_term, GUI_CHAT),1);
                break;
                
            case BUTTON_UNDO:
                request_undo(g, b);
                break;
                
            case BUTTON_SETTINGS:
                add_gui(b->gui, queue, create_settings_gui(b), 1);
                break;
        }
    }
    
    switch(wgt->parent->id)
    {
        case GUI_SETTINGS:
        {
            if(ev->type == WZ_BUTTON_PRESSED){
                switch(ev->user.data1){
                    case BUTTON_OK:
                        apply_settings_gui(b, wgt->parent);
                        remove_gui(wgt->parent,1);
                        break;
                    case BUTTON_CANCEL:
                        remove_gui(wgt->parent,1);
                        break;
                    case BUTTON_COLOR:
                    {
                        ALLEGRO_USTR *text = ((WZ_BUTTON*)wgt)->text;
                        if(al_ustr_has_prefix_cstr(text, "White"))
                            al_ustr_assign_cstr(text, "Black");
                        else if(al_ustr_has_prefix_cstr(text, "Black"))
                            al_ustr_assign_cstr(text, "Any");
                        else if(al_ustr_has_prefix_cstr(text, "Any"))
                            al_ustr_assign_cstr(text, "White");
                        break;
                    }
                }
            }
            break;
        }
        case GUI_CHAT:
        {
            if(ev->type == WZ_TEXT_CHANGED){
             // xxx todo: check that satus is playing on irc and opponent exists!
                if(b->connected <= 0)
                    break;
                if(b->game_state == GAME_PLAYING_IRC)
                    send_privmsg(b, al_cstr(b->opponent), (char *) al_cstr(((WZ_EDITBOX*)wgt)->text));
                else
                    send_chanmsg(b, al_cstr(b->channel), (char *) al_cstr(((WZ_EDITBOX*)wgt)->text));
                wz_set_text(wgt, USTR_NULL);
            } else if(ev->type == WZ_BUTTON_PRESSED){
                if(ev->user.data1 == BUTTON_CANCEL){
                    remove_gui(wgt->parent, 1);
                    hilight_chat_button(b, 0);
                }
            }
            break;
        }
        case GUI_MATCH:
        {
            if(ev->type == WZ_BUTTON_PRESSED)
            {
                if(ev->user.data1 == BUTTON_CANCEL)
                {
                    remove_gui(wgt->parent, 1);
                }
                else if(ev->user.data1 == BUTTON_OK)
                {
                    if(apply_match_gui(b, wgt->parent))
                    {
                        request_match(b);
                        add_gui(b->gui, queue, create_msg_gui(b, GUI_MESSAGE, al_ustr_newf("Request sent to %s.", al_cstr(b->new_opponent))),1);
                    }
                    remove_gui(wgt->parent, 1);
                }
            }
            break;
        }
        case GUI_MATCH_INCOMING:
        {
            if(ev->type == WZ_BUTTON_PRESSED)
            {
                if(ev->user.data1 == BUTTON_OK)
                    ack_seek(b);
                else if(ev->user.data1 == BUTTON_CANCEL)
                    reject_match(b);
                remove_gui(wgt->parent, 1);
            }
            break;
        }

        case GUI_UNDO_INCOMING:
        {
            if(ev->type == WZ_BUTTON_PRESSED)
            {
                if(ev->user.data1 == BUTTON_OK)
                {
                    if(!b->request_undo) break;
                    send_privmsg(b, al_cstr(b->opponent), ":ACK UNDO");
                    while(g->moves >= b->request_undo)
                        execute_undo(g,b);
                }
                else
                {
                    send_privmsg(b, al_cstr(b->opponent), ":REJECT UNDO");
                }
                b->request_undo = 0;
                remove_gui(wgt->parent, 1);
            }
            break;
        }
        case GUI_CONFIRM_EXIT:
        {
            if(ev->type == WZ_BUTTON_PRESSED)
            {
                if(ev->user.data1 == BUTTON_OK){
                        emit_event(EVENT_EXIT);
                }
                remove_gui(wgt->parent,1);
            }
            break;
        }
        case GUI_CONFIRM_DISCONNECT:
        {
            if(ev->type == WZ_BUTTON_PRESSED)
            {
                if(ev->user.data1 == BUTTON_OK)
                    irc_disconnect_request(b);
                remove_gui(wgt->parent,1);
            }
            break;
        }
        case GUI_MESSAGE:
        {
            if(ev->type == WZ_BUTTON_PRESSED)
                remove_gui(wgt->parent,1);
        }
//        case GUI_ACTION_1:
//        {
//            if(ev->type == WZ_BUTTON_PRESSED && ev->user.data1 == BUTTON_ACTION)
//            {
//                add_gui(wgt->parent->parent, queue, create_action_gui_2(NULL, b, wgt->parent->x, wgt->parent->y, wgt->parent->w), 0);
//                remove_gui(wgt->parent, 0);
//            }
//            break;
//        }
//        case GUI_ACTION_2:
//        {
//            if(ev->type == WZ_BUTTON_PRESSED && ev->user.data1 == BUTTON_CANCEL)
//            {
//                add_gui(wgt->parent->parent, queue, create_action_gui_1(NULL, b, wgt->parent->x, wgt->parent->y, wgt->parent->w),0);
//                remove_gui(wgt->parent, 0);
//            }
//            break;
//        }
    }
}


void game_over(Game *g, Board *b)
{
    b->allow_move = 0;
    b->game_state = GAME_OVER;
    emit_event(EVENT_GAME_OVER);
}

void create_base_gui(Board *b, Game *g, ALLEGRO_EVENT_QUEUE *queue){
    if(!b->gui){
        b->gui = init_gui(b->x, b->y, b->size*(1.0+PANEL_PORTION + PANEL_SPACE), b->size, b->theme);
        add_gui(NULL, queue, b->gui, 0);
        b->i_gui = create_info_gui(b, g);
        add_gui(b->gui, queue, b->i_gui, 1);
    }
    wz_set_theme(b->player1_wgt, b->theme_alt);
}

void destroy_base_gui(Board *b){
    wz_destroy(b->gui);
    b->gui = NULL;
}

void create_board(Board *b, Game *g){
    ALLEGRO_BITMAP *target = al_get_target_bitmap();
    int size;
    b->xsize = al_get_bitmap_width(al_get_target_bitmap());
    b->ysize = al_get_bitmap_height(al_get_target_bitmap());
    size = min(b->xsize/(1.0+PANEL_PORTION+PANEL_SPACE), b->ysize);
    b->tsize = size/20;
    b->size = b->tsize*20;
    b->panel_width = PANEL_PORTION*b->size;
    b->x=0;
    b->y=0;
    b->pr = b->tsize * 0.45;
    b->board_bmp = al_create_bitmap(b->size,b->size);
    al_set_target_bitmap(b->board_bmp);
    al_clear_to_color(NULL_COLOR);
    draw_board(b);
    al_set_target_bitmap(target);
    b->board_input = 1;
    b->fsize = b->tsize*0.5;
    b->font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -b->fsize);
    init_theme(b);
}

void destroy_board(Board *b){
    if(b->font){
        al_destroy_font(b->font);
        b->font = NULL;
    }
    
    //remove_gui(b->gui);
    //b->gui=NULL;
    ndestroy_bitmap(b->board_bmp);
}

void resize_all(Board *b, Game *g){
    int osize = b->size;
    float factor;
    
    destroy_board(b);
    create_board(b, g);
    factor = b->size/((float) osize);
    wz_resize(b->gui, factor); //resize_wz_widget(b->gui, factor);
}



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

void unlock_block(Game *g, Board *b){
    drop_block(g->brd, b->lock_i, b->lock_j, &b->lock_blk);
    b->lock = 0;
}

int try_move(Game *g, Board *b, int i, int j){
    int ret = 0;
    
    if(b->move_mark[i][j] < 2){
        unlock_block(g,b);
        return 0;
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
        g->moves++;
        g->brd->child = NULL;
        coords_to_str(g->brd->last_move, b->lock_i, b->lock_j, i, j);
        drop_block(g->brd->parent, b->lock_i, b->lock_j, &b->lock_blk);
        b->draw_last = 1; //xxx todo: if set->draw_last
        ret = 1;
        swap_turn(g,b);
        // check for win condition
        if ( !(has_ring(g->brd) & g->turn ) )
        {
            swap_turn(g,b);
            game_over(g,b);
        }
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
    
 
    if(b->lock){
        if(try_move(g, b, b->fi, b->fj))
            if(b->game_state == GAME_PLAYING_IRC){
                send_move(g, b);
                b->request_undo=0;
            }
    } else {
        try_lock(g, b, b->fi, b->fj);
    }
}

void halt(ALLEGRO_EVENT_QUEUE *queue){
    ALLEGRO_DISPLAY *disp =al_get_current_display();
    al_acknowledge_drawing_halt(disp);
    deblog("ACKNOWLEDGED HALT");
    ALLEGRO_EVENT ev;
 //   al_set_default_voice(NULL); // otherwise it keeps streaming when the app is on background
    do{
        al_wait_for_event(queue, &ev);
    }while(ev.type != ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING);
//    al_restore_default_mixer();
    al_acknowledge_drawing_resume(disp);
    deblog("ACKNOWLEDGED RESUME");
    al_rest(0.01);
    al_flush_event_queue(queue);
}


// create a match struct. pass it as event on restart
//typedef struct Match {
//    ALLEGRO_USTR *opponent;
//    int player;
//    int type;
//} Match;

void update_displayed_names(Board *b)
{
    if(!b->opponent)
    {
        al_ustr_assign_cstr(b->player1_name, "Player 1");
        al_ustr_assign_cstr(b->player2_name, "Player 2");
    }
    else
    {
        if(b->player == 1)
        {
            al_ustr_assign(b->player1_name, b->nick);
            al_ustr_assign(b->player2_name, b->opponent);
        }
        else
        {
            al_ustr_assign(b->player1_name, b->opponent);
            al_ustr_assign(b->player2_name, b->nick);
        }
    }
}

void process_irc_event(Game *g, Board *b, int type, ALLEGRO_USER_EVENT *ev, ALLEGRO_EVENT_QUEUE *queue)
{
    char *origin, *msg; // we should free these
    char chat_print[512];
    int undo;
    
    origin = (char*) ev->data1;
    msg = (char *) ev->data2;

    deblog("RECEIVED: %s | %s", origin, msg);
    if(type == EVENT_PRIVMSG_RECEIVED)
        snprintf(chat_print, 511, "<%s> %s", origin, msg);
    else if (type == EVENT_CHANMSG_RECEIVED)
        snprintf(chat_print, 511, "<%s|%s> %s", al_cstr(b->channel), origin, msg);
    else
        snprintf(chat_print, 511, "%s | %s", origin, msg);
    
    term_add_line(b->chat_term, chat_print);
    
    switch(type)
    {
        case EVENT_IRC_NICK:
        {
            if(!strcmp(origin, al_cstr(b->nick)))
            {
                // player nick has changed
                al_ustr_assign_cstr(b->nick, msg);
                update_displayed_names(b);
            }
            else
            {
                if(b->opponent && !strcmp(al_cstr(b->opponent), origin))
                {
                    // opponent nick has changed
                    al_ustr_assign_cstr(b->opponent, msg);
                    update_displayed_names(b);
                }
            }
            break;
        }
        case EVENT_IRC_QUIT:
            if( (b->game_state == GAME_PLAYING_IRC) && b->opponent && !strcmp(origin, al_cstr(b->opponent)) )
            {
                add_gui(b->gui, queue, create_msg_gui(b, GUI_MESSAGE, al_ustr_new("Your opponent has disconnected.")), 1);
                b->allow_move = 0;
                b->game_state = GAME_OVER;
                break;
            }
        case EVENT_PRIVMSG_RECEIVED:
        {
            if(msg[0] != ':')
            {
                hilight_chat_button(b,1);
                break;
            }
            if(!strncmp(msg, ":seek", 5))
            {
                if((b->game_state == GAME_PLAYING_IRC) || b->new_opponent)
                {
                    send_privmsg(b, origin, ":BUSY");
                    break;
                }
            
                if(sscanf(msg, ":seek %d", &b->new_player) == 0)
                    b->new_player = 0;
                
                if((b->new_player <= 0) || (b->new_player > 2))
                    b->new_player = 0;
                else
                    b->new_player = 3-b->new_player;
                
                b->new_opponent = al_ustr_new(origin);
                
                if(b->game_state == GAME_SEEKING) {
                    if(!b->player || b->player == b->new_player)
                        ack_seek(b);
                } else {
                    add_gui(b->gui, queue, create_yesno_gui(b, GUI_MATCH_INCOMING, al_ustr_newf("Incoming match request from <%s>. Accept?", origin)), 1);
                }
            }
            else if(!strcasecmp(msg, ":ACK seek"))
            {
                if((b->game_state == GAME_PLAYING_IRC) || (b->new_opponent && strcmp(al_cstr(b->new_opponent), origin)) )
                {
                    send_privmsg(b, origin, ":BUSY");
                    break;
                }

                if(!b->new_opponent) b->new_opponent = al_ustr_new(origin);
                b->new_player =  b->request_player ? b->request_player : ((rand()%2)+1);
                b->new_game_type = MODE_ONLINE;
                emit_event(EVENT_RESTART);
            }
            else if(b->new_opponent && !strcmp(al_cstr(b->new_opponent), origin))
            { //xxx todo: just in case need to check incoming match accepted here
                if (!strcasecmp(msg, ":P1"))                {
                    b->new_player = 2;
                    b->new_game_type = MODE_ONLINE;
                    emit_event(EVENT_RESTART);
                }
                else if(!strcasecmp(msg, ":P2"))
                {
                    b->new_player = 1;
                    b->new_game_type = MODE_ONLINE;
                    emit_event(EVENT_RESTART);
                }
                
                // xxx todo: log chat message into chat console.
                //            else
                //            {
                //                chat_msg_add(origin, msg);
                //            }
            }
            else if((b->game_state == GAME_PLAYING_IRC) && (sscanf(msg, ":UNDO %d", &undo) == 1))
            {
                if( (g->moves != undo && g->moves != undo + 1) || b->request_undo )
                {
                    send_privmsg(b, al_cstr(b->opponent), ":REJECT UNDO");
                    break;
                }
                b->request_undo = undo;
                add_gui(b->gui, queue, create_yesno_gui(b, GUI_UNDO_INCOMING, al_ustr_newf("Request from %s to undo last move. Accept?", origin)), 1);
                        break;
            }
            else if((b->game_state == GAME_PLAYING_IRC) && (!strcmp(msg, ":ACK UNDO")) && b->request_undo)
            {
                while(g->moves >= b->request_undo)
                    execute_undo(g,b);
                b->request_undo=0;
                break;
            }
            else if((b->game_state == GAME_PLAYING_IRC) && (!strcmp(msg, ":REJECT UNDO")))
            {
                b->request_undo=0;
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
            break;
        }
        case EVENT_CHANMSG_RECEIVED:
        {
            if(b->game_state == GAME_SEEKING)
            {
                if(!strncmp(msg, "seek", 4))
                {
                    if(b->new_opponent) al_ustr_free(b->new_opponent);
                    b->new_opponent = al_ustr_new(origin);
                    send_privmsg(b, origin, ":ACK seek");
                }
            }
            break;
        }
    }
    
    free(origin);
    free(msg);
}

void init_irc_game(Board *b, Game *g){
    if(b->opponent) al_ustr_free(b->opponent);
    b->opponent = b->new_opponent;
    b->new_opponent = NULL;
    b->player = b->new_player;

    //xxx todo: fix player switching on flip board (flip widgets instead?);
    if(b->player == 1){
        al_ustr_assign(b->player1_name, b->nick);
        al_ustr_assign(b->player2_name, b->opponent);
    } else {
        al_ustr_assign(b->player1_name, b->opponent);
        al_ustr_assign(b->player2_name, b->nick);
    }

    set_pov(b, b->player);
    b->game_state = GAME_PLAYING_IRC;
    b->allow_move = (b->player == 1) ? 1 : 0;
    send_privmsg(b, al_cstr(b->opponent), (b->player == 1) ? ":P1" : ":P2"); // tell: i am player 2
}

int main(int argc, char **argv){
    ALLEGRO_EVENT ev;
    ALLEGRO_DISPLAY *display = NULL;
    double resize_time, old_time, play_time;
    int noexit, mouse_click,redraw, mouse_move,keypress, resizing, resize_update, mouse_button_down, restart;
// for touch input:
//    double mouse_up_time = 0, mouse_down_time = 0;
//    int wait_for_double_click = 0, hold_click_check = 0;
//    float DELTA_DOUBLE_CLICK = 0.2;
//    float DELTA_SHORT_CLICK = 0.1;
//    float DELTA_HOLD_CLICK = 0.3;
    int mbdown_x, mbdown_y;
    Board b;
    Game g;
    int key_coords = 0;
    int type_coords = 0;
    int i;
    int resized = 0;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;

    
    // seed random number generator. comment out for debug
    srand((unsigned int) time(NULL));
   
    if (init_allegro()) return -1;

//#ifndef _WIN32
     // use anti-aliasing if available (seems to cause problems in windows)
     al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
     al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
//#endif
    
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

    resized = 0;
    restart = 0;
    
RESTART:
    
    if(restart){
        destroy_board(&b);
        destroy_base_gui(&b);
        destroy_game(&g);
    } else {
        restart = 1;
    }
    
    init_game(&g);
    create_board(&b, &g);
    create_base_gui(&b, &g, event_queue);

    if((b.new_game_type == MODE_ONLINE) && b.new_opponent)
        init_irc_game(&b, &g);
   
    al_set_target_backbuffer(display);

//  initialize flags
    redraw=1; mouse_click=0;
    noexit=1; mouse_move=0;
    keypress=0;
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
    
    if(b.game_state == GAME_PLAYING_IRC)
    {
        add_gui(b.gui, event_queue, create_msg_gui(&b, GUI_MESSAGE, al_ustr_newf("Network game started:\n %s vs %s", al_cstr(b.nick), al_cstr(b.opponent))),1);
    }
    
    while(noexit)
    {
        double dt = al_current_time() - old_time;
        al_rest(fixed_dt - dt); //rest at least fixed_dt
        dt = al_get_time() - old_time;
        old_time = al_get_time();
        
        wz_update(b.gui, fixed_dt);

        if(!b.focus_board) redraw=1; // temporary. don't want constant refresh during game (?)
        
        while(al_get_next_event(event_queue, &ev)){ // empty out the event queue
            if(ev.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING){
                // first thing to process
                deblog("RECEIVED HALT");
                halt(event_queue);
                break;
            }
            
            // send event only to topmost gui
            // only if focus is not on board
            if((b.gui->last_child != b.gui->first_child) || !b.focus_board)
                wz_send_event(b.gui->last_child, &ev);
            
            switch(ev.type){
                case EVENT_RESTART:
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
                    if(!b.board_input || !b.focus_board) break;
                    ev.mouse.x = ev.touch.x;
                    ev.mouse.y = ev.touch.y;
                    ev.mouse.button = 1;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    if(!b.board_input || !b.focus_board) break;
                    enter_move(&g, &b);
                    redraw = 1;
                    break;
                    
                case ALLEGRO_EVENT_MOUSE_AXES:
                    if(!b.board_input) break;
                    get_tile(&b, &b.fi, &b.fj, ev.mouse.x, ev.mouse.y);
                    if(b.fi<0 || (b.gui && (b.gui->first_child != b.gui->last_child))){
                        b.focus_board = 0;
                    } else {
                        b.focus_board = 1;
                        wz_focus(b.gui, 0);
                    }
                    redraw=1;
                    break;
                    
                case EVENT_GAME_OVER:
                    add_gui(b.gui, event_queue, create_msg_gui(&b, GUI_MESSAGE, al_ustr_newf("Player %d wins.", g.turn)), 1);
                    break;
                    
                case EVENT_PRIVMSG_RECEIVED:
                case EVENT_CHANMSG_RECEIVED:
                case EVENT_IRC_QUIT:
                case EVENT_IRC_NICK:
                    process_irc_event(&g, &b, ev.type, &ev.user, event_queue);
                    redraw=1;
                    break;
                    
                case EVENT_IRC_CONNECT:
                    printf("MAIN THREAD: irc conencted.\n");
                    b.connected = 1;
                    al_ustr_assign_cstr(b.irc_status_msg, "Disconnect");
                    redraw=1;
                    break;
                    
                case ALLEGRO_EVENT_KEY_CHAR:
                    keypress=1;
                    if(b.gui->first_child && (b.gui->first_child == b.gui->last_child))
                    {
                        if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                        {
                           add_gui(b.gui, event_queue, create_yesno_gui(&b, GUI_CONFIRM_EXIT, al_ustr_new("Exit application?")),1);
                            redraw=1;
                        }
                        // xxx todo: handle keyboard focus
                        else if(ev.keyboard.keycode == ALLEGRO_KEY_TAB)
                        {
                            if(b.focus_board)
                            {
                                b.focus_board=0;
                                wz_focus(b.gui, 1);
                                b.fi=-1;
                                b.fj=-1;
                            }
                            else{
                                wz_focus(b.gui, 0);
                                b.focus_board=1;
                                b.fi =19;
                                b.fj =19;
                            }
                        }
                    }
            
                    if (b.focus_board)
                    {
                        switch(ev.keyboard.keycode){
                            case ALLEGRO_KEY_BACKSPACE:
                                execute_undo(&g, &b);
                                redraw=1;
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
                            case ALLEGRO_KEY_SPACE:
                                emit_event(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
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
                    if (fullscreen || resizing) break;
                    resizing=1; resize_time=al_get_time();
                    break;
                case EVENT_REDRAW:
                    redraw=1;
                    break;
            }
        }
	
        if(resizing){
            if(al_get_time()-resize_time > RESIZE_DELAY){
                resizing =0; resize_update=1;
            }
        }
    
        if(resize_update){
            resize_update=0;
            al_set_target_backbuffer(display);
			al_acknowledge_resize(display);
            resize_all(&b, &g);
     // produces artifacts:
    //            al_resize_display(display, b.xsize, b.size + 1);
            al_set_target_backbuffer(display);
            redraw=1;
            resized = 1;
// android workaround, try removing:
//            al_clear_to_color(BLACK_COLOR);
//            al_flip_display();
//            continue;
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
