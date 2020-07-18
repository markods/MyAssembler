// _____________________________________________________________________________________________________________________________________________
// HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE.
// _____________________________________________________________________________________________________________________________________________
// HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE.
// _____________________________________________________________________________________________________________________________________________
// HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE.

#include <iostream>
#include <termios.h>
#include <unistd.h>
#include "exception.h"
#include "hardware.h"


// ==========================
// struct StatusRegister
// ==========================

// assignment operator
StatusRegister& StatusRegister::operator=(uns16 val) { rx = val; return *this; }
// access the entire register data
StatusRegister::operator uns16() { return rx; }

// check if one or more flags are set
bool StatusRegister::IsSet(uns16 flags) const { return ( rx & flags) == flags; }
// check if one or more flags are not set
bool StatusRegister::IsRst(uns16 flags) const { return (~rx & flags) == flags; }
// set one or more flags
void StatusRegister::Set(uns16 flags) { rx |=  flags; }
// reset one or more flags
void StatusRegister::Rst(uns16 flags) { rx &= ~flags; }
// set or reset one or more flags to be equal to the given value
void StatusRegister::Equ(uns16 flags, bool val)
{
    if( val ) Set(flags);
    else      Rst(flags);
}



// ==========================
// struct Register
// ==========================

// initialize the register
void Register::Init() { rx = 0; }
// assignment operator
Register& Register::operator=(uns16 val) { rx = val; return *this; }
// access the entire register data
Register::operator uns16() { return rx; }

// get all of the register bytes
uns16 Register::get() const { return rx; }

// get the register bytes of the given size and at the given position in the register
uns16 Register::get(uns8 valsz, uns8 lohibyte) const
{
    if( !LoHiByte::IsValid(lohibyte) ) throw RangeError("invalid lo/hi byte option");

    switch( valsz )
    {
        case ValSize::word : { return rx; }
        case ValSize::byte : { return *((uns8*)&rx + lohibyte); }
        default: throw RangeError("invalid value size");
    }
}

// set all of the register bytes
void Register::set(uns16 val) { rx = val; }

// set the register bytes of the given size and at the given position in the register
void Register::set(uns16 val, uns8 valsz, uns8 lohibyte)
{
    if( !LoHiByte::IsValid(lohibyte) ) throw RangeError("invalid lo/hi byte option");

    switch( valsz )
    {
        case ValSize::word : { rx = val; break; }
        case ValSize::byte : { *((uns8*)&rx + lohibyte) = (uns8)val; break; }
        default: throw RangeError("invalid value size");
    }
}



// ==========================
// struct Cpu
// ==========================

// initialize the cpu registers
void Cpu::Init()
{
    // initialize all of the general purpose registers
    for( uns32 i = 0; i < RegCnt; i++ ) reg[i].Init();
    // initialize the program status word register
    pswreg.Init();

    // initialize all of the interrupt request lines
    for( uns32 i = 0; i < IrqLines; i++ ) intreq[i] = false;
    // set that the cpu has power
    cpupower = true;
}

// access the general purpose registers or the psw register (as an ordinary register)
Register& Cpu::rx(uns8 alias)
{
    if( !RegAlias::IsValid(alias) ) throw RangeError("invalid general purpose register alias");

    if( alias != RegAlias::psw ) return reg[alias];
    else                         return pswreg;
}
// access the program counter register
Register& Cpu::pc() { return reg[Lay::Cpu::PcLoc]; }
// access the stack pointer register
Register& Cpu::sp() { return reg[Lay::Cpu::SpLoc]; }
// access the program status word register
StatusRegister& Cpu::psw() { return pswreg; }

// access the interrupt request lines
bool& Cpu::irq(uns8 idx)
{
    if( idx >= IrqLines ) throw RangeError("invalid interrupt request line index");
    return intreq[idx];
}
// access the cpu power
bool& Cpu::power() { return cpupower; }



// ==========================
// struct Stack
// ==========================

// construct the stack
Stack::Stack(Memory& memory, Register& stackpointer, uns16 stackbase, uns16 maxstacksz)
{
    mem = &memory.get8(0, AccsRight::r|AccsRight::w);
    sp = &stackpointer;
    base = stackbase;
    maxsz = maxstacksz;
}

// initialize the stack (also initialize the stack pointer register)
void Stack::Init() { *sp = base; }

// check if the stack pointer is inside the memory reserved for the stack, if not throw
void Stack::EnsureSpInStack()
{
    if( *sp > base         ) throw RangeError("stack underflow");
    if( *sp < base - maxsz ) throw RangeError("stack overflow" );
}

// push a value onto the stack
void Stack::Push(uns16 val)
{
    EnsureSpInStack();
    *(uns16*) &mem[*sp] = val;

    *sp = *sp - 2;
}

// pop a value from the stack
void Stack::Pop(uns16& val)
{
    *sp = *sp + 2;
    
    EnsureSpInStack();
    val = *(uns16*) &mem[*sp];
}

// ==========================
// struct Terminal
// ==========================

// construct the terminal
Terminal::Terminal(Hardware& hw, uns16 inputloc, uns16 outputloc)
{
    cpu = &hw.cpu;
    in  = &hw.mem.get16(inputloc,  AccsRight::r|AccsRight::w);
    out = &hw.mem.get16(outputloc, AccsRight::r|AccsRight::w);
}

// initialize the terminal
void Terminal::Init() { *in = 0; *out = 0; }

// get the terminal input
uns16 Terminal::input() const { return *in; }
// get the terminal output
uns16 Terminal::output() const { return *out; }

// set the terminal input
void Terminal::input(uns16 val) { *in = val; }
// set the terminal output
void Terminal::output(uns16 val) { *out = val; write(STDOUT_FILENO, out, 1); }

// check if the terminal input is ready
bool Terminal::CheckInputReady()
{
    if( cpu->irq(Interrupt::Term) ) return false;

    char c {};
    if( read(STDIN_FILENO, &c, 1) <= 0 ) return false;
    
    input( c );
    cpu->irq( Interrupt::Term ) = true;
    return true;
}




// ==========================
// struct Timer
// ==========================

// get the current unix epoch time in milliseconds
Timer::Millis Timer::GetTime()
{
    return std::chrono::duration_cast<Millis>(Clock::now().time_since_epoch());;
}

// construct the timer
Timer::Timer(Hardware& hw, uns16 deltaloc)
{
    cpu = &hw.cpu;
    status = &hw.mem.get16(deltaloc, AccsRight::r|AccsRight::w);
    start = GetTime();
}

// initialize the timer
void Timer::Init()
{
    *status = TimerDelta::sec0p5;
    start = GetTime();
}

// get the timer delta
uns16 Timer::delta() const { return *status; }
// set the timer delta
void Timer::delta(uns16 val)
{
    *status = ( (val >> Lay::Timer::Delta::Offs) & Lay::Timer::Delta::Mask );
    CheckTimeElapsed();
}

// has the delta elapsed since the previous function call
bool Timer::CheckTimeElapsed()
{
    Millis time { GetTime() };
    Millis timedelta { TimerDelta::GetMillis( delta() ) };

    if( time - start < timedelta ) return false;
    start = time;

    cpu->irq( Interrupt::Timer ) = true;
    return true;
}



// ==========================
// struct InterruptTable
// ==========================

// construct the interrupt table
InterruptTable::InterruptTable(Memory& memory, uns16 ivtloc, uns16 ivtsize)
{
    entry = &memory.get16(ivtloc, AccsRight::r|AccsRight::w);
    ivtsz = ivtsize;
}

// initialize the interrupt table
void InterruptTable::Init()
{
    // initialize the interrupt routines' addresses to be invalid
    for( uns32 i = 0; i < Lay::Mem::Data2::Ivt::Size; i++ ) entry[i] = 0;
}

// get the interrupt routine with the given index
uns16& InterruptTable::operator[](uns8 idx)
{
    if( idx >= ivtsz ) throw RangeError("requested interrupt entry outside of table");
    return entry[idx];
}

// get the interrupt table size
uns16 InterruptTable::Size() { return ivtsz; }



// ==========================
// struct Memory
// ==========================

// construct the operating memory
Memory::Memory(Hardware& hardware)
{
    hw = &hardware;
}

// initialize the operating memory
void Memory::Init() { for( uns32 i = 0; i < Size; i++ ) cont[i] = 0xcd; }

// get the 1B data from the given address given the access rights
uns8& Memory::get8(uns16 addr, uns8 accsrights)
{
    MemSeg::EnsureValAccs(addr, ValSize::byte, accsrights);
    return *( (uns8*) &cont[addr] );
}

// get the 2B data from the given address given the access rights
uns16& Memory::get16(uns16 addr, uns8 accsrights)
{
    MemSeg::EnsureValAccs(addr, ValSize::word, accsrights);
    return *( (uns16*) &cont[addr] );
}

// get the data of the given size from the given address given the access rights
uns16 Memory::get(uns16 addr, uns8 datasz, uns8 accsrights)
{
    MemSeg::EnsureValAccs(addr, datasz, accsrights);

    switch( datasz )
    {
        case ValSize::byte: return *( (uns8*)  &cont[addr] );
        case ValSize::word: return *( (uns16*) &cont[addr] );
        default: throw RangeError("invalid value size");
    }
}

// set the data of the given size at the given address given the access rights
void Memory::set(uns16 value, uns16 addr, uns8 datasz, uns8 accsrights)
{
    MemSeg::EnsureValAccs(addr, datasz, accsrights);

    switch( datasz )
    {
        case ValSize::byte: { *( (uns8*)  &cont[addr] ) = (uns8)  value; break; }
        case ValSize::word: { *( (uns16*) &cont[addr] ) = (uns16) value; break; }
        default: throw RangeError("invalid value size");
    }

    switch( (uns32) addr )
    {
        case Lay::Mem::IoMap::Term ::OutLoc   : { hw->term.output( value ); break; }
        case Lay::Mem::IoMap::Timer::DeltaLoc : { hw->timer.delta( value ); break; }
    }
}


// ==========================
// struct Hardware
// ==========================

// initialize the system hardware
void Hardware::Init()
{
    cpu.Init();
    mem.Init();

    stack.Init();
    ivt.Init();
    term.Init();
    timer.Init();
}





