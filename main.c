#include "socket.h"
#include "irc.h"
#include "var.h"

int main(int argc, char **argv)
{
    irc_t irc;

    if ( irc_connect(&irc, "irc.chat.twitch.tv", "6667") < 0 )
    {
        fprintf(stderr, "Connection failed.\n");
        goto exit_err;
    } else {
         fprintf(stdout, "Connected\n");
    }

    irc_set_output(&irc, "/dev/stdout");

    if ( irc_login(&irc, ircUser, ircPass) < 0 )
    {
        fprintf(stderr, "Couldn't log in.\n");
        goto exit_err;
    } else {
         fprintf(stdout, "Logged in\n");
    }

    if ( irc_join_channel(&irc, "#sentdex") < 0 )
    {
        fprintf(stderr, "Couldn't join channel.\n");
        goto exit_err;
    } else {
         fprintf(stdout, "Joined channel\n");
    }
    
    while ( irc_handle_data(&irc) >= 0 );

    irc_close(&irc);
    return 0;

exit_err:
    irc_close(&irc);
    exit(1);
}

