#include "BigLittleEndian.h"

typedef char byte;

void B2L_Endian(void *m_access, unsigned int size)
{
    byte oneTempByte;
    byte *access = (byte *)m_access;
    for (unsigned int i = 0; i < size / 2; i++)
    {
        oneTempByte = access[size - 1 - i];
        access[size - 1 - i] = access[i];
        access[i] = oneTempByte;
    }
}