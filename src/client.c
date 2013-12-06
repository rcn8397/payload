#include <stdio.h>
#include <glib.h>


int main(int argc, char *argv[])
{
    const char *port = "1234";
    const char *addr = "127.0.0.1";

    GOptionEntry options[] =
    {
        { "port", 0, 0, G_OPTION_ARG_STRING, &port, "port number", "port" },
        { "addr", 0, 0, G_OPTION_ARG_STRING, &addr, "host address", "host ip" },
        { NULL }
    };


    GOptionContext *context = NULL;
    context = g_option_context_new( "- Client ECC" );
    g_option_context_add_main_entries( context, options, NULL );
    g_option_context_parse( context, &argc, &argv, NULL );

    g_option_context_free( context );


    return 0;
}
