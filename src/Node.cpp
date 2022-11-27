#include <iostream>
#include "Node.h"

using namespace std;

void Node::Show()
{
        cout << "offset : " << offset << endl;
        cout << "size : " << size << endl;
        cout << "flags : " << flags << endl;
        cout << "path : " << path << endl;
}