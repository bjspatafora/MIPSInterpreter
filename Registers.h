#ifndef REGISTERS_H
#define REGISTERS_H

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>

class RegisterException {};

class Registers
{
  public:
    Registers()
    {
        for(int i = 0; i < 32; i++)
            r[i] = 0;
    }
    uint32_t operator[](int) const;
    uint32_t & operator[](int);
    static uint32_t getRegNum(const std::string &);
    
  private:
    uint32_t r[32];
};

std::ostream & operator<<(std::ostream &, const Registers &);

#endif
