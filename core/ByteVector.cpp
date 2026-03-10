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

std::string ByteVector::toString(Encoding encoding) const
{
    if (encoding == Encoding::ASCII || encoding == Encoding::UTF8)
    {
        return std::string(this->begin(), this->end());
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
