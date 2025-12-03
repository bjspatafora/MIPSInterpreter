#include "Memory.h"

uint32_t Memory::getCurrtext() const
{
    return (currtext - &s[0]) * 4 + 0x400000;
}

uint32_t Memory::getCurrdata() const
{
    return (currdata - textend) * 4 + 0x400000 + 400000 + datacurrbyte;
}

bool Memory::databyte(uint32_t byte)
{
    *currdata |= (byte & 127) << (datacurrbyte * 8);
    datacurrbyte++;
    if(datacurrbyte == 4)
    {
        currdata++;
        datacurrbyte = 0;
    }
    return currdata >= currstack;
}

bool Memory::dataword(uint32_t word)
{
    if(datacurrbyte != 0)
    {
        currdata++;
        datacurrbyte = 0;
    }
    *currdata = word;
    currdata++;
    return currdata >= currstack;
}

bool Memory::dataasciiz(const std::string & str)
{
    for(auto c : str)
    {
        bool max = this->databyte(c);
        if(max)
            return 0;
    }
    return this->databyte(0);
}

bool Memory::dataspace(uint32_t amount)
{
    for(unsigned int i = 0; i < amount; i++)
    {
        bool max = this->databyte(0);
        if(max)
            return 0;
    }
    return currdata >= currstack;
}
