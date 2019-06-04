#ifndef __PROTOCOLS_H__
#define __PROTOCOLS_H__


/*
 * File that define the protocol
*/


#include <string>
#include "common.h"

#define SERVER_PORT 9898

/* ======== ERRORS ===========*/
#define REPLY_FALSE       0x00
#define REPLY_TRUE        0x01

#define SERVER_FULL       0x60
#define WRONG_FILENAME    0x61
#define FILE_TOO_BIG      0x62
#define FILE_NOT_EXISTS   0x63

/* ======= COMMANDS ======== */
#define COMMAND_FILELIST     0x02
#define COMMAND_UPLOADFILE   0x03
#define COMMAND_DOWNLOADFILE 0x04
#define COMMAND_AMIALLOWED   0x0E
#define COMMAND_QUIT         0x0F

#define FILE_ELEMENT         0x05
#define FILELIST_SIZE        0x06

#define FILEBLOCK_ELEMENT    0X07
#define FILEBLOCK_LAST       0X08


/* ======= PROTOCOLS ======== */
#define DHEC_TYPE    	     0X98
#define DH_TYPE              0X99

/* ======= SIZES =========== */
#define MESSAGE_SIZE           sizeof(Message)
#define C_MESSAGE_SIZE         (sizeof(Message)/16*16+16)
#define SIZE_AFTER_AES(A)      ((A)/16*16+16)

#define MAX_FILENAME_LENGTH    100
#define MAX_FILESIZE           (4L*1024L*1024L*1024L)

#define DEFAULT_BLOCK_SIZE     (1L*1024L)

#define IV_SIZE                16


class Message {

public:
  uint32_t command;
  uint32_t order;
  byte data[256];

  Message();

}__attribute__((packed));


struct FileBlockDescriptor {

  uint32_t order;
  uint32_t flag;
  uint64_t blockDim;
  byte blockIV[IV_SIZE];

  FileBlockDescriptor();

}__attribute__((packed));



#endif
