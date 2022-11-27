/*
 * @Author: ZeLin SNZCSNZC@outlook.com
 * @Date: 2022-11-23 08:05:33
 * @LastEditors: ZeLin SNZCSNZC@outlook.com
 * @LastEditTime: 2022-11-27 10:11:00
 * @Description: Phigros谱面和音频文件替换
 */
#ifndef _WIN64
#error NO Windows64 System
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <io.h>
#include "lz4_64/lz4.h"

#include "ShowHex.h"
#include "BigLittleEndian.h"
#include "Stream.h"
#include "CommonString.h"
#include "ClassIDType.h"

#include "Header.h"
#include "StorageBlock.h"
#include "Node.h"
#include "SerializedFile.h"
#include "Object.h"

#define BLOCK_SIZE 131072

typedef char byte;
typedef unsigned int uint;

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using ZeLin::ShowHex;

enum CompressionType
{
    None,
    Lzma,
    Lz4,
    Lz4HC,
    Lzham
};

struct unpackFile
{
    string name;
    long long size;
    bool isAsset;
    Stream *pStreamData;
};
vector<unpackFile> unpackFileList;

Header header;
StorageBlock *pBlocksInfo;
Node *pDirectoryInfo;

SerializedFile serializedFile;

AssetBundle assetBundle;
AudioClip audioClip;
TextAsset textAsset;

int blocksInfoCount;
int nodesCount;
int objectCount;
unsigned int lastblockSize;

bool debugInfo = false;

void error()
{
    cout << "Error : This isn't Phigros music bundle file or Phigros version maybe too early." << endl;
    cout << "Note : This program can be run in Phigros 2.4.3." << endl;
    exit(EXIT_FAILURE);
}

// 读文件头
void ReadHeader(FileReadStream &fileReadStream)
{
    fileReadStream.stringRead(header.signature);
    fileReadStream.ReadByte_B2L(&header.version, 4);
    fileReadStream.stringRead(header.unityVersion);
    fileReadStream.stringRead(header.unityRevision);
    fileReadStream.ReadByte_B2L(&header.size, 8);
    fileReadStream.ReadByte_B2L(&header.compressedBlocksInfoSize, 4);
    fileReadStream.ReadByte_B2L(&header.uncompressedBlocksInfoSize, 4);
    fileReadStream.ReadByte_B2L(&header.flags, 4);

    if (debugInfo)
    {
        header.Show();
        cout << endl;
    }
}

// 写文件头
void WriteHeader(Stream &DataStream)
{
    DataStream.stringWrite(header.signature);

    DataStream.WriteByte_B2L(&header.version, 4);
    DataStream.stringWrite(header.unityVersion);
    DataStream.stringWrite(header.unityRevision);
    DataStream.WriteByte_B2L(&header.size, 8);
    DataStream.WriteByte_B2L(&header.compressedBlocksInfoSize, 4);
    DataStream.WriteByte_B2L(&header.uncompressedBlocksInfoSize, 4);
    DataStream.WriteByte_B2L(&header.flags, 4);

    if (debugInfo)
    {
        header.Show();
        cout << endl;
    }
}

void ReadBlocksInfoAndDirectory(FileReadStream &fileReadStream)
{
    int numWrite = 0;

    byte *blocksInfoBytes;
    byte *uncompressedBytes = nullptr;
    byte *uncompressedDataHash;

    Stream blocksInfoReader;

    if (header.version >= 7)
        fileReadStream.AlignStream(16);

    if ((header.flags & Header::ArchiveFlags::BlocksInfoAtTheEnd) != 0)
    {
        cout << "Header.flags is BlocksInfoAtTheEnd." << endl;
        error();
    }
    else
    {
        blocksInfoBytes = new byte[header.compressedBlocksInfoSize];
        fileReadStream.ReadByte(blocksInfoBytes, header.compressedBlocksInfoSize);
    }

    if (debugInfo)
    {
        ShowHex(blocksInfoBytes, header.compressedBlocksInfoSize);
        cout << endl;
    }

    int compressionType = (CompressionType)(header.flags & Header::ArchiveFlags::CompressionTypeMask);
    switch (compressionType)
    {
    case CompressionType::None:
        blocksInfoReader.init(blocksInfoBytes, header.compressedBlocksInfoSize);
        break;
    case CompressionType::Lzma:
        cout << "Header.flags Lzma decompression isn't support." << endl;
        error();
    case CompressionType::Lz4:
    case CompressionType::Lz4HC:
        uncompressedBytes = new byte[header.uncompressedBlocksInfoSize];
        numWrite = LZ4_decompress_safe(blocksInfoBytes, uncompressedBytes, header.compressedBlocksInfoSize, header.uncompressedBlocksInfoSize);
        if (header.uncompressedBlocksInfoSize != numWrite)
            cout << "Lz4 decompression error : " << numWrite << endl;
        blocksInfoReader.init(uncompressedBytes, header.uncompressedBlocksInfoSize);
        break;
    default:
        cout << "Header.flags unknown decompression isn't support." << endl;
        error();
    }
    if (debugInfo)
    {
        blocksInfoReader.ShowHex();
        cout << endl;
    }

    uncompressedDataHash = new byte[16];
    blocksInfoReader.ReadByte(uncompressedDataHash, 16);
    if (debugInfo)
        cout << "uncompressedDataHash : " << uncompressedDataHash << endl;

    blocksInfoReader.ReadByte_B2L(&blocksInfoCount, 4);
    if (debugInfo)
    {
        cout << "blocksInfoCount : " << blocksInfoCount << endl;
        cout << endl;
    }

    pBlocksInfo = new StorageBlock[blocksInfoCount];
    for (int i = 0; i < blocksInfoCount; i++)
    {
        StorageBlock &BlockInfo = pBlocksInfo[i];

        blocksInfoReader.ReadByte_B2L(&BlockInfo.uncompressedSize, sizeof 4);
        blocksInfoReader.ReadByte_B2L(&BlockInfo.compressedSize, sizeof 4);
        blocksInfoReader.ReadByte_B2L(&BlockInfo.flags, 2);

        if (debugInfo)
        {
            cout << i << " > " << endl;
            BlockInfo.Show();
            cout << endl;
        }
    }

    blocksInfoReader.ReadByte_B2L(&nodesCount, 4);

    if (debugInfo)
    {
        cout << "nodesCount : " << nodesCount << endl;
        cout << endl;
    }

    // 暂时规定nodesCount不能大于2
    if (nodesCount > 2)
    {
        cout << "Nodes Count Error." << endl;
        error();
    }

    pDirectoryInfo = new Node[nodesCount];
    for (int i = 0; i < nodesCount; i++)
    {
        Node &DirectoryInfo = pDirectoryInfo[i];

        blocksInfoReader.ReadByte_B2L(&DirectoryInfo.offset, 8);
        blocksInfoReader.ReadByte_B2L(&DirectoryInfo.size, 8);
        blocksInfoReader.ReadByte_B2L(&DirectoryInfo.flags, 4);
        blocksInfoReader.stringRead(DirectoryInfo.path);

        if (debugInfo)
        {
            cout << i << " > " << endl;
            DirectoryInfo.Show();
            cout << endl;
        }
    }

    if ((header.flags & Header::ArchiveFlags::BlockInfoNeedPaddingAtStart) != 0)
        fileReadStream.AlignStream(16);

    delete[] blocksInfoBytes;
    if (uncompressedBytes)
        delete[] uncompressedBytes;
    delete[] uncompressedDataHash;
}

void WriteBlocksInfoAndDirectory(Stream &dataStream)
{
    int numWrite = 0;

    byte *blocksInfoBytes;
    byte *uncompressedBytes;
    byte *uncompressedDataHash;

    Stream blocksInfoWriter;

    if (header.version >= 7)
        dataStream.AlignStream(16);
    if ((header.flags & Header::ArchiveFlags::BlocksInfoAtTheEnd) != 0)
    {
        cout << "Header.flags is BlocksInfoAtTheEnd." << endl;
        error();
    }

    uncompressedDataHash = new byte[16];
    memset(uncompressedDataHash, 0, 16);
    blocksInfoWriter.WriteByte(uncompressedDataHash, 16);
    if (debugInfo)
        cout << "uncompressedDataHash : " << uncompressedDataHash << endl;

    blocksInfoWriter.WriteByte_B2L(&blocksInfoCount, 4);
    if (debugInfo)
    {
        cout << "blocksInfoCount : " << blocksInfoCount << endl;
        cout << endl;
    }

    for (int i = 0; i < blocksInfoCount; i++)
    {
        StorageBlock &BlockInfo = pBlocksInfo[i];

        blocksInfoWriter.WriteByte_B2L(&BlockInfo.uncompressedSize, 4);
        blocksInfoWriter.WriteByte_B2L(&BlockInfo.compressedSize, 4);
        blocksInfoWriter.WriteByte_B2L(&BlockInfo.flags, 2);

        if (debugInfo)
        {
            cout << i << " > " << endl;
            BlockInfo.Show();
            cout << endl;
        }
    }

    blocksInfoWriter.WriteByte_B2L(&nodesCount, 4);
    if (debugInfo)
    {
        cout << "nodesCount : " << nodesCount << endl;
        cout << endl;
    }

    Node *pOldDirectoryInfo = pDirectoryInfo;
    pDirectoryInfo = new Node[nodesCount];

    int64_t m_offset = 0;
    for (int i = 0; i < unpackFileList.size(); i++)
    {
        unpackFile &unpackFileItem = unpackFileList[i];
        Node &DirectoryInfo = pDirectoryInfo[i];
        Node &OldDirectoryInfo = pOldDirectoryInfo[i];

        if (i != 0)
            m_offset += pDirectoryInfo[i - 1].size;
        DirectoryInfo.offset = m_offset;

        DirectoryInfo.size = unpackFileItem.size;
        DirectoryInfo.flags = OldDirectoryInfo.flags;

        DirectoryInfo.path = new char[strlen(OldDirectoryInfo.path) + 1];
        strcpy(DirectoryInfo.path, OldDirectoryInfo.path);
    }
    delete[] pOldDirectoryInfo;

    for (int i = 0; i < nodesCount; i++)
    {
        Node &DirectoryInfo = pDirectoryInfo[i];

        blocksInfoWriter.WriteByte_B2L(&DirectoryInfo.offset, 8);
        blocksInfoWriter.WriteByte_B2L(&DirectoryInfo.size, 8);
        blocksInfoWriter.WriteByte_B2L(&DirectoryInfo.flags, 4);
        blocksInfoWriter.stringWrite(DirectoryInfo.path);

        if (debugInfo)
        {
            cout << i << " > " << endl;
            DirectoryInfo.Show();
            cout << endl;
        }
    }
    if (debugInfo)
    {
        blocksInfoWriter.ShowHex();
        cout << endl;
    }

    blocksInfoBytes = new byte[blocksInfoWriter.sizeg()];
    uncompressedBytes = new byte[blocksInfoWriter.sizeg()];
    blocksInfoWriter.seekg(0);
    blocksInfoWriter.ReadByte(uncompressedBytes, blocksInfoWriter.sizeg());

    int compressionType = (CompressionType)(header.flags & Header::ArchiveFlags::CompressionTypeMask);
    switch (compressionType)
    {
    case CompressionType::None:
        cout << "Header.flags None decompression isn't support." << endl;
        error();
    case CompressionType::Lzma:
        cout << "Header.flags Lzma decompression isn't support." << endl;
        error();
    case CompressionType::Lz4:
    case CompressionType::Lz4HC:
        numWrite = LZ4_compress(uncompressedBytes, blocksInfoBytes, blocksInfoWriter.sizeg());
        dataStream.WriteByte(blocksInfoBytes, numWrite);
        header.uncompressedBlocksInfoSize = blocksInfoWriter.sizeg();
        header.compressedBlocksInfoSize = numWrite;
        break;
    default:
        cout << "Header.flags unknown decompression isn't support." << endl;
        error();
    }

    if (debugInfo)
    {
        ShowHex(blocksInfoBytes, numWrite);
        cout << endl;
    }

    if ((header.flags & Header::ArchiveFlags::BlockInfoNeedPaddingAtStart) != 0)
        dataStream.AlignStream(16);

    delete[] blocksInfoBytes;
    delete[] uncompressedBytes;
    delete[] uncompressedDataHash;
}

void ReadBlocks(FileReadStream &fileReadStream, Stream &blocksStream)
{
    int numWrite;
    int compressionType;
    int compressedSize;

    byte *compressedBytes;
    byte *uncompressedBytes;

    for (int i = 0; i < blocksInfoCount; i++)
    {
        if (debugInfo)
            cout << i << " > " << endl;
        StorageBlock &BlockInfo = pBlocksInfo[i];
        compressionType = (CompressionType)(BlockInfo.flags & StorageBlock::StorageBlockFlags::CompressionTypeMask);
        switch (compressionType)
        {
        case CompressionType::None:
            uncompressedBytes = new byte[BlockInfo.uncompressedSize];
            fileReadStream.ReadByte(uncompressedBytes, BlockInfo.uncompressedSize);
            blocksStream.write(uncompressedBytes, BlockInfo.uncompressedSize);
            if (debugInfo)
            {
                ShowHex(uncompressedBytes, BlockInfo.uncompressedSize, 128);
                cout << endl;
            }

            delete[] uncompressedBytes;
            break;
        case CompressionType::Lzma:
            cout << "BlockInfo Lzma decompression isn't support." << endl;
            error();
        case CompressionType::Lz4:
        case CompressionType::Lz4HC:
            compressedBytes = new byte[BlockInfo.compressedSize];
            fileReadStream.ReadByte(compressedBytes, BlockInfo.compressedSize);
            if (debugInfo)
            {
                ShowHex(compressedBytes, BlockInfo.compressedSize, 128);
                cout << endl;
            }

            uncompressedBytes = new byte[BlockInfo.uncompressedSize];
            numWrite = LZ4_decompress_safe(compressedBytes, uncompressedBytes, BlockInfo.compressedSize, BlockInfo.uncompressedSize);
            if (BlockInfo.uncompressedSize != numWrite)
            {
                cout << "Lz4 decompression error : " << numWrite << endl;
                error();
            }
            blocksStream.write(uncompressedBytes, BlockInfo.uncompressedSize);
            if (debugInfo)
            {
                ShowHex(uncompressedBytes, BlockInfo.uncompressedSize, 128);
                cout << endl;
            }

            delete[] compressedBytes;
            delete[] uncompressedBytes;
            break;
        default:
            cout << "BlockInfo unknown decompression isn't support." << endl;
            error();
        }
    }
}

void FormatBlocks(Stream &blockStream)
{
    blocksInfoCount = blockStream.sizeg() / BLOCK_SIZE + 1;
    lastblockSize = blockStream.sizeg() % BLOCK_SIZE;

    if (debugInfo)
        cout << "lastblockSize : " << lastblockSize << endl;

    delete[] pBlocksInfo;
    pBlocksInfo = new StorageBlock[blocksInfoCount];
    for (int i = 0; i < blocksInfoCount - 1; i++)
    {
        pBlocksInfo[i].compressedSize = BLOCK_SIZE;
        pBlocksInfo[i].uncompressedSize = BLOCK_SIZE;
        pBlocksInfo[i].flags = CompressionType::None;
    }

    // lastblock
    pBlocksInfo[blocksInfoCount - 1].compressedSize = lastblockSize;
    pBlocksInfo[blocksInfoCount - 1].uncompressedSize = lastblockSize;
    pBlocksInfo[blocksInfoCount - 1].flags = CompressionType::None;
}

bool IsSerializedFile(Stream &streamData)
{
    streamData.seekg(0);
    if (streamData.sizeg() < 20)
        return false;

    uint m_MetadataSize;
    int64_t m_FileSize = 0;
    uint m_Version;
    int64_t m_DataOffset = 0;
    byte m_Endianess;
    byte *m_Reserved = new byte[3];
    streamData.ReadByte_B2L(&m_MetadataSize, 4);
    streamData.ReadByte_B2L(&m_FileSize, 4);
    streamData.ReadByte_B2L(&m_Version, 4);
    streamData.ReadByte_B2L(&m_DataOffset, 4);
    streamData.ReadByte_B2L(&m_Endianess, 1);
    streamData.ReadByte_B2L(m_Reserved, 3);

    if (m_Version >= 22)
    {
        if (streamData.sizeg() < 48)
        {
            streamData.seekg(0);
            return false;
        }
        streamData.ReadByte_B2L(&m_MetadataSize, 4);
        streamData.ReadByte_B2L(&m_FileSize, 8);
        streamData.ReadByte_B2L(&m_DataOffset, 8);
    }
    streamData.seekg(0);

    if (m_FileSize != streamData.sizeg())
        return false;
    if (m_DataOffset > streamData.sizeg())
        return false;

    return true;
}

char *TypeTreeBlobRead_ReadString(Stream &stringBufferReader, uint value)
{
    if ((value & 0x80000000) == 0)
    {
        stringBufferReader.seekg(value);
        char *pStr;
        stringBufferReader.stringRead(pStr);
        return pStr;
    }
    uint32_t offset = value & 0x7FFFFFFF;
    if (CommonString::Dictionary[offset].c_str()[0] != '\0')
    {
        char *pStr = new char[CommonString::Dictionary[offset].length() + 1];
        strcpy(pStr, CommonString::Dictionary[offset].c_str());
        return pStr;
    }
    char *pStr = new char[2];
    strcpy(pStr, "#");
    return pStr;
}

void TypeTreeBlobRead(Stream &fileReadStream, TypeTree &m_Type)
{
    int numberOfNodes;
    fileReadStream.ReadByte(&numberOfNodes, 4);
    if (debugInfo)
        cout << "numberOfNodes : " << numberOfNodes << endl;

    int stringBufferSize;
    fileReadStream.ReadByte(&stringBufferSize, 4);
    if (debugInfo)
        cout << "stringBufferSize : " << stringBufferSize << endl;

    for (int i = 0; i < numberOfNodes; i++)
    {
        TypeTreeNode *ptypeTreeNode = new TypeTreeNode();
        TypeTreeNode &typeTreeNode = *ptypeTreeNode;

        fileReadStream.ReadByte(&typeTreeNode.m_Version, 2);
        fileReadStream.ReadByte(&typeTreeNode.m_Level, 1);
        fileReadStream.ReadByte(&typeTreeNode.m_TypeFlags, 1);
        fileReadStream.ReadByte(&typeTreeNode.m_TypeStrOffset, 4);
        fileReadStream.ReadByte(&typeTreeNode.m_NameStrOffset, 4);
        fileReadStream.ReadByte(&typeTreeNode.m_ByteSize, 4);
        fileReadStream.ReadByte(&typeTreeNode.m_Index, 4);
        fileReadStream.ReadByte(&typeTreeNode.m_MetaFlag, 4);

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::TypeTreeNodeWithTypeFlags)
            fileReadStream.ReadByte(&typeTreeNode.m_RefTypeHash, 8);

        m_Type.m_Nodes.push_back(typeTreeNode);
    }
    m_Type.m_StringBuffer = new byte[stringBufferSize];
    fileReadStream.ReadByte(m_Type.m_StringBuffer, stringBufferSize);
    if (debugInfo)
    {
        ShowHex(m_Type.m_StringBuffer, stringBufferSize);
        cout << endl;
    }

    Stream stringBufferReader(m_Type.m_StringBuffer, stringBufferSize);

    for (int i = 0; i < numberOfNodes; i++)
    {
        auto &m_Node = m_Type.m_Nodes[i];

        m_Node.m_Type = TypeTreeBlobRead_ReadString(stringBufferReader, m_Node.m_TypeStrOffset);
        m_Node.m_Name = TypeTreeBlobRead_ReadString(stringBufferReader, m_Node.m_NameStrOffset);

        if (debugInfo)
        {
            cout << i << " > " << endl;
            m_Node.Show();
            cout << endl;
        }
    }
}

SerializedType &ReadSerializedType(Stream &fileReadStream, bool isRefType)
{
    SerializedType *ptype = new SerializedType();
    SerializedType &type = *ptype;

    fileReadStream.ReadByte(&type.classID, 4);

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::RefactoredClassId)
        fileReadStream.ReadByte(&type.m_IsStrippedType, 1);

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::RefactorTypeData)
        fileReadStream.ReadByte(&type.m_ScriptTypeIndex, 2);

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::HasTypeTreeHashes)
    {
        type.m_ScriptID = new byte[16];
        memset(type.m_ScriptID, 0, 16);

        if (isRefType && type.m_ScriptTypeIndex >= 0)
            fileReadStream.ReadByte(type.m_ScriptID, 16);
        else if ((serializedFile.header.m_Version < SerializedFileFormatVersion::RefactoredClassId && type.classID < 0) || (serializedFile.header.m_Version >= SerializedFileFormatVersion::RefactoredClassId && type.classID == 114))
            fileReadStream.ReadByte(type.m_ScriptID, 16);

        type.m_OldTypeHash = new byte[16];
        fileReadStream.ReadByte(type.m_OldTypeHash, 16);
    }

    if (debugInfo)
        type.Show();

    if (serializedFile.m_EnableTypeTree)
    {
        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_12 || serializedFile.header.m_Version == SerializedFileFormatVersion::Unknown_10)
            TypeTreeBlobRead(fileReadStream, type.m_Type);
        else
        {
            cout << "Unsupported Serialized File Format Version." << endl;
            error();
        }

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::StoresTypeDependencies)
        {
            if (isRefType)
            {
                fileReadStream.stringRead(type.m_KlassName);
                fileReadStream.stringRead(type.m_NameSpace);
                fileReadStream.stringRead(type.m_AsmName);
            }
            else
            {
                int arraySize;
                fileReadStream.ReadByte(&arraySize, 4);
                type.m_TypeDependencies = new int[arraySize];
                fileReadStream.ReadByte(type.m_TypeDependencies, arraySize);
                if (debugInfo)
                {
                    cout << "arraySize : " << arraySize << endl;
                    cout << "filePos : " << fileReadStream.tellg() << endl;
                }
            }
        }
    }
    if (debugInfo)
        cout << endl;

    return type;
}

void InitSerializedFile(Stream &fileReadStream)
{
    // ReadHeader
    fileReadStream.ReadByte_B2L(&serializedFile.header.m_MetadataSize, 4);
    fileReadStream.ReadByte_B2L(&serializedFile.header.m_FileSize, 4);
    fileReadStream.ReadByte_B2L(&serializedFile.header.m_Version, 4);
    fileReadStream.ReadByte_B2L(&serializedFile.header.m_DataOffset, 4);

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_9)
    {
        fileReadStream.ReadByte_B2L(&serializedFile.header.m_Endianess, 1);
        fileReadStream.ReadByte_B2L(serializedFile.header.m_Reserved, 3);
    }
    else
    {
        fileReadStream.seekg(serializedFile.header.m_FileSize - serializedFile.header.m_MetadataSize);
        fileReadStream.ReadByte_B2L(&serializedFile.header.m_Endianess, 1);
    }
    serializedFile.m_FileEndianess = serializedFile.header.m_Endianess;

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::LargeFilesSupport)
    {

        fileReadStream.ReadByte_B2L(&serializedFile.header.m_MetadataSize, 4);
        fileReadStream.ReadByte_B2L(&serializedFile.header.m_FileSize, 8);
        fileReadStream.ReadByte_B2L(&serializedFile.header.m_DataOffset, 8);
        fileReadStream.seekg(fileReadStream.tellg() + 8); // unknown data
    }

    // ReadMetadata
    if (serializedFile.m_FileEndianess == 0)
        fileReadStream.Endian = EndianType::LittleEndian;
    else
    {
        cout << "Serialized file is BigEndian but not support." << endl;
        error();
        // fileReadStream.Endian = EndianType::BigEndian;
    }

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_7)
        fileReadStream.stringRead(serializedFile.unityVersion);

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_8)
        fileReadStream.ReadByte(&serializedFile.m_TargetPlatform, 4);

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::HasTypeTreeHashes)
        fileReadStream.ReadByte(&serializedFile.m_EnableTypeTree, 1);

    fileReadStream.ReadByte(&serializedFile.typeCount, 4);

    if (debugInfo)
    {
        serializedFile.Show();
        cout << endl;
    }

    vector<SerializedType> m_Types;
    for (int i = 0; i < serializedFile.typeCount; i++)
        m_Types.push_back(ReadSerializedType(fileReadStream, false));

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_7 && serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
        fileReadStream.ReadByte(&serializedFile.bigIDEnabled, 4);

    // Read Objects
    fileReadStream.ReadByte(&objectCount, 4);
    if (debugInfo)
    {
        cout << "objectCount : " << objectCount << endl;
        cout << endl;
    }

    for (int i = 0; i < objectCount; i++)
    {
        ObjectInfo *pobjectInfo = new ObjectInfo();
        ObjectInfo &objectInfo = *pobjectInfo;
        if (serializedFile.bigIDEnabled != 0)
            fileReadStream.ReadByte(&objectInfo.m_PathID, 8);
        else if (serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
        {
            uint m_PathID = 0;
            fileReadStream.ReadByte(&m_PathID, 4);
            objectInfo.m_PathID = m_PathID;
        }
        else
        {
            fileReadStream.AlignStream(4);
            fileReadStream.ReadByte(&objectInfo.m_PathID, 8);
        }

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::LargeFilesSupport)
            fileReadStream.ReadByte(&objectInfo.byteStart, 8);
        else
        {
            uint byteStart = 0;
            fileReadStream.ReadByte(&byteStart, 4);
            objectInfo.byteStart = byteStart;
        }

        objectInfo.byteStart += serializedFile.header.m_DataOffset;
        fileReadStream.ReadByte(&objectInfo.byteSize, 4);
        fileReadStream.ReadByte(&objectInfo.typeID, 4);
        if (serializedFile.header.m_Version < SerializedFileFormatVersion::RefactoredClassId)
        {
            cout << "Unsupported Serialized File Format Version." << endl;
            error();
        }
        else
        {
            objectInfo.serializedType = m_Types[objectInfo.typeID];
            objectInfo.classID = m_Types[objectInfo.typeID].classID;
        }

        if (serializedFile.header.m_Version < SerializedFileFormatVersion::HasScriptTypeIndex)
            fileReadStream.ReadByte(&objectInfo.isDestroyed, 2);

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::HasScriptTypeIndex && serializedFile.header.m_Version < SerializedFileFormatVersion::RefactorTypeData)
        {
            int16_t m_ScriptTypeIndex = 0;
            fileReadStream.ReadByte(&m_ScriptTypeIndex, 2);
            objectInfo.serializedType.m_ScriptTypeIndex = m_ScriptTypeIndex;
        }
        if (serializedFile.header.m_Version == SerializedFileFormatVersion::SupportsStrippedObject || serializedFile.header.m_Version == SerializedFileFormatVersion::RefactoredClassId)
            fileReadStream.ReadByte(&objectInfo.stripped, 1);

        serializedFile.m_Objects.push_back(objectInfo);
        if (debugInfo)
        {
            objectInfo.Show();
            cout << endl;
        }
    }

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::HasScriptTypeIndex)
    {
        int scriptCount;
        fileReadStream.ReadByte(&scriptCount, 4);
        if (debugInfo)
            cout << "scriptCount : " << scriptCount << endl;
        if (scriptCount != 0)
        {
            cout << "Unsupported Serialized File scriptCount." << endl;
            error();
        }
    }

    int externalsCount;
    fileReadStream.ReadByte(&externalsCount, 4);
    if (debugInfo)
        cout << "externalsCount : " << externalsCount << endl;
    if (externalsCount != 0)
    {
        cout << "Unsupported Serialized File externalsCount." << endl;
        error();
    }

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::SupportsRefObject)
    {
        int refTypesCount;
        fileReadStream.ReadByte(&refTypesCount, 4);
        if (debugInfo)
            cout << "refTypesCount : " << refTypesCount << endl;

        if (refTypesCount != 0)
        {
            cout << "Unsupported Serialized File refTypesCount." << endl;
            error();
        }
    }

    if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_5)
        fileReadStream.stringRead(serializedFile.userInformation);

    cout << endl;
}

void ReadAssets(Stream &fileReadStream)
{
    int length;
    char *pStr;

    for (int i = 0; i < objectCount; i++)
    {
        auto &objectInfo = serializedFile.m_Objects[i];
        fileReadStream.seekg(objectInfo.byteStart);

        // AssetInfo
        if (objectInfo.classID == ClassIDType::AssetBundle)
        {
            fileReadStream.ReadByte(&length, 4);
            if (length > 0 && length <= fileReadStream.sizeg() - fileReadStream.tellg())
            {
                assetBundle.m_Name = new char[length + 1];
                memset(assetBundle.m_Name, 0, length + 1);
                fileReadStream.ReadByte(assetBundle.m_Name, length);
                fileReadStream.AlignStream(4);
            }
            else
            {
                assetBundle.m_Name = new char[2];
                strcpy(assetBundle.m_Name, "#");
            }
            if (debugInfo)
                assetBundle.Show();

            int m_PreloadTableSize;
            fileReadStream.ReadByte(&m_PreloadTableSize, 4);
            if (debugInfo)
                cout << "m_PreloadTableSize : " << m_PreloadTableSize << endl;
            for (int i = 0; i < m_PreloadTableSize; i++)
            {
                PPtr m_PPtr;

                fileReadStream.ReadByte(&m_PPtr.m_FileID, 4);
                if (serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
                {
                    cout << "Unsupported Serialized File Format Version." << endl;
                    error();
                }
                else
                    fileReadStream.ReadByte(&m_PPtr.m_PathID, 8);
                assetBundle.m_PreloadTable.push_back(m_PPtr);

                if (debugInfo)
                {
                    cout << i << " > " << endl;
                    m_PPtr.Show();
                    cout << endl;
                }
            }

            int m_ContainerSize;
            fileReadStream.ReadByte(&m_ContainerSize, 4);
            if (debugInfo)
                cout << "m_ContainerSize : " << m_ContainerSize << endl;
            for (int i = 0; i < m_ContainerSize; i++)
            {
                fileReadStream.ReadByte(&length, 4);
                if (length > 0 && length <= fileReadStream.sizeg() - fileReadStream.tellg())
                {
                    pStr = new char[length + 1];
                    memset(pStr, 0, length + 1);
                    fileReadStream.ReadByte(pStr, length);
                    fileReadStream.AlignStream(4);
                }
                else
                {
                    pStr = new char[2];
                    strcpy(pStr, "#");
                }

                AssetInfo assetInfo;
                fileReadStream.ReadByte(&assetInfo.preloadIndex, 4);
                fileReadStream.ReadByte(&assetInfo.preloadSize, 4);

                fileReadStream.ReadByte(&assetInfo.asset.m_FileID, 4);
                if (serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
                {
                    cout << "Unsupported Serialized File Format Version." << endl;
                    error();
                }
                else
                    fileReadStream.ReadByte(&assetInfo.asset.m_PathID, 8);

                assetBundle.m_Container[pStr] = assetInfo;

                if (debugInfo)
                {
                    cout << i << " > " << endl;
                    cout << "pStr : " << pStr << endl;
                    assetInfo.Show();
                    cout << endl;
                }
            }
        }

        // AudioClip
        else if (objectInfo.classID == ClassIDType::AudioClip)
        {
            fileReadStream.ReadByte(&length, 4);
            if (length > 0 && length <= fileReadStream.sizeg() - fileReadStream.tellg())
            {
                audioClip.m_Name = new char[length + 1];
                memset(audioClip.m_Name, 0, length + 1);
                fileReadStream.ReadByte(audioClip.m_Name, length);
                fileReadStream.AlignStream(4);
            }
            else
            {
                audioClip.m_Name = new char[2];
                strcpy(audioClip.m_Name, "#");
            }

            if (atoi(header.unityVersion) < 5)
            {
                cout << "Unsupported Unity Version." << endl;
                error();
            }
            else
            {
                fileReadStream.ReadByte(&audioClip.m_LoadType, 4);
                fileReadStream.ReadByte(&audioClip.m_Channels, 4);
                fileReadStream.ReadByte(&audioClip.m_Frequency, 4);
                fileReadStream.ReadByte(&audioClip.m_BitsPerSample, 4);
                fileReadStream.ReadByte(&audioClip.m_Length, 4);
                fileReadStream.ReadByte(&audioClip.m_IsTrackerFormat, 1);
                fileReadStream.AlignStream(4);
                fileReadStream.ReadByte(&audioClip.m_SubsoundIndex, 4);
                fileReadStream.ReadByte(&audioClip.m_PreloadAudioData, 1);
                fileReadStream.ReadByte(&audioClip.m_LoadInBackground, 1);
                fileReadStream.ReadByte(&audioClip.m_Legacy3D, 1);
                fileReadStream.AlignStream(4);

                // StreamedResource m_Resource
                fileReadStream.ReadByte(&length, 4);
                if (length > 0 && length <= fileReadStream.sizeg() - fileReadStream.tellg())
                {
                    audioClip.m_Source = new char[length + 1];
                    memset(audioClip.m_Source, 0, length + 1);
                    fileReadStream.ReadByte(audioClip.m_Source, length);
                    fileReadStream.AlignStream(4);
                }
                else
                {
                    audioClip.m_Source = new char[2];
                    strcpy(audioClip.m_Source, "#");
                }

                fileReadStream.ReadByte(&audioClip.m_Offset, 8);
                fileReadStream.ReadByte(&audioClip.m_Size, 8);

                fileReadStream.ReadByte(&audioClip.m_CompressionFormat, 4);
            }

            audioClip.Show();
            cout << endl;
        }

        // TextAsset
        else if (objectInfo.classID == ClassIDType::TextAsset)
        {
            fileReadStream.ReadByte(&length, 4);
            if (length > 0 && length <= fileReadStream.sizeg() - fileReadStream.tellg())
            {
                textAsset.m_Name = new char[length + 1];
                memset(textAsset.m_Name, 0, length + 1);
                fileReadStream.ReadByte(textAsset.m_Name, length);
                fileReadStream.AlignStream(4);
            }
            else
            {
                textAsset.m_Name = new char[2];
                strcpy(textAsset.m_Name, "#");
            }

            fileReadStream.ReadByte(&textAsset.m_Size, 4);
            textAsset.m_Script = new byte[textAsset.m_Size + 1];
            memset(textAsset.m_Script, 0, textAsset.m_Size + 1);
            fileReadStream.ReadByte(textAsset.m_Script, textAsset.m_Size);

            textAsset.Show();
            cout << endl;
        }

        else
        {
            cout << "Unsupported classID." << endl;
            error();
        }
    }
}

void ReadFiles(Stream &blockStream)
{
    byte *byteData;
    bool isAsset;

    for (int i = 0; i < nodesCount; i++)
    {
        Stream *pStreamData = new Stream;
        Node &node = pDirectoryInfo[i];

        blockStream.seekg(node.offset);
        byteData = new byte[node.size];
        blockStream.ReadByte(byteData, node.size);
        pStreamData->WriteByte(byteData, node.size);
        delete[] byteData;

        isAsset = IsSerializedFile(*pStreamData);
        unpackFileList.push_back(unpackFile{string(pDirectoryInfo[i].path), pDirectoryInfo[i].size, isAsset, pStreamData});
    }
}

bool FileCheck(const char *filename)
{
    FILE *pfile = fopen(filename, "r");
    if (!pfile)
        return false;
    fclose(pfile);
    return true;
}

// 返回值类型 unsigned long
_fsize_t GetFilesize(const char *filename)
{
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    if ((hFile = _findfirst(filename, &fileinfo)) != -1)
    {
        return fileinfo.size;
        _findclose(hFile);
    }
    return 0;
}

Stream *SetAudioClip_ReadAudioFile()
{
    Stream *pStreamData = new Stream;

_inputAgain:
    cout << "Enter music filename : ";
    string filename;
    getline(cin, filename, '\n');

    if (!FileCheck(filename.c_str()))
    {
        cout << "Can't open file." << endl;
        goto _inputAgain;
    }

    audioClip.m_Size = GetFilesize(filename.c_str());
    cout << "fileSize : " << audioClip.m_Size << endl;

    cout << "Channels : " << audioClip.m_Channels << endl;
    cout << "Set : ";
    cin >> audioClip.m_Channels;
    cout << audioClip.m_Channels << " : OK" << endl
         << endl;
    cin.clear();

    cout << "Frequency : " << audioClip.m_Frequency << endl;
    cout << "Set : ";
    cin >> audioClip.m_Frequency;
    cout << audioClip.m_Frequency << " : OK" << endl
         << endl;
    cin.clear();

    cout << "BitsPerSample : " << audioClip.m_BitsPerSample << endl;
    cout << "Set : ";
    cin >> audioClip.m_BitsPerSample;
    cout << audioClip.m_BitsPerSample << " : OK" << endl
         << endl;
    cin.clear();

    cout << "Length : " << audioClip.m_Length << endl;
    cout << "Set : ";
    cin >> audioClip.m_Length;
    cout << audioClip.m_Length << " : OK" << endl
         << endl;
    cin.clear();

    cout << "PCM = 0" << endl;
    cout << "Vorbis = 1" << endl;
    cout << "ADPCM = 2" << endl;
    cout << "MP3 = 3" << endl;
    cout << "PSMVAG = 4" << endl;
    cout << "HEVAG = 5" << endl;
    cout << "XMA = 6" << endl;
    cout << "AAC = 7" << endl;
    cout << "GCADPCM = 8" << endl;
    cout << "ATRAC9 = 9" << endl;
    cout << "CompressionFormat : " << audioClip.m_CompressionFormat << endl;
    cout << "Set : ";
    int CompressionFormat;
    cin >> CompressionFormat;
    audioClip.m_CompressionFormat = (AudioCompressionFormat)CompressionFormat;
    cout << audioClip.m_CompressionFormat << " : OK" << endl
         << endl;

    FileReadStream fileReadStream(filename.c_str());

    byte *byteData;
    byteData = new byte[audioClip.m_Size];
    fileReadStream.ReadByte(byteData, audioClip.m_Size);
    pStreamData->WriteByte(byteData, audioClip.m_Size);
    delete[] byteData;

    return pStreamData;
}

void WriteTextAsset_ReadTextFile()
{
    Stream StreamData;

_inputAgain:
    cout << "Enter text filename : ";
    string filename;
    getline(cin, filename, '\n');

    if (!FileCheck(filename.c_str()))
    {
        cout << "Can't open file." << endl;
        goto _inputAgain;
    }
    textAsset.m_Size = GetFilesize(filename.c_str());

    FileReadStream fileReadStream(filename.c_str());

    delete[] textAsset.m_Script;
    textAsset.m_Script = new byte[textAsset.m_Size + 1];
    memset(textAsset.m_Script, 0, textAsset.m_Size + 1);
    fileReadStream.ReadByte(textAsset.m_Script, textAsset.m_Size);
}

void WriteAudioClip(Stream &fileReadStream)
{
    fileReadStream.Endian = (EndianType)serializedFile.m_FileEndianess;
    for (auto objectInfo : serializedFile.m_Objects)
    {
        if (objectInfo.classID == ClassIDType::AudioClip)
        {
            fileReadStream.seekg(objectInfo.byteStart);
            {
                int length;
                fileReadStream.ReadByte(&length, 4);
                fileReadStream.seekg(fileReadStream.tellg() + length);
                fileReadStream.AlignStream(4);
                {
                    fileReadStream.seekg(fileReadStream.tellg() + 4);

                    fileReadStream.WriteByte(&audioClip.m_Channels, 4);
                    fileReadStream.WriteByte(&audioClip.m_Frequency, 4);
                    fileReadStream.WriteByte(&audioClip.m_BitsPerSample, 4);
                    fileReadStream.WriteByte(&audioClip.m_Length, 4);

                    fileReadStream.seekg(fileReadStream.tellg() + 1);
                    fileReadStream.AlignStream(4);
                    fileReadStream.seekg(fileReadStream.tellg() + 7);
                    fileReadStream.AlignStream(4);

                    // StreamedResource m_Resource
                    fileReadStream.ReadByte(&length, 4);
                    fileReadStream.seekg(fileReadStream.tellg() + length);
                    fileReadStream.AlignStream(4);

                    fileReadStream.seekg(fileReadStream.tellg() + 8);
                    fileReadStream.WriteByte(&audioClip.m_Size, 8);
                    fileReadStream.WriteByte(&audioClip.m_CompressionFormat, 4);
                }

                audioClip.Show();
                cout << endl;
            }
        }
    }
}

void WriteTextAsset(Stream &fileReadStream)
{
    int length;
    int textAsset_byteSize;
    byte *byteData;
    Stream streamData;

    for (auto &objectInfo : serializedFile.m_Objects)
        if (objectInfo.classID == ClassIDType::TextAsset)
            textAsset_byteSize = objectInfo.byteStart;

    streamData.Endian = (EndianType)serializedFile.m_FileEndianess;
    for (auto &objectInfo : serializedFile.m_Objects)
    {
        if (objectInfo.classID == ClassIDType::TextAsset)
        {
            fileReadStream.seekg(objectInfo.byteStart);
            {
                streamData.WriteByte(fileReadStream, fileReadStream.tellg());

                length = strlen(textAsset.m_Name);
                streamData.WriteByte(&length, 4);
                streamData.WriteByte(textAsset.m_Name, length);
                streamData.AlignStream(4);

                streamData.WriteByte(&textAsset.m_Size, 4);

                streamData.Endian = EndianType::LittleEndian;
                streamData.WriteByte(textAsset.m_Script, textAsset.m_Size + 1);
                streamData.Endian = (EndianType)serializedFile.m_FileEndianess;

                objectInfo.byteSize = streamData.tellg() - objectInfo.byteStart;
                streamData.AlignStream(4);
            }
        }
        else if (objectInfo.byteStart > textAsset_byteSize && objectInfo.classID == ClassIDType::AssetBundle)
        {
            objectInfo.byteStart = streamData.tellg();

            length = strlen(assetBundle.m_Name);
            streamData.WriteByte(&length, 4);
            streamData.WriteByte(assetBundle.m_Name, length);
            streamData.AlignStream(4);

            int m_PreloadTableSize = assetBundle.m_PreloadTable.size();
            streamData.WriteByte(&m_PreloadTableSize, 4);

            for (auto &m_PPtr : assetBundle.m_PreloadTable)
            {
                streamData.WriteByte(&m_PPtr.m_FileID, 4);
                if (serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
                {
                    cout << "Unsupported Serialized File Format Version." << endl;
                    error();
                }
                else
                    streamData.WriteByte(&m_PPtr.m_PathID, 8);
            }

            int m_ContainerSize = assetBundle.m_Container.size();
            streamData.WriteByte(&m_ContainerSize, 4);

            for (auto &assetInfo : assetBundle.m_Container)
            {
                length = strlen(assetInfo.first);
                streamData.WriteByte(&length, 4);
                streamData.WriteByte(assetInfo.first, length);
                streamData.AlignStream(4);

                streamData.WriteByte(&assetInfo.second.preloadIndex, 4);
                streamData.WriteByte(&assetInfo.second.preloadSize, 4);
                streamData.WriteByte(&assetInfo.second.asset.m_FileID, 4);

                if (serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
                {
                    cout << "Unsupported Serialized File Format Version." << endl;
                    error();
                }
                else
                    streamData.WriteByte(&assetInfo.second.asset.m_PathID, 8);
            }

            // Unknown
            {
                byteData = new byte[20];
                memset(byteData, 0, 20);
                streamData.WriteByte(byteData, 20);
                delete[] byteData;

                int iNumber = 1;
                streamData.WriteByte(&iNumber, 4);

                length = strlen(assetBundle.m_Name);
                streamData.WriteByte(&length, 4);
                streamData.WriteByte(assetBundle.m_Name, length);
                streamData.AlignStream(4);

                uint64_t ui64Number = 0;
                streamData.WriteByte(&ui64Number, 8);

                ui64Number = 1;
                streamData.WriteByte(&ui64Number, 8);

                iNumber = 0;
                streamData.WriteByte(&iNumber, 4);
            }

            objectInfo.byteSize = streamData.tellg() - objectInfo.byteStart;
        }    
    }
    fileReadStream = streamData;

    fileReadStream.seekg(0);
    // InitSerializedFile : Seek
    {
        bool debugInfoFlag = debugInfo;
        debugInfo = false;

        uint fileSize = fileReadStream.sizeg();
        int64_t fileSize64_t = fileReadStream.sizeg();

        // ReadHeader
        fileReadStream.seekg(fileReadStream.tellg() + 4);
        fileReadStream.WriteByte_B2L(&fileSize, 4);
        fileReadStream.seekg(fileReadStream.tellg() + 8);

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_9)
            fileReadStream.seekg(fileReadStream.tellg() + 4);
        else
            fileReadStream.seekg(serializedFile.header.m_FileSize - serializedFile.header.m_MetadataSize + 1);

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::LargeFilesSupport)
        {
            fileReadStream.seekg(fileReadStream.tellg() + 4);
            fileReadStream.WriteByte_B2L(&fileSize64_t, 8);
            fileReadStream.seekg(fileReadStream.tellg() + 16);
        }

        // ReadMetadata
        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_7)
        {
            byte *noneUse;
            fileReadStream.stringRead(noneUse);
            delete[] noneUse;
        }

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_8)
            fileReadStream.seekg(fileReadStream.tellg() + 4);

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::HasTypeTreeHashes)
            fileReadStream.seekg(fileReadStream.tellg() + 1);

        fileReadStream.seekg(fileReadStream.tellg() + 4);

        for (int i = 0; i < serializedFile.typeCount; i++)
            ReadSerializedType(fileReadStream, false);

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::Unknown_7 && serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
            fileReadStream.seekg(fileReadStream.tellg() + 4);

        // Read Objects
        fileReadStream.seekg(fileReadStream.tellg() + 4);

        debugInfo = debugInfoFlag;
    }

    for (auto objectInfo : serializedFile.m_Objects)
    {
        // objectInfo.m_PathID
        if (serializedFile.bigIDEnabled != 0)
            fileReadStream.seekg(fileReadStream.tellg() + 8);
        else if (serializedFile.header.m_Version < SerializedFileFormatVersion::Unknown_14)
            fileReadStream.seekg(fileReadStream.tellg() + 4);
        else
        {
            fileReadStream.AlignStream(4);
            fileReadStream.seekg(fileReadStream.tellg() + 8);
        }

        objectInfo.byteStart -= serializedFile.header.m_DataOffset;
        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::LargeFilesSupport)
            fileReadStream.WriteByte(&objectInfo.byteStart, 8);
        else
        {
            uint byteStart = objectInfo.byteStart;
            fileReadStream.WriteByte(&byteStart, 4);
        }

        fileReadStream.WriteByte(&objectInfo.byteSize, 4);
        fileReadStream.seekg(fileReadStream.tellg() + 4);

        if (serializedFile.header.m_Version < SerializedFileFormatVersion::HasScriptTypeIndex)
            fileReadStream.seekg(fileReadStream.tellg() + 2);

        if (serializedFile.header.m_Version >= SerializedFileFormatVersion::HasScriptTypeIndex && serializedFile.header.m_Version < SerializedFileFormatVersion::RefactorTypeData)
            fileReadStream.seekg(fileReadStream.tellg() + 2);

        if (serializedFile.header.m_Version == SerializedFileFormatVersion::SupportsStrippedObject || serializedFile.header.m_Version == SerializedFileFormatVersion::RefactoredClassId)
            fileReadStream.seekg(fileReadStream.tellg() + 1);

        if (debugInfo)
        {
            objectInfo.Show();
            cout << endl;
        }
    }
}

int main(int argc, char const *argv[])
{
    system("cls");
    if (argc >= 2 && argv[1][0] == 'd')
    {
        debugInfo = true;
        cout << "(DebugInfo)";
    }

    cout << "Enter Phigros bundle filename : ";
    string filename;
    getline(cin, filename, '\n');

    if (!FileCheck(filename.c_str()))
    {
        cout << "Can't open file." << endl;
        return 0;
    }

    Stream mergeStream;

    // 解包bunlde文件
    {
        FileReadStream fileReadStream(filename.c_str());
        Stream blockStream;

        ReadHeader(fileReadStream);
        ReadBlocksInfoAndDirectory(fileReadStream);
        ReadBlocks(fileReadStream, blockStream);
        ReadFiles(blockStream);

        if (debugInfo)
        {
            FileWriteStream fileWriteStream("Debug");
            byte *byteData;
            blockStream.seekg(0);
            byteData = new byte[blockStream.sizeg()];
            blockStream.ReadByte(byteData, blockStream.sizeg());
            fileWriteStream.WriteByte(byteData, blockStream.sizeg());
            delete[] byteData;
        }
    }

    // 替换文件
    {
        if (nodesCount >= 2)
        {
            for (auto &unpackFile : unpackFileList)
                if (unpackFile.isAsset)
                {
                    if (debugInfo)
                        cout << "process " << unpackFile.name << "..." << endl;

                    InitSerializedFile(*unpackFile.pStreamData);
                    ReadAssets(*unpackFile.pStreamData);
                    break;
                }

            for (auto &unpackFile : unpackFileList)
                if (!unpackFile.isAsset)
                {
                    if (debugInfo)
                        cout << "process " << unpackFile.name << "..." << endl;

                    unpackFile.pStreamData = SetAudioClip_ReadAudioFile();
                    unpackFile.size = audioClip.m_Size;
                    break;
                }

            for (auto &unpackFile : unpackFileList)
            {
                if (unpackFile.isAsset)
                {
                    WriteAudioClip(*unpackFile.pStreamData);
                    mergeStream.WriteByte(*unpackFile.pStreamData);
                }
                if (!unpackFile.isAsset)
                    mergeStream.WriteByte(*unpackFile.pStreamData);
            }
        }
        else if (nodesCount == 1)
        {
            auto &unpackFile = unpackFileList[0];
            if (debugInfo)
                cout << "process " << unpackFile.name << "..." << endl;

            InitSerializedFile(*unpackFile.pStreamData);
            ReadAssets(*unpackFile.pStreamData);

            WriteTextAsset_ReadTextFile();
            WriteTextAsset(*unpackFile.pStreamData);
            unpackFile.size = unpackFile.pStreamData->sizeg();
            mergeStream.WriteByte(*unpackFile.pStreamData);
        }
    }

    // 打包bunlde文件
    {
        FileWriteStream fileWriteStream("New.bundle");
        Stream fileData;

        FormatBlocks(mergeStream);

        WriteHeader(fileData);
        WriteBlocksInfoAndDirectory(fileData);

        fileData.WriteByte(mergeStream);

        header.size = fileData.sizeg();
        fileData.seekg(0);
        WriteHeader(fileData);

        byte *byteData;
        fileData.seekg(0);
        byteData = new byte[fileData.sizeg()];
        fileData.ReadByte(byteData, fileData.sizeg());
        fileWriteStream.WriteByte(byteData, fileData.sizeg());
        delete[] byteData;
    }

    cout << "Success.(Press to exit)" << endl;
    cin.get();
    cin.get();

    return 0;
}