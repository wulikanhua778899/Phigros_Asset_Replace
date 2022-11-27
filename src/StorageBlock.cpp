#include <iostream>
#include "StorageBlock.h"

using namespace std;

void StorageBlock::Show()
{
    cout << "uncompressedSize : " << uncompressedSize << endl;
    cout << "compressedSize : " << compressedSize << endl;
    cout << "flags : " << flags << endl;
}