// _____________________________________________________________________________________________________________________________________________
// INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...I
// _____________________________________________________________________________________________________________________________________________
// INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...I
// _____________________________________________________________________________________________________________________________________________
// INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...I

#pragma once
#include "!global.h"
#include "layout.h"


// struct that represents an instruction -- instruction descriptor and its operands
struct InstructionData
{
private:
    friend class Instruction;
private:
    uns8 desc {};                             // instruction descriptor
    uns8 data[Lay::Instr::MaxInstrSz-1] {};   // space possibly used by up to two operands

public:
    // get the operation code
    uns8 opcode() const;
    // get the operand value size
    uns8 opervalsz() const;
    // get the unused bits in the instruction descriptor (should always be zero)
    uns8 unused() const;
    
    // get the number of instruction operands
    uns8 opercnt() const;
    // get the number of instruction results
    uns8 rescnt() const;

    // set the operation code
    void opcode(uns8 opcode);
    // set the operand value size
    void opervalsz(uns8 valuesz);
    // set the unused bits in the instruction descriptor (should always be zero)
    void unused(uns8 unused);
};

// struct that represents an operand -- operand descriptor and data the size of up to two bytes
struct OperandData
{
private:
    uns8 desc {};                           // operand descriptor
    uns8 data[Lay::Oper::MaxOperSz-1] {};   // operand data (up to two bytes)

public:
    // get the operand address mode
    uns8 addrmode() const;
    // get the register alias
    uns8 regalias() const;
    // get if the low or high byte of the operand contents should be used (if applicable)
    uns8 lohibyte() const;
    // get the operand content given its size
    uns16 cont(uns8 contsz) const;

    // set the operand address mode
    void addrmode(uns8 addrmode);
    // set the register alias
    void regalias(uns8 regalias);
    // set if the low or high byte of the operand contents should be used (if applicable)
    void lohibyte(uns8 lohibyte);
    // set the operand content given its size
    void cont(uns16 cont, uns8 contsz);
};

class Instruction;   // forward declaration
class Operand;       // forward declaration

// class that parses an instruction from memory and serializes it to memory
class Instruction
{
private:
    InstructionData* instr { nullptr };           // the instruction data in memory
    uns8 maxinstrsz { Lay::Instr::MaxInstrSz };   // maximum instruction size
    mutable uns8 parsed { 1 };                    // size of the parsed part of the instruction
    mutable uns8 operidx { 0 };                   // current operand index

public:
    // construct the instruction
    Instruction(uns8* Loc = nullptr, uns32 availspace = Lay::Instr::MaxInstrSz);

    // throw if the instruction is not valid
    void EnsureValid() const;
    // return if the instruction is valid
    bool IsValid() const;

    // get the operation code
    uns8 opcode() const;
    // get the operand value size
    uns8 opervalsz() const;
    // get the unused bits in the instruction descriptor (should always be zero)
    uns8 unused() const;

    // get the number of instruction operands
    uns8 opercnt() const;
    // get the number of instruction results
    uns8 rescnt() const;

    // get the current instruction operand if possible and move to the next one
    bool GetOper(Operand& operand) const;
    // check if there are no more operands left
    bool IsOperEnd() const;
    // get the current parsed instruction size
    uns8 ParsedSz() const;

    // set the operation code
    void opcode(uns8 opcode);
    // set the operand value size
    void opervalsz(uns8 opervalsz);
    // set the unused bits in the instruction descriptor (should always be zero)
    void unused(uns8 unused);

    // set the current instruction operand if possible and move to the next one
    bool SetOper(Operand& operand);
};

// class that parses an operand from memory and serializes it to memory
class Operand
{
private:
    OperandData* oper { nullptr };            // the operand data in memory
    uns8 opervalsz { 2 };                     // operand value size
    uns8 maxopersz { Lay::Oper::MaxOperSz };  // maximum operand size
    mutable uns8 parsed { 0 };                // size of the parsed part of the operand

public:
    // construct the operand
    Operand(uns8* Loc = nullptr, uns8 opervalsize = 2/*B*/, uns32 availspace = Lay::Oper::MaxOperSz);

    // throw if the operand is not valid
    void EnsureValid() const;
    // return if the operand is valid
    bool IsValid() const;

    // get the operand address mode
    uns8 addrmode() const;
    // get the register alias
    uns8 regalias() const;
    // get the operand value size
    uns8 valsz() const;
    // if the low or high byte of the operand contents should be used (if applicable)
    uns8 lohibyte() const;
    // get the operand content given its size
    uns16 cont(uns8 contsz) const;

    // get the current parsed operand size
    uns8 ParsedSz() const;

    // set the operand address mode
    void addrmode(uns8 addrmode);
    // set the register alias
    void regalias(uns8 regalias);
    // set if the low or high byte of the operand contents should be used (if applicable)
    void lohibyte(uns8 lohibyte);
    // set the operand content given its size
    void cont(uns16 cont, uns8 contsz);
};





