#ifndef STORAGE_BLOCK_H_
#define STORAGE_BLOCK_H_

#include <stdint.h>

class StorageBlock
{
private:
    typedef unsigned int uint;

public:
    enum StorageBlockFlags
    {
        CompressionTypeMask = 0x3f,
        Streamed = 0x40
    };

    uint compressedSize;
    uint uncompressedSize;
    int16_t flags;

    void Show();
};

#endif