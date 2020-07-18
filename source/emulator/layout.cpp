// _____________________________________________________________________________________________________________________________________________
// LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT
// _____________________________________________________________________________________________________________________________________________
// LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT
// _____________________________________________________________________________________________________________________________________________
// LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT

#include "layout.h"
#include "exception.h"


// ==========================
// struct Psw
// ==========================

// check if the given psw flags are valid
bool Psw::IsValid(uns16 flags) { return !( flags & ((1U << 13) - (1U << 4)) ); }



// ==========================
// struct Opcode
// ==========================

// number of instruction operands
const uns8 Opcode::opercnttable[Cnt] =
{
    0, 0, 0,                                       // halt iret ret
    1, 1,   1, 1, 1, 1,   1, 1,                    // int call   jmp jeq jne jgt   push pop
    2, 2,   2, 2, 2, 2, 2,   2, 2, 2, 2, 2, 2, 2   // xchg mov   add sub mul div cmp   not and or xor test shl shr
};

// number of instruction results for a given opcode
const uns8 Opcode::rescnttable[Cnt] =
{
    0, 0, 0,                                       // halt iret ret
    0, 0,   0, 0, 0, 0,   1, 1,                    // int call   jmp jeq jne jgt   push pop
    2, 1,   1, 1, 1, 1, 0,   1, 1, 1, 1, 0, 1, 1   // xchg mov   add sub mul div cmp   not and or xor test shl shr
};

// check if the given opcode is valid
bool Opcode::IsValid(uns8 opcode) { return opcode < Cnt; }

// get the number of instruction operands for the given opcode
uns8 Opcode::OperCnt(uns8 opcode) { return opercnttable[opcode]; }

// get the number of instruction results for the given opcode
uns8 Opcode::ResCnt(uns8 opcode) { return rescnttable[opcode]; }



// ==========================
// struct AddrMode
// ==========================

// check if the given address mode is valid
bool AddrMode::IsValid(uns8 addrmode) { return addrmode < Cnt; }

// get the size of the operand given the address mode and operand value size
uns8 AddrMode::OperSz(uns8 addrmode, uns8 opervalsz)
{
    switch( addrmode )
    {
        case immed:     return 1 + opervalsz;   // 1B + 1B|2B  |  immediate address mode (value given in operand content)
        case regdir:    return 1;               // 1B + --|--  |  register direct address mode
        case regind:    return 1;               // 1B + --|--  |  register indirect address mode
        case regindoff: return 1 + 2;           // 1B + --|2B  |  register indirect address mode with int16 offset (offset given in operand content)
        case memdir:    return 1 + 2;           // 1B + --|2B  |  memory direct address mode (memory location given in operand content)
        default:        return 0;               // -- + --|--  |  invalid address mode
    }
}



// ==========================
// struct RegAlias
// ==========================

// check if the given register alias is valid
bool RegAlias::IsValid(uns8 regalias) { return regalias < Lay::Cpu::RegCnt || regalias == Value::psw; }



// ==========================
// struct LoHiByte
// ==========================

// check if the given lohibyte option is valid
bool LoHiByte::IsValid(uns8 lohibyte) { return lohibyte <= Value::high; }



// ==========================
// struct AccsRight
// ==========================


// check if the given access rights are valid
bool AccsRight::IsValid(uns8 accsrights) { return accsrights < 0x8; }



// ==========================
// struct MemSeg
// ==========================

// check if the given memory segment is valid
bool MemSeg::IsValid(uns8 memseg) { return memseg <= Value::iomap; }

// get the segment in which the value starting from the given size and with the given size resides in
uns8 MemSeg::GetValSeg(uns16 addr, uns8 valsz)
{
    if( ( addr >= Lay::Mem::Code ::Loc ) && ( int32(addr) + valsz <= Lay::Mem::Code ::Loc + Lay::Mem::Code ::Size ) ) return MemSeg::code;
    if( ( addr >= Lay::Mem::Data ::Loc ) && ( int32(addr) + valsz <= Lay::Mem::Data ::Loc + Lay::Mem::Data ::Size ) ) return MemSeg::data;
    if( ( addr >= Lay::Mem::Data2::Loc ) && ( int32(addr) + valsz <= Lay::Mem::Data2::Loc + Lay::Mem::Data2::Size ) ) return MemSeg::data2;
    if( ( addr >= Lay::Mem::IoMap::Loc ) && ( int32(addr) + valsz <= Lay::Mem::IoMap::Loc + Lay::Mem::IoMap::Size ) ) return MemSeg::iomap;
    
    return MemSeg::invld;
}

// check if the value starting from the given address and of the given size is accessible with the given access rights
bool MemSeg::IsValAccs(uns16 addr, uns8 valsz, uns8 accsrights)
{
    // TODO: remove line when the loader starts supporting segments and privileges
    accsrights |= AccsRight::r|AccsRight::w|AccsRight::x;

    switch( GetValSeg(addr, valsz) )
    {
        case code :
        {
            return !!( accsrights & AccsRight::x );
        }
        case data : case data2 :
        {
            return !!( accsrights & (AccsRight::r|AccsRight::w) );
        }
        case iomap :
        {
            // prevent unaligned access accross word boundaries
            if( addr % 2 == 1 && valsz > 1 ) return false;
            return !!( accsrights & (AccsRight::r|AccsRight::w) );
        }
        default :
        {
            return false;
        }
    }

    return true;
}

// ensure that the value starting from the given address and of the given size is accessible with the given access rights
void MemSeg::EnsureValAccs(uns16 addr, uns8 valsz, uns8 accsrights)
{
    if( !IsValAccs(addr, valsz, accsrights) )
        throw RangeError("data inaccessible or iomap unaligned or between segments or outside of operating memory bounds");
}



// ==========================
// struct ValSize
// ==========================

// check if the given value size is valid
bool ValSize::IsValid(uns8 valsz) { return valsz >= Value::byte && valsz <= Value::word; }



// ==========================
// struct Interrupt
// ==========================

// check if the given interrupt routine index is valid
bool Interrupt::IsValid(uns8 intidx) { return intidx < Lay::Mem::Data2::Ivt::Size; }



// ==========================
// struct TimerDelta
// ==========================

// the number of milliseconds the deltas represent
const uns32 TimerDelta::millistable[Cnt]
{
      500,  // sec0p5
     1000,  // sec1
     1500,  // sec1p5
     2000,  // sec2
     5000,  // sec5
    10000,  // sec10
    30000,  // sec30
    60000   // sec60
};

// check if the given timer delta is valid
bool TimerDelta::IsValid(uns16 timerdelta) { return timerdelta <= Value::sec60; }
// get the number of milliseconds the delta represents
uns32 TimerDelta::GetMillis(uns16 timerdelta)
{
    if( !IsValid(timerdelta) ) timerdelta = Value::sec60;
    return millistable[timerdelta];
}






