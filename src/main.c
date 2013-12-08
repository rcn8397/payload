#include <stdio.h>
#include <glib.h>


#if defined( CLIENT )
  #include "client.h"    
#else
  #include "server.h"    
#endif




int main(int argc, char *argv[])
{
    const char *port = "1234";
    const char *addr = "127.0.0.1";
    const char *file = NULL;

    GOptionEntry options[] =
    {
        { "port", 0, 0, G_OPTION_ARG_STRING, &port, "port number",  "port" },
        { "addr", 0, 0, G_OPTION_ARG_STRING, &addr, "host address", "host ip" },
        { "file", 0, 0, G_OPTION_ARG_STRING, &file, "file path",    "tx data" },
        { NULL }
    };


    GOptionContext *context = NULL;
    context = g_option_context_new( "- ECC Prototype" );
    g_option_context_add_main_entries( context, options, NULL );
    g_option_context_parse( context, &argc, &argv, NULL );

    g_option_context_free( context );


#if defined( CLIENT )
    client( addr, port, file );
#else
    server( addr, port );
#endif

    return 0;
}

