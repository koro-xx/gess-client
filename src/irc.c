#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include "main.h"

// to do: use movement event to move. set my_nick and opponent nick correctly
// check win/lose condition

int conn;
char sbuf[512];
char my_nick[32];

void raw(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(sbuf, 512, fmt, ap);
    va_end(ap);
    printf("<< %s", sbuf);
    write(conn, sbuf, strlen(sbuf));
}


void irc_connect(char *nick) {
//    char *channel = "#lalala";
    char *host = "irc.freenode.net";
    char *port = "6667";
    struct addrinfo hints, *res;
    strncpy(my_nick, nick, 31);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, &hints, &res);
    conn = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(conn, res->ai_addr, res->ai_addrlen);
    
    raw("USER %s 0 0 :%s\r\n", nick, nick);
    raw("NICK %s\r\n", nick);
}

void emit_move(char *str){
    int j, jj;
    char ci, cii;
    
    if(strlen(str) < 5) errlog("Can't parse move string %s.", str);
    sscanf(str, "%c%d-%c%d", &ci, &j, &cii, &jj);
    ci -= 'a';
    cii -= 'a';
    j--;
    jj--;
    emit_data_event(EVENT_OPPONENT_MOVE, j, ci, jj, cii);
}


void irc_get_next(char *opponent) {
    char move_str[64];
    char *user, *command, *where, *message, *sep, *target;
    int i, j, l, sl, o = -1, start, wordcount;
    char buf[513];
    
    while ((sl = read(conn, sbuf, 512))) {
        for (i = 0; i < sl; i++) {
            o++;
            buf[o] = sbuf[i];
            if ((i > 0 && sbuf[i] == '\n' && sbuf[i - 1] == '\r') || o == 512) {
                buf[o + 1] = '\0';
                l = o;
                o = -1;
                
                printf(">> %s", buf);
                
                if (!strncmp(buf, "PING", 4)) {
                    buf[1] = 'O';
                    raw(buf);
                } else if (buf[0] == ':') {
                    wordcount = 0;
                    user = command = where = message = NULL;
                    for (j = 1; j < l; j++) {
                        if (buf[j] == ' ') {
                            buf[j] = '\0';
                            wordcount++;
                            switch(wordcount) {
                                case 1: user = buf + 1; break;
                                case 2: command = buf + start; break;
                                case 3: where = buf + start; break;
                            }
                            if (j == l - 1) continue;
                            start = j + 1;
                        } else if (buf[j] == ':' && wordcount == 3) {
                            if (j < l - 1) message = buf + j + 1;
                            break;
                        }
                    }
                    
                    if (wordcount < 2) continue;
                    
                    if (!strncmp(command, "PRIVMSG", 7) || !strncmp(command, "NOTICE", 6)) {
                        if (where == NULL || message == NULL) continue;
                        if ((sep = strchr(user, '!')) != NULL) user[sep - user] = '\0';
                        if (where[0] == '#' || where[0] == '&' || where[0] == '+' || where[0] == '!') target = where; else target = user;
                        printf("[from: %s] [reply-with: %s] [where: %s] [reply-to: %s] %s", user, command, where, target, message);
                        if(opponent && (!strncmp(where, my_nick, 31))){
                            if(!strncmp(user, opponent, strlen(opponent))){
                                //strncpy(move_str, message, 63);
                                emit_move(message);
                            }
                        }
                        //raw("%s %s :%s", command, target, message); // If you enable this the IRCd will get its "*** Looking up your hostname..." messages thrown back at it but it works...
                    }
                }
                
            }
        }
        
    }
    
}

void *irc_thread(ALLEGRO_THREAD *thr, void *arg){
    irc_get_next((char *) arg);
    return NULL;
}
