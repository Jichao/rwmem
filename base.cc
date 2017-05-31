#include <string>
#include <bitset>
#include <sstream>

void quad_memcpy( uint32_t * const out, const uint32_t * const in, size_t len)
{
	for (size_t i = 0 ; i < len ; i += 4)
	{
		out[i/4] = in[i/4];
	}
}

std::string int2bits(unsigned int value)
{
    std::bitset<32> x(value);
    std::string str =
            x.to_string<char,std::string::traits_type,std::string::allocator_type>();
    int pos = 4;
    while (pos < str.length()) {
        str.insert(pos, ",");
        pos += 5;
    }
    return str;
}

std::string int_to_bool_str(int value)
{
    if (value)
        return "enabled";
    else
        return "disabled";
}

std::string uint64_to_readable_mem(uint64_t value)
{
    if (!value) {
        return "0";
    }
    std::stringstream result;
    auto gb = value / (1 << 30);
    if (gb) {
        result << gb << "GB";
        value -= gb * (1 << 30);
    }
    auto mb = value / (1 << 20);
    if (mb) {
        result << " " << mb << "MB";
        value -= mb * (1 << 20);
    }

    auto kb = value / (1 << 10);
    if (kb) {
        result << " " << kb << "KB";
        value -= kb * (1 << 10);
    }
    return result.str();
}
uint32_t getbits(uint32_t value, int startBit, int endBit) {
    value = value & (-1 << startBit);
    value = value & ~(-1 << endBit);
    value = value >> startBit;
    return value;
}
