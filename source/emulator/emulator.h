// _____________________________________________________________________________________________________________________________________________
// EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR.
// _____________________________________________________________________________________________________________________________________________
// EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR.
// _____________________________________________________________________________________________________________________________________________
// EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR.

#pragma once
#include <memory>
#include "!global.h"
#include "hardware.h"
#include "instruction.h"


// class that represents the emulator
class Emulator
{
private:
    Hardware hw;   // emulated hardware
    std::unique_ptr<Hardware> hwbak { new Hardware() };   // hardware back-up, used when the emulator is reset

private:
    // push a value onto the stack
    void PushToStack( uns16 val );
    // pop a value from the stack
    void PopFromStack( uns16& val );
    // pop a register value from the stack
    void PopFromStack( Register& reg );
    // pop a status register from the stack
    void PopFromStack( StatusRegister& reg );

    // fetch an operand value from instruction/cpu/memory
    void FetchOperVal( const Operand& oper, uns16& val );
    // save an operand value to cpu/memory
    void SaveOperVal( const Operand& oper, uns16 val );

    // update psw flags and move the result to the destination
    void SyncAluFlags( uns16 opcode, int32 res, uns16 dst, uns16 src );

    // fetch instruction and operands
    void FetchInstrAndOper( Instruction& instr, Operand& dst, uns16& dstval, Operand& src, uns16& srcval );
    // execute instruction
    void ExecInstr( uns8 opcode, uns16& dst, uns16& src );
    // save instruction results
    void SaveInstrRes( uns8 opcode, Operand& dst, uns16& dstval, Operand& src, uns16& srcval );
    // handle interrupts
    void HandleInter();

public:
    // initialize the emulator
    bool Init( int argc, char* argv[] );
    // run the emulation
    void StartEmulation();
    // print the contents of emulated registers and memory
    void PrintHw();

};





