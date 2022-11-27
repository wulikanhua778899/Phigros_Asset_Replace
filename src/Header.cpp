#include <iostream>
#include "Header.h"

using namespace std;

void Header::Show()
{
    cout << "signature : " << signature << endl;
    cout << "version : " << version << endl;
    cout << "unityVersion : " << unityVersion << endl;
    cout << "unityRevision : " << unityRevision << endl;
    cout << "size : " << size << endl;
    cout << "compressedBlocksInfoSize : " << compressedBlocksInfoSize << endl;
    cout << "uncompressedBlocksInfoSize : " << uncompressedBlocksInfoSize << endl;
    hex(cout);
    cout << "flags : 0x" << flags << endl;
    dec(cout);
}