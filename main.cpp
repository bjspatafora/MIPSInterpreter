#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>
#include <cstdint>
#include <unordered_map>
#include <functional>
#include <limits>
#include "Registers.h"

class BadInstructException {};
class BadFormatException {};
class BadRegException {};

std::string split_input(std::string res[4], const std::string & input)
{
    int j = 0;
    std::string LABEL = "";
    for(int i = 0; i < input.length(); i++)
    {
        if(input[i] == ':' && j == 0)
        {
            if(LABEL == "")
                LABEL = input.substr(0, i);
            else
                throw BadFormatException();
        }
        else if(std::isspace(input[i]) || input[i] == ',')
        {
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
        res |= std::stoi(input[3]) << 6;
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

uint32_t encode(std::string input[4])
{
    static std::unordered_map< std::string, std::vector< uint32_t >> opcodes = {
        {"add", {32, 0}}, {"addu", {33, 0}}, {"sub", {34, 0}},
        {"subu", {35, 0}}, {"slt", {42, 0}}, {"stlu", {43, 0}},
        {"and", {36, 0}}, {"or", {37, 0}}, {"nor", {39, 0}}, {"sll", {0, 1}},
        {"srl", {2, 1}}};
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
    
uint32_t add(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] + reg[(instr>>16)&31];
    return 4;
}

uint32_t sub(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] - reg[(instr>>16)&31];
    return 4;
}

uint32_t sltu(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] < reg[(instr>>16)&31];
    return 4;
}

uint32_t slt(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = (int32_t)reg[(instr>>21)&31] < reg[(instr>>16)&31];
    return 4;
}

uint32_t andop(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] & reg[(instr>>16)&31];
    return 4;
}

uint32_t orop(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] | reg[(instr>>16)&31];
    return 4;
}

uint32_t norop(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = ~(reg[(instr>>21)&31] | reg[(instr>>16)&31]);
    return 4;
}

uint32_t sll(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>16)&31] << ((instr >> 6) & 31);
    return 4;
}

uint32_t srl(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>16)&31] >> ((instr >> 6) & 31);
    return 4;
}

uint32_t executeR(Registers & reg, uint32_t instr)
{
    static std::unordered_map<uint32_t,
                              std::function<uint32_t(Registers &,
                                                     uint32_t)>> rOps={
        {32, add}, {33, add}, {34, sub}, {35, sub}, {36, andop}, {37, orop},
        {39, norop}, {42, slt}, {43, sltu}, {0, sll}, {2, srl}};
    return rOps[instr&63](reg, instr);
}

uint32_t execute(Registers & reg, uint32_t instr)
{
    static std::unordered_map<uint32_t,
                              std::function<uint32_t(Registers &,
                                                     uint32_t)>> ops={
        {0, executeR}};
    if(ops.find((instr>>26)&63) == ops.end())
        throw BadInstructException();
    try
    {
        return ops[(instr>>26)&63](reg, instr);
    }
    catch(BadInstructException & e)
    {
        throw e;
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

int main()
{
    Registers reg;
    // 0 for text, 1 for data, 2 for simulator commands
    int segment = 0;
    uint32_t PC = 4194304;
    std::string input = "";
    std::string splitinput[4];
    std::unordered_map< std::string, std::string > formats = {
        {"add", "add reg, reg, reg"}, {"addu", "addu reg, reg, reg"},
        {"sub", "sub reg, reg, reg"}, {"subu", "subu reg, reg, reg"},
        {"slt", "slt reg, reg, reg"}, {"sltu", "sltu reg, reg, reg"},
        {"and", "and reg, reg, reg"}, {"or", "or reg, reg, reg"},
        {"nor", "nor reg, reg, reg"}, {"sll", "sll reg, reg, shamt"},
        {"srl", "srl reg, reg, shamt"}};
    uint32_t instr = 0;
    bool run = 1;
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
                std::cout << "TEXT:0x" << std::hex << PC << " > ";
                std::getline(std::cin, input);
                if(input == "?")
                {
                    segment = 2;
                    break;
                }
                LABEL = split_input(splitinput, input);
                // Put label tracking stuff here when get to it
                try
                {
                    instr = encode(splitinput);
                    PC += execute(reg, instr);
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
                break;
            case 2:
                std::cout << "*** OPTIONS ***\n"
                          << "t - resume interactive text segment\n"
                          << "r - display register states\n"
                          << "q - quit\n";
                std::cin >> input;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                                '\n');
                if(input == "t")
                    segment = 0;
                else if(input == "r")
                    std::cout << reg << std::endl;
                else if(input == "q")
                    run = 0;
                else
                    std::cout << "Please enter a valid option\n";
                break;
        }
    }
    
    return 0;
}
