#include <ostream>
#include <iomanip>
#include <cstdint>

#include "bytestream.h"

void ByteStream::add_8(uint8_t value) {
    data.push_back(value);
}

void ByteStream::add_16(uint16_t value) {
    data.push_back(value & 0xFF);
    data.push_back((value >> 8) & 0xFF);
}

void ByteStream::add_32(uint32_t value) {
    data.push_back(value & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    data.push_back((value >> 16) & 0xFF);
    data.push_back((value >> 24) & 0xFF);
}

void ByteStream::append(const ByteStream &other) {
    data.insert(data.end(), other.data.begin(), other.data.end());
}

void ByteStream::padTo(unsigned toMultiple) {
    if (toMultiple == 0) return;
    while (data.size() == 0 || data.size() % toMultiple != 0) {
        data.push_back(0);
    }
}

int ByteStream::read_8(unsigned where) const {
    if (where > data.size()) return 0;
    return data[where];
}

int ByteStream::read_16(unsigned where) const {
    if (where + 1 > data.size()) return 0;
    uint32_t value = 0;
    value |= data[where];
    ++where;
    value |= data[where] << 8;
    return value;
}

int ByteStream::read_32(unsigned where) const {
    if (where + 3 > data.size()) return 0;
    uint32_t value = 0;
    value |= data[where];
    ++where;
    value |= data[where] << 8;
    ++where;
    value |= data[where] << 16;
    ++where;
    value |= data[where] << 24;
    return value;
}

void ByteStream::overwrite_8(unsigned where, uint32_t value) {
    if (where >= data.size()) return;
    data[where] = value;
}

void ByteStream::overwrite_16(unsigned where, uint32_t value) {
    if (where + 1 >= data.size()) return;
    data[where]     = value & 0xFF;
    data[where + 1] = (value >> 8) & 0xFF;
}

void ByteStream::overwrite_32(unsigned where, uint32_t value) {
    if (where + 3 >= data.size()) return;
    data[where]     = value & 0xFF;
    data[where + 1] = (value >> 8) & 0xFF;
    data[where + 2] = (value >> 16) & 0xFF;
    data[where + 3] = (value >> 24) & 0xFF;
}

unsigned ByteStream::size() const {
    return data.size();
}

void ByteStream::write(std::ostream &out) const {
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void ByteStream::dump(std::ostream &out, int indentSize) const {
    int oldFill = out.fill();
    out.fill('0');
    out << std::hex;
    for (unsigned i = 0; i < data.size(); ++i) {
        if (i % 16 == 0) {
            out << '\n';
            for (int i = 0; i < indentSize; ++i) out << ' ';
            out << std::setw(4) << i << "  ";
        } else if (i % 8 == 0) {
            out << "  ";
        }
        out << ' ' << std::setw(2) << static_cast<int>(data[i]);
    }
    out << '\n' << std::dec;
    out.fill(oldFill);
}
