#ifndef HEADER_H_
#define HEADER_H_

#include <stdint.h>

class Header
{
private:
    typedef char *string;
    typedef unsigned int uint;

public:
    enum ArchiveFlags
    {
        CompressionTypeMask = 0x3f,
        BlocksAndDirectoryInfoCombined = 0x40,
        BlocksInfoAtTheEnd = 0x80,
        OldWebPluginCompatibility = 0x100,
        BlockInfoNeedPaddingAtStart = 0x200
    };

    string signature = nullptr;
    uint version;
    string unityVersion = nullptr;
    string unityRevision = nullptr;
    int64_t size;
    uint compressedBlocksInfoSize;
    uint uncompressedBlocksInfoSize;
    int flags;

    void Show();

    ~Header()
    {
        if (signature)
            delete[] signature;
        if (unityVersion)
            delete[] unityVersion;
        if (unityRevision)
            delete[] unityRevision;
    };
};

#endif