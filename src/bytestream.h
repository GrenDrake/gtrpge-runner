#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include <cstdint>
#include <iosfwd>
#include <vector>

class ByteStream {
public:
    void add_8(uint8_t value);
    void add_16(uint16_t value);
    void add_32(uint32_t value);
    void append(const ByteStream &other);
    void padTo(unsigned toMultiple);
    int read_8(unsigned where) const;
    int read_16(unsigned where) const;
    int read_32(unsigned where) const;
    void overwrite_8(unsigned where, uint32_t value);
    void overwrite_16(unsigned where, uint32_t value);
    void overwrite_32(unsigned where, uint32_t value);
    unsigned size() const;
    void write(std::ostream &out) const;

    void dump(std::ostream &out, int indentSize = 0) const;
private:
    std::vector<uint8_t> data;
};

#endif
