#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>
#include <cstdint>
#include <unordered_map>
#include <functional>
#include <limits>
#include "Registers.h"
#include "Memory.h"

class BadInstructException {};
class BadFormatException {};
class BadRegException {};
class LabelException {};
class TypeException {};
class SpaceException {};

// Splitting

std::string split_input(std::string res[4], const std::string & input)
{
    int j = 0;
    std::string LABEL = "";
    for(int i = 0; i < input.length(); i++)
    {
        if(input[i] == '#')
            break;
        if(input[i] == ':')
        {
            if(j == 0)
            {
                LABEL = res[0];
                res[0] = "";
            }
            else
            {
                throw LabelException();
            }
        }
        else if(std::isspace(input[i]) || input[i] == ',')
        {
            if(res[j] != "")
                j++;
            i++;
            while(std::isspace(input[i]))
            {
                i++;
                if(i == input.length())
                    break;
            }
            if(i == input.length())
                break;
            i--;
        }
        else
            res[j] += input[i];
    }
    return LABEL;
}

std::string split_input(std::vector< std::string > & res,
                        const std::string & input)
{
    res.clear();
    res.push_back("");
    unsigned int j = 0;
    std::string LABEL = "";
    bool instr = 0;
    for(unsigned int i = 0; i < input.length(); i++)
    {
        if(input[i] == '#' && !instr)
            break;
        if(input[i] == ':' && !instr)
        {
            if(j == 0)
            {
                if(res[0] == "")
                    throw LabelException();
                LABEL = res[0];
                res[0] = "";
            }
            else
            {
                throw LabelException();
            }
        }
        else if(std::isspace(input[i]) && !instr)
        {
            if(res[j] != "")
            {
                res.push_back("");
                j++;
            }
            i++;
            while(std::isspace(input[i]))
            {
                i++;
                if(i == input.length())
                    break;
            }
            if(i == input.length())
                break;
            i--;
        }
        else if(input[i] == '\"')
            instr = !instr;
        else if(input[i] == '\\')
        {
            if(!instr)
                throw BadFormatException();
            i++;
            switch(input[i])
            {
                case '\'':
                    res[j] += '\'';
                    break;
                case '"':
                    res[j] += '\"';
                    break;
                case 'n':
                    res[j] += '\n';
                    break;
                case 't':
                    res[j] += '\t';
                    break;
                default:
                    res[j] += '\\';
                    res[j] += input[i];
            }
        }
        else if(input[i] == '\'')
        {
            if(instr)
                res[j] += '\'';
            else
            {
                i++;
                if(input[i] == '\\')
                {
                    i++;
                    switch(input[i])
                    {
                        case '\'':
                            res[j] += '\'';
                            break;
                        case '"':
                            res[j] += '\"';
                            break;
                        case 'n':
                            res[j] += '\n';
                            break;
                        case 't':
                            res[j] += '\t';
                            break;
                        default:
                            throw BadFormatException();
                    }
                }
                else
                {
                    res[j] += input[i];
                }
                i++;
                if(input[i] != '\'')
                    throw BadFormatException();
            }
        }
        else
        {
            res[j] += input[i];
        }
    }
    if(res[j] == "")
        res.pop_back();
    return LABEL;
}

// Encoding

uint32_t normRencode(std::string input[4])
{
    try
    {
        uint32_t res = 0;
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0" || input[1] == "zero")
            throw RegisterException();
        res |= Registers::getRegNum(input[1]) << 11;
        res |= Registers::getRegNum(input[2]) << 21;
        res |= Registers::getRegNum(input[3]) << 16;
        return res;
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
}

uint32_t shiftRencode(std::string input[4])
{
    try
    {
        uint32_t res = 0;
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] < '0' ||
           input[3][0] > '9')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        if(input[1] == "0" || input[1] == "zero")
            throw RegisterException();
        res |= Registers::getRegNum(input[1]) << 11;
        res |= Registers::getRegNum(input[2]) << 16;
        res |= (std::stoi(input[3]) & 31) << 6;
        return res;
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
}

uint32_t imIencode(std::string input[4])
{
    try
    {
        uint32_t res = 0;
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || (input[3][0] != '-' &&
                                                        (input[3][0] < '0' ||
                                                         input[3][0] > '9')))
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        if(input[1] == "0" || input[1] == "zero")
            throw RegisterException();
        res |= Registers::getRegNum(input[1]) << 16;
        res |= Registers::getRegNum(input[2]) << 21;
        res |= std::stoi(input[3]) & 65534;
        return res;
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
}

uint32_t lsencode(std::string input[4])
{
    try
    {
        uint32_t res = 0;
        if(input[3] != "" || input[2] == "" || input[1] == ""
           || input[1][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        if(input[2][0] != '$')
        {
            unsigned int num = 0;
            for(; num < input[2].length(); num++)
            {
                if(input[2][num] == '(')
                    break;
                input[3] += input[2][num];
            }
            if(input[2][num+1] != '$')
                throw BadFormatException();
            input[2].erase(0, num + 2);
            if(input[2][input[2].length()-1] != ')')
                throw BadFormatException();
            input[2].erase(input[2].length() - 1, 1);
        }
        else
        {
            input[2].erase(0, 1);
            input[3] = "0";
        }
        if(input[1] == "0" || input[1] == "zero")
            throw RegisterException();
        res |= Registers::getRegNum(input[1]) << 16;
        res |= Registers::getRegNum(input[2]) << 21;
        res |= std::stoi(input[3]) & 65534;
        return res;
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
}

uint32_t singleAddrencode(std::string input[4], std::unordered_map< std::string, uint32_t > & labels)
{
    try
    {
        uint32_t res = 0;
        if(input[3] != "" || input[2] != "" || input[1] == "")
            throw BadFormatException();
        if(input[1][0] >= '0' && input[1][0] <= '9')
            res = std::stoi(input[1]);
        else if(labels.find(input[1]) != labels.end())
            res = labels[input[1]];
        else
            throw LabelException();
        res =  (res >> 2) & 67108863;
        return res;
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
}

uint32_t singleRegencode(std::string input[4])
{
    try
    {
        uint32_t res = 0;
        if(input[3] != "" || input[2] != "" || input[1] == "" ||
           input[1][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        res |= Registers::getRegNum(input[1]) << 21;
        return res;
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
}
                          
uint32_t encode(std::string input[4],
                std::unordered_map< std::string, uint32_t > & labels)
{
    static std::unordered_map< std::string, std::vector< uint32_t >> opcodes = {
        {"add", {32, 0}}, {"addu", {33, 0}}, {"sub", {34, 0}},
        {"subu", {35, 0}}, {"slt", {42, 0}}, {"stlu", {43, 0}},
        {"and", {36, 0}}, {"or", {37, 0}}, {"nor", {39, 0}}, {"sll", {0, 1}},
        {"srl", {2, 1}}, {"addi", {536870912, 2}}, {"addiu", {603979776, 2}},
        {"andi", {805306368, 2}}, {"ori", {872415232, 2}},
        {"slti", {671088640, 2}}, {"sltiu", {738197504, 2}},
        {"lw", {2348810240, 3}}, {"sw", {2885681152, 3}},
        {"j", {134217728, 4}}, {"jr", {8, 5}}};
    if(opcodes.find(input[0]) == opcodes.end())
        throw BadInstructException();
    uint32_t ret = opcodes[input[0]][0];
    try
    {
        switch(opcodes[input[0]][1])
        {
            case 0:
                return ret | normRencode(input);
            case 1:
                return ret | shiftRencode(input);
            case 2:
                return ret | imIencode(input);
            case 3:
                return ret | lsencode(input);
            case 4:
                return ret | singleAddrencode(input, labels);
            case 5:
                return ret | singleRegencode(input);
        }
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
}

// Execution

uint32_t add(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] + reg[(instr>>16)&31];
    return 4;
}

uint32_t sub(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] - reg[(instr>>16)&31];
    return 4;
}

uint32_t sltu(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] < reg[(instr>>16)&31];
    return 4;
}

uint32_t slt(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = (int32_t)reg[(instr>>21)&31] < reg[(instr>>16)&31];
    return 4;
}

uint32_t andop(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] & reg[(instr>>16)&31];
    return 4;
}

uint32_t orop(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] | reg[(instr>>16)&31];
    return 4;
}

uint32_t norop(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = ~(reg[(instr>>21)&31] | reg[(instr>>16)&31]);
    return 4;
}

uint32_t sll(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>16)&31] << ((instr >> 6) & 31);
    return 4;
}

uint32_t srl(Registers & reg, uint32_t instr, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>16)&31] >> ((instr >> 6) & 31);
    return 4;
}

uint32_t jr(Registers & reg, uint32_t instr, uint32_t & PC)
{
    PC = reg[(instr>>21)&31];
    return 4;
}

uint32_t executeR(Registers & reg, uint32_t instr, Memory & mem,
                  uint32_t & PC)
{
    static std::unordered_map<uint32_t,
                              std::function<uint32_t(Registers &,
                                                     uint32_t,
                                                     uint32_t &)>> rOps={
        {32, add}, {33, add}, {34, sub}, {35, sub}, {36, andop}, {37, orop},
        {39, norop}, {42, slt}, {43, sltu}, {0, sll}, {2, srl}, {8, jr}};
    return rOps[instr&63](reg, instr, PC);
}

uint32_t addiu(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    reg[(instr>>16)&31] = reg[(instr>>21)&31] + im;
    return 4;
}

uint32_t andopi(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    reg[(instr>>16)&31] = reg[(instr>>21)&31] & im;
    return 4;
}

uint32_t oropi(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    reg[(instr>>16)&31] = reg[(instr>>21)&31] | im;
    return 4;
}

uint32_t slti(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    reg[(instr>>16)&31] = (int32_t)reg[(instr>>21)&31] < im;
    return 4;
}

uint32_t sltiu(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    reg[(instr>>16)&31] = reg[(instr>>21)&31] < im;
    return 4;
}

uint32_t lw(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    reg[(instr>>16)&31] = mem.getWord(reg[(instr>>21)&31] + im);
    return 4;
}

uint32_t sw(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    mem.storeWord(reg[(instr>>16)&31], reg[(instr>>21)&31] + im);
    return 4;
}

uint32_t j(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    PC &= 15 << 28;
    PC |= (instr & 67108863) << 2;
    PC -= 4;
    return 4;
}

uint32_t execute(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    static std::unordered_map<uint32_t,
                              std::function<uint32_t(Registers &,
                                                     uint32_t, Memory &,
                                                     uint32_t &)>> ops={
        {0, executeR}, {8, addiu}, {9, addiu}, {12, andopi}, {13, oropi},
        {10, slti}, {11, sltiu}, {35, lw}, {43, sw}, {2, j}};
    if(reg[29] < mem.getCurrstack())
        mem.incStack();
    return ops[(instr>>26)&63](reg, instr, mem, PC);
}

void newData(Memory & mem, std::vector< std::string > splitinput)
{
    if(splitinput[0] == ".byte")
    {
        for(unsigned int i = 1; i < splitinput.size(); i++)
        {
            bool warn = 0;
            try
            {
                warn = mem.databyte(std::stoi(splitinput[i]));
            }
            catch(const std::invalid_argument& e)
            {
                warn = mem.databyte(splitinput[i][0]);
            }
            if(warn)
                throw SpaceException();
        }
    }
    else if(splitinput[0] == ".word")
    {
        for(unsigned int i = 1; i < splitinput.size(); i++)
        {
            try
            {
                bool warn = mem.dataword(std::stoi(splitinput[i]));
                if(warn)
                    throw SpaceException();
            }
            catch(const std::invalid_argument & e)
            {
                throw e;
            }
        }
    }
    else if(splitinput[0] == ".asciiz")
    {
        for(unsigned int i = 1; i < splitinput.size(); i++)
        {
            bool warn = mem.dataasciiz(splitinput[i]);
            if(warn)
                throw SpaceException();
        }
    }
    else if(splitinput[0] == ".space")
    {
        for(unsigned int i = 1; i < splitinput.size(); i++)
        {
            try
            {
                bool warn = mem.dataspace(std::stoi(splitinput[i]));
                if(warn)
                    throw SpaceException();
            }
            catch(const std::invalid_argument & e)
            {
                throw e;
            }
        }
    }
    else
    {
        throw TypeException();
    }
}

int main()
{
    Registers reg;
    Memory mem;
    // 0 for text, 1 for data, 2 for simulator commands
    int segment = 0;
    uint32_t PC = 0x400000;
    std::unordered_map< std::string, uint32_t > labels;
    std::string input = "";
    std::string splitinput[4];
    std::vector< std::string > datasplitinput;
    std::unordered_map< std::string, std::string > formats = {
        {"add", "add reg, reg, reg"}, {"addu", "addu reg, reg, reg"},
        {"sub", "sub reg, reg, reg"}, {"subu", "subu reg, reg, reg"},
        {"slt", "slt reg, reg, reg"}, {"sltu", "sltu reg, reg, reg"},
        {"and", "and reg, reg, reg"}, {"or", "or reg, reg, reg"},
        {"nor", "nor reg, reg, reg"}, {"sll", "sll reg, reg, shamt"},
        {"srl", "srl reg, reg, shamt"}, {"addi", "addi reg, reg, im"},
        {"addiu", "addiu reg, reg, im"}, {"andi", "andi reg, reg, im"},
        {"ori", "ori reg, reg, im"}, {"slti", "stli reg, reg, im"},
        {"sltiu", "sltiu reg, reg, im"}, {"lw", "lw reg, im(reg)"},
        {"sw", "sw reg, im(reg)"}, {"j", "j addr"}, {"jr", "jr reg"}};
    uint32_t instr = 0;
    bool run = true;
    std::cout << "MIPS interpreter/SPIM simulator\n"
              << "Enter ? for sim commands\n\n";
    while(run)
    {
        for(int i = 0; i < 4; i++)
            splitinput[i] = "";
        std::string LABEL = "";
        instr = 0;
        switch(segment)
        {
            case 0: // Text segment input
                if(PC < mem.getCurrtext())
                {
                    instr = mem.getWord(PC);
                    PC += execute(reg, instr, mem, PC);
                }
                else
                {
                    std::cout << "TEXT:0x" << std::hex << PC << " > ";
                    std::getline(std::cin, input);
                    if(input == "?")
                    {
                        segment = 2;
                        break;
                    }
                    LABEL = split_input(splitinput, input);
                    if(LABEL != "")
                        labels[LABEL] = PC;
                    try
                    {
                        instr = encode(splitinput, labels);
                        mem.storeWord(instr, PC);
                        mem.incText();
                        PC += execute(reg, instr, mem, PC); 
                    }
                    catch(LabelException & e)
                    {
                        std::cout << "\tPlease ensure labels are at the front "
                                  << "of input and are alphanumeric\n";
                    }
                    catch(BadInstructException & e)
                    {
                        std::cout << "\tSorry, that instruction doesn't exist "
                                  << "or hasn't been implemented yet...\n";
                    }
                    catch(BadFormatException & e)
                    {
                        std::cout << "\tPlease enter the correct instruction "
                                  << "format: " << formats[splitinput[0]]
                                  << std::endl;
                    }
                    catch(RegisterException & e)
                    {
                        std::cout << "\tPlease use valid registers" << std::endl;
                    }
                    catch(AddressException & e)
                    {
                        std::cout << "\tATTEMPTING TO ACCESS INVALID MEMORY, STOP\n";
                    }
                }
                break;
            case 1:
                std::cout << "DATA:0x" << std::hex << mem.getCurrdata()
                          << " > ";
                std::getline(std::cin, input);
                if(input == "?")
                {
                    segment = 2;
                    break;
                }
                try
                {
                    LABEL = split_input(datasplitinput, input);
                }
                catch(LabelException & e)
                {
                    std::cout << "\tPlease ensure labels are at the front "
                              << "of input and are alphanumeric\n";
                }
                catch(BadFormatException & e)
                {
                    std::cout << "\tPlease ensure all characters are in "
                              << "quotes and all quotes are properly "
                              << "closed\n";
                }
                if(LABEL != "")
                    labels[LABEL] = mem.getCurrdata();
                try
                {
                    newData(mem, datasplitinput);
                }
                catch(const std::invalid_argument & e)
                {
                    std::cout << "\tMake sure inputs for this type are "
                              << "numeric\n";
                }
                catch(TypeException & e)
                {
                    std::cout << "\tThat is not a valid type, please use "
                              << "byte, word, asciiz, or space\n";
                }
                catch(SpaceException & e)
                {
                    std::cout << "\tWARNING: DATA IS ABOUT TO OR CURRENTLY "
                              << "ATTEMPTING OVERWRITING STACK, DO NOT ENTER "
                              << "MORE DATA\n";
                }
                break;
            case 2:
                std::cout << "*** OPTIONS ***\n"
                          << "t - resume interactive text segment\n"
                          << "d - resume interactive data segment\n"
                          << "R - display register states\n"
                          << "D - display data segment\n"
                          << "L - display labels\n"
                          << "S - display current stack\n"
                          << "q - quit\n"
                          << "OPTION >> ";
                std::cin >> input;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                                '\n');
                if(input == "t")
                    segment = 0;
                else if(input == "d")
                    segment = 1;
                else if(input == "R")
                    std::cout << reg << std::endl;
                else if(input == "D")
                    mem.showData();
                else if(input == "L")
                {
                    std::cout << "*** LABELS ***\n";
                    for(auto l : labels)
                        std::cout << l.first << ": 0x" << l.second
                                  << std::endl;
                    std::cout << std::endl;
                }
                else if(input == "S")
                    mem.showStack(reg[29]);
                else if(input == "q")
                    run = 0;
                else
                    std::cout << "Please enter a valid option\n";
                break;
        }
    }
    
    return 0;
}
