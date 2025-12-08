#include "Memory.h"

uint32_t Memory::getCurrtext() const
{
    return currtext * 4 + 0x400000;
}

uint32_t Memory::getCurrdata() const
{
    return currdata * 4 + 0x400000 + datacurrbyte;
}

uint32_t Memory::getCurrstack() const
{
    return currstack * 4 + 0x400000;
}

bool Memory::databyte(uint32_t byte)
{
    s[currdata] |= (byte & 255) << ((3 - datacurrbyte) * 8);
    std::cout << '\t' << std::dec << byte << " stored at " << currdata * 4 + 0x400000 + datacurrbyte
              << std::endl;
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
    std::cout << "\tStoring " << std::dec << word << std::endl;
    if(datacurrbyte != 0)
    {
        currdata++;
        datacurrbyte = 0;
    }
    for(int shift = 24; shift >= 0; shift -= 8)
    {
        bool warn = this->databyte(word & (255 << shift));
        if(warn)
            return 1;
    }
    std::cout << "\t" << s[currdata-1] << " stored\n";
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

uint32_t Memory::getByte(uint32_t addr) const
{
    int offset = (3 - addr % 4) * 8;
    addr -= addr % 4;
    return (this->getWord(addr) >> offset) & 255;
}

uint32_t Memory::getWord(uint32_t addr) const
{
    if(addr % 4 != 0 || addr < 0x400000)
        throw AddressException();
    return s[(addr - 0x400000) / 4];
}

void Memory::storeWord(uint32_t val, uint32_t addr)
{
    if(addr % 4 != 0 || addr < 0x400000)
        throw AddressException();
    s[(addr - 0x400000) / 4] = val;
}

void Memory::incText()
{
    currtext++;
}

void Memory::incStack()
{
    currstack--;
}

void Memory::showData() const
{
    std::cout << "*** DATA SEGMENT ***\n"
              << std::setfill('=') << std::setw(67) << "" << std::endl
              << " addr (int) | addr (hex) | value (int) | value (hex) | value (char)\n"
              << std::setfill('-') << std::setw(67) << "" << std::endl;
    for(unsigned int i = 100001; i <= currdata; i++)
    {
        uint32_t addr = i * 4 + 0x400000;
        uint32_t value = this->getWord(addr);
        std::cout << ' ' << std::dec << std::setfill(' ') << std::setw(10)
                  << addr << " | " << std::hex << std::setw(10) << addr
                  << " | " << std::dec << std::setw(11) << value << " | "
                  << std::hex;
        for(int shift = 24; shift >= 0; shift -= 8)
            std::cout << std::setw(2) << ((value >> shift) & 255) << ' ';
        std::cout << "| ";
        for(int shift = 24; shift >= 0; shift -= 8)
        {
            char c = (value >> shift) & 255;
            switch(c)
            {
                case '\n':
                    std::cout << "\\n ";
                    break;
                case '\t':
                    std::cout << "\\t ";
                    break;
                case '\0':
                    std::cout << "\\0 ";
                    break;
                default:
                    std::cout << ' ' << c << ' ';
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::setfill('-') << std::setw(67) << "" << std::endl;
}

void Memory::showStack(uint32_t sp) const
{
    std::cout << "*** CURRENT STACK ***\n"
              << std::setfill('=') << std::setw(52) << "" << std::endl
              << " addr (int) | addr (hex) | value (int) | value (hex)\n"
              << std::setfill('-') << std::setw(52) << "" << std::endl;
    for(unsigned int i = sp; i < 6194304; i += 4)
    {
        uint32_t value = this->getWord(i);
        std::cout << ' ' << std::dec << std::setfill(' ') << std::setw(10)
                  << i << " | " << std::hex << std::setw(10) << i
                  << " | " << std::dec << std::setw(11) << value << " | "
                  << std::hex;
        for(int shift = 24; shift >= 0; shift -= 8)
            std::cout << std::setw(2) << ((value >> shift) & 255) << ' ';
        std::cout << std::endl;
    }
    std::cout << std::setfill('-') << std::setw(52) << "" << std::endl;
}   
