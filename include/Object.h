#ifndef OBJECT_H_
#define OBJECT_H_

#include <vector>
#include <map>
#include "SerializedFile.h"

enum AudioCompressionFormat
{
    PCM = 0,
    Vorbis = 1,
    ADPCM = 2,
    MP3 = 3,
    PSMVAG = 4,
    HEVAG = 5,
    XMA = 6,
    AAC = 7,
    GCADPCM = 8,
    ATRAC9 = 9
};

class ObjectInfo
{
private:
    typedef char byte;
    typedef char *string;
    typedef unsigned int uint;

public:
    int64_t byteStart;
    uint byteSize;
    int typeID;
    int classID;
    uint16_t isDestroyed;
    byte stripped;

    int64_t m_PathID;
    SerializedType serializedType;

    void Show();
};

class PPtr
{
public:
    int m_FileID;
    int64_t m_PathID;

    void Show();
};

class AssetInfo
{
public:
    int preloadIndex;
    int preloadSize;
    PPtr asset;

    void Show();
};

class AssetBundle
{
private:
    typedef char *string;

public:
    string m_Name;
    std::vector<PPtr> m_PreloadTable;
    std::map<string, AssetInfo> m_Container;

    void Show();
};

class AudioClip
{
private:
    typedef char *string;

public:
    string m_Name;

    // version 5
    int m_LoadType;
    int m_Channels;
    int m_Frequency;
    int m_BitsPerSample;
    float m_Length;
    bool m_IsTrackerFormat;
    int m_SubsoundIndex;
    bool m_PreloadAudioData;
    bool m_LoadInBackground;
    bool m_Legacy3D;

    string m_Source;
    uint64_t m_Offset; // ulong
    uint64_t m_Size;   // ulong
    AudioCompressionFormat m_CompressionFormat;

    void Show();
};

class TextAsset
{
private:
    typedef char byte;
    typedef char *string;

public:
    string m_Name;
    int m_Size;
    byte *m_Script;

    void Show();
};

#endif