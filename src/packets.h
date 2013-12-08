#ifndef _CSE5344_PACKETS_H_
#define _CSE5344_PACKETS_H_

#define REQUEST_OPCODE  1234
#define RESPONSE_OPCODE 5678

typedef struct REQUEST_PACKET
{
  unsigned int opcode;
  unsigned int id;
} REQUEST_PACKET;

typedef struct RESPONSE_PACKET
{
  unsigned int opcode;
  unsigned int id;
  float value;
} RESPONSE_PACKET;

#endif //_CSE5344_PACKETS_H_
