#include "protocol.h"
#include <string.h>

Message::Message(){
    memset(this->data, 0x00, sizeof(this->data));
}


FileBlockDescriptor::FileBlockDescriptor() {
	memset(this->blockIV, 0x00, sizeof(this->blockIV));
}
