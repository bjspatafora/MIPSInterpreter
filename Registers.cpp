#include "Registers.h"

uint32_t & Registers::operator[](const std::string & s)
{
    if(s[0] <= '9' && s[0] >= '0')
        return r[std::stoi(s)];
    int i;
    switch(s[0])
    {
        case 'v':
            return r[2+(s[1]-'0')];
        case 'a':
            return r[4+(s[1]-'0')];
        case 't':
            i = s[1] - '0';
            if(i < 8)
                return r[8+i];
            return r[16+i];
        case 's':
            if(s[1] == 'p')
                return r[29];
            return r[16+(s[1]-'0')];
        case 'g':
            return r[28];
        case 'f':
            return r[30];
        case 'r':
            return r[31];
    }
}

uint32_t Registers::operator[](const std::string & s) const
{
    if(s[0] <= '9' && s[0] >= '0')
        return r[std::stoi(s)];
    int i;
    switch(s[0])
    {
        case 'z':
            return r[0];
        case 'v':
            return r[2+(s[1]-'0')];
        case 'a':
            return r[4+(s[1]-'0')];
        case 't':
            i = s[1] - '0';
            if(i < 8)
                return r[8+i];
            return r[16+i];
        case 's':
            if(s[1] == 'p')
                return r[29];
            return r[16+(s[1]-'0')];
        case 'g':
            return r[28];
        case 'f':
            return r[30];
        case 'r':
            return r[31];
    }
}

uint32_t Registers::operator[](int i) const
{
    return r[i];
}

std::ostream & operator<<(std::ostream & cout, const Registers & reg)
{
    cout << "*** REGISTERS ***\n"
         << std::setfill('=') << std::setw(37) << ""
         << std::hex << std::setfill('0')
         << "\n|  $0: 0x" << std::setw(8) << reg[0]
         << " | $v0: 0x" << std::setw(8) << reg[2]
         << " |\n| $v1: 0x" << std::setw(8) << reg[3]
         << " | $a0: 0x" << std::setw(8) << reg[4]
         << " |\n| $a1: 0x" << std::setw(8) << reg[5]
         << " | $a2: 0x" << std::setw(8) << reg[6]
         << " |\n| $a3: 0x" << std::setw(8) << reg[7]
         << " | $t0: 0x" << std::setw(8) << reg[8]
         << " |\n| $t1: 0x" << std::setw(8) << reg[9]
         << " | $t2: 0x" << std::setw(8) << reg[10]
         << " |\n| $t3: 0x" << std::setw(8) << reg[11]
         << " | $t4: 0x" << std::setw(8) << reg[12]
         << " |\n| $t5: 0x" << std::setw(8) << reg[13]
         << " | $t6: 0x" << std::setw(8) << reg[14]
         << " |\n| $t7: 0x" << std::setw(8) << reg[15]
         << " | $t8: 0x" << std::setw(8) << reg[24]
         << " |\n| $t9: 0x" << std::setw(8) << reg[25]
         << " | $s0: 0x" << std::setw(8) << reg[16]
         << " |\n| $s1: 0x" << std::setw(8) << reg[17]
         << " | $s2: 0x" << std::setw(8) << reg[18]
         << " |\n| $s3: 0x" << std::setw(8) << reg[19]
         << " | $s4: 0x" << std::setw(8) << reg[20]
         << " |\n| $s5: 0x" << std::setw(8) << reg[21]
         << " | $s6: 0x" << std::setw(8) << reg[22]
         << " |\n| $s7: 0x" << std::setw(8) << reg[23]
         << " | $gp: 0x" << std::setw(8) << reg[28]
         << " |\n| $sp: 0x" << std::setw(8) << reg[29]
         << " | $fp: 0x" << std::setw(8) << reg[30]
         << " |\n| $ra: 0x" << std::setw(8) << reg[31]
         << " |" << std::setfill(' ') << std::setw(18) << "|"
         << std::endl << std::setfill('=') << std::setw(37) << "";
    return cout;
}

         
