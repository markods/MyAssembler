// _____________________________________________________________________________________________________________________________________________
// LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT
// _____________________________________________________________________________________________________________________________________________
// LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT
// _____________________________________________________________________________________________________________________________________________
// LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT...LAYOUT

#pragma once
#include "!global.h"


// various constants related to the hardware layout
struct Lay
{
    // various constants related to the cpu layout
    struct Cpu
    {
        static const uns32 RegCnt = 8;   // number of general purpose registers in the cpu
        static const uns32 PcLoc  = 7;   // location of the program counter register in the general purpose registers
        static const uns32 SpLoc  = 6;   // location of the stack pointer register in the general purpose registers
    };

    // various constants related to the memory layout
    struct Mem
    {
        static const uns32 Loc  = 0x00000;   // start location of the operating memory
        static const uns32 Size = 0x10000;   // size of the operating memory

        // data2 segment
        struct Data2
        {
            static const uns32 Loc  = 0x0000;   // start location of the data2 segment (includes the ivt)
            static const uns32 Size = 0x0100;   // size of the data2 segment (includes the ivt)

            // interrupt vector table location and size
            struct Ivt { static const uns32 Loc = 0x0000, Size = 0x0008; };
        };

        // code segment
        struct Code
        {
            static const uns32 Loc  = 0x0100;   // start location of the code segment
            static const uns32 Size = 0x5f00;   // size of the code segment
        };

        // data segment
        struct Data
        {
            static const uns32 Loc  = 0x6000;   // start location of the data segment
            static const uns32 Size = 0x9f00;   // size of the data segment

            // stack that grows towards lower memory addresses (loc is the highest stack address)
            struct Stack { static const uns32 Loc = 0xfefe, Size = 0x1000; };
        };

        // io mapped segment
        struct IoMap
        {
            static const uns32 Loc  = 0xff00;   // start location of the io segment
            static const uns32 Size = 0x0100;   // size of the io segment

            // terminal location and size
            struct Term  { static const uns32 OutLoc = 0xff00, InLoc = 0xff02; };
            // timer location and size
            struct Timer { static const uns32 DeltaLoc = 0xff10; };
            // emulator location and size
            struct Emu   { static const uns32 ComLoc = 0xff20; };
        };
    };

    // various constants related to the timer layout
    struct Timer
    {
        struct Delta { static const uns16 Mask = (1U << 3)-1, Offs = 0; };   // timer delta bits in the timer delta register
    };

    // various constants related to the instruction layout
    struct Instr
    {
        // instruction descriptor
        struct Opcode    { static const uns8 Mask = (1U << 5)-1, Offs = 3; };   // operation code     bits in the instruction descriptor
        struct OperValSz { static const uns8 Mask = (1U << 1)-1, Offs = 2; };   // operand value size bits in the instruction descriptor
        struct Unused    { static const uns8 Mask = (1U << 2)-1, Offs = 0; };   // unused             bits in the instruction descriptor

        static const uns32 MaxInstrSz   = 7;    // maximum instruction size in bytes
        static const uns32 MaxOperCnt   = 2;    // maximum number of operands in an instruction

        static const uns32 MaxOpcodeCnt = 32;   // maximum possible number of instructions
        static const uns32 OpcodeCnt    = 25;   // size of the instruction set (number of different opcodes)
    };

    // various constants related to the operand layout
    struct Oper
    {
        struct AddrMode { static const uns8 Mask = (1U << 3)-1, Offs = 5; };   // addressing mode  bits in the operand descriptor
        struct RegAlias { static const uns8 Mask = (1U << 4)-1, Offs = 1; };   // register index   bits in the operand descriptor
        struct LoHiByte { static const uns8 Mask = (1U << 1)-1, Offs = 0; };   // low/high bool    bits in the operand descriptor

        static const uns32 MaxOperSz    = 3;    // maximum operand size in bytes
        static const uns32 AddrModeCnt  = 5;    // number of possible address modes
    };
};



// flags in the program status word cpu register
struct Psw
{
    // flags
    enum Value : uns16
    {
        i  = (1U << 15),   // 0 if interrupts are allowed (not masked)
        tr = (1U << 14),   // 0 if the terminal interrupt is allowed (not masked)
        ti = (1U << 13),   // 0 if the timer interrupt is allowed (not masked)

        n  = (1U << 3 ),   // 1 if the number is negative
        c  = (1U << 2 ),   // 1 if the add or sub operation had carry
        o  = (1U << 1 ),   // 1 if the add or sub operation had overflow
        z  = (1U << 0 ),   // 1 if the number is zero
    };

    // check if the given psw flags are valid
    static bool IsValid(uns16 flags);
};



// every possible operation code the cpu recognizes
struct Opcode
{
    // opcodes
    enum Value : uns8
    {
        halt =  0,   // halt          |  power = false;                               |  - - - - -  |  halt program (cut power to the cpu)
        iret =  1,   // iret          |  pop psw; pop pc;                             |     psw     |  return from interrupt
        ret  =  2,   // ret           |           pop pc;                             |  - - - - -  |  return from subroutine
        intr =  3,   // int  dst      |  push pc; push psw; intd; pc = mem16[dst*2];  |  i - - - -  |  call interrupt
        call =  4,   // call dst      |  push pc;           pc = dst;                 |  - - - - -  |  call subroutine
        
        jmp  =  5,   // jmp  dst      |                            pc = dst;          |  - - - - -  |  jump unconditionally
        jeq  =  6,   // jeq  dst      |  if( psw::z              ) pc = dst;          |  - - - - -  |  jump if the psw flag zero is set
        jne  =  7,   // jne  dst      |  if( !psw::z             ) pc = dst;          |  - - - - -  |  jump if the psw flag zero is unset
        jgt  =  8,   // jgt  dst      |  if( !(psw::z || psw::n) ) pc = dst;          |  - - - - -  |  jump if the psw flags zero and negative are unset
        
        push =  9,   // push dst      |  sp  = sp-2; mem16[sp] = dst;                 |  - - - - -  |  push the operand onto the stack
        pop  = 10,   // pop  dst      |  dst = mem16[sp]; sp = sp+2;                  |  - - - - -  |  pop the operand from the stack
        xchg = 11,   // xchg src dst  |  tmp = dst; dst = src; src = tmp;             |  - - - - -  |  exchange two operands
        mov  = 12,   // mov  src dst  |  dst = src;                                   |  - z - - n  |  move an operand into another operand
        
        add  = 13,   // add  src dst  |  dst = dst+src;                               |  - z o c n  |  add two operands
        sub  = 14,   // sub  src dst  |  dst = dst-src;                               |  - z o c n  |  subtract two operands
        mul  = 15,   // mul  src dst  |  dst = dst*src;                               |  - z - - n  |  multiply two operands
        divv = 16,   // div  src dst  |  dst = dst/src;                               |  - z - - n  |  divide two operands
        cmp  = 17,   // cmp  src dst  |  tmp = dst-src;                               |  - z o c n  |  compare two operands, don't move the result into the dst operand
        
        nott = 18,   // not  src dst  |  dst =    ~src;                               |  - z - - n  |  bitwise negate an operand
        andd = 19,   // and  src dst  |  dst = dst&src;                               |  - z - - n  |  bitwise and two operands
        orr  = 20,   // or   src dst  |  dst = dst|src;                               |  - z - - n  |  bitwise or two operands
        xorr = 21,   // xor  src dst  |  dst = dst^src;                               |  - z - - n  |  bitwise xor two operands
        test = 22,   // test src dst  |  tmp = dst&src;                               |  - z - - n  |  bitwise compare (and) two operands, don't move the result into the dst operand
        shl  = 23,   // shl  src dst  |  dst = dst<<src;                              |  - z - c n  |  bitwise shift left an operand given the shift in the other operand
        shr  = 24,   // shr  src dst  |  dst = dst>>src;                              |  - z - c n  |  bitwise shift right an operand given the shift in the other operand
    };

    // number of opcodes
    static const uns32 Cnt = Lay::Instr::OpcodeCnt;
private:
    // number of instruction operands for a given opcode
    static const uns8 opercnttable[Cnt];
    // number of instruction results for a given opcode
    static const uns8 rescnttable[Cnt];

public:
    // check if the given opcode is valid
    static bool IsValid(uns8 opcode);
    // get the number of instruction operands for the given opcode
    static uns8 OperCnt(uns8 opcode);
    // get the number of instruction results for the given opcode
    static uns8 ResCnt(uns8 opcode);
};

// various addressing modes for the operands
struct AddrMode
{
    // address modes
    enum Value : uns8
    {
        immed     = 0x0,   // 1B + 1B|2B  |  immediate address mode (value given in operand content)
        regdir    = 0x1,   // 1B + --|--  |  register direct address mode
        regind    = 0x2,   // 1B + --|--  |  register indirect address mode
        regindoff = 0x3,   // 1B + --|2B  |  register indirect address mode with int16 offset (offset given in operand content)
        memdir    = 0x4,   // 1B + --|2B  |  memory direct address mode (memory location given in operand content)
    };
    
    // number of different address modes
    static const uns32 Cnt = Lay::Oper::AddrModeCnt;

    // check if the given address mode is valid
    static bool IsValid(uns8 addrmode);
    // get the size of the operand given the address mode and operand value size
    static uns8 OperSz(uns8 addrmode, uns8 opervalsz);
};



// aliases of the cpu registers
struct RegAlias
{
    // register aliases
    enum Value : uns8
    {
        rx0 = 0x0,   // general purpose register 0
        rx1 = 0x1,   // general purpose register 1
        rx2 = 0x2,   // general purpose register 2
        rx3 = 0x3,   // general purpose register 3
        rx4 = 0x4,   // general purpose register 4
        rx5 = 0x5,   // general purpose register 5
        rx6 = 0x6,   // general purpose register 6
        rx7 = 0x7,   // general purpose register 7
        psw = 0xf,   // program status word
    };

    // check if the given register alias is valid
    static bool IsValid(uns8 regalias);
};

// if the operation uses the high or low byte of the operand
struct LoHiByte
{
    // low or high byte constants
    enum Value : uns8
    {
        low  = 0x0,   // operation uses the low  byte of the operand
        high = 0x1,   // operation uses the high byte of the operand
    };

    // check if the given lohibyte option is valid
    static bool IsValid(uns8 lohibyte);
};



// all the memory access rights
struct AccsRight
{
    // privileges
    enum Value : uns8
    {
        r = 0x1,   // read access right
        w = 0x2,   // write access right
        x = 0x4,   // execute access right
    };

    // check if the given access right is valid
    static bool IsValid(uns8 accsrights);
};

// all of the memory segments
struct MemSeg
{
    // memory segments
    enum Value : uns8
    {
        code  = 0x0,   // code segment
        data  = 0x1,   // data segment
        data2 = 0x2,   // data2 segment
        iomap = 0x3,   // io map segment
        invld = 0x4,   // invalid segment
    };

    // check if the given memory segment is valid
    static bool IsValid(uns8 memseg);
    // get the segment in which the value starting from the given size and with the given size resides in
    static uns8 GetValSeg(uns16 addr, uns8 valsz);
    // check if the value starting from the given address and of the given size is accessible with the given access rights
    static bool IsValAccs(uns16 addr, uns8 valsz, uns8 accsrights);
    // ensure that the value starting from the given address and of the given size is accessible with the given access rights
    static void EnsureValAccs(uns16 addr, uns8 valsz, uns8 accsrights);
};

// all the possible storage sizes for a value
struct ValSize
{
    // storage sizes
    enum Value : uns8
    {
        byte = 0x1,   // value that is stored in one byte
        word = 0x2,   // value that is stored in two bytes
    };

    // check if the given value size is valid
    static bool IsValid(uns8 valsz);
};



// default interrupt routines
struct Interrupt
{
    // interrupt routines' locations
    enum Value : uns8
    {
        Unmaskable = 0,   // start of the unmaskable interrupts
        ResetCpu   = 0,   // interrupt that resets the cpu
        InstrErr   = 1,   // instruction error interrupt

        Maskable   = 2,   // start of the maskable interrupts
        Timer      = 2,   // timer interrupt
        Term       = 3,   // terminal interrupt
    };

    // check if the given interrupt routine index is valid
    static bool IsValid(uns8 intidx);
};

// timer delta constants
struct TimerDelta
{
    // timer deltas
    enum Value : uns16
    {
        sec0p5 = 0x0,   // timer measures half a second
        sec1   = 0x1,   // timer measures one second
        sec1p5 = 0x2,   // timer measures one and a half second
        sec2   = 0x3,   // timer measures two seconds
        sec5   = 0x4,   // timer measures five seconds
        sec10  = 0x5,   // timer measures ten seconds
        sec30  = 0x6,   // timer measures thirty seconds
        sec60  = 0x7,   // timer measures sixty seconds
    };
    // number of different timer deltas
    static const uns32 Cnt = 8;

private:
    // the number of milliseconds the deltas represent
    static const uns32 millistable[Cnt];

public:
    // check if the given timer delta is valid
    static bool IsValid(uns16 timerdelta);
    // get the number of milliseconds the delta represents
    static uns32 GetMillis(uns16 timerdelta);
};





