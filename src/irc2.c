/*
 * Copyright (C) 2004-2009 Georgy Yunaev gyunaev@ulduzsoft.com
 *
 * This example is free, and not covered by LGPL license. There is no
 * restriction applied to their modification, redistribution, using and so on.
 * You can study them, modify them, use them in your own program - either
 * completely or partially. By using it you may give me some credits in your
 * program, but you don't have to.
 *
 *
 * This example tests most features of libirc. It can join the specific
 * channel, welcoming all the people there, and react on some messages -
 * 'help', 'quit', 'dcc chat', 'dcc send', 'ctcp'. Also it can reply to
 * CTCP requests, receive DCC files and accept DCC chats.
 *
 * Features used:
 * - nickname parsing;
 * - handling 'channel' event to track the messages;
 * - handling dcc and ctcp events;
 * - using internal ctcp rely procedure;
 * - generating channel messages;
 * - handling dcc send and dcc chat events;
 * - initiating dcc send and dcc chat.
 *
 * $Id: irctest.c 109 2012-01-24 03:06:42Z gyunaev $
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <libircclient.h>
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
    
    printf ("%s\n", buf);
    
    if ( (fp = fopen ("irctest.log", "ab")) != 0 )
    {
        fprintf (fp, "%s\n", buf);
        fclose (fp);
    }
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
    
    emit_data_event(EVENT_PRIVMSG_RECEIVED, (intptr_t) strdup(origin), (intptr_t) strdup(params[1]),  0, 0);
}


void dcc_recv_callback (irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length)
{
    //    static int count = 1;
    //    char buf[12];
    //
    //    switch (status)
    //    {
    //        case LIBIRC_ERR_CLOSED:
    //            printf ("DCC %d: chat closed\n", id);
    //            break;
    //
    //        case 0:
    //            if ( !data )
    //            {
    //                printf ("DCC %d: chat connected\n", id);
    //                irc_dcc_msg	(session, id, "Hehe");
    //            }
    //            else
    //            {
    //                printf ("DCC %d: %s\n", id, data);
    //                sprintf (buf, "DCC [%d]: %d", id, count++);
    //                irc_dcc_msg	(session, id, buf);
    //            }
    //            break;
    //
    //        default:
    //            printf ("DCC %d: error %s\n", id, irc_strerror(status));
    //            break;
    //    }
}


void dcc_file_recv_callback (irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length)
{
    //    if ( status == 0 && length == 0 )
    //    {
    //        printf ("File sent successfully\n");
    //
    //        if ( ctx )
    //            fclose ((FILE*) ctx);
    //    }
    //    else if ( status )
    //    {
    //        printf ("File sent error: %d\n", status);
    //
    //        if ( ctx )
    //            fclose ((FILE*) ctx);
    //    }
    //    else
    //    {
    //        if ( ctx )
    //            fwrite (data, 1, length, (FILE*) ctx);
    //        printf ("File sent progress: %d\n", length);
    //    }
}


void event_channel (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    //  char nickbuf[128];
    
    if ( count != 2 )
        return;
    
    if(!origin) return;
    emit_data_event(EVENT_CHANMSG_RECEIVED, (intptr_t) strdup(origin), (intptr_t) strdup(params[1]), (intptr_t) strdup(params[0]), 0);
}


void irc_event_dcc_chat (irc_session_t * session, const char * nick, const char * addr, irc_dcc_t dccid)
{
    printf ("DCC chat [%d] requested from '%s' (%s)\n", dccid, nick, addr);
    
    // irc_dcc_accept (session, dccid, 0, dcc_recv_callback);
}


void irc_event_dcc_send (irc_session_t * session, const char * nick, const char * addr, const char * filename, unsigned long size, irc_dcc_t dccid)
{
    //    FILE * fp;
    printf ("DCC send [%d] requested from '%s' (%s): %s (%lu bytes)\n", dccid, nick, addr, filename, size);
    
    //    if ( (fp = fopen ("file", "wb")) == 0 )
    //        abort();
    //
    //    irc_dcc_accept (session, dccid, fp, dcc_file_recv_callback);
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
    for(i=0; i<2;i++){
        printf("Attempt to connect %i/10...\n", i);
        if ( irc_run (g_irc_s) )
            sleep(1);
        printf ("Could not connect or I/O error: %s\n", irc_strerror(irc_errno(g_irc_s)));
    }
    return NULL;
}


int IRC_connect(char *server, int port, char *nick, char *channel)
{
    ALLEGRO_THREAD *irc_thread;
    irc_callbacks_t	callbacks;
    
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
    
    ctx.nick = nick;
    ctx.channel = channel;
    
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


