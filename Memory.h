#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>

class AddressException {};

class Memory
{
  public:
    Memory()
    {
        for(unsigned int i = 0; i < 500000; i++)
            s[i] = 0;
        currtext = 0;
        textend = 100000;
        currdata = 100001;
        currstack = 499999;
        datacurrbyte = 0;
    }

    uint32_t getCurrtext() const;
    uint32_t getCurrdata() const;
    uint32_t getCurrstack() const;

    bool databyte(uint32_t);
    bool dataword(uint32_t);
    bool dataasciiz(const std::string &);
    bool dataspace(uint32_t);

    uint32_t getByte(uint32_t) const;
    uint32_t getWord(uint32_t) const;

    void storeByte(uint32_t, uint32_t);
    void storeWord(uint32_t, uint32_t);

    void incText();
    void incStack();

    void reset();

    void showData() const;
    void showStack(uint32_t) const;
    
  private:
    uint32_t s[500000];
    uint32_t currtext;
    uint32_t textend;
    uint32_t currdata;
    uint32_t datacurrbyte;
    uint32_t currstack;
};

#endif
