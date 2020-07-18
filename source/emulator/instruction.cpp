// _____________________________________________________________________________________________________________________________________________
// INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...I
// _____________________________________________________________________________________________________________________________________________
// INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...I
// _____________________________________________________________________________________________________________________________________________
// INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...INSTRUCTION...I

#include <string>
#include "exception.h"
#include "instruction.h"


// ==========================
// struct InstructionData
// ==========================

// get the operation code
uns8 InstructionData::opcode() const    { return          ( ( desc >> Lay::Instr::Opcode   ::Offs ) & Lay::Instr::Opcode   ::Mask ); }
// get the operand value size
uns8 InstructionData::opervalsz() const { return 1/*B*/ + ( ( desc >> Lay::Instr::OperValSz::Offs ) & Lay::Instr::OperValSz::Mask ) /*0B|1B*/; }
// get the unused bits in the instruction descriptor (should always be zero)
uns8 InstructionData::unused() const    { return          ( ( desc >> Lay::Instr::Unused   ::Offs ) & Lay::Instr::Unused   ::Mask ); }

// get the number of instruction operands
uns8 InstructionData::opercnt() const { return Opcode::OperCnt(opcode()); }
// get the number of instruction results
uns8 InstructionData::rescnt() const  { return Opcode::ResCnt(opcode()); }

// set the operation code
void InstructionData::opcode(uns8 opcode)       { uns8 mask = Lay::Instr::Opcode   ::Mask; uns8 offs = Lay::Instr::Opcode   ::Offs;   desc = (desc & ~(mask << offs)) | ((opcode    & mask) << offs); }
// set the operand value size
void InstructionData::opervalsz(uns8 opervalsz) { uns8 mask = Lay::Instr::OperValSz::Mask; uns8 offs = Lay::Instr::OperValSz::Offs;   desc = (desc & ~(mask << offs)) | ((opervalsz & mask) << offs); }
// set the unused bits in the instruction descriptor (should always be zero)
void InstructionData::unused(uns8 unused)       { uns8 mask = Lay::Instr::Unused   ::Mask; uns8 offs = Lay::Instr::Unused   ::Offs;   desc = (desc & ~(mask << offs)) | ((unused    & mask) << offs); }



// ==========================
// struct OperandData
// ==========================

// get the operand address mode
uns8 OperandData::addrmode() const { return ( ( desc >> Lay::Oper::AddrMode::Offs ) & Lay::Oper::AddrMode::Mask ); }
// get the register alias
uns8 OperandData::regalias() const { return ( ( desc >> Lay::Oper::RegAlias::Offs ) & Lay::Oper::RegAlias::Mask ); }
// get if the low or high byte of the operand contents should be used (if applicable)
uns8 OperandData::lohibyte() const { return ( ( desc >> Lay::Oper::LoHiByte::Offs ) & Lay::Oper::LoHiByte::Mask ); }

// get the operand content given its size
uns16 OperandData::cont(uns8 contsz) const
{
    switch( contsz )
    {
        case 1  /* 1B */ : return *((uns8* ) data);
        case 2  /* 2B */ : return *((uns16*) data);
        default          : throw RangeError("invalid operand content size");
    }
}

// set the operand address mode
void OperandData::addrmode(uns8 addrmode) { uns8 mask = Lay::Oper::AddrMode::Mask; uns8 offs = Lay::Oper::AddrMode::Offs;   desc = (desc & ~(mask << offs)) | ((addrmode & mask) << offs); }
// set the register alias
void OperandData::regalias(uns8 regalias) { uns8 mask = Lay::Oper::RegAlias::Mask; uns8 offs = Lay::Oper::RegAlias::Offs;   desc = (desc & ~(mask << offs)) | ((regalias & mask) << offs); }
// set if the low or high byte of the operand contents should be used (if applicable)
void OperandData::lohibyte(uns8 lohibyte) { uns8 mask = Lay::Oper::LoHiByte::Mask; uns8 offs = Lay::Oper::LoHiByte::Offs;   desc = (desc & ~(mask << offs)) | ((lohibyte & mask) << offs); }
// set the operand content given its size
void OperandData::cont(uns16 cont, uns8 contsz)
{
    switch( contsz )
    {
        case 1  /* 1B */ : { *((uns8* ) data) = (uns8)  cont;   break; }
        case 2  /* 2B */ : { *((uns16*) data) = (uns16) cont;   break; }
        default          : throw RangeError("invalid operand content size");
    }
}




// ==========================
// class Instruction
// ==========================

// construct the instruction
Instruction::Instruction(uns8* loc, uns32 availspace)
{
    instr = (InstructionData*) loc;
    // +0 because apparently the gcc compiler doesn't allow rvalue references to static constants (it probably optimizes them away)
    maxinstrsz = (uns8) std::min(availspace, Lay::Instr::MaxInstrSz+0);
}

// throw if the instruction is not valid
void Instruction::EnsureValid() const
{
    // if the instruction descriptor is out of bounds, throw an exception
    if( maxinstrsz == 0 ) throw ParseError("instruction out of bounds");

    // if the opcode is invalid, throw an exception
    if( !Opcode::IsValid(opcode()) ) throw ParseError("invalid opcode");
    // if the unused instruction bits are not zero, throw an exception
    if( instr->unused() != 0 ) throw ParseError("unused instruction bits are not zero");

    // create a temporary variable for storing the current instruction operand
    Operand oper {};
    // create temporary variables for storing the old parsed and operidx values
    uns8 parsed_old { parsed }, operidx_old { operidx };

    try
    {
        // for all the instruction operands
        for( int32 i = 0; i < opercnt(); i++ )
        {
            try
            {
                // if the operand could not be read, throw an exception
                if( !GetOper(oper) ) throw ParseError("operand missing or out of bounds");
                
                // ensure that the operand is valid
                oper.EnsureValid();

                // if the operand is a destination operand (its index starts after all the non-destination operands) and the address mode is immediate, throw an exception
                if( i >= opercnt() - rescnt() && oper.addrmode() == AddrMode::immed ) throw ParseError("invalid address mode for the destination operand");
            }
            // if any of the operands is not valid, throw an exception
            catch(const LogicError& ex)
            {
                throw ParseError(std::string("operand ") + std::to_string(i) + ": " + ex.what());
            }
        }

        // restore the old parsed and operidx values
        parsed  = parsed_old;
        operidx = operidx_old;
    }
    catch(const LogicError&)
    {
        // restore the old parsed and operidx values
        parsed  = parsed_old;
        operidx = operidx_old;
        throw;
    }
}

// return if the instruction is valid
bool Instruction::IsValid() const
{
    try
    {
        // ensure that the instruction is valid
        EnsureValid();
        // return that the instruction is valid
        return true;
    }
    catch(const LogicError&)
    {
        // if the instruction is not valid, return false
        return false;
    }
}

// get the operation code
uns8 Instruction::opcode() const { return instr->opcode(); }
// get the operand value size
uns8 Instruction::opervalsz() const { return instr->opervalsz(); }
// get the unused bits in the instruction descriptor (should always be zero)
uns8 Instruction::unused() const { return instr->unused(); }
// get the number of instruction operands
uns8 Instruction::opercnt() const { return instr->opercnt(); }
// get the number of instruction results
uns8 Instruction::rescnt() const { return instr->rescnt(); }

// get the current instruction operand if possible and move to the next one
bool Instruction::GetOper(Operand& operand) const
{
    // if the current operand to be parsed doesn't exist, return
    if( operidx >= instr->opercnt() ) return false;
    // if the current operand would start outside the instruction bounds, return
    if( parsed >= maxinstrsz ) return false;

    // create a temporary operand and initialize it with the operand from the instruction
    Operand temp { &instr->data[ parsed-1 ], instr->opervalsz(), uns8(maxinstrsz - parsed) };
    // calculate the operand's size
    uns8 opersz = temp.ParsedSz();

    // if the operand doesn't fit in the instruction bounds, return
    if( parsed + opersz > maxinstrsz ) return false;

    // increase the instruction size
    parsed += opersz;
    // move to the next operand in the instruction (possibly nonexistent)
    operidx++;
    // copy the temporary operand to the given operand
    operand = temp;

    // return that the operand was sucessfully parsed and saved in the given operand
    return true;
}

// check if there are no more operands left
bool Instruction::IsOperEnd() const { return operidx >= instr->opercnt(); }
// get the current parsed instruction size
uns8 Instruction::ParsedSz() const { return parsed; }

// set the operation code
void Instruction::opcode(uns8 opcode) { instr->opcode(opcode); }
// set the operand value size
void Instruction::opervalsz(uns8 opervalsz) { instr->opervalsz(opervalsz); }
// set the unused bits in the instruction descriptor (should always be zero)
void Instruction::unused(uns8 unused) { instr->unused(unused); }

// set the current instruction operand if possible and move to the next one
bool Instruction::SetOper(Operand& operand)
{
    // if the current operand to be serialized doesn't exist, return
    if( operidx >= instr->opercnt() ) return false;
    // if the operand value size doesn't match the instruction value size, return
    if( operand.valsz() != instr->opervalsz() ) return false;
    // if the operand doesn't fit in the instruction bounds, return
    if( parsed + operand.ParsedSz() > maxinstrsz ) return false;

    // create an operand inside the instruction data starting from the first not-yet-serialized byte location
    Operand ioperand { &instr->data[ parsed-1 ], instr->opervalsz(), uns8(maxinstrsz - parsed) };

    // serialize the given operand to the instruction data
    ioperand = operand;
    // increase the instruction size
    parsed += ioperand.ParsedSz();
    // move to the next operand in the instruction (possibly nonexistent)
    operidx++;

    // return that the operand was sucessfully serialized to the instruction data
    return true;
}




// ==========================
// class Operand
// ==========================

// construct the operand
Operand::Operand(uns8* loc, uns8 opervalsize, uns32 /*availspace*/)
{
    oper = (OperandData*) loc;
    opervalsz = opervalsize;
    maxopersz = Lay::Oper::MaxOperSz;   // std::min((uns8) availspace, ParsedSz());
}

// throw if the operand is not valid
void Operand::EnsureValid() const
{
    // if the address mode is invalid, throw an exception
    if( !AddrMode::IsValid(addrmode()) ) throw ParseError("invalid address mode");
    
    // if the register alias is invalid, throw an exception
    if( !RegAlias::IsValid(regalias()) ) throw ParseError("invalid register alias");
    
    // if the lo/hi byte option is invalid, throw an exception
    if( !LoHiByte::IsValid(lohibyte()) ) throw ParseError("invalid lo/hi byte option");

    // if the operand size is out of bounds, throw an exception
    if( ParsedSz() > maxopersz )
        throw ParseError("invalid operand size");
}

// return if the operand is valid
bool Operand::IsValid() const
{
    try
    {
        // ensure that the operand is valid
        EnsureValid();
        // return that the operand is valid
        return true;
    }
    catch(const LogicError&)
    {
        // if the operand is not valid, return false
        return false;
    }
}

// get the operand address mode
uns8 Operand::addrmode() const { return oper->addrmode(); }
// get the register alias
uns8 Operand::regalias() const { return oper->regalias(); }
// get the operand value size
uns8 Operand::valsz() const { return opervalsz; }
// if the low or high byte of the operand contents should be used (if applicable)
uns8 Operand::lohibyte() const { return oper->lohibyte(); }
// get the operand content given its size
uns16 Operand::cont(uns8 contsz) const { return oper->cont(contsz); }

// get the current parsed operand size
uns8 Operand::ParsedSz() const
{
    if( parsed != 0 ) return parsed;
    
    parsed = AddrMode::OperSz(addrmode(), opervalsz);
    return parsed;
}

// set the operand address mode
void Operand::addrmode(uns8 addrmode) { oper->addrmode(addrmode); }
// set the register alias
void Operand::regalias(uns8 regalias) { oper->regalias(regalias); }
// set if the low or high byte of the operand contents should be used (if applicable)
void Operand::lohibyte(uns8 lohibyte) { oper->lohibyte(lohibyte); }
// set the operand content given its size
void Operand::cont(uns16 cont, uns8 contsz) { oper->cont(cont, contsz); }





