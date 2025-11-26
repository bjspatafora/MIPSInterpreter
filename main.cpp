#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>
#include <cstdint>
#include <unordered_map>
#include <functional>
#include "Registers.h"

class BadInstructException {};
class BadFormatException {};
class BadRegException {};

void split_input(std::string res[4], const std::string & input)
{
    int j = 0;
    for(int i = 0; i < input.length(); i++)
    {
        if(std::isspace(input[i]) || input[i] == ',')
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
}

uint32_t add(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = reg[input[2]] + reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t sub(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = reg[input[2]] - reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t sltu(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = reg[input[2]] < reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t slt(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = (int32_t)reg[input[2]] < (int32_t)reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t sgtu(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = reg[input[2]] > reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t sgt(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = (int32_t)reg[input[2]] < (int32_t)reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t andop(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = reg[input[2]] & reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t orop(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = reg[input[2]] | reg[input[3]];
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t norop(Registers & reg, std::string input[4])
{
    try
    {
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$' || input[3][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        input[3].erase(0, 1);
        if(input[1] == "0")
            throw RegisterException();
        reg[input[1]] = ~(reg[input[2]] | reg[input[3]]);
    }
    catch(BadFormatException & e)
    {
        throw e;
    }
    catch(RegisterException & e)
    {
        throw e;
    }
    return 4;
}

uint32_t execute(Registers & reg, std::string input[4])
{
    static std::unordered_map<std::string,
                              std::function<uint32_t(Registers &,
                                                     std::string [4])>> ops={
        {"add", add}, {"addu", add}, {"sub", sub}, {"subu", sub},
        {"slt", slt}, {"sltu", sltu}, {"sgt", sgt}, {"sgtu", sgtu},
        {"and", andop}, {"or", orop}, {"nor", norop}};
    if(ops.find(input[0]) == ops.end())
        throw BadInstructException();
    try
    {
        return ops[input[0]](reg, input);
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
        {"add", "add reg, reg, reg"}};
    int t = 0;
    std::cout << "MIPS interpreter/SPIM simulator\n\n";
    while(1)
    {
        for(int i = 0; i < 4; i++)
            splitinput[i] = "";
        switch(segment)
        {
            case 0: // Text segment input
                std::cout << "TEXT:0x" << std::hex << PC << " > ";
                std::getline(std::cin, input);
                split_input(splitinput, input);
                try
                {
                    PC += execute(reg, splitinput);
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
        }
    }
    
    return 0;
}
