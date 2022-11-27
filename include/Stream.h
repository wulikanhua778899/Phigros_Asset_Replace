#ifndef STREAM_H_
#define STREAM_H_

#include <iostream>
#include <fstream>

enum EndianType
{
    LittleEndian,
    BigEndian
};

class Stream
{
private:
    typedef char byte;   
    bool eofFlag = false;
    unsigned int size = 0;
    unsigned int pos = 0;
    byte *bData = nullptr;

    // 缓存区（暂未实现）
    // byte *buffer;
    // unsigned int bufferSize;
    // unsigned int bufferPos;

public:   
    EndianType Endian = EndianType::LittleEndian;

    void init(byte *m_bData, unsigned int m_size);
    Stream &read(byte *outData, unsigned int m_size);
    Stream &write(const byte *inData, unsigned int m_size);
    inline unsigned int sizeg() { return size; };
    inline bool eof() { return eofFlag; };
    inline void clear() { eofFlag = false; };
    inline unsigned int tellg() { return pos; };
    inline void seekg(unsigned int m_pos) { pos = m_pos; };

    void ReadByte(void *access, unsigned int size);
    void ReadByte_B2L(void *access, unsigned int size);
    void stringRead(char *&pStr);

    void WriteByte(const void *access, unsigned int size);
    void WriteByte(Stream &m_stream);
    void WriteByte(Stream &m_stream, unsigned int size);
    void WriteByte_B2L(const void *access, unsigned int size);
    void stringWrite(const char *pStr);

    void AlignStream(int alignment);

    void ShowHex();

    Stream & operator=(const Stream &m_stream);

    Stream(){};
    Stream(byte *m_bData, unsigned int m_size)
    {
        init(m_bData, m_size);
    };
    ~Stream()
    {
        if (bData)
            delete[] bData;
    };
};

class FileReadStream : public std::ifstream
{
private:
    typedef char byte;

public:
    EndianType Endian = EndianType::LittleEndian;

    FileReadStream();
    FileReadStream(const char *m_filePath) : std::ifstream(m_filePath, ios_base::in | ios_base::binary){};
    ~FileReadStream() { this->std::ifstream::close(); };

    inline void open(const char *m_filePath)
    {
        this->std::ifstream::open(m_filePath, ios_base::in | ios_base::binary);
    }

    void ReadByte(void *access, unsigned int size);
    void ReadByte_B2L(void *access, unsigned int size);
    void stringRead(char *&pStr);
    void AlignStream(int alignment);
};

class FileWriteStream : public std::ofstream
{
private:
    typedef char byte;
    unsigned int pos = 0;

public:
    EndianType Endian = EndianType::LittleEndian;

    FileWriteStream();
    FileWriteStream(const char *m_filePath) : std::ofstream(m_filePath, ios_base::out | ios_base::binary){};
    ~FileWriteStream() { this->std::ofstream::close(); };

    inline void open(const char *m_filePath)
    {
        this->std::ofstream::open(m_filePath, ios_base::out | ios_base::binary);
    }

    void WriteByte(const void *access, unsigned int size);
    void WriteByte_B2L(const void *access, unsigned int size);
    void stringWrite(const char *pStr);
    void AlignStream(int alignment);
};

#endif