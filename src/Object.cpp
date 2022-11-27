#include <iostream>
#include "Object.h"

using namespace std;

void ObjectInfo::Show()
{
    cout << "PathID : " << m_PathID << endl;
    cout << "byteStart : " << byteStart << endl;
    cout << "byteSize : " << byteSize << endl;
    cout << "typeID : " << typeID << endl;
    cout << "classID : " << classID << endl;
}

void PPtr::Show()
{
    cout << "FileID : " << m_FileID << endl;
    cout << "PathID : " << m_PathID << endl;
}

void AssetInfo::Show()
{
    cout << "preloadIndex : " << preloadIndex << endl;
    cout << "preloadSize : " << preloadSize << endl;
    cout << "-----------asset-----------" << endl;
    asset.Show();
    cout << "---------------------------" << endl;
}

void AssetBundle::Show()
{
    cout << "Name : " << m_Name << endl;
}

void AudioClip::Show()
{
    cout << "Name : " << m_Name << endl;
    cout << "LoadType : " << m_LoadType << endl;
    cout << "Channels : " << m_Channels << endl;
    cout << "Frequency : " << m_Frequency << endl;
    cout << "BitsPerSample : " << m_BitsPerSample << endl;
    cout << "Length : " << m_Length << endl;
    cout << "IsTrackerFormat : " << m_IsTrackerFormat << endl;
    cout << "SubsoundIndex : " << m_SubsoundIndex << endl;
    cout << "PreloadAudioData : " << m_PreloadAudioData << endl;
    cout << "LoadInBackground : " << m_LoadInBackground << endl;
    cout << "Legacy3D : " << m_Legacy3D << endl;
    cout << "Source : " << m_Source << endl;
    cout << "Offset : " << m_Offset << endl;
    cout << "Size : " << m_Size << endl;
    cout << "CompressionFormat : " << m_CompressionFormat << endl;
}

void TextAsset::Show()
{
    cout << "Name : " << m_Name << endl;
    // cout << "Script : " << m_Script << endl;
}