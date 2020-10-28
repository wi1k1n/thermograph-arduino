#ifndef RBCARRAY_H
#define RBCARRAY_H

#include <BCArray.h>

// Round Byte Compressed Array
// This is just a small wrapper to incapsulate the cursor wrapping (eating its tail if not enough space)
class RBCArray: public BCArray {
    public:
        RBCArray(const uint16_t byteLength, const uint8_t &cap = 8);
        ~RBCArray();

        void push(const uint8_t val);  // lazily pushes value and move cursor (it will override tail if not enough space)
        void put(const uint16_t idx, const uint8_t val);  // inserts data by index
        uint8_t get(const uint16_t idx);  // returns data by index
        void clear();  // lazily clears data

        uint16_t count();  // the exact number of stored values
    private:
        uint16_t _cursor = 0;
        boolean _cycled = false;
};


RBCArray::RBCArray(const uint16_t byteLength, const uint8_t &cap = 8)
            :BCArray(cap, byteLength * 8 / cap) {
}
RBCArray::~RBCArray() {}

void RBCArray::push(const uint8_t val) {
    if (_cycled) {
        BCArray::put(_cursor % length(), val);
    } else {
        BCArray::put(_cursor, val);
    }
    if (_cursor == BCArray::length() - 1) {
        _cycled = true;
        _cursor = 0;
    } else _cursor++;
}

void RBCArray::put(const uint16_t idx, const uint8_t val) {
    if (_cycled) {
        BCArray::put((_cursor + idx) % length(), val);
    } else {
        BCArray::put(idx % _cursor, val);
    }
}

uint8_t RBCArray::get(const uint16_t idx) {
    if (_cycled) {
        return BCArray::get((_cursor + idx) % length());
    } else {
        return BCArray::get(idx % _cursor);
    }
}

void RBCArray::clear() {
    _cursor = 0;
    _cycled = false;
}
uint16_t RBCArray::count() {
    return _cycled ? length() : _cursor;
}

#endif // RBCARRAY_H