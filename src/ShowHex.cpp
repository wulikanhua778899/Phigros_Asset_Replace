#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "ShowHex.h"

#ifdef __cplusplus
void ZeLin::ShowHex(const char *bData, unsigned int size, unsigned int displaySize, bool title, bool columnCount, bool ascii)
#else
void ShowHex(const char *bData, unsigned int size, unsigned int displaySize, bool title, bool columnCount, bool ascii)
#endif
{
    bool moreFlag = false;
    if (displaySize != 0 && size > displaySize)
    {
        size = displaySize;
        moreFlag = true;
    }

    unsigned int i = 0, j = 0;
    if (title)
    {
        if (columnCount)
            printf_s("---- > ");
        printf_s("0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F\n");
    }
    for (i = 0; i < size; i++)
    {
        if (i % 16 == 0 && i != 0)
        {
            if (ascii)
            {
                printf_s("| ");
                for (j = i - 16; j < i; j++)
                {
                    if (isprint(bData[j]))
                        printf_s("%c", bData[j]);
                    else
                        printf_s(" ");
                }
            }
            printf_s("\n");
        }
        else if (i % 8 == 0 && i != 0)
            printf_s("- ");
        if (i % 16 == 0 && columnCount)
            printf_s("%04d > ", i);
        printf_s("%02X ", (unsigned char)bData[i]);
    }

    if (ascii)
    {
        if (i % 16 != 0)
        {
            j = i;
            i = i + 16 - (i % 16);
            for (; j < i; j++)
            {
                if (j % 8 == 0 && j != 0)
                    printf_s("  ");
                printf_s("   ");
            }
        }
        else
        {
            j = i;
            i = i + 16 - (i % 16);
        }

        printf_s("| ");
        for (j = i - 16; j < size; j++)
        {
            if (isprint(bData[j]))
                printf_s("%c", bData[j]);
            else
                printf_s(" ");
        }
    }

    printf_s("\n");
    if (moreFlag)
        printf_s("...more...\n");
}