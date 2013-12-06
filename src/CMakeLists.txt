##########################################################
## Include directories:
include_directories( "${GLIB2_INCLUDE_DIRS}" )


##########################################################
## Executables

add_executable( ecc_client
                main.c 
                udp_socket.c )

add_executable( ecc_server
                main.c 
                udp_socket.c )


##########################################################
## Optional Link Libraries

target_link_libraries ( ecc_client 
     		        glib-2.0 )

target_link_libraries ( ecc_server 
     		        glib-2.0 )


##########################################################
## Installation

install ( TARGETS ecc_client ecc_server DESTINATION ${PROJECT_BINARY_DIR}/bin )
install ( DIRECTORY "${CMAKE_SOURCE_DIR}/data"
          DESTINATION "${PROJECT_BINARY_DIR}" )
