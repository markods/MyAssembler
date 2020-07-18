// _____________________________________________________________________________________________________________________________________________
// HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE.
// _____________________________________________________________________________________________________________________________________________
// HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE.
// _____________________________________________________________________________________________________________________________________________
// HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE...HARDWARE.

#pragma once
#include <chrono>
#include "!global.h"
#include "layout.h"


// struct that represents a register
struct Register
{
protected:
    uns16 rx {};   // the entire register data (two bytes)

public:
    // initialize the register
    void Init();
    // assignment operator
    Register& operator=(uns16 val);
    // access the entire register data
    operator uns16();

    // get all of the register bytes
    uns16 get() const;
    // get the register bytes of the given size and at the given position in the register
    uns16 get(uns8 valsz, uns8 lohibyte) const;
    // set all of the register bytes
    void set(uns16 val);
    // set the register bytes of the given size and at the given position in the register
    void set(uns16 val, uns8 valsz, uns8 lohibyte);
};

// struct that represents a status register
struct StatusRegister : public Register
{
    // assignment operator
    StatusRegister& operator=(uns16 val);
    // access the entire register data
    operator uns16();
    
    // check if one or more flags are set
    bool IsSet(uns16 flags) const;
    // check if one or more flags are not set
    bool IsRst(uns16 flags) const;
    // set one or more flags
    void Set(uns16 flags);
    // reset one or more flags
    void Rst(uns16 flags);
    // set or reset one or more flags to be equal to the given value
    void Equ(uns16 flags, bool val);
};

// struct that represents a cpu
struct Cpu
{
public:
    static const uns32 RegCnt   = Lay::Cpu::RegCnt;             // number of general purpose registers in the cpu
    static const uns32 IrqLines = Lay::Mem::Data2::Ivt::Size;   // number of interrupt request pins connected to the cpu

private:
    Register reg[RegCnt] {};    // general purpose registers (including the program counter and stack pointer)
    StatusRegister pswreg {};   // program status word register

    bool intreq[IrqLines] {};   // interrupt request pin states (on/off)
    bool cpupower { true };     // cpu power (on/off)

public:
    // initialize the cpu registers
    void Init();

    // access the general purpose registers or the psw register (as an ordinary register)
    Register& rx(uns8 alias);
    // access the program counter register
    Register& pc();
    // access the stack pointer register
    Register& sp();
    // access the program status word register
    StatusRegister& psw();

    // access the interrupt request lines
    bool& irq(uns8 idx);
    // access the cpu power
    bool& power();
};



struct Memory;     // forward declaration
struct Hardware;   // forward declaration

// struct that represents a stack that grows towards lower memory addresses and always points to an empty 2B location
struct Stack
{
private:
    uns8* mem;      // start of the operating memory in which the stack resides
    Register* sp;   // stack pointer located in the cpu
    uns16 base;     // stack base location in the operating memory
    uns16 maxsz;    // maximum stack size

public:
    // construct the stack
    Stack(Memory& memory, Register& stackpointer, uns16 stackbase = Lay::Mem::Data::Stack::Loc, uns16 maxstacksz = Lay::Mem::Data::Stack::Size);

    // initialize the stack (also initialize the stack pointer cpu register)
    void Init();

private:
    // check if the stack pointer is inside the memory reserved for the stack, if not throw
    void EnsureSpInStack();

public:
    // push a value onto the stack
    void Push(uns16 val);
    // pop a value from the stack
    void Pop(uns16& val);
};

// struct that represents an interrupt (vector) table
struct InterruptTable
{
private:
    uns16* entry {};   // start location of the interrupt table
    uns16 ivtsz {};    // size of the interrupt table

public:
    // construct the interrupt table
    InterruptTable(Memory& memory, uns16 ivtloc = Lay::Mem::Data2::Ivt::Loc, uns16 ivtsize = Lay::Mem::Data2::Ivt::Size);

    // initialize the interrupt table
    void Init();
    // get the address of the interrupt routine with the given index
    uns16& operator[](uns8 idx);

    // get the interrupt table size
    uns16 Size();
};

// struct that represents a terminal
struct Terminal
{
private:
    Cpu* cpu {};     // used to signal interrupt to cpu
    uns16* in {};    // data input register (from the keyboard)
    uns16* out {};   // data output register (to the screen)

public:
    // construct the terminal
    Terminal(Hardware& hw, uns16 inputloc = Lay::Mem::IoMap::Term::InLoc, uns16 outputloc = Lay::Mem::IoMap::Term::OutLoc);
    // initialize the terminal
    void Init();

    // get the terminal input
    uns16 input() const;
    // get the terminal output
    uns16 output() const;

    // set the terminal input
    void input(uns16 val);
    // set the terminal output
    void output(uns16 val);

    // check if the terminal input is ready
    bool CheckInputReady();
};

// struct that represents a timer
struct Timer
{
private:
    using Clock  = std::chrono::steady_clock;
    using Millis = std::chrono::milliseconds;

private:
    Cpu* cpu {};        // used to signal interrupt to cpu
    uns16* status {};   // periodically (with this period) call the timer interrupt routine
    Millis start {};    // timer start point in milliseconds

private:
    // get the current unix epoch time in milliseconds
    static Millis GetTime();

public:
    // construct the timer
    Timer(Hardware& hw, uns16 deltaloc = Lay::Mem::IoMap::Timer::DeltaLoc);
    // initialize the timer
    void Init();

    // get the timer delta
    uns16 delta() const;
    // set the timer delta
    void delta(uns16 val);

    // check if the delta elapsed since the previous function call
    bool CheckTimeElapsed();
};

// struct that represents operating memory and memory mapped io devices
// the smallest addressable unit is a byte
struct Memory
{
public:
    static const uns32 Size = Lay::Mem::Mem::Size;   // size of the memory in bytes
    
private:
    uns8 cont[Size] {};   // memory contents
    Hardware* hw {};      // hardware to which this memory belongs to

public:
    // construct the operating memory
    Memory(Hardware& hw);
    // initialize the operating memory
    void Init();

    // get a reference to the 1B data from the given address given the access rights
    uns8& get8(uns16 addr, uns8 accsrights);
    // get a reference to the the 2B data from the given address given the access rights
    uns16& get16(uns16 addr, uns8 accsrights);

    // get the data of the given size from the given address given the access rights
    uns16 get(uns16 addr, uns8 datasz, uns8 accsrights);
    // set the data of the given size at the given address given the access rights
    void set(uns16 value, uns16 addr, uns8 datasz, uns8 accsrights);
};



// struct that represents the system hardware
struct Hardware
{
    Cpu cpu {};                      // central processing unit
    Memory mem { *this };            // operating memory

    Terminal term { *this };         // terminal
    Timer timer { *this };           // timer
    InterruptTable ivt { mem };      // interrupt table
    Stack stack { mem, cpu.sp() };   // stack

    // initialize the system hardware
    void Init();
};





