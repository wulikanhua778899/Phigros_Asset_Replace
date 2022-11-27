#ifndef SHOWHEX_H_
#define SHOWHEX_H_

#ifdef __cplusplus
namespace ZeLin
{
    /**
     * 以16进制显示指定内存空间
     * @param bData 内存空间地址
     * @param size 内存空间大小
     * @param displaySize 最大显示大小，超出此大小不显示并提示‘...more...’，置0则为不作限制（C++中默认为0）
     * @param title 显示标题行
     * @param columnCount 显示计数列
     * @param ascii 显示ASCII码
     */
    void ShowHex(const char *bData, unsigned int size, unsigned int displaySize = 0, bool title = true, bool columnCount = true, bool ascii = true);
};
#else
void ShowHex(const char *bData, unsigned int size, unsigned int displaySize, _Bool title, _Bool columnCount, _Bool ascii);
#endif

#endif