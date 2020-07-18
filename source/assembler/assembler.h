// _____________________________________________________________________________________________________________________________________________
// ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER
// _____________________________________________________________________________________________________________________________________________
// ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER
// _____________________________________________________________________________________________________________________________________________
// ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER

#pragma once
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include "!global.h"

// for manual asembly checking
struct LineDump
{
    std::string line {};
    int32       section {};
    int32       beg {};
    int32       end {};
};

// forward reference used for backpatching
struct FwdRef
{
    int32 patchline {};       // to which line of the code this patch applies
    int32 patchsection {};    // in which section this symbol should be patched  
    int32 patchoffset {};     // on which offset from the beginning of a section this symbol should be patched
    int32 patchsize {};       // size (in bytes) of the space in which this symbol should be patched
    int32 patchfmt {};        // should symbol be patched as signed (signed = true, unsigned = false)
    int32 patchmode {};       // should patching be done in absolute or relative way
};

// a symbol
struct SymItem
{
    int32                    rn {};           // running number
    std::string              symbol {};       // symbol
    std::string              alias {};        // name of the extern symbol which this symbol represents (on which this symbol is based on)
    int32                    section {};      // section to which this symbol belongs to
    int32                    value {};        // value of the simbol (in case of a label: offset of symbol from the beginning of a section the label belongs to)
    char                     origin {};       // origin of the symbol (local, global, lvalue, extern ...)
                                           
    bool                     isdefined {};    // if the symbol is already defined in any of preceeding lines
    int32                    depcnt {};       // number of symbols which this symbol depends on
    std::vector<std::string> depends {};      // list of symbols which this symbol depends on
    std::vector<std::string> resolves {};     // list of symbols which this symbol resolves (appears in expression of .equ directive)
    int32                    depcntinsection[SECTION_CNT] = { 0, 0, 0, 0, 0 };  // counters of section[i] - used to resolve equ directive declared symbols
                                                                                // limitation -- there must be max one section of each type in the assembly file, since there is only one counter per section

    std::vector<FwdRef> forwardref {};        // list of forward references relative to section beg
                                              // if symbol is undefined: for assembler backpatching when symbol becomes defined
                                              //              defined:   for loader backpatching
};


// class that represents an assembler
class Assembler
{
    public:
        Assembler()
        { 
            linecnt     = 0;
            currsection = SECTION_UNKNOWN;
            EraseCmd(); 
        }  

        inline int32 GetLineCnt() { return linecnt; };
        bool ParseCmd(std::string line);
        void PrintSymbolTable();
        bool Finish( std::ofstream& outfile );

    public:
        static void Trim(std::string& s, int32 trimscope = TRIMSCOPE_INSTRUCTION);
        static std::string GetToken(std::string& s, char breakchar = ' ');
        static bool IsNumber(std::string& s);

    private:
        void EraseCmd();
        int32 ChangeSection( std::string s );
        void PrintCmd();

        bool IsLiteral(std::string& s, bool issignallowed = SIGN_ALLOWED, int32 action = NO_ACTION, int32* value = nullptr );
        bool IsSymbol(std::string& s, bool issignallowed = SIGN_NOT_ALLOWED, int32 action = USE_THIS_SYMBOL );
        bool IsList(std::string& s, int32 listtype);
        bool IsExpression(std::string& s);

        bool IsSectionName(std::string& s);
        bool IsSectionType(std::string& s);

        bool IsLabel(std::string& s);
        bool IsDirective(std::string& s, int32 dirtype = ANY_DIRECTIVE);
        bool IsInstructionCore(std::string& s);
        bool IsInstruction(std::string& s);
        bool IsDirOperand(std::string& s, int32 opidx);
        bool IsInstrOperandNonImmediate(std::string& s, int32 opidx);
        bool IsInstrOperand(std::string& s, int32 opidx);

        bool IsValueStorable( int32 value, int32 size, int32 fmt );
        bool IsSubopStorable( int32 opidx );
        bool StoreValue( int32 value, int32 size, int32 fmt, int32 patchmode, int32 storeinsection, int32& loc );
        bool StoreData( SymItem& symitem );
        bool StoreInstrCode();

        bool Backpatch( std::string _symbol );

    public:
        inline std::string GetCmd() { return !ins.empty() ? ins : dir; }
        std::string GetSectionName( int32 s );
        std::string GetSectionType( int32 s );
        int32 GetSectionByName( std::string s );
        int32 GetSectionByType( std::string s );

    private:
        bool ProcessLiteral( int32 literalval, int32 action = NO_ACTION );
        bool ProcessSymbol( std::string _symbol, int32 action = USE_THIS_SYMBOL );

        bool ResolveDependency( SymItem& depsymitem, SymItem& symitem, int32 sign );
        bool ResolveDependentSymbols( std::string symbol );
        bool CheckIfSymbolResolved( std::string _symbol );

        void PrintSection( int32 s );
        void PrintDump();  // for manual assembly checking

        bool IsSymbolExportable( SymItem& symitem );
        void ExportSectionTable( std::ofstream& outfile );
        void ExportSymbolTable( std::ofstream& outfile );
        void ExportRelocationTable( int32 s, std::ofstream& outfile );
        void ExportSection( int32 s, std::ofstream& outfile );


    private:
        int32         linecnt;                                           // number of parsed lines in the input file
        std::unordered_map< std::string, SymItem > symboltable;          // table of symbols
        int32         currsection;                                       // section which is currently being processed
        uns8          section[SECTION_CNT][SECTION_SIZE];                // byte buffer for each section
        int32         locationcounter[SECTION_CNT] = { 0, 0, 0, 0, 0 };  // number of bytes used in the corresponding section buffer
        std::string   sectionname[SECTION_CNT];                          // name of the sections in the file (max one of each type)

        // info about the current command (directive or instruction) being processed
        int32       tokencnt;               // number of recognized tokens inside the current line
        std::string lbl;                    // token recognized as label
        std::string dir;                    // token recognized as directive
        std::string ins;                    // token recognized as instruction
        std::string coremnemonic;           // instruction/directive mnemonic (in case of instruction: without 'b' and 'w' at the end)
        int32       code;                   // instruction/directive numeric code
        int32       datawidth;              // data witdth in bytes (if 'b' ili 'w' definied) or UNKNOWN_DATAWIDTH
        char        type;                   // instruction/directive type 
        int32       opcnt;                  // operand count
        int32       dstpos;                 // position of destination operand(s) (for instruction operands only)
        std::string op[OPCNT];              // token recognized as i-th operand of instruction/directive
        int32       optype[OPCNT];          // type of i-th operand (for instruction operands: addressing type)
        int32       opregnum[OPCNT];        // register number (0-7) of i-th operand (for instruction operands only)
        int32       opreglowhigh[OPCNT];    // register byte usage (0 = low, 1 = high) of i-th operand (for instruction operands only)
        std::string subop[OPCNT];           // suboperand of i-th operand (it is <literal> or <symbol> that will be placed in Im/Di/Ad)
        int32       subopvalue[OPCNT];      // value of suboperand of i-th operand (if it could be resolved immediately)
        int32       subopsize[OPCNT];       // byte size for coding suboperand of i-th operand
        bool        subopsigned[OPCNT];     // is suboperand of i-th operand coded as signed (unsigned = false, signed = true)

        // for manual assembly checking
        LineDump* linedump;                       
        std::vector< LineDump > dump;          
};

