/*
 * Based on an example by Georgy Yunaev
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <libircclient.h>
#include "macros.h"
#include "main.h"

irc_session_t *g_irc_s;


/*
 * We store data in IRC session context.
 */
typedef struct
{
    char 	* channel;
    char 	* nick;
} irc_ctx_t;

irc_ctx_t ctx;

char *my_strdup(const char *s) {
    char *d = malloc(strlen(s) + 1);   // Allocate memory
    if (d != NULL) strcpy(d, s);         // Copy string if okay
    return d;                            // Return new memory
}


void addlog (const char * fmt, ...)
{
    FILE * fp;
    char buf[1024];
    va_list va_alist;
    
    va_start (va_alist, fmt);
#if defined (_WIN32)
    _vsnprintf (buf, sizeof(buf), fmt, va_alist);
#else
    vsnprintf (buf, sizeof(buf), fmt, va_alist);
#endif
    va_end (va_alist);
    
    deblog("%s", buf);
   
#if 0
    if ( (fp = fopen ("irctest.log", "ab")) != 0 )
    {
        fprintf (fp, "%s\n", buf);
        fclose (fp);
    }
#endif
}


void dump_event (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    char buf[512];
    int cnt;
    
    buf[0] = '\0';
    
    for ( cnt = 0; cnt < count; cnt++ )
    {
        if ( cnt )
            strcat (buf, "|");
        
        strcat (buf, params[cnt]);
    }
    
    
    addlog ("Event \"%s\", origin: \"%s\", params: %d [%s]", event, origin ? origin : "NULL", cnt, buf);
}


void event_join (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    dump_event (session, event, origin, params, count);
    irc_cmd_user_mode (session, "+i");
    emit_event(EVENT_IRC_JOIN);
    //irc_cmd_msg (session, params[0], "Hi all");
}


void event_connect (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
    dump_event (session, event, origin, params, count);
    emit_event(EVENT_IRC_CONNECT);
    
    irc_cmd_join (session, ctx->channel, 0);
}


void event_privmsg (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    dump_event (session, event, origin, params, count);
    
    emit_data_event(EVENT_PRIVMSG_RECEIVED, (intptr_t) my_strdup(origin), (intptr_t) my_strdup(params[1]),  0, 0);
}


void event_channel (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    
    if ( count != 2 )
        return;
    
    dump_event (session, event, origin, params, count);

    if(!origin) return;
    emit_data_event(EVENT_CHANMSG_RECEIVED, (intptr_t) my_strdup(origin), (intptr_t) my_strdup(params[1]), (intptr_t) my_strdup(params[0]), 0);
}


void irc_event_dcc_chat (irc_session_t * session, const char * nick, const char * addr, irc_dcc_t dccid)
{
    deblog("DCC chat [%d] requested from '%s' (%s)\n", dccid, nick, addr);
}


void irc_event_dcc_send (irc_session_t * session, const char * nick, const char * addr, const char * filename, unsigned long size, irc_dcc_t dccid)
{
    deblog("DCC send [%d] requested from '%s' (%s): %s (%lu bytes)\n", dccid, nick, addr, filename, size);
}

void event_numeric (irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
    char buf[24];
    sprintf (buf, "%d", event);
    
    dump_event (session, buf, origin, params, count);
}


void *create_irc_thread(ALLEGRO_THREAD *thr, void *arg){
    int i;
    // and run into forever loop, generating events
    for(i=0; i<10;i++){
        printf("Attempt to connect %i/10...\n", i);
        if ( irc_run (g_irc_s) )
            sleep(1);
        else
            break;
        printf ("Could not connect or I/O error: %s\n", irc_strerror(irc_errno(g_irc_s)));
    }
    emit_event(EVENT_IRC_DISCONNECT);
    return NULL;
}

/*
#ifdef _WIN32
int winsock_init(void){
    static int ws_init=0;
    WSADATA wsaData;
    if(ws_init<=0){
        if ( WSAStartup ( MAKEWORD (2, 2), &wsaData) != 0 ){
            errlog("Winsock API init error.");
            ws_init = -1;
            return 1;
        }
    } else {
        ws_init = 1;
    }
    return 0;
}
#endif
*/

int IRC_connect(const char *server, int port, const char *nick, const char *channel)
{
    ALLEGRO_THREAD *irc_thread;
    irc_callbacks_t	callbacks;

	/*
#ifdef _WIN32 // need to initialize exactly once per app.
    if (winsock_init()) return 1;
#endif
    */
        
    memset (&callbacks, 0, sizeof(callbacks));
    
    callbacks.event_connect = event_connect;
    callbacks.event_join = event_join;
    callbacks.event_nick = dump_event;
    callbacks.event_quit = dump_event;
    callbacks.event_part = dump_event;
    callbacks.event_mode = dump_event;
    callbacks.event_topic = dump_event;
    callbacks.event_kick = dump_event;
    callbacks.event_channel = event_channel;
    callbacks.event_privmsg = event_privmsg;
    callbacks.event_notice = dump_event;
    callbacks.event_invite = dump_event;
    callbacks.event_umode = dump_event;
    callbacks.event_ctcp_rep = dump_event;
    callbacks.event_ctcp_action = dump_event;
    callbacks.event_unknown = dump_event;
    callbacks.event_numeric = event_numeric;
    
    callbacks.event_dcc_chat_req = irc_event_dcc_chat;
    callbacks.event_dcc_send_req = irc_event_dcc_send;
    
    g_irc_s = irc_create_session (&callbacks);
    
    if ( !g_irc_s )
    {
        printf ("Could not create IRC session\n");
        return 1;
    }
    
    ctx.nick = my_strdup(nick);
    ctx.channel = my_strdup(channel);
    
    irc_set_ctx (g_irc_s, &ctx);
    irc_option_set(g_irc_s, LIBIRC_OPTION_STRIPNICKS);
    
    // Initiate the IRC server connection
    if ( irc_connect (g_irc_s, server, port, 0, nick, 0, 0) )
    {
        printf ("Could not connect: %s\n", irc_strerror(irc_errno(g_irc_s)));
        return 1;
    }
    
    // add sleep interval here to avoid racing condition
    sleep(1);
    
    irc_thread = al_create_thread(create_irc_thread, (void *) NULL);
    al_start_thread(irc_thread);
    
    return 0;
}


