#ifndef NODE_H_
#define NODE_H_

#include <stdint.h>

class Node
{
private:
    typedef unsigned int uint;
    typedef char *string;

public:
    int64_t offset;
    int64_t size;
    uint flags;
    string path = nullptr;

    void Show();

    ~Node()
    {
        if (path)
            delete[] path;
    };
};

#endif