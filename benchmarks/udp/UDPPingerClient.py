#  UDPPingerClient.py
#
#  CSE 5344: Computer Networks
#  Fall 2013
#
#  Programming Assignment 2
#  Due: 10/2/2013
#
#  Submitted by Carson Clanton
#

__author__     = "Carson Clanton"
__credits__    = ["James Kurose", "Keith Ross"]
__version__    = "1.0"
__maintainer__ = "Carson Clanton"
__email__      = "clanton@uta.edu"



# We will need the following modules to determining and formatting time
import time
import datetime
# include the following module for opening and communicating using a UDP socket
from socket import *
# include this module for getting and parsing command line arguments
import sys, string

# Create a UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
clientSocket = socket(AF_INET, SOCK_DGRAM) # Assign IP address and port number to socket

# set socket timeout to 1 second
clientSocket.settimeout(1)


 
# get server name from command line arg, 
# assumes server name is last cmd param
serverName = 'localhost' #sys.argv[len(sys.argv)-1]
serverPort = 12000

# echo server info for user check
#print "pinging %s:%d" % (serverName, serverPort)
f = open("../../data/christmas_carol.txt","r")

# loop 10 iterations as required by assignment statement
#for i in range (1,11):
for line in f:
  
  for c in line:
    #sys.stdout.write('%s' % c)
    clientSocket . sendto (c,(serverName,serverPort))

f.close()
clientSocket.close()
