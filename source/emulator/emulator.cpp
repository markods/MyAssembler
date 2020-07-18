// _____________________________________________________________________________________________________________________________________________
// EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR.
// _____________________________________________________________________________________________________________________________________________
// EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR.
// _____________________________________________________________________________________________________________________________________________
// EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR...EMULATOR.

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "exception.h"
#include "emulator.h"


// push a value onto the stack
void Emulator::PushToStack( uns16 val ) { hw.stack.Push(val); }

// pop a value from the stack
void Emulator::PopFromStack( uns16& val ) { hw.stack.Pop(val); }

// pop a register value from the stack
void Emulator::PopFromStack( Register& reg ) { uns16 val; hw.stack.Pop(val); reg = val; }

// pop a status register from the stack
void Emulator::PopFromStack( StatusRegister& reg ) { uns16 val; hw.stack.Pop(val); reg = val; }



// fetch an operand value from instruction/cpu/memory
void Emulator::FetchOperVal( const Operand& oper, uns16& val )
{
    // temporary variable used to hold the address of the operand value in memory
    uns16 addr {};

    // calculate the operand value or address in memory
    switch( oper.addrmode() )
    {
        // if the address mode is immediate
        case AddrMode::immed :
        {
            // get the value of the given size from the operand content
            val = oper.cont(oper.valsz());
            // return the value
            return;
        }
        // if the address mode is register direct
        case AddrMode::regdir :
        {
            // get the register from the cpu holding the value
            Register reg = hw.cpu.rx(oper.regalias());
            // get the value of the given size and with the given lo/hi option from the register
            val = reg.get(oper.valsz(), oper.lohibyte());
            // return the value
            return;
        }
        // if the address mode is register indirect
        case AddrMode::regind :
        {
            // get the register from the cpu holding the address of the value
            Register reg = hw.cpu.rx(oper.regalias());
            // get the address of the value from the register
            addr = reg.get();
            // break
            break;
        }
        // if the address mode is register indirect with offset
        case AddrMode::regindoff :
        {
            // get the register from the cpu holding the address of the value
            Register reg = hw.cpu.rx(oper.regalias());
            // get the address of the value from the register and add to it the 2B offset
            addr = uns32(reg.get()) + int32(oper.cont(ValSize::word));
            // break
            break;
        }
        // if the address mode is memory direct
        case AddrMode::memdir :
        {
            // get the address of the value from the operand content
            addr = oper.cont(ValSize::word);
            // break
            break;
        }
        // if the address mode is invalid
        default :
        {
            // throw an exception
            throw RangeError("invalid address mode");
        }
    }

    // read the operand from memory
    val = hw.mem.get(addr, oper.valsz(), AccsRight::r);
}

// save an operand value to cpu/memory
void Emulator::SaveOperVal( const Operand& oper, uns16 val )
{
    // temporary variable used to hold the address of the operand value in memory
    uns16 addr {};

    // save the operand value or calculate the operand address in memory
    switch( oper.addrmode() )
    {
        // if the address mode is register direct
        case AddrMode::regdir :
        {
            // get the register from the cpu that will hold the value
            Register& reg = hw.cpu.rx(oper.regalias());
            // save the value of the given size and with the given lo/hi option to the register
            reg.set(val, oper.valsz(), oper.lohibyte());
            // return
            return;
        }
        // if the address mode is register indirect
        case AddrMode::regind :
        {
            // get the register from the cpu holding the address of the value
            Register& reg = hw.cpu.rx(oper.regalias());
            // get the address of the value from the register
            addr = reg.get();
            // break
            break;
        }
        // if the address mode is register indirect with offset
        case AddrMode::regindoff :
        {
            // get the register from the cpu holding the address of the value
            Register& reg = hw.cpu.rx(oper.regalias());
            // get the address of the value from the register and add to it the 2B offset 
            addr = uns32(reg.get()) + int32(oper.cont(ValSize::word));
            // break
            break;
        }
        // if the address mode is memory direct
        case AddrMode::memdir :
        {
            // get the address of the value from the operand content
            addr = oper.cont(ValSize::word);
            // break
            break;
        }
        // if the address mode is invalid
        default :
        {
            // throw an exception
            throw RangeError("invalid address mode");
        }
    }

    // write the operand to memory
    hw.mem.set(val, addr, oper.valsz(), AccsRight::w);
}



// update psw flags and move the result to the destination
void Emulator::SyncAluFlags( uns16 opcode, int32 res, uns16 dst, uns16 src )
{
    // shorthand notation to common cpu registers
    StatusRegister& psw { hw.cpu.psw() };

    // update carry and overflow flags
    switch( opcode )
    {
        case Opcode::add : { psw.Equ(Psw::c, dst +         src > int16max); psw.Equ(Psw::o, res < int16min || res > int16max); break; }
        case Opcode::sub : { psw.Equ(Psw::c, dst + (uns16)-src > int16max); psw.Equ(Psw::o, res < int16min || res > int16max); break; }
        case Opcode::cmp : { psw.Equ(Psw::c, dst + (uns16)-src > int16max); psw.Equ(Psw::o, res < int16min || res > int16max); break; }
        case Opcode::shl : { psw.Equ(Psw::c, (uns16)res & (1U << 15));                                                         break; }
        case Opcode::shr : { psw.Equ(Psw::c, (uns16)res & (1U << 0 ));                                                         break; }
    }

    // update zero and negative flags
    psw.Equ(Psw::z, res == 0);
    psw.Equ(Psw::n, res <  0);
}



// fetch instruction and operands
void Emulator::FetchInstrAndOper( Instruction& instr, Operand& dst, uns16& dstval, Operand& src, uns16& srcval )
{
    // fetch the instruction and operands
    try
    {
        // shorthand notation to common cpu registers
        Register& pc { hw.cpu.pc() };
        
        // if the program counter is not in the text segment, throw an exception
     // if( (uns16)pc <  Lay::Mem::Code::Loc
     //  || (uns16)pc >= Lay::Mem::Code::Loc + Lay::Mem::Code::Size ) throw RangeError("pc out of code segment");

        // fetch the instruction descriptor from the code segment
     // instr = Instruction { &hw.mem.get8(pc, AccsRight::x), Lay::Mem::Code::Loc + Lay::Mem::Code::Size - pc };
        instr = Instruction { &hw.mem.get8(pc, AccsRight::x), Lay::Instr::MaxInstrSz };
        // ensure that the instruction is valid before accessing its operands and update the program counter
        instr.EnsureValid(); pc = pc + instr.ParsedSz();

        // fetch the source operand if it exists, update the program counter and fetch the operand value
        if( instr.opercnt() >= 2 ) { instr.GetOper(src); pc = pc + src.ParsedSz(); FetchOperVal(src, srcval); }
        // fetch the destination operand if it exists, update the program counter and fetch the operand value
        if( instr.opercnt() >= 1 ) { instr.GetOper(dst); pc = pc + dst.ParsedSz(); FetchOperVal(dst, dstval); }
    }
    // if there is an exception during the fetch phase
    catch(const LogicError& ex)
    {
        // throw an instruction/operand fetch error
        throw FetchError(std::string("fetch error: ") + ex.what());
    }
}

// execute instruction
void Emulator::ExecInstr( uns8 opcode, uns16& dst, uns16& src )
{
    try
    {
        // shorthand notation to common cpu registers
        Register& pc { hw.cpu.pc() };
        StatusRegister& psw { hw.cpu.psw() };
        // temporary value used in arithmetic and logic operations
        int32 res { 0 };

        // execute the instruction
        switch( opcode )
        {
            case Opcode::halt: { hw.cpu.power() = false; break; }
            case Opcode::iret: { PopFromStack(psw); PopFromStack(pc); break; }
            case Opcode::ret : {                    PopFromStack(pc); break; }
            
            case Opcode::intr: { PushToStack(pc); PushToStack(psw); psw.Set(Psw::i); pc = hw.ivt[(uns8)dst]; break; }
            case Opcode::call: { PushToStack(pc);                                    pc = dst;               break; }

            case Opcode::jmp : {                                pc = dst; break; }
            case Opcode::jeq : { if( psw.IsSet(Psw::z       ) ) pc = dst; break; }
            case Opcode::jne : { if( psw.IsRst(Psw::z       ) ) pc = dst; break; }
            case Opcode::jgt : { if( psw.IsRst(Psw::z|Psw::n) ) pc = dst; break; }

            case Opcode::push: { PushToStack(dst);  break; }
            case Opcode::pop : { PopFromStack(dst); break; }

            case Opcode::xchg: { std::swap(dst, src);                                      break; }
            case Opcode::mov : { res = dst = src;     SyncAluFlags(opcode, res, dst, src); break; }

            case Opcode::add : { res = (int16)dst  + (int16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::sub : { res = (int16)dst  - (int16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::mul : { res = (int16)dst  * (int16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::divv: { res = (int16)dst  / (int16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::cmp : { res = (int16)dst  - (int16)src; SyncAluFlags(opcode, res, dst, src);                   break; }

            case Opcode::nott: { res =             ~ (uns16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::andd: { res = (uns16)dst  & (uns16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::orr : { res = (uns16)dst  | (uns16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::xorr: { res = (uns16)dst  ^ (uns16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::test: { res = (uns16)dst  & (uns16)src; SyncAluFlags(opcode, res, dst, src);                   break; }
            case Opcode::shl : { res = (uns16)dst << (uns16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            case Opcode::shr : { res = (uns16)dst >> (uns16)src; SyncAluFlags(opcode, res, dst, src); dst = (uns16)res; break; }
            default          : throw RangeError("invalid opcode");
        }
    }
    // if there is an exception during the execution phase
    catch(const LogicError& ex)
    {
        // throw an execution error
        throw ExecError(std::string("exec error: ") + ex.what());
    }
}

// save instruction results
void Emulator::SaveInstrRes( uns8 opcode, Operand& dst, uns16& dstval, Operand& src, uns16& srcval )
{
    // store the instruction results
    try
    {
        // save the source instruction result to its operand if it exists
        if( Opcode::ResCnt(opcode) >= 2 ) { SaveOperVal(src, srcval); }
        // save the destination instruction result to its operand if it exists
        if( Opcode::ResCnt(opcode) >= 1 ) { SaveOperVal(dst, dstval); }
    }
    // if there is an exception during the operand store phase
    catch(const LogicError& ex)
    {
        // throw an operand store error
        throw StoreError(std::string("store error: ") + ex.what());
    }
}

// handle interrupts
void Emulator::HandleInter()
{
    // handle interrupts
    try
    {
        // if the cpu power is off, return 
        if( !hw.cpu.power() ) return;

        // create a variable for storing the interrupt request line that will be handled
        uns8 line;

        // check if there are interrupts coming from interrupt request lines
        for( line = 0; line < hw.ivt.Size(); line++ )
        {
            // if all of the maskable interrupts are masked, skip them
            if( line >= Interrupt::Maskable && hw.cpu.psw().IsSet(Psw::i) ) { line = hw.ivt.Size(); break; }

            // if the interrupt request line is set and an interrupt handler is registered
            if( hw.cpu.irq(line) && hw.ivt[line] != 0 )
            {
                // and the current interrupt is not masked, save the interrupt number and break
                if     ( line == Interrupt::Timer && hw.cpu.psw().IsSet(Psw::ti) ) break;
                else if( line == Interrupt::Term  && hw.cpu.psw().IsSet(Psw::tr) ) break;
                else                                                               break;
            }
        }

        // if the emulator's initial state should be restored
        if( line == 0 )
        {
            hw.cpu.irq(line) = false;
            hw = *hwbak;
            return;
        }

        // handle only the most important interrupt
        if( line < hw.ivt.Size() )
        {
            hw.cpu.irq(line) = false;
            uns16 dst { line }, src {};
            ExecInstr(Opcode::intr, dst, src);
        }
    }
    // if there is an exception during the interrupt handle phase
    catch(const LogicError& ex)
    {
        throw InterError(std::string("interrupt error: ") + ex.what());
    }
}




// table of sections
struct Section
{
    std::string SecName {};
    std::string SecType {};
    int32 SecSize {};            // declared size of the section in bytes
    int32 SecReqAddr {};         // requested position for the section in memory
    int32 SecBegAddr {};         // final position of the section in memory
    int32 SecEndAddr {};         // address of the first byte outside the section in memory
    int32 SecStatus {};          // status of the section
};
std::unordered_map< std::string, Section > Sec;

// table of symbols
struct Symbol
{
    std::string SymName {};
    std::string SymSection {}; // section to which this symbol belongs
    int32 SymOffs {};          // offset of the symbol from the beginning of a section
    char SymType {};         
        
    int32 SymAddr {};          // final position of the symbol in memory
};
std::unordered_map< std::string, Symbol > Sym;

// table of relocations
struct Relocation
{
    std::string RelSection {};  // the section in which relocation should be done
    std::string RelSymbol {};   // when the value of the symbol becomes known...
    int32 RelOffs {};           // ...place it on this offset inside the section...
    std::string RelType {};     // ...using this format
};
std::vector< Relocation > Rel;

// table of section contents
std::unordered_map< std::string, std::vector<uns8> > SecContent;

// extract string column from a line
bool ExtractStr( std::string& col, std::string& line, std::string ColName, std::string TblName, int32 ExtractMode = EXTRACT_DEFAULT )
{
    col = "";

    // find the separator between the current and the next column
    std::size_t TabPos = (int32) line.find( '\t' );
    if( !(ExtractMode & EXTRACT_LASTCOLUMN) && TabPos == std::string::npos ) { std::cout << "missing colums(s) from table '" << TblName << "'\n"; return false; } 
    if(  (ExtractMode & EXTRACT_LASTCOLUMN) && TabPos != std::string::npos ) { std::cout << "excess column(s) in table '"  << TblName << "'\n"; return false; } 

    // extract col from the line
    col = line.substr( 0, TabPos );
    line = line.substr( (size_t)TabPos+1, line.length()-TabPos-1 );

    // check if col is empty
    if( !(ExtractMode & EXTRACT_EMPTY) && col.length() == 0 ) { std::cout << "missing column '" << ColName << "' from table '" << TblName << "'\n"; return false; }

    return true;
}

// extract integer column from a line
bool ExtractDec( int32& col, std::string& line, std::string ColName, std::string TblName, int32 ExtractMode = EXTRACT_DEFAULT )
{
    col = 0;

    // extract column as string
    std::string str;
    if( ! ExtractStr( str, line, ColName, TblName, ExtractMode ) ) return false;

    // convert string to int
    std::size_t pos = 0;
    col = std::stoi( str, &pos, 10 ); // decimal conversion
    if( pos != str.length() ) { std::cout << "extra character(s) found in column '" << ColName << "' in table '" << TblName << "'\n"; return false; }

    return true;
}

// extract char column from a line
bool ExtractChar( char& col, std::string& line, std::string ColName, std::string TblName, int32 ExtractMode = EXTRACT_DEFAULT )
{
    col = ' ';

    // extract column as string
    std::string str;
    if( ! ExtractStr( str, line, ColName, TblName, ExtractMode ) ) return false;

    // convert string to char
    if( str.length() != 1 ) { std::cout << "extra character(s) found in column '" << ColName << "' in table '" << TblName << "'\n"; return false; }
    col = str[0];

    return true;
}

// extract hex column (formated as "hh ") from a line
bool ExtractHex2( uns8& byte, std::string& line, std::string TblName )
{
    byte = 0;

    if( line.length() < 3 ) { std::cout << "incomplete lowercase hex byte found on line '" << line << "' in table '" << TblName << "'\n"; return false; }

    // convert two hex digits to byte
    for( int32 i = 0; i < 2; i++ )
    {
             if( '0' <= line[i] && line[i] <= '9' ) byte = byte * 16 + line[i] - '0';
        else if ('a' <= line[i] && line[i] <= 'f' ) byte = byte * 16 + line[i] - 'a' + 10;
        else { std::cout << "unexpected character in lowercase hex byte found on line '" << line[i] << "' in table '" << TblName << "'\n"; return false; }
    }

    if( line[2] != ' ' ) { std::cout << "invalid separator after lowercase hex byte found on line '" << line[2] << "' in table '" << TblName << "'\n"; return false; }

    // remove extracted part of the line
    line = line.substr( 3, line.length() - 3 );

    return true;
}




// initialize the emulator
bool Emulator::Init( int argc, char* argv[] )
{
    // initialize hardware
    hw.Init();

    std::cout << "========================== loading started ==========================\n";



    // cycle through all command-line arguments - in the first step declare placing of sections, and load files in the second step
    std::cout << "argc = " << argc << '\n';
    for( int32 step = 1; step <= 2; step++ )
    {
        std::cout << "----- " << (step == 1 ? "placing" : "reading") << " started -----\n";
        
        for( int32 a = 1; a < argc; a++ )
        {
            std::string arg = argv[a];

            if( arg.substr(0,7) == "-place=" ) // command-line argument "-place"
            {
                if( step == 1 ) // "-place" command-line arguments are processed in the first step
                {
                    std::cout << "argv[" << a << "] = " << arg << '\n';

                    // find the separator between the section name and requested section address columns
                    arg = arg.substr( 7, arg.length()-7 );
                    std::size_t at = arg.find( '@' );
                    if( at == std::string::npos ) { std::cout << "missing '@' in '-place' declaration\n"; return false; } 

                    // section name
                    std::string sec = arg.substr( 0, at );
                    if( sec.length() == 0 ) { std::cout << "missing section name in '-place' declaration\n"; return false; }

                    // requested section address
                    std::string addrstr = arg.substr( (uns64)at+1, arg.length()-at-1 );
                    if( addrstr.length() == 0 ) { std::cout << "missing address in '-place' declaration\n"; return false; }
                    std::size_t pos = 0;
                    int32 addr = std::stoi( addrstr, &pos, 16 ); // hexadecimal conversion
                    if( pos != addrstr.length() ) { std::cout << "extra characters found in address '" << addrstr << "' in '-place' declaration\n"; return false; }
                    if( Sec.find( sec ) != Sec.end() ) { std::cout << "section '" << sec << "' already placed\n"; return false; }

                    // declare section placing
                    Section* section = new Section;
                    section->SecName       = sec;
                    section->SecType       = "";
                    section->SecSize       = SIZE_UNKNOWN;
                    section->SecReqAddr    = addr;
                    section->SecBegAddr    = ADDR_UNKNOWN;
                    section->SecEndAddr    = ADDR_UNKNOWN;
                    section->SecStatus     = SEC_PLACED;
                    Sec.insert( {sec, *section} );
                }
            }
            else // command-line argument "filename" 
            {
                if( step == 2 )  // "filename" command-line arguments are processed in the second step
                {
                    std::cout << "argv[" << a << "] = " << arg << '\n';

                    // open the input file
                    std::ifstream infile(arg);
                    if( !infile.is_open() ) { std::cout << "file '" << arg << "' could not be opened\n"; return false; }

                    // for all lines in the input file
                    std::string line;
                    std::string CurrSecName;
                    int32 reading = READING_BEG;
                    while( std::getline(infile, line) )
                    {
                        std::cout << "   '" << line << '\n';   // print the line currently being analyzed

                        if( line.length() == 0 ) break; // empty line is a signal to stop processing any further lines
                        if( line[0] == ';' ) continue;  // skip comment line

                        // the section table is expected at the beginning
                        if( line == ".sections" )  // section table
                        { 
                            if( reading != READING_BEG ) { std::cout << "section table missing from the beginning of file\n"; return false; }

                            reading = READING_SECTBL; 
                            continue; 
                        } 

                        // the symbol table is expected after the section table
                        if( line == ".symbols" ) 
                        { 
                            if( reading != READING_SECTBL ) { std::cout << "symbol table missing after section table in file\n"; return false; }

                            reading = READING_SYMTBL; 
                            continue; 
                        } 

                        // a relocation table is expected anywhere (except at the beginning), after all section names are already known
                        if( line.substr(0,5) == ".rel." )
                        {
                            if( reading == READING_BEG ) { std::cout << "relocation table must not start at the beginning of file\n"; return false; }

                            std::string SecName = line.substr(5, line.length()-5);

                            if( Sec.find( SecName ) == Sec.end() )        { std::cout << "found relocation table for an unknown section '" << SecName << '\n'; return false; } 
                            if( Sec[SecName].SecType == "bss" )           { std::cout << "bss section '" << SecName << "' can't have a relocation table\n"; return false; } 
                            if( Sec[SecName].SecStatus & SEC_RELDEFINED ) { std::cout << "relocation table of section '" << SecName << "' is already defined\n"; return false; } 

                            Sec[ SecName ].SecStatus |= SEC_RELDEFINED;  // mark that the relation table of this section has been found
                            CurrSecName = SecName;                       // switch context to the new section
                            reading = READING_RELTBL; 
                            continue; 
                        } 
                
                        // section content is expected anywhere (except at the beginning), after all section names are already known
                        if( line[0] == '.' )
                        {
                            if( reading == READING_BEG ) { std::cout << "unexpected section content at the beginning of file\n"; return false; }

                            std::string SecName = line.substr(1, line.length()-1);

                            if( Sec.find( SecName ) == Sec.end() )               { std::cout << "found content of an unknown section '" << SecName << '\n'; return false; } 
                            if( Sec[SecName].SecType == "bss" )                  { std::cout << "bss section '" << SecName << "' can't have content\n"; return false; } 
                            if( Sec[SecName].SecStatus & SEC_CONTENTDEFINED )    { std::cout << "content of section '" << SecName << "' is aleady defined\n"; return false; } 
                            if( SecContent.find( SecName ) != SecContent.end() ) { std::cout << "content of section '" << SecName << "' is aleady defined\n"; return false; } // just in case, for debugging purpose

                            Sec[SecName].SecStatus |= SEC_CONTENTDEFINED;   // mark that the content of a section is defined
                            SecContent.insert( { SecName, {} } );           // allocate place where the section content will be stored
                            CurrSecName = SecName;                          // switch context to the new section
                            reading = READING_SECCONTENT; 
                            continue;  
                        }
                
                        // a new section declaration is expected
                        if( reading == READING_SECTBL && line[0] != '.' ) 
                        {
                            // extract section name
                            std::string SecName;  
                            if( ! ExtractStr( SecName, line, "Name", "Section" ) ) return false;

                            // extract section type
                            std::string SecType;  
                            if( ! ExtractStr( SecType, line, "Type", "Section" ) ) return false;
                            if( SecType != "bss" && SecType != "data" && SecType != "text" ) { std::cout << "unknown type '" << SecType << "'of section '" << SecName << "'\n"; return false; }

                            // extract section size (the last column)
                            int32 SecSize;
                            if( ! ExtractDec( SecSize, line, "Size", "Section", EXTRACT_LASTCOLUMN ) ) return false;

                            // declare a new section, or update existing (previously "placed") section
                            Section* section;
                            auto s = Sec.find( SecName );
                            if( s != Sec.end() ) // section already exists in the table of sections
                            {
                                section = &s->second;
                                if( section->SecStatus & SEC_DECLARED ) { std::cout << "section is already declared\n"; return false; }

                                // declare existing section
                                section->SecType       = SecType;
                                section->SecSize       = SecSize;
                                section->SecStatus    |= SEC_DECLARED; 
                            }
                            else // section does not exist in the table of sections
                            {
                                // declare new section
                                Section* section = new Section;
                                section->SecName       = SecName;
                                section->SecType       = SecType;
                                section->SecSize       = SecSize;
                                section->SecReqAddr    = ADDR_UNKNOWN;
                                section->SecBegAddr    = ADDR_UNKNOWN;
                                section->SecEndAddr    = ADDR_UNKNOWN;
                                section->SecStatus     = SEC_DECLARED;
                                Sec.insert( {SecName, *section} );
                            }
                            continue;
                        }

                        // a new symbol declaration is expected
                        if( reading == READING_SYMTBL && line[0] != '.' ) 
                        {
                            // extract symbol name
                            std::string SymName;
                            if( ! ExtractStr( SymName, line, "Name", "Symbol" ) ) return false;

                            // extract section to which this symbol belongs
                            std::string SymSection;
                            if( ! ExtractStr( SymSection, line, "Section", "Symbol", EXTRACT_EMPTY ) ) return false;

                            // extract offset of the symbol from the beginning of a section
                            int32 SymOffs;
                            if( ! ExtractDec( SymOffs, line, "Offset", "Symbol" ) ) return false;

                            // extract symbol type (the last column)
                            char SymType;
                            if( ! ExtractChar( SymType, line, "Type", "Symbol", EXTRACT_LASTCOLUMN ) ) return false;
                            if( SymType != 'E' && SymType != 'G' && SymType != 'S' && SymType != 'L' ) { std::cout << "unknown symbol type '" << SymType << "' for symbol '" << SymName << "'\n"; return false; }

                            // symbol must not exist in the table of symbols
                            if( Sym.find( SymName ) != Sym.end() ) { std::cout << "symbol '" << SymName << "' is already declared\n"; return false; }  

                            // extern symbols are not kept in table of symbols
                            if( SymType == 'E' ) continue;

                            // declare a new symbol
                            Symbol* symbol = new Symbol;
                            symbol->SymName       = SymName;
                            symbol->SymSection    = SymSection;
                            symbol->SymOffs       = SymOffs;
                            symbol->SymType       = SymType;
                            symbol->SymAddr       = ADDR_UNKNOWN;
                            Sym.insert( {SymName, *symbol} );
                            continue;
                        }

                        // a new relocation declaration is expected
                        if( reading == READING_RELTBL && line[0] != '.' ) 
                        {
                            // extract symbol name
                            std::string RelSymbol;
                            if( ! ExtractStr( RelSymbol, line, "Symbol", "Relocation" ) ) return false;

                            // extract the relocated symbol offset in the current section
                            int32 RelOffs;
                            if( ! ExtractDec( RelOffs, line, "Offset", "Relocation" ) ) return false;

                            // extract the relocation type
                            std::string RelType;
                            if( ! ExtractStr( RelType, line, "Type", "Relocation", EXTRACT_LASTCOLUMN ) ) return false;
                            if( RelType != "abs16" && RelType != "abs8" && RelType != "rel16" && RelType != "rel8" ) { std::cout << "unknown relocation type '" << RelType << "' for symbol '" << RelSymbol << "'\n"; return false; }

                            // declare a new relocation
                            Relocation* rel = new Relocation;
                            rel->RelSection = CurrSecName;
                            rel->RelSymbol  = RelSymbol;
                            rel->RelOffs    = RelOffs;
                            rel->RelType    = RelType;
                            Rel.push_back( { *rel } );
                            continue;
                        }

                        // content of the section is expected
                        if( reading == READING_SECCONTENT && line[0] != '.' ) 
                        {
                            uns8 byte;
                            while( line.length() > 0 )
                            {
                                //  number of bytes already loaded               expected section size
                                if( int32( SecContent[ CurrSecName ].size() ) >= Sec[ CurrSecName ].SecSize ) { std::cout << "unexpected bytes past the declared end of the section\n"; return false; } ;

                                if( ! ExtractHex2( byte, line, CurrSecName ) ) return false;

                                SecContent[ CurrSecName ].push_back( { byte } );  // store extracted byte for later use
                            }
                            continue;
                        }

                        std::cout << "unrecognized line '" << line << '\n'; return false;
                    }
                }
            }
        }
        std::cout << "----- " << (step == 1 ? "placing" : "reading") << " finished -----\n";
    }
   


    // check if all placed sections are declared
    for( auto& e : Sec )
        if( (e.second.SecStatus & SEC_PLACED) && !(e.second.SecStatus & SEC_DECLARED) ) { std::cout << "section '" << e.second.SecName << "' is being placed but has not been declared\n"; return false; } ;

    // check if all declared sections have content (except for the bss sections)
    for( auto& e : Sec )
        if( e.second.SecType != "bss" && (e.second.SecStatus & SEC_DECLARED) && !(e.second.SecStatus & SEC_CONTENTDEFINED) ) { std::cout << "missing content of declared section '" << e.second.SecName << '\n'; return false; } ;

    // check if all declared bytes are present in the section content
    for( auto& e : Sec )
        if( (e.second.SecStatus & SEC_CONTENTDEFINED) && e.second.SecSize != (int32) SecContent[e.second.SecName].size() ) { std::cout << "incomplete content of section '" << e.second.SecName << '\n'; return false; } ;



    // arrange all placed sections in memory according to their desired starting addresses (calculate BegAddr and EndAddr)
    // also find out the maximum EndAddr (the starting position after which all non-placed sections will be loaded) and check for overlap and memory overflow
    int32 MaxEndAddr = 0;
    for( auto& e : Sec )
    {
        if( !(e.second.SecStatus & SEC_PLACED) ) continue; // only "-place" sections are arranged here

        e.second.SecBegAddr = e.second.SecReqAddr;
        e.second.SecEndAddr = e.second.SecBegAddr + e.second.SecSize;  // SecEndAddr does not! belong to its section

        if( e.second.SecBegAddr < 0                   ) { std::cout << "ERROR - section '" << e.second.SecName << "' must begin on a positive address\n"; return false;}
        if( e.second.SecEndAddr > (int32) hw.mem.Size ) { std::cout << "ERROR - not enough memory for section '" << e.second.SecName << '\n'; return false;}

        // check if the current section is overlapping with any other section
        for( auto& x : Sec )
        {
            if( x.second.SecName    == e.second.SecName ) continue; // skip the current section when checking for overlap with the current section
            if( x.second.SecBegAddr <= e.second.SecEndAddr && e.second.SecBegAddr <= x.second.SecEndAddr ) { std::cout << "ERROR - section '" << e.second.SecName << "' overlaps with section '" << x.second.SecName << '\n'; return false; }
        }

        if( MaxEndAddr < e.second.SecEndAddr )
            MaxEndAddr = e.second.SecEndAddr;
    }

    // arrange all non-placed sections in memory (calculate BegAddr and EndAddr) and increase EndAddr by the section size in each step
    // check for memory overflow
    for( auto& e : Sec )
    {
        if( e.second.SecStatus & SEC_PLACED ) continue; // only non-placed sections are arranged here

        e.second.SecBegAddr = MaxEndAddr;
        e.second.SecEndAddr = e.second.SecBegAddr + e.second.SecSize;  // SecEndAddr does not! belong to its section

        if( e.second.SecEndAddr > (int32) hw.mem.Size ) { std::cout << "ERROR - not enough memory for section '" << e.second.SecName << '\n'; return false;}

        MaxEndAddr = e.second.SecEndAddr;
    }

    // resolve all symbols in the symbol table (calculate Addr)
    for( auto& e : Sym )
    {
        if( e.second.SymSection == "" ) continue; // only symbols inside known sections are resolved

        if( Sec.find( e.second.SymSection ) == Sec.end() ) { std::cout << "ERROR - symbol '" << e.second.SymName << "' belongs to an unknown section '" << e.second.SymSection << '\n'; return false;}

        e.second.SymAddr = Sec[ e.second.SymSection ].SecBegAddr + e.second.SymOffs;
    }


    // print the section table
    std::cout << "-------------------------- Section Table ------------------------------\n";
    std::cout << "SecName\tSecType\tSecSize\tReqAddr\tBegAddr\tEndAddr\tStatus\n";
    for( auto& e : Sec )
    {
        std::cout << e.second.SecName << "\t" << e.second.SecType << "\t" << e.second.SecSize << "\t" 
                  << e.second.SecReqAddr << "\t" << e.second.SecBegAddr << "\t" << e.second.SecEndAddr << "\t"
                  << (e.second.SecStatus & SEC_PLACED         ? "Placed "         : "") 
                  << (e.second.SecStatus & SEC_DECLARED       ? "Declared "       : "") 
                  << (e.second.SecStatus & SEC_RELDEFINED     ? "RelDefined "     : "") 
                  << (e.second.SecStatus & SEC_CONTENTDEFINED ? "ContentDefined " : "") 
                  << (e.second.SecStatus & SEC_LOADED         ? "Loaded "         : "") << '\n';
    }
    std::cout << "-----------------------------------------------------------------------\n";

    // print the symbol table
    std::cout << "---------------- Symbol Table -----------------------------------------\n";
    std::cout << "SymName\tSection\tOffs\tType\tAddr\n";
    for( auto& e : Sym )
        std::cout << e.second.SymName << "\t" << e.second.SymSection << "\t" << e.second.SymOffs << "\t" << e.second.SymType << "\t" << e.second.SymAddr << '\n';
    std::cout << "-----------------------------------------------------------------------\n";

    // print the relocation table
    std::cout << "---------------- Relocation Table -------------------------------------\n";
    std::cout << "Section\tSymbol\tOffs\tType\n";
    for( auto& e : Rel )
        std::cout << e.RelSection << "\t" << e.RelSymbol << "\t" << e.RelOffs << "\t" << e.RelType << '\n';
    std::cout << "-----------------------------------------------------------------------\n";

    // print the contents for all sections
    for( auto& e : SecContent )
    {
        std::cout << "---------------- '" << e.first << "' Section Contents ---------------------------\n";
        int addr = 0;
        for( auto& b : e.second )
        {
            if( addr%16 == 0 && addr != 0 ) std::cout << '\n';
            std::cout << std::setfill('0') << std::setw(2) << std::hex << (int32) b << std::dec << std::setfill(' ') << " ";
            addr++;
        }
        std::cout << '\n';
        std::cout << "-----------------------------------------------------------------------\n";
    }



    // initialize the content of all bss sections with 0xcd (just for readability)
    for( auto& e : Sec )
    {
        if( e.second.SecType == "bss" )
        {
            std::cout << "----- initializing BSS section '" << Sec[ e.first ].SecName << "' in memory between " << Sec[e.first].SecBegAddr << " and " << Sec[e.first].SecEndAddr << " -----\n";

            for( int32 i = e.second.SecBegAddr; i < e.second.SecEndAddr; i++ )
                hw.mem.set( 0xcd, i, ValSize::byte, AccsRight::w ); // fill i-th byte in memory with 0xcd

            std::cout << "----- initializing BSS section '" << Sec[ e.first ].SecName << "' finished -----\n";
        }
    }

    // load content of all non-bss sections between their corresponding [BegAddr, EndAddr) in memory
    for( auto& e : SecContent )
    {
        std::cout << "----- loading section '" << Sec[ e.first ].SecName << "' in memory between " << Sec[e.first].SecBegAddr << " and " << Sec[e.first].SecEndAddr << " -----\n";

        for( int32 i = 0; i < (int32) e.second.size(); i++ )
            hw.mem.set( e.second[i], uns16(Sec[ e.first ].SecBegAddr + i), ValSize::byte, AccsRight::w ); // load i-th byte in memory

        std::cout << "----- loading section '" << Sec[ e.first ].SecName << "' finished -----\n";
    }



    // relocate all symbols in the relocation table
    std::cout << "----- relocation started -----\n";
    for( auto& r : Rel )
    {
        if( r.RelOffs < 0 || int32( SecContent[r.RelSection].size() ) < r.RelOffs ) { std::cout << "ERROR - relocation offset of symbol '" << r.RelSymbol << "' is outside its section '" << r.RelSection << '\n'; return false;}

        uns16 Addr = Sec[r.RelSection].SecBegAddr + r.RelOffs;                        // address of data (in memory) that will be relocated
        int32 DataSize = ( (r.RelType == "abs8" || r.RelType == "rel8" ) ? 1 : 2 );   // size of a data (in memory) that will be relocated
        int32 Displacement = Sym[r.RelSymbol].SymAddr;                                // displacement  that will be added to the memory location
        std::string Mode = r.RelType.substr(0,3);                                     // relocation mode "abs" or "rel"
        
        std::cout << "relocating symbol '" << r.RelSymbol << "' (displacement = " << Displacement << ") in section '" << r.RelSection << "' on offs = " << r.RelOffs << " (addr = " << Addr << ")";

        if( DataSize == 1 )
        {
            if( Displacement < -128 || 255 < Displacement ) { std::cout << "ERROR - displacement '" << Displacement << "' too large for 1-byte relocation of symbol '" << r.RelSymbol << "' in section '" << r.RelSection << '\n'; return false;}

            uns8& Data = hw.mem.get8( Addr, AccsRight::w );        // reference to an 1-byte data that will be relocated

            std::cout << " - original value = " << Data;
            Data += Displacement;
            if( Mode == "rel" )
                Data -= Addr + DataSize;  // in case of relative relocation, subtract the address of the next! byte after data 
            std::cout << ", relocated value = " << Data << '\n';
        }
        else // DataSize == 2
        {
            if( Displacement < -32768 || 65535 < Displacement ) { std::cout << "ERROR - displacement '" << Displacement << "' too large for 2-byte relocation of symbol '" << r.RelSymbol << "' in section '" << r.RelSection << '\n'; return false;}

            uns16& Data = hw.mem.get16( Addr, AccsRight::w );      // reference to a 2-byte data that will be relocated

            std::cout << " - original value = " << Data;
            Data += (uns16)Displacement;
            if( Mode == "rel" )
                Data -= uns16(Addr + DataSize);  // in case of relative relocation, subtract the address of the next! byte after data 
            std::cout << ", relocated value = " << Data << '\n';
        }
    }
    std::cout << "----- relocation finished -----\n";



    // set the starting value for the program counter register
    if( Sym.find( "start" ) == Sym.end() ) { std::cout << "ERROR - missing 'start' symbol (program entry point)\n"; return false; };  // check if the program entry point is defined
    hw.cpu.pc() = (uns16) Sym[ "start" ].SymAddr;
    *hwbak = hw;

    std::cout << "========================== loading finished ==========================\n";
    return true;
}

// run the emulation
void Emulator::StartEmulation()
{
    std::cout << "========================== emulation started =========================\n";

    // while the power is turned on
    while( hw.cpu.power() )
    {
        Instruction instr {};
        Operand src {}, dst {};
        uns16 srcval {}, dstval {};
        uns16 pcbak {};

        try
        {
            // back-up the program counter
            pcbak = hw.cpu.pc();
            
            // fetch instruction and operands
            FetchInstrAndOper(instr, dst, dstval, src, srcval);
            // execute instruction
            ExecInstr(instr.opcode(), dstval, srcval);
            // save result
            SaveInstrRes(instr.opcode(), dst, dstval, src, srcval);

            // check if the terminal input is ready
            hw.term.CheckInputReady();
            // check if the preset time has elapsed
            hw.timer.CheckTimeElapsed();
        }
        catch(const InstrCycleError& ex)
        {
            // log error to standard error stream
            std::cerr << ex.what() << '\n';
            // print the hardware
         // PrintHw();

            // if an instruction error has happened twice already, stop the emulator
            if( hw.cpu.irq(Interrupt::InstrErr) ) { hw.cpu.power() = false; break; }

            // set the instruction error interrupt
            hw.cpu.irq(Interrupt::InstrErr) = true;
            // restore the old program counter before the instruction fetch phase (to enable the instruction to execute a second time if needed)
            hw.cpu.pc() = pcbak;
        }

        try
        {
            // handle possible interrupts
            HandleInter();
        }
        catch(const InstrCycleError& ex)
        {
            // log error to standard error stream
            std::cerr << ex.what() << '\n';

            // stop the emulator
            hw.cpu.power() = false; break;
        }
    }

    std::cout << "========================== emulation finished ========================\n";
}


void Emulator::PrintHw()
{
    std::cout << "-----> Content of Registers\n";

    for( int32 i = 0; i < (int32) hw.cpu.RegCnt; i++ )
        std::cout << "r" << i << " = 0x" << std::setfill('0') << std::setw(4) << std::hex << hw.cpu.rx(i) << std::dec << std::setfill(' ') << " = " << hw.cpu.rx(i) << '\n';

    // print final memory contents of bss and data sections
    for( auto& e : Sec )
    {
        if( e.second.SecType != "bss" && e.second.SecType != "data" ) continue;

        std::cout << "-----> Content of Memory Section '" << e.first << "'\n";
        for( int32 addr = e.second.SecBegAddr; addr < e.second.SecEndAddr; addr++ )
        {
            if( addr%16 == 0 && addr != 0 ) std::cout << '\n';
            std::cout << std::setfill('0') << std::setw(2) << std::hex << (int32) hw.mem.get8( (uns16)addr, AccsRight::r ) << std::dec << std::setfill(' ') << " ";
        }
        std::cout << '\n';

    }
}


