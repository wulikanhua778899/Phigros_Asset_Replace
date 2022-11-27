#include <cstring>
#include "Stream.h"
#include "BigLittleEndian.h"

#include "ShowHex.h"

using namespace std;

// 初始化流
void Stream::init(byte *m_bData, unsigned int m_size)
{
    if (bData)
        delete[] bData;

    size = m_size;
    pos = 0;
    eofFlag = false;

    bData = new byte[m_size];

    // for(unsigned int i = 0;i < m_size;i++)
    //     bData[i] = m_bData[i];
    memcpy(bData, m_bData, m_size);
}

// 从流中读取字节
Stream &Stream::read(byte *outData, unsigned int m_size)
{
    for (unsigned int i = 0; i < m_size; i++)
    {
        outData[i] = bData[pos++];
        if (pos > size)
        {
            eofFlag = true;
            break;
        }
    }
    return *this;
}

// 向流中写入字节
Stream &Stream::write(const byte *inData, unsigned int m_size)
{
    unsigned int lastSize = size - pos;
    if (m_size > lastSize)
    {
        if (bData)
        {
            byte *new_bData = new byte[size + m_size - lastSize];
            memcpy(new_bData, bData, size);
            delete[] bData;
            bData = new_bData;
            clear();
        }
        else
            bData = new byte[m_size];

        size += m_size - lastSize;
    }
    memcpy(bData + pos, inData, m_size);
    pos += m_size;

    return *this;
}

void Stream::ReadByte(void *access, unsigned int size)
{
    this->read((byte *)access, size);
}

void Stream::ReadByte_B2L(void *access, unsigned int size)
{
    this->read((byte *)access, size);
    B2L_Endian(access, size);
}

// 需要手动delete[]
void Stream::stringRead(char *&pStr)
{
    byte oneTempByte;
    unsigned int index = 0;
    auto origalPos = this->tellg();
    while (!this->read(&oneTempByte, 1).eof())
    {
        index++;
        if (oneTempByte == '\0')
            break;
    }
    this->seekg(origalPos);
    pStr = new char[index];
    this->ReadByte(pStr, index);
}

void Stream::WriteByte(Stream &m_stream)
{
    byte *byteData = new byte[m_stream.sizeg()];
    auto origalPos = m_stream.tellg();
    auto origalEndian = m_stream.Endian;
    m_stream.Endian = EndianType::LittleEndian;
    m_stream.seekg(0);

    m_stream.ReadByte(byteData, m_stream.sizeg());
    this->WriteByte(byteData, m_stream.sizeg());

    m_stream.Endian = origalEndian;
    m_stream.seekg(origalPos);
    delete[] byteData;
}

void Stream::WriteByte(Stream &m_stream, unsigned int size)
{
    if (size > m_stream.sizeg())
        WriteByte(m_stream);
    else
    {
        byte *byteData = new byte[size];
        auto origalPos = m_stream.tellg();
        auto origalEndian = m_stream.Endian;
        m_stream.Endian = EndianType::LittleEndian;
        m_stream.seekg(0);

        m_stream.ReadByte(byteData, size);
        this->WriteByte(byteData, size);

        m_stream.Endian = origalEndian;
        m_stream.seekg(origalPos);
        delete[] byteData;
    }
}

void Stream::WriteByte(const void *access, unsigned int size)
{
    if (Endian == EndianType::BigEndian)
        WriteByte_B2L(access, size);
    else
        this->write((byte *)access, size);
}

void Stream::WriteByte_B2L(const void *access, unsigned int size)
{
    byte *m_access = new byte[size];
    memcpy(m_access, access, size);
    B2L_Endian(m_access, size);
    this->write(m_access, size);
    delete[] m_access;
}

void Stream::stringWrite(const char *pStr)
{
    WriteByte(pStr, strlen(pStr) + 1);
}

void Stream::AlignStream(int alignment)
{
    unsigned int mod = pos % alignment;
    if (mod != 0)
    {
        byte *byteZero = new byte[alignment - mod];
        memset(byteZero, 0, alignment - mod);
        WriteByte(byteZero, alignment - mod);
        delete[] byteZero;
    }
}

void Stream::ShowHex()
{
    ZeLin::ShowHex(bData, size);
}

Stream &Stream::operator=(const Stream &m_stream)
{
    eofFlag = m_stream.eofFlag;
    pos = m_stream.pos;
    size = m_stream.size;
    Endian = m_stream.Endian;

    if (bData)
        delete[] bData;

    bData = new byte[m_stream.size];
    memcpy(bData, m_stream.bData, m_stream.size);
    return *this;
}

void FileReadStream::ReadByte(void *access, unsigned int size)
{
    if (Endian == EndianType::BigEndian)
        ReadByte_B2L(access, size);
    else
        this->read((byte *)access, size);
}

void FileReadStream::ReadByte_B2L(void *access, unsigned int size)
{
    this->read((byte *)access, size);
    B2L_Endian(access, size);
}

// 需要手动delete[]
void FileReadStream::stringRead(char *&pStr)
{
    byte oneTempByte;
    unsigned int index = 0;
    auto origalPos = this->tellg();
    while (!this->read(&oneTempByte, 1).eof())
    {
        index++;
        if (oneTempByte == '\0')
            break;
    }
    this->seekg(origalPos);
    pStr = new char[index];
    this->ReadByte(pStr, index);
}

/**
 * 流指针对齐
 * @param {int} alignment 对齐字节量
 */
void FileReadStream::AlignStream(int alignment)
{
    std::streampos pos = this->tellg();
    std::streampos mod = pos % alignment;
    if (mod != 0)
        this->seekg(pos + (std::streampos)alignment - mod);
}

void FileWriteStream::WriteByte(const void *access, unsigned int size)
{
    if (Endian == EndianType::BigEndian)
        WriteByte_B2L(access, size);
    else
    {
        this->write((byte *)access, size);
        pos += size;
    }
}

void FileWriteStream::WriteByte_B2L(const void *access, unsigned int size)
{
    byte *m_access = new byte[size];
    memcpy(m_access, access, size);
    B2L_Endian(m_access, size);
    this->write(m_access, size);
    pos += size;
}

void FileWriteStream::stringWrite(const char *pStr)
{
    WriteByte(pStr, strlen(pStr) + 1);
}

void FileWriteStream::AlignStream(int alignment)
{
    unsigned int mod = pos % alignment;
    if (mod != 0)
    {
        byte *byteZero = new byte[alignment - mod];
        memset(byteZero, 0, alignment - mod);
        WriteByte(byteZero, alignment - mod);
        delete[] byteZero;
    }
}