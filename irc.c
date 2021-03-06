// Source at: https://github.com/Themaister/simple-irc-bot

#include "socket.h"
#include "irc.h"
#include <string.h>
#include <time.h>
#include <fnmatch.h>
#include <unistd.h>

int irc_connect(irc_t *irc, const char* server, const char* port)
{
    if ( (irc->s =  get_socket(server, port)) < 0 )
    {
        return -1;
    }

    irc->bufptr = 0;
    
    return 0;
}

int irc_login(irc_t *irc, const char* nick, const char* pass)
{
    return irc_reg(irc->s, nick, pass, "St0ner1995", "Kasuma Desu");
}

int irc_join_channel(irc_t *irc, const char* channel)
{
    strncpy(irc->channel, channel, 254);
    irc->channel[254] = '\0';
    return irc_join(irc->s, channel);
}

int irc_leave_channel(irc_t *irc)
{
    return irc_part(irc->s, irc->channel);
}

int irc_handle_data(irc_t *irc)
{
    char tempbuffer[512];
    int rc, i;

    if ( (rc = sck_recv(irc->s, tempbuffer, sizeof(tempbuffer) - 2 ) ) <= 0 )
    {
        fprintf(stderr, ":v\n");
        return -1;
    }

    tempbuffer[rc] = '\0';

    for ( i = 0; i < rc; ++i )
    {
        switch (tempbuffer[i])
        {
            case '\r':
            case '\n':
            {
                irc->servbuf[irc->bufptr] = '\0';
                irc->bufptr = 0;

                if ( irc_parse_action(irc) < 0 )
                    return -1;

                break;
            }

            default:
            {
                irc->servbuf[irc->bufptr] = tempbuffer[i];
                if ( irc->bufptr >= (sizeof ( irc->servbuf ) -1 ) )
                    // Overflow!
                    ;
                else
                    irc->bufptr++;
            }
        }
    }
    return 0;
}

int irc_parse_action(irc_t *irc)
{
    
    char irc_nick[128];
    char irc_msg[512];


    if ( strncmp(irc->servbuf, "PING :", 6) == 0 )
    {
        return irc_pong(irc->s, &irc->servbuf[6]);
    }
    else if ( strncmp(irc->servbuf, "NOTICE AUTH :", 13) == 0 )
    {
        // Don't care
        return 0;
    }
    else if ( strncmp(irc->servbuf, "ERROR :", 7) == 0 )
    {
        // Still don't care
        return 0;
    }
    
    // Here be lvl. 42 dragonn boss
    // Parses IRC message that pulls out nick and message. 
    else
    {
        char *ptr;
        int privmsg = 0;
        char irc_nick[128];
        char irc_msg[512];
        *irc_nick = '\0';
        *irc_msg = '\0';

        // Checks if we have non-message string
        if ( strchr(irc->servbuf, 1) != NULL )
            return 0;
    
        if ( irc->servbuf[0] == ':' )
        {
            ptr = strtok(irc->servbuf, "!");
            if ( ptr == NULL )
            {
                printf("ptr == NULL\n");
                return 0;
            }
            else
            {
                strncpy(irc_nick, &ptr[1], 127);
                irc_nick[127] = '\0';
            }

            while ( (ptr = strtok(NULL, " ")) != NULL )
            {
                if ( strcmp(ptr, "PRIVMSG") == 0 )
                {
                    privmsg = 1;
                    break;
                }
            }

            if ( privmsg )
            {
                if ( (ptr = strtok(NULL, ":")) != NULL && (ptr = strtok(NULL, "")) != NULL )
                {
                    strncpy(irc_msg, ptr, 511);
                    irc_msg[511] = '\0';
                }
            }
            
            if ( privmsg == 1 && strlen(irc_nick) > 0 && strlen(irc_msg) > 0 )
            {
                irc_log_message(irc, irc_nick, irc_msg);
                if ( irc_reply_message(irc, irc_nick, irc_msg) < 0 )
                    return -1;
            }
        }
    }
    return 0;
}

int irc_set_output(irc_t *irc, const char* file)
{
    irc->file = fopen(file, "w");
    if ( irc->file == NULL )
        return -1;
    return 0;
}

int irc_reply_message(irc_t *irc, char *irc_nick, char *msg) // Reply to Messages in twitch channel
{
    // Reply to messages
    if ( fnmatch("@st0ner1995*do you want*rocket*", msg, 0x10) == 0) // From AICharlesBot
    {
        // Sleep for a bit
        srand(time(NULL));
        sleep(rand()%3+3);
        // Respond
        if ( irc_msg(irc->s, irc->channel, "!rockets yes") < 0 ) // respond to AICharlesBot
            return -1;
        // Sleep for a bit
        sleep(rand()%3+3);
        // Respond
        if ( irc_msg(irc->s, irc->channel, "Neat") < 0 )
            return -1;
    }
    else if ( fnmatch("@st0ner1995*do you want*nuke*", msg, 0x10) == 0) // From AICharlesBot
    {
        // Sleep for a bit
        srand(time(NULL));
        sleep(rand()%3+3);
        // Respond
        if ( irc_msg(irc->s, irc->channel, "!nuke yes") < 0 ) // respond to AICharlesBot
            return -1;
        // Sleep for a bit
        sleep(rand()%3+3);
        // Respond
        if ( irc_msg(irc->s, irc->channel, "Save that for later") < 0 )
            return -1;
    }
    
    return 0;
}

int irc_log_message(irc_t *irc, const char* nick, const char* message)
{
    char timestring[128];
    time_t curtime;
    time(&curtime);
    strftime(timestring, 127, "%F - %H:%M:%S", localtime(&curtime));
    timestring[127] = '\0';

    fprintf(irc->file, "%s - [%s] <%s> %s\n", irc->channel, timestring, nick, message);
    fflush(irc->file);
}

void irc_close(irc_t *irc)
{
    close(irc->s);
    fclose(irc->file);
}


// irc_pong: For answering pong requests...
int irc_pong(int s, const char *data)
{
    return sck_sendf(s, "PONG :%s\r\n", data);
}

// irc_reg: For registering upon login
int irc_reg(int s, const char *nick, const char *pass, const char *username, const char *fullname)
{
    sck_sendf(s, "PASS %s\r\n", pass);
    return sck_sendf(s, "NICK %s\r\nUSER %s localhost 0 :%s\r\n", nick, username, fullname);
}

// irc_join: For joining a channel
int irc_join(int s, const char *data)
{
    return sck_sendf(s, "JOIN %s\r\n", data);

}

// irc_part: For leaving a channel
int irc_part(int s, const char *data)
{
    return sck_sendf(s, "PART %s\r\n", data);

}

// irc_nick: For changing your nick
int irc_nick(int s, const char *data)
{
    return sck_sendf(s, "NICK %s\r\n", data);

}

// irc_quit: For quitting IRC
int irc_quit(int s, const char *data)
{
    return sck_sendf(s, "QUIT :%s\r\n", data);
}

// irc_topic: For setting or removing a topic
int irc_topic(int s, const char *channel, const char *data)
{
    return sck_sendf(s, "TOPIC %s :%s\r\n", channel, data);
}

// irc_action: For executing an action (.e.g /me is hungry)
int irc_action(int s, const char *channel, const char *data)
{
    return sck_sendf(s, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
}

// irc_msg: For sending a channel message or a query
int irc_msg(int s, const char *channel, const char *data)
{
    return sck_sendf(s, "PRIVMSG %s :%s\r\n", channel, data);
}

