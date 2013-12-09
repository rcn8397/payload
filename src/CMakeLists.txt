cmake_minimum_required(VERSION 2.6)
##########################################################
## Include directories:
MESSAGE( STATUS "SYSTEM: ${CMAKE_SYSTEM_NAME}" )

if (UNIX OR APPLE)
MESSAGE( STATUS "Setting GLIB2_HEADERS" )
execute_process(COMMAND pkg-config --cflags glib-2.0
                OUTPUT_VARIABLE GLIB2_HEADERS OUTPUT_STRIP_TRAILING_WHITESPACE)
MESSAGE( STATUS "GLIB2_HEADERS: "${GLIB2_HEADERS})
endif()

if ( APPLE )
execute_process( COMMAND pkg-config --libs glib-2.0 
	         OUTPUT_VARIABLE GLIB2_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE )
MESSAGE( STATUS "GLIB2_LIBS: "${GLIB2_LIBS})
endif()

#########################################################
## Executables

add_executable( ecc_client
                main.c 
                client.c
                udp_socket.c 
                hamming.c
                better_udp_socket.c )

add_executable( ecc_server
                main.c 
                server.c
                udp_socket.c
                hamming.c
                better_udp_socket.c )

add_library( ecc SHARED
             server.c
             udp_socket.c
             hamming.c
             better_udp_socket.c )


##########################################################
## Setup Flags

set_target_properties(ecc_client PROPERTIES COMPILE_DEFINITIONS "CLIENT")
set_target_properties(ecc_server PROPERTIES COMPILE_DEFINITIONS "SERVER")

if( UNIX OR APPLE )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLIB2_HEADERS}" )
endif()

if( APPLE )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLIB2_LIBS}")
endif()

MESSAGE( STATUS "COMPILE_FLAGS: ${CMAKE_C_FLAGS} ")

##########################################################
## Optional Link Libraries

target_link_libraries ( ecc_client 
     		        glib-2.0 )

target_link_libraries ( ecc_server 
     		        glib-2.0 )

target_link_libraries ( ecc
     		        glib-2.0 )


##########################################################
## Installation

install ( TARGETS ecc_client ecc_server DESTINATION ${PROJECT_BINARY_DIR}/bin )
install ( TARGETS ecc DESTINATION ${PROJECT_BINARY_DIR}/bin ) 
install ( DIRECTORY "${CMAKE_SOURCE_DIR}/data"
          DESTINATION "${PROJECT_BINARY_DIR}" )
