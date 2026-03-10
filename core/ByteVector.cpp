#include "ByteVector.h"
#include <cstdint>
#include <sstream>
#include <stdexcept>

std::string ByteVector::stringify() const
{
    std::ostringstream stream;
    for (auto val : *this)
    {
        stream << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)val << " ";
    }
    std::string string = stream.str();
    string.pop_back();
    return string;
}

// Converts a UTF-16LE byte sequence to a UTF-8 string.
static std::string utf16leToUtf8(const uint8_t *data, size_t size)
{
    if (size % 2 != 0)
        throw std::runtime_error("UTF-16LE data must have an even number of bytes.");

    std::string result;
    result.reserve(size);

    for (size_t i = 0; i < size; i += 2)
    {
        uint16_t unit = static_cast<uint16_t>(data[i]) | (static_cast<uint16_t>(data[i + 1]) << 8);
        uint32_t codepoint;

        if (unit >= 0xD800 && unit <= 0xDBFF) // high surrogate
        {
            if (i + 4 > size)
                throw std::runtime_error("Incomplete surrogate pair in UTF-16LE data.");
            uint16_t low = static_cast<uint16_t>(data[i + 2]) | (static_cast<uint16_t>(data[i + 3]) << 8);
            if (low < 0xDC00 || low > 0xDFFF)
                throw std::runtime_error("Invalid surrogate pair in UTF-16LE data.");
            codepoint = 0x10000u + ((static_cast<uint32_t>(unit - 0xD800) << 10) | (low - 0xDC00));
            i += 2;
        }
        else if (unit >= 0xDC00 && unit <= 0xDFFF) // lone low surrogate
        {
            throw std::runtime_error("Unexpected low surrogate in UTF-16LE data.");
        }
        else
        {
            codepoint = unit;
        }

        if (codepoint <= 0x7F)
        {
            result.push_back(static_cast<char>(codepoint));
        }
        else if (codepoint <= 0x7FF)
        {
            result.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0xFFFF)
        {
            result.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else
        {
            result.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
    }

    return result;
}

// Returns true if [data, data+size) is a valid UTF-8 sequence.
static bool isValidUtf8(const uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size;)
    {
        uint8_t b = data[i];
        size_t seqLen;
        if (b < 0x80)         { seqLen = 1; }
        else if (b < 0xC2)    { return false; } // continuation or overlong
        else if (b < 0xE0)    { seqLen = 2; }
        else if (b < 0xF0)    { seqLen = 3; }
        else if (b < 0xF5)    { seqLen = 4; }
        else                  { return false; }

        if (i + seqLen > size) return false;
        for (size_t j = 1; j < seqLen; ++j)
            if ((data[i + j] & 0xC0) != 0x80) return false;
        i += seqLen;
    }
    return true;
}

// Interprets every byte as a Latin-1 (ISO-8859-1) code point and returns
// the equivalent UTF-8 string.  All 256 byte values are valid Latin-1, so
// this never fails.
static std::string latin1ToUtf8(const uint8_t *data, size_t size)
{
    std::string result;
    result.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        uint8_t b = data[i];
        if (b < 0x80)
        {
            result.push_back(static_cast<char>(b));
        }
        else
        {
            // U+0080..U+00FF encoded as 2-byte UTF-8
            result.push_back(static_cast<char>(0xC0 | (b >> 6)));
            result.push_back(static_cast<char>(0x80 | (b & 0x3F)));
        }
    }
    return result;
}

std::string ByteVector::toString(Encoding encoding) const
{
    if (encoding == Encoding::ASCII || encoding == Encoding::UTF8)
    {
        const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this->data());
        size_t size = this->size();
        // If the bytes are already valid UTF-8 (or pure ASCII), use them directly.
        // Otherwise fall back to Latin-1 interpretation, which is common in older
        // medical devices that label text as "UTF-8" but write Windows-1252/Latin-1.
        if (isValidUtf8(bytes, size))
            return std::string(this->begin(), this->end());
        return latin1ToUtf8(bytes, size);
    }
    else if (encoding == Encoding::UTF16LE)
    {
        return utf16leToUtf8(this->data(), this->size());
    }
    else
    {
        throw std::runtime_error("Unsupported encoding.");
    }
}
