#ifndef SERIALIZEDFILE_H_
#define SERIALIZEDFILE_H_

#include <stdint.h>
#include <vector>

class ObjectInfo;

enum SerializedFileFormatVersion
{
    Unsupported = 1,
    Unknown_2 = 2,
    Unknown_3 = 3,
    Unknown_5 = 5,
    Unknown_6 = 6,
    Unknown_7 = 7,
    Unknown_8 = 8,
    Unknown_9 = 9,
    Unknown_10 = 10,
    HasScriptTypeIndex = 11,
    Unknown_12 = 12,
    HasTypeTreeHashes = 13,
    Unknown_14 = 14,
    SupportsStrippedObject = 15,
    RefactoredClassId = 16,
    RefactorTypeData = 17,
    RefactorShareableTypeTreeData = 18,
    TypeTreeNodeWithTypeFlags = 19,
    SupportsRefObject = 20,
    StoresTypeDependencies = 21,
    LargeFilesSupport = 22
};

enum BuildTarget
{
    NoTarget = -2,
    AnyPlayer = -1,
    ValidPlayer = 1,
    StandaloneOSX = 2,
    StandaloneOSXPPC = 3,
    StandaloneOSXIntel = 4,
    StandaloneWindows,
    WebPlayer,
    WebPlayerStreamed,
    Wii = 8,
    iOS = 9,
    PS3,
    XBOX360,
    Broadcom = 12,
    Android = 13,
    StandaloneGLESEmu = 14,
    StandaloneGLES20Emu = 15,
    NaCl = 16,
    StandaloneLinux = 17,
    FlashPlayer = 18,
    StandaloneWindows64 = 19,
    WebGL,
    WSAPlayer,
    StandaloneLinux64 = 24,
    StandaloneLinuxUniversal,
    WP8Player,
    StandaloneOSXIntel64,
    BlackBerry,
    Tizen,
    PSP2,
    PS4,
    PSM,
    XboxOne,
    SamsungTV,
    N3DS,
    WiiU,
    tvOS,
    Switch,
    Lumin,
    Stadia,
    CloudRendering,
    GameCoreXboxSeries,
    GameCoreXboxOne,
    PS5,
    EmbeddedLinux,
    QNX,
    UnknownPlatform = 9999
};

class TypeTreeNode
{
private:
    typedef unsigned int uint;
    typedef char *string;

public:
    string m_Type;
    string m_Name;
    int m_ByteSize;
    int m_Index;
    int m_TypeFlags; // m_IsArray
    int m_Version;
    int m_MetaFlag;
    int m_Level;
    uint m_TypeStrOffset;
    uint m_NameStrOffset;
    uint64_t m_RefTypeHash;

    void Show();
};

class TypeTree
{
private:
    typedef char byte;

public:
    std::vector<TypeTreeNode> m_Nodes;
    byte *m_StringBuffer;
};

class SerializedFileHeader
{
private:
    typedef char byte;
    typedef unsigned int uint;

public:
    uint m_MetadataSize;
    int64_t m_FileSize = 0;
    SerializedFileFormatVersion m_Version;
    int64_t m_DataOffset = 0;
    byte m_Endianess;
    byte m_Reserved[3];

    void Show();
};

class SerializedType
{
private:
    typedef char byte;
    typedef char *string;

public:
    int classID;
    bool m_IsStrippedType;
    int16_t m_ScriptTypeIndex = -1;
    TypeTree m_Type;
    byte *m_ScriptID;    // Hash128
    byte *m_OldTypeHash; // Hash128
    int *m_TypeDependencies;
    string m_KlassName;
    string m_NameSpace;
    string m_AsmName;

    void Show();
};

class SerializedFile
{
private:
    typedef char byte;
    typedef char *string;

public:
    byte m_FileEndianess;

    SerializedFileHeader header;
    string unityVersion;
    BuildTarget m_TargetPlatform;
    std::vector<ObjectInfo> m_Objects;
    bool m_EnableTypeTree;
    int typeCount;
    int bigIDEnabled = 0;
    string userInformation = nullptr;

    void Show();
};

#endif