#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <string>

class Memory
{
  public:
    Memory()
    {
        currtext = &s[0];
        textend = &s[100000];
        currdata = &s[100001];
        currstack = &s[499999];
        datacurrbyte = 0;
    }

    uint32_t getCurrtext() const;
    uint32_t getCurrdata() const;
    bool databyte(uint32_t);
    bool dataword(uint32_t);
    bool dataasciiz(const std::string &);
    bool dataspace(uint32_t);

  private:
    uint32_t s[500000];
    uint32_t * currtext;
    uint32_t * textend;
    uint32_t * currdata;
    uint32_t datacurrbyte;
    uint32_t * currstack;
};

#endif
