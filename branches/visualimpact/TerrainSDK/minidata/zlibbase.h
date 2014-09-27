// (c) by Stefan Roettger

#ifndef ZLIBBASE_H
#define ZLIBBASE_H

namespace zlibbase {

unsigned char *decompressZLIB(unsigned char *chunk,unsigned int chunklen,long long *bytes);
void compressZLIB(unsigned char *data,unsigned int bytes,unsigned char **chunk,long long *chunklen,int level=9);

}

#endif
