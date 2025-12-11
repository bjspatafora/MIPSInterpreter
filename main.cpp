#include <iostream>
#include <fstream>
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

void lower(std::string & s)
{
    for(int i = 0; i < s.size(); i++)
    {
        if(s[i] >= 'A' && s[i] <= 'Z')
            s[i] = s[i] - 'A' + 'a';
    }
}

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
            {
                j++;
                if(j == 4)
                {
                    for(int k = i + 1; k < input.size(); k++)
                    {
                        if(input[k] == '#')
                            return LABEL;
                        if(!std::isspace(input[k]))
                            throw BadFormatException();
                    }
                    return LABEL;
                }
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
        else
            res[j] += input[i];
    }
    lower(res[0]);
    for(unsigned int i = 1; i < 4; i++)
    {
        if(res[i][0] == '$')
            lower(res[i]);
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
    lower(res[0]);
    return LABEL;
}


// Decode

std::string Rdecode(uint32_t instr)
{
    static std::unordered_map<uint32_t, std::string> Ropsdecode = {
        {32, "add "}, {33, "addu "}, {34, "sub "}, {35, "subu "},
        {36, "and "}, {37, "or "}, {39, "nor "}, {42, "slt "}, {43, "sltu "},
        {0, "sll "}, {2, "srl "}, {8, "jr "}, {12, "syscall"}, {26, "div "},
        {27, "divu "}, {16, "mfhi "}, {18, "mflo "}, {24, "mult "},
        {25, "multu "}};
    static std::unordered_map<uint32_t, uint32_t> Rdecodefuncts = {
        {0, 0}, {2, 0}, {8, 2}, {32, 1}, {33, 1}, {34, 1}, {35, 1}, {36, 1},
        {37, 1}, {39, 1}, {42, 1}, {43, 1}, {12, -1}, {26, 3}, {27, 3},
        {16, 4}, {18, 4}, {24, 3}, {25, 3}};
    std::string ret = Ropsdecode[instr & 63];
    switch(Rdecodefuncts[instr&63])
    {
        case 0:
            return ret + Registers::regNames[(instr>>11)&31] +
                std::string(", ") + Registers::regNames[(instr>>16)&31] +
                std::string(", ") + std::to_string((instr >> 6) & 31);
        case 1:
            return ret + Registers::regNames[(instr>>11)&31] +
                std::string(", ") + Registers::regNames[(instr>>21)&31] +
                std::string(", ") + Registers::regNames[(instr>>16)&31];
        case 2:
            return ret + Registers::regNames[(instr>>21)&31];
        case 3:
            return ret + Registers::regNames[(instr>>21)&31] +
                std::string(", ") + Registers::regNames[(instr>>16)&31];
        case 4:
            return ret + Registers::regNames[(instr>>11)&31];
    }
    return ret;
}

std::string specRdecode(uint32_t instr)
{
    static std::unordered_map<uint32_t, std::string> specRopsdecode = {
        {2, "mul "}};
    static std::unordered_map<uint32_t, uint32_t> specRdecodefuncts = {
        {2, 0}};
    std::string ret = specRopsdecode[instr & 63];
    switch(specRdecodefuncts[instr&63])
    {
        case 0:
            return ret + Registers::regNames[(instr>>11)&31] +
                std::string(", ") + Registers::regNames[(instr>>16)&31] +
                std::string(", ") + Registers::regNames[(instr>>21)&31];
    }
    return ret;
}

std::string decode(uint32_t instr, uint32_t PC,
                   std::unordered_map< uint32_t, std::string > & labels)
{
    static std::unordered_map<uint32_t, std::string> opsdecode = {
        {0, ""}, {8, "addi "}, {9, "addiu "}, {12, "andi "}, {13, "ori "},
        {15, "lui "}, {10, "slti "}, {11, "sltiu "}, {32, "lb "}, {40, "sb "},
        {35, "lw "}, {43, "sw "}, {2, "j "}, {3, "jal "}, {28, ""},
        {4, "beq "}, {5, "bne "}};
    static std::unordered_map<uint32_t, uint32_t> decodefuncts = {
        {0, 0}, {8, 1}, {9, 1}, {12, 1}, {13, 1}, {15, 5}, {10, 1}, {11, 1},
        {32, 2}, {40, 2}, {35, 2}, {43, 2}, {2, 3}, {3, 3}, {28, 4}, {4, 6},
        {5, 6}};
    std::string ret = opsdecode[(instr>>26)&63];
    switch(decodefuncts[(instr>>26)&63])
    {
        case 0:
            return Rdecode(instr);
        case 1:
            return ret + Registers::regNames[(instr>>16)&31] +
                std::string(", ") + Registers::regNames[(instr>>21)&31] +
                std::string(", ") + std::to_string((int16_t)(instr & 65535));
        case 2:
            ret += Registers::regNames[(instr>>16)&31] + std::string(", ");
            if(instr & 65534 == 0)
                ret += Registers::regNames[(instr>>21)&31];
            else
            {
                ret += std::to_string((int16_t)(instr & 65534)) +
                    std::string("(") + Registers::regNames[(instr>>21)&31] +
                    std::string(")");
            }
            return ret;
        case 3:
            if(labels.find((PC & (15 << 28)) | ((instr & 67108863) << 2)) !=
               labels.end())
                ret += labels[(PC&(15<<28))|((instr&67108863)<<2)];
            else
                ret += std::to_string((PC & (15 << 28)) |
                                      ((instr & 67108863) << 2));
            return ret;
        case 4:
            return specRdecode(instr);
        case 5:
            return ret + Registers::regNames[(instr>>16)&31] +
                std::string(", ") + std::to_string((instr & 65535));
        case 6:
            ret += Registers::regNames[(instr>>21)&31] +
                std::string(", ") + Registers::regNames[(instr>>16)&31] +
                std::string(", ");
            if((instr & 65535) == 0)
                ret += std::string("0");
            else
                ret += std::to_string(PC + (instr & 65535));
            return ret;
    }
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

uint32_t nonretRencode(std::string input[4])
{
    try
    {
        uint32_t res = 0;
        if(input[3] != "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        res |= Registers::getRegNum(input[1]) << 21;
        res |= Registers::getRegNum(input[2]) << 16;
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
        res |= std::stoi(input[3]) & 65535;
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
        if(input[2] == "0" || input[2] == "zero")
            throw RegisterException();
        res |= Registers::getRegNum(input[1]) << 16;
        res |= Registers::getRegNum(input[2]) << 21;
        res |= std::stoi(input[3]) & 32767;
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

uint32_t singleAddrencode(std::string input[4], uint32_t PC, std::unordered_map< std::string, uint32_t > & labels, std::unordered_map< std::string, std::vector< uint32_t >> & nonregLabels)
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
            nonregLabels[input[1]].push_back(PC);
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

uint32_t singleRegSpecencode(std::string input[4])
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

uint32_t singleRegencode(std::string input[4])
{
    try
    {
        uint32_t res = 0;
        if(input[3] != "" || input[2] != "" || input[1] == "" ||
           input[1][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        res |= Registers::getRegNum(input[1]) << 11;
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

uint32_t bencode(std::string input[4], uint32_t PC,
                 std::unordered_map< std::string, uint32_t > & labels, std::unordered_map< std::string, std::vector< uint32_t >> & nonregLabels)
{
    try
    {
        uint32_t res = 0;
        if(input[3] == "" || input[2] == "" || input[1] == "" ||
           input[1][0] != '$' || input[2][0] != '$')
            throw BadFormatException();
        input[1].erase(0, 1);
        input[2].erase(0, 1);
        res |= Registers::getRegNum(input[1]) << 21;
        res |= Registers::getRegNum(input[2]) << 16;
        if(input[3][0] == '-' || (input[3][0] >= '0' && input[3][0] <= '9'))
            res |= std::stoi(input[3]) & 65535;
        else
        {
            if(labels.find(input[3]) == labels.end())
                nonregLabels[input[3]].push_back(PC);
            else
                res |= (labels[input[3]] - PC) & 65535;
        }
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

uint32_t encode(std::string input[4], uint32_t & PC,
                std::unordered_map< std::string, uint32_t > & labels, std::unordered_map< std::string, std::vector< uint32_t >> & nonregLabels,
                Memory & mem)
{
    static std::unordered_map< std::string, std::vector< uint32_t >> opcodes = {
        {"add", {32, 0}}, {"addu", {33, 0}}, {"sub", {34, 0}},
        {"subu", {35, 0}}, {"slt", {42, 0}}, {"stlu", {43, 0}},
        {"and", {36, 0}}, {"or", {37, 0}}, {"nor", {39, 0}}, {"sll", {0, 1}},
        {"srl", {2, 1}}, {"addi", {536870912, 2}}, {"addiu", {603979776, 2}},
        {"andi", {805306368, 2}}, {"ori", {872415232, 2}},
        {"lui", {1006632960, 11}}, {"slti", {671088640, 2}},
        {"sltiu", {738197504, 2}}, {"lb", {2147483648, 3}},
        {"sb", {2684354560, 3}}, {"lw", {2348810240, 3}},
        {"sw", {2885681152, 3}}, {"j", {134217728, 4}}, {"jr", {8, 5}},
        {"jal", {201326592, 4}}, {"li", {0, 6}}, {"la", {0, 6}},
        {"syscall", {12, 7}}, {"move", {33, 8}}, {"div", {26, 9}},
        {"divu", {27, 9}}, {"mfhi", {16, 10}}, {"mflo", {18, 10}},
        {"mul", {1879048194, 0}}, {"mult", {24, 9}}, {"multu", {25, 9}},
        {"beq", {268435456, 12}}, {"bne", {335544320, 12}}, {"bge", {0, 13}},
        {"bgt", {0, 14}}, {"ble", {0, 15}}, {"blt", {0, 16}}};
    std::unordered_map< std::uint32_t, std::string > revLabels;
    if(opcodes.find(input[0]) == opcodes.end())
        throw BadInstructException();
    uint32_t ret = opcodes[input[0]][0];
    uint32_t t;
    try
    {
        switch(opcodes[input[0]][1])
        {
            case 0:
                t = ret | normRencode(input);
                mem.storeWord(t, PC);
                mem.incText();
                return 4;
            case 1:
                t = ret | shiftRencode(input);
                if(t != 0)
                {
                    mem.storeWord(ret | shiftRencode(input), PC);
                    mem.incText();
                }
                return 4;
            case 2:
                mem.storeWord(ret | imIencode(input), PC);
                mem.incText();
                return 4;
            case 3:
                mem.storeWord(ret | lsencode(input), PC);
                mem.incText();
                return 4;
            case 4:
                mem.storeWord(ret | singleAddrencode(input, PC, labels,
                                                     nonregLabels), PC);
                mem.incText();
                return 4;
            case 5:
                mem.storeWord(ret | singleRegSpecencode(input), PC);
                mem.incText();
                return 4;
            case 6: // LOAD IMMEDIATE
                if(input[1] == "" || input[1][0] != '$' || input[3] != "")
                    throw BadFormatException();
                if(input[2][0] >= 'a' && input[2][0] <= 'z' ||
                   input[2][0] >= 'A' && input[2][0] <= 'Z')
                {
                    if(labels.find(input[2]) != labels.end())
                        t = labels[input[2]];
                    else
                    {
                        nonregLabels[input[2]].push_back(PC);
                        nonregLabels[input[2]].push_back(PC + 4);
                        t = 0;
                    }
                }
                else
                    t = std::stoi(input[2]);
                input[0] = "lui";
                input[2] = std::to_string(t >> 16);
                encode(input, PC, labels, nonregLabels, mem);
                PC += 4;
                input[0] = "ori";
                input[1] = std::string("$") + input[1];
                input[2] = input[1];
                input[3] = std::to_string(t & 65535);
                encode(input, PC, labels, nonregLabels, mem);
                return 8;
            case 7:
                if(input[1] != "" || input[2] != "" || input[3] != "")
                    throw BadFormatException();
                mem.storeWord(ret, PC);
                mem.incText();
                return 4;
            case 8:
                input[3] = input[2];
                input[2] = "$0";
                mem.storeWord(ret | normRencode(input), PC);
                mem.incText();
                return 4;
            case 9:
                mem.storeWord(ret | nonretRencode(input), PC);
                mem.incText();
                return 4;
            case 10:
                mem.storeWord(ret | singleRegencode(input), PC);
                mem.incText();
                return 4;
            case 11:
                input[3] = input[2];
                input[2] = "$0";
                mem.storeWord(ret | imIencode(input), PC);
                mem.incText();
                return 4;
            case 12:
                t = ret | bencode(input, PC, labels, nonregLabels);
                mem.storeWord(t, PC);
                mem.incText();
                return 4;
            case 13: // BRANCH GREATER EQ
                t = 0;
                if(input[3] == "" || input[2] == "" || input[1] == "" ||
                   input[1][0] != '$' || input[2][0] != '$')
                    throw BadFormatException();
                if(input[3][0] >= '0' && input[3][0] <= '9' ||
                   input[3][0] == '-')
                    t = std::stoi(input[3]);
                else if(labels.find(input[3]) == labels.end())
                    nonregLabels[input[3]].push_back(PC + 4);
                else
                    t = labels[input[3]];
                input[3] = input[1];
                input[1] = "$at";
                input[0] = "slt";
                encode(input, PC, labels, nonregLabels, mem);
                PC += 4;
                input[0] = "beq";
                input[1] = "$at";
                input[2] = "$0";
                input[3] = std::to_string(t);
                encode(input, PC, labels, nonregLabels, mem);
                return 8;
            case 14: // BRANCH GREATER
                t = 0;
                if(input[3] == "")
                    throw BadFormatException();
                if(input[3][0] >= '0' && input[3][0] <= '9' ||
                   input[3][0] == '-')
                    t = std::stoi(input[3]);
                else if(labels.find(input[3]) == labels.end())
                    nonregLabels[input[3]].push_back(PC + 4);
                else
                    t = labels[input[3]];
                input[3] = input[1];
                input[1] = "$at";
                input[0] = "slt";
                encode(input, PC, labels, nonregLabels, mem);
                PC += 4;
                input[0] = "bne";
                input[1] = "$at";
                input[2] = "$0";
                input[3] = std::to_string(t);
                encode(input, PC, labels, nonregLabels, mem);
                return 8;
            case 15: // BRANCH LESS EQ
                t = 0;
                if(input[3] == "")
                    throw BadFormatException();
                if(input[3][0] >= '0' && input[3][0] <= '9' ||
                   input[3][0] == '-')
                    t = std::stoi(input[3]);
                else if(labels.find(input[3]) == labels.end())
                    nonregLabels[input[3]].push_back(PC + 4);
                else
                    t = labels[input[3]];
                input[3] = input[2];
                input[2] = input[1];
                input[1] = "$at";
                input[0] = "slt";
                encode(input, PC, labels, nonregLabels, mem);
                PC += 4;
                input[0] = "bne";
                input[1] = "$at";
                input[2] = "$0";
                input[3] = std::to_string(t);
                encode(input, PC, labels, nonregLabels, mem);
                return 8;
            case 16: // BRANCH LESS
                t = 0;
                if(input[3] == "")
                    throw BadFormatException();
                if(input[3][0] >= '0' && input[3][0] <= '9' ||
                   input[3][0] == '-')
                    t = std::stoi(input[3]);
                else if(labels.find(input[3]) == labels.end())
                    nonregLabels[input[3]].push_back(PC + 4);
                else
                    t = labels[input[3]];
                input[3] = input[2];
                input[2] = input[1];
                input[1] = "$at";
                input[0] = "slt";
                encode(input, PC, labels, nonregLabels, mem);
                PC += 4;
                input[0] = "beq";
                input[1] = "$at";
                input[2] = "$0";
                input[3] = std::to_string(t);
                encode(input, PC, labels, nonregLabels, mem);
                return 8;
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

uint32_t add(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] + reg[(instr>>16)&31];
    return 4;
}

uint32_t sub(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] - reg[(instr>>16)&31];
    return 4;
}

uint32_t sltu(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] < reg[(instr>>16)&31];
    return 4;
}

uint32_t slt(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = (int32_t)reg[(instr>>21)&31] < reg[(instr>>16)&31];
    return 4;
}

uint32_t andop(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] & reg[(instr>>16)&31];
    return 4;
}

uint32_t orop(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] | reg[(instr>>16)&31];
    return 4;
}

uint32_t norop(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = ~(reg[(instr>>21)&31] | reg[(instr>>16)&31]);
    return 4;
}

uint32_t sll(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>16)&31] << ((instr >> 6) & 31);
    return 4;
}

uint32_t srl(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[(instr>>16)&31] >> ((instr >> 6) & 31);
    return 4;
}

uint32_t jr(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    PC = reg[(instr>>21)&31] - 4;
    return 4;
}

uint32_t syscall(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    uint32_t addr;
    uint32_t i;
    char c;
    std::string s;
    switch(reg[2])
    {
        case 1:
            std::cout << std::dec << (int32_t)reg[4];
            break;
        case 4:
            if(reg[4] < 0x400000)
            {
                std::cout << "\tPlease ensure $a0 holds a valid addr\n";
                break;
            }
            addr = reg[4];
            c = mem.getByte(addr);
            addr++;
            while(c != '\0')
            {
                std::cout << c;
                c = mem.getByte(addr);
                addr++;
            }
            break;
        case 5:
            std::cout << "INT INPUT > ";
            std::cin >> reg[2];
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        case 8:
            if(reg[4] < 0x400000)
            {
                std::cout << "\tPlease ensure $a0 holds a valid addr\n";
                break;
            }
            std::cout << "STRING INPUT > ";
            std::cin >> s;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            addr = reg[4];
            if(reg[5] == 0)
                break;
            for(i = 0; i < reg[5] - 1 && i < s.size(); i++)
            {
                mem.storeByte(s[i], addr);
                addr++;
            }
            for(; i < reg[5]; i++)
            {
                mem.storeByte(0, addr);
                addr++;
            }
            break;
        case 10:
            PC = 0;
            break;
        case 12:
            std::cout << "CHAR INPUT > ";
            std::cin >> reg[2];
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        default:
            std::cout << "\tSorry! This syscall has not been implemented or "
                      << "does not exist.\n";
            break;
    }
    return 4;
}

uint32_t divop(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t x = reg[(instr>>21)&31];
    int32_t y = reg[(instr>>16)&31];
    if(y == 0)
    {
        std::cout << "\tDIVISION BY 0!!! STOP\n";
        return 4;
    }
    reg[32] = x % y;
    reg[33] = x / y;
    return 4;
}

uint32_t divuop(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    uint32_t x = reg[(instr>>21)&31];
    uint32_t y = reg[(instr>>16)&31];
    if(y == 0)
    {
        std::cout << "\tDIVISION BY 0!!! STOP\n";
        return 4;
    }
    reg[32] = x % y;
    reg[33] = x / y;
    return 4;
}

uint32_t mfhi(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[32];
    return 4;
}

uint32_t mflo(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr>>11)&31] = reg[33];
    return 4;
}

uint32_t multop(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int64_t x = (int32_t)reg[(instr>>21)&31];
    int64_t y = (int32_t)reg[(instr>>16)&31];
    int64_t res = x * y;
    reg[32] = res >> 31;
    reg[33] = res;
    return 4;
}

uint32_t multuop(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    uint64_t x = reg[(instr>>21)&31];
    uint64_t y = reg[(instr>>16)&31];
    uint64_t res = x * y;
    reg[32] = res >> 31;
    reg[33] = res;
    return 4;
}  

uint32_t executeR(Registers & reg, uint32_t instr, Memory & mem,
                  uint32_t & PC)
{
    static std::unordered_map<uint32_t,
                              std::function<uint32_t(Registers &,
                                                     uint32_t, Memory &,
                                                     uint32_t &)>> rOps={
        {32, add}, {33, add}, {34, sub}, {35, sub}, {36, andop}, {37, orop},
        {39, norop}, {42, slt}, {43, sltu}, {0, sll}, {2, srl}, {8, jr},
        {12, syscall}, {26, divop}, {27, divuop}, {16, mfhi}, {18, mflo},
        {24, multop}, {25, multuop}};
    return rOps[instr&63](reg, instr, mem, PC);
}

uint32_t mul(Registers & reg, uint32_t instr)
{
    reg[(instr>>11)&31] = reg[(instr>>21)&31] * reg[(instr>>16)&31];
    return 4;
}

uint32_t executeSpecR(Registers & reg, uint32_t instr, Memory & mem,
                      uint32_t & PC)
{
    static std::unordered_map<uint32_t,
                              std::function<uint32_t(Registers &,
                                                     uint32_t)>> specrOps={
        {2, mul}};
    return specrOps[instr&63](reg, instr);
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

uint32_t lui(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[(instr >> 16) & 31] = instr << 16;
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

uint32_t lb(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    if(reg[(instr>>21)&31] + im < 0x400000 ||
       reg[(instr>>21)&31] + im > 0x5e8480)
    {
        std::cout << "\tTrying to access invalid memory at 0x" << std::hex
                  << (reg[(instr>>21)&31] + im) << ", stop\n";
        return 4;
    }
    reg[(instr>>16)&31] = mem.getByte(reg[(instr>>21)&31] + im);
    return 4;
}

uint32_t sb(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    if(reg[(instr>>21)&31] + im < 0x400000 ||
       reg[(instr>>21)&31] + im > 0x5e8480)
    {
        std::cout << "\tTrying to access invalid memory at 0x" << std::hex
                  << (reg[(instr>>21)&31] + im) << ", stop\n";
        return 4;
    }
    mem.storeByte(reg[(instr>>16)&31], reg[(instr>>21)&31] + im);
    return 4;
}

uint32_t lw(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    if(reg[(instr>>21)&31] + im < 0x400000 ||
       reg[(instr>>21)&31] + im > 0x5e8480)
    {
        std::cout << "\tTrying to access invalid memory at 0x" << std::hex
                  << (reg[(instr>>21)&31] + im) << ", stop\n";
        return 4;
    }
    reg[(instr>>16)&31] = mem.getWord(reg[(instr>>21)&31] + im);
    return 4;
}

uint32_t sw(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    int32_t im = (int16_t)(instr & 65535);
    if(reg[(instr>>21)&31] + im < 0x400000 ||
       reg[(instr>>21)&31] + im > 0x5e8480)
    {
        std::cout << "\tTrying to access invalid memory at 0x" << std::hex
                  << (reg[(instr>>21)&31] + im) << ", stop\n";
        return 4;
    }
    mem.storeWord(reg[(instr>>16)&31], reg[(instr>>21)&31] + im);
    return 4;
}

uint32_t j(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    PC &= 15 << 28;
    PC |= (instr & 67108863) << 2;
    return 0;
}

uint32_t jal(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    reg[31] = PC + 4;
    PC &= 15 << 28;
    PC |= (instr & 67108863) << 2;
    return 0;
}

uint32_t beq(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    if(reg[(instr>>21)&31] == reg[(instr>>16)&31])
        PC += (instr & 65535) - 4;
    return 4;
}

uint32_t bne(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    if(reg[(instr>>21)&31] != reg[(instr>>16)&31])
        PC += (instr & 65535) - 4;
    return 4;
}

uint32_t execute(Registers & reg, uint32_t instr, Memory & mem, uint32_t & PC)
{
    static std::unordered_map<uint32_t,
                              std::function<uint32_t(Registers &,
                                                     uint32_t, Memory &,
                                                     uint32_t &)>> ops={
        {0, executeR}, {8, addiu}, {9, addiu}, {12, andopi}, {13, oropi},
        {15, lui}, {10, slti}, {11, sltiu}, {32, lb}, {40, sb}, {35, lw},
        {43, sw}, {2, j}, {3, jal}, {28, executeSpecR}, {4, beq}, {5, bne}};
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

int textinput(uint32_t PC, uint32_t tPC, Registers & reg, Memory & mem,
              std::unordered_map< std::string, uint32_t > & labels, std::unordered_map< std::string, std::vector< uint32_t >> & nonregLabels,
              const std::string & input, uint32_t & instr, bool verbose = 1)
{
    static std::unordered_map< std::string, std::string > formats = {
        {"add", "add reg, reg, reg"}, {"addu", "addu reg, reg, reg"},
        {"sub", "sub reg, reg, reg"}, {"subu", "subu reg, reg, reg"},
        {"slt", "slt reg, reg, reg"}, {"sltu", "sltu reg, reg, reg"},
        {"and", "and reg, reg, reg"}, {"or", "or reg, reg, reg"},
        {"nor", "nor reg, reg, reg"}, {"sll", "sll reg, reg, shamt"},
        {"srl", "srl reg, reg, shamt"}, {"addi", "addi reg, reg, im"},
        {"addiu", "addiu reg, reg, im"}, {"andi", "andi reg, reg, im"},
        {"ori", "ori reg, reg, im"}, {"slti", "stli reg, reg, im"},
        {"sltiu", "sltiu reg, reg, im"}, {"lw", "lw reg, im(reg)"},
        {"sw", "sw reg, im(reg)"}, {"j", "j addr"}, {"jr", "jr reg"},
        {"li", "li reg, im"}, {"syscall", "syscall"}, {"la", "la reg, im"}};
    std::string splitinput[4];
    std::string LABEL;
    try
    {
        LABEL = split_input(splitinput, input);
    }
    catch(...)
    {
        return -1;
    }
    if(LABEL != "")
    {
        if(labels.find(LABEL) != labels.end())
        {
            std::cout << "\tDuplicate labels\n";
            return -1;
        }
        labels[LABEL] = PC;
        auto nonregi = nonregLabels.find(LABEL);
        if(nonregi != nonregLabels.end())
        {
            for(uint32_t i : nonregLabels[LABEL])
            {
                uint32_t instr = mem.getWord(i);
                if(((instr >> 26) & 63) == 15)
                    instr |= mem.getCurrtext() >> 16;
                else if(((instr >> 26) & 63) == 13)
                    instr |= mem.getCurrtext() & 65535;
                else if(((instr >> 26) & 63) == 4 ||
                        ((instr >> 26) & 63) == 5)
                    instr |= (mem.getCurrtext() - i) & 65535;
                else
                    instr |= (mem.getCurrtext() >> 2) & 67108863;
                mem.storeWord(instr, i);
            }
            nonregLabels.erase(nonregi);
        }
    }
    if(splitinput[0] == "" || splitinput[0] == ".text")
        return 0;
    if(splitinput[0] == ".data")
        return 1;
    if(splitinput[0] == ".globl")
        splitinput[0] = "j";
    try
    {
        instr = encode(splitinput, PC, labels, nonregLabels, mem);
    }
    catch(LabelException & e)
    {
        if(verbose)
            std::cout << "\tPlease ensure labels are at the front "
                      << "of input and are alphanumeric\n";
        return -1;
    }
    catch(BadInstructException & e)
    {
        if(verbose)
            std::cout << "\tSorry, that instruction doesn't exist "
                      << "or hasn't been implemented yet...\n";
        return -1;
    }
    catch(BadFormatException & e)
    {
        if(verbose)
            std::cout << "\tPlease enter the correct instruction "
                      << "format: " << formats[splitinput[0]]
                      << std::endl;
        return -1;
    }
    catch(RegisterException & e)
    {
        if(verbose)
            std::cout << "\tPlease use valid registers" << std::endl;
        return -1;
    }
    catch(AddressException & e)
    {
        if(verbose)
            std::cout << "\tATTEMPTING TO ACCESS INVALID MEMORY, STOP\n";
        return -1;
    }
    return 0;
}

int datainput(Memory & mem,
              std::unordered_map< std::string, uint32_t > & labels, std::unordered_map< std::string, std::vector< uint32_t >> & nonregLabels,
              const std::string & input, bool verbose = 1)
{
    std::string LABEL;
    std::vector< std::string > splitinput;
    try
    {
        LABEL = split_input(splitinput, input);
        auto nonregi = nonregLabels.find(LABEL);
        if(nonregi != nonregLabels.end())
        {
            for(uint32_t i : nonregLabels[LABEL])
            {
                uint32_t instr = mem.getWord(i);
                if(((instr >> 26) & 63) == 15)
                {
                    instr |= mem.getCurrdata() >> 16;
                }
                else if(((instr >> 26) & 63) == 13)
                    instr |= mem.getCurrdata() & 65535;
                else if(((instr >> 26) & 63) == 4 ||
                        ((instr >> 26) & 63) == 5)
                    instr |= (mem.getCurrdata() - i) & 65535;
                else
                    instr |= (mem.getCurrdata() >> 2) & 67108863;
                mem.storeWord(instr, i);
            }
            nonregLabels.erase(nonregi);
        }
    }
    catch(LabelException & e)
    {
        if(verbose)
            std::cout << "\tPlease ensure labels are at the front "
                      << "of input and are alphanumeric\n";
        return -1;
    }
    catch(BadFormatException & e)
    {
        if(verbose)
            std::cout << "\tPlease ensure all characters are in "
                      << "quotes and all quotes are properly "
                      << "closed\n";
        return -1;
    }
    if(splitinput[0] == ".data" || splitinput[0] == "")
        return 1;
    if(splitinput[0] == ".text")
        return 0;
    if(LABEL != "")
        labels[LABEL] = mem.getCurrdata();
    try
    {
        newData(mem, splitinput);
    }
    catch(const std::invalid_argument & e)
    {
        if(verbose)
            std::cout << "\tMake sure inputs for this type are "
                      << "numeric\n";
        return -1;
    }
    catch(TypeException & e)
    {
        if(verbose)
            std::cout << "\tThat is not a valid type, please use "
                      << "byte, word, asciiz, or space\n";
        return -1;
    }
    catch(SpaceException & e)
    {
        if(verbose)
            std::cout << "\tWARNING: DATA IS ABOUT TO OR CURRENTLY "
                      << "ATTEMPTING OVERWRITING STACK, DO NOT ENTER "
                      << "MORE DATA\n";
        return -1;
    }
    return 1;
}

void reset(Registers & reg, Memory & mem, uint32_t & PC, uint32_t & tPC,
           std::unordered_map< std::string, uint32_t > & labels,
           std::unordered_map< std::string, std::vector< uint32_t >> & nonregLabels)
{
    reg.reset();
    mem.reset();
    PC = 0x400000;
    tPC = PC;
    labels.clear();
    nonregLabels.clear();
}

int main()
{
    Registers reg;
    Memory mem;
    // 0 for text, 1 for data, 2 for simulator commands
    int segment = 0;
    uint32_t PC = 0x400000;
    uint32_t tPC = PC;
    std::unordered_map< std::string, uint32_t > labels;
    std::unordered_map< std::string, std::vector< uint32_t >> nonregLabels;
    std::string input = "";
    uint32_t instr = 0;
    bool run = true;
    std::cout << "MIPS interpreter/SPIM simulator\n"
              << "Enter ? for sim commands\n\n";
    while(run)
    {
        // std::unordered_map< std::uint32_t, std::string > revLabels;
        // for(auto & l : labels)
        //     revLabels[l.second] = l.first;
        instr = 0;
        switch(segment)
        {
            case 0: // Text segment input
                if(PC < mem.getCurrtext() && tPC == PC)
                {
                    instr = mem.getWord(PC);
                    // std::cout << "\texecuting " << decode(instr, PC,
                    //                                       revLabels)
                    //           << " at " << std::hex << PC << std::endl;
                    PC += execute(reg, instr, mem, PC);
                    if(PC == 4)
                    {
                        PC = tPC + 4;
                        tPC = 0;
                    }
                    else
                        tPC = PC;
                    if(reg[29] < mem.getCurrstack())
                        mem.incStack(reg[29]);
                }
                else
                {
                    std::cout << "TEXT:0x" << std::hex << mem.getCurrtext()
                              << " > ";
                    std::getline(std::cin, input);
                    if(input == "?")
                    {
                        segment = 2;
                        break;
                    }
                    segment = textinput(PC, tPC, reg, mem, labels,
                                        nonregLabels, input, instr);
                    if(segment == -1 || instr == 0)
                    {
                        segment = 0;
                        break;
                    }
                    if(tPC == 0)
                    {
                        std::cout << "\tExecution is paused due to syscall "
                                  << "10\n";
                        PC = mem.getCurrtext();
                        break;
                    }
                    else if(nonregLabels.empty())
                        PC = tPC;
                    else
                    {
                        std::cout << "\tExecution stopped until all "
                                  << "labels initialized\n";
                        PC = mem.getCurrtext();
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
                segment = datainput(mem, labels, nonregLabels, input);
                if(segment == -1)
                    segment = 1;
                break;
            case 2:
                std::cout << "*** OPTIONS ***\n"
                          << "t - resume interactive text segment\n"
                          << "d - resume interactive data segment\n"
                          << "R - display register states\n"
                          << "T - display text segment\n"
                          << "D - display data segment\n"
                          << "L - display labels\n"
                          << "S - display current stack\n"
                          << "r - run from beginning\n"
                          << "c - clear\n"
                          << "w - save to file\n"
                          << "o - load from file\n"
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
                else if(input == "T")
                {
                    std::cout << "*** TEXT ***\n";
                    if(!nonregLabels.empty())
                        std::cout << "\t*** WARNING ***\n"
                                  << "Uninitialized labels will have 0 "
                                  << "placeholders in their stead\n"
                                  << std::setfill('-') << std::setw(62) << ""
                                  << std::endl;
                    std::unordered_map< uint32_t, std::string > revlabels;
                    for(auto p : labels)
                        revlabels[p.second] = p.first;
                    for(unsigned int i = 0x400000; i < mem.getCurrtext();
                        i += 4)
                        std::cout << "0x" << std::hex << i << " | "
                                  << decode(mem.getWord(i), i, revlabels)
                                  << std::endl;
                    std::cout << std::setw(62) << "" << std::endl;
                }
                else if(input == "D")
                    mem.showData();
                else if(input == "L")
                {
                    std::cout << "*** LABELS ***\n";
                    for(auto & l : labels)
                        std::cout << l.first << ": 0x" << std::hex << l.second
                                  << std::endl;
                    std::cout << "*** UNREGISTERED LABELS ***\n";
                    for(auto & l : nonregLabels)
                        std::cout << l.first << std::endl;
                    std::cout << std::endl;
                }
                else if(input == "S")
                    mem.showStack(reg[29]);
                else if(input == "r")
                {
                    if(nonregLabels.empty())
                    {
                        PC = 0x400000;
                        tPC = PC;
                        segment = 0;
                    }
                    else
                        std::cout << "\tPlease initialize all labels before "
                                  << "running. Uninitialized labels:\n";
                    for(auto & l: nonregLabels)
                        std::cout << "\t\t" << l.first << std::endl;
                }
                else if(input == "c")
                    reset(reg, mem, PC, tPC, labels, nonregLabels);
                else if(input == "w")
                {
                    if(!nonregLabels.empty())
                    {
                        std::cout << "\tPlease initialize all labels before "
                                  << "saving. Uninitialized labels:\n";
                        for(auto & l : nonregLabels)
                            std::cout << "\t\t" << l.first << std::endl;
                        break;
                    }
                    std::string filename;
                    std::cout << "\tEnter file name: ";
                    std::cin >> filename;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::ofstream outputFile(filename);
                    outputFile << ".text\n.globl main\n\nmain:\n";
                    std::unordered_map< uint32_t, std::string > revlabels;
                    for(auto p : labels)
                        revlabels[p.second] = p.first;
                    for(unsigned int i = 0x400000; i < mem.getCurrtext();
                        i += 4)
                    {
                        if(revlabels.find(i) != revlabels.end())
                            outputFile << revlabels[i] << ": ";
                        outputFile << decode(mem.getWord(i), i, revlabels)
                                   << std::endl;
                    }
                    outputFile << "\n.data\n";
                    for(unsigned int i = 0x461a84; i < mem.getCurrdata();
                        i += 1)
                    {
                        if(revlabels.find(i) != revlabels.end())
                            outputFile << revlabels[i] << ": ";
                        outputFile << ".byte " << mem.getByte(i) << std::endl;
                    }
                    outputFile.close();
                }
                else if(input == "o")
                {
                    std::string filename;
                    std::cout << "\tEnter file name: ";
                    std::cin >> filename;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::ifstream inputFile(filename);
                    if(!inputFile.is_open())
                    {
                        std::cout << "\tPlease enter an actual file\n";
                        break;
                    }
                    reset(reg, mem, PC, tPC, labels, nonregLabels);
                    segment = 0;
                    while(std::getline(inputFile, input))
                    {
                        if(segment == 0)
                        {
                            instr = 0;
                            segment = textinput(PC, tPC, reg, mem, labels,
                                                nonregLabels, input, instr);
                            PC = mem.getCurrtext();
                            tPC = PC;
                        }
                        else
                            segment = datainput(mem, labels, nonregLabels,
                                                input);
                        if(segment == -1)
                        {
                            std::cout << "\tError in file, cannot load\n"
                                      << "\tLine: " << input << std::endl;
                            reset(reg, mem, PC, tPC, labels, nonregLabels);
                            break;
                        }
                    }
                    inputFile.close();
                    segment = 2;
                }
                else if(input == "q")
                    run = 0;
                else
                    std::cout << "Please enter a valid option\n";
                break;
        }
    }
    
    return 0;
}
