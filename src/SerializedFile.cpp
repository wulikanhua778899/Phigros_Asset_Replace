#include <iostream>
#include "SerializedFile.h"
#include "ShowHex.h"

using namespace std;
using namespace ZeLin;

void TypeTreeNode::Show()
{
    cout << "Type : " << m_Type << endl;
    cout << "Name : " << m_Name << endl;
    cout << "m_ByteSize : " << m_ByteSize << endl;
    cout << "Index : " << m_Index << endl;
    cout << "TypeFlags : " << m_TypeFlags << endl;
    cout << "Version : " << m_Version << endl;
    cout << "MetaFlag : ";
    ShowHex((char *)&m_MetaFlag, 4, 0, false, false, false);
    cout << "Level : " << m_Level << endl;
    cout << "TypeStrOffset : ";
    ShowHex((char *)&m_TypeStrOffset, 4, 0, false, false, false);
    cout << "NameStrOffset : ";
    ShowHex((char *)&m_NameStrOffset, 4, 0, false, false, false);
    cout << "RefTypeHash : " << m_RefTypeHash << endl;
}

void SerializedFileHeader::Show()
{
    cout << "MetadataSize : " << m_MetadataSize << endl;
    cout << "FileSize : " << m_FileSize << endl;
    cout << "Version : " << m_Version << endl;
    cout << "DataOffset : " << m_DataOffset << endl;
    cout << "Endianess : " << (uint)m_Endianess << endl;
    cout << "Reserved : ";
    ShowHex(m_Reserved, 3, 0, false, false, false);
}

void SerializedType::Show()
{
    cout << "classID : " << classID << endl;
    cout << "IsStrippedType : " << m_IsStrippedType << endl;
    cout << "ScriptTypeIndex : " << m_ScriptTypeIndex << endl;
    cout << "ScriptID : ";
    ShowHex(m_ScriptID, 16, 0, false, false, false);

    cout << "OldTypeHash : ";
    ShowHex(m_OldTypeHash, 16, 0, false, false, false);
    cout << endl;
}

void SerializedFile::Show()
{
    cout << "-----------header-----------" << endl;
    header.Show();
    cout << "----------------------------" << endl;
    cout << "unityVersion : " << unityVersion << endl;
    cout << "TargetPlatform : " << m_TargetPlatform << endl;
    cout << "EnableTypeTree : " << m_EnableTypeTree << endl;
    cout << "typeCount : " << typeCount << endl;
    if (userInformation)
        cout << "userInformation : " << userInformation << endl;
}