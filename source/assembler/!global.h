// _____________________________________________________________________________________________________________________________________________
// GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL
// _____________________________________________________________________________________________________________________________________________
// GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL
// _____________________________________________________________________________________________________________________________________________
// GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL...GLOBAL

#pragma once


// explicitly define standard C++ types with known bit sizes and additional useful types
using uns8  = unsigned char;        // unsigned 8-bit number
using uns16 = unsigned short;       // unsigned 16-bit number
using uns32 = unsigned long;        // unsigned 32-bit number
using uns64 = unsigned long long;   // unsigned 64-bit number

using int8  = signed char;          // signed 8-bit number
using int16 = signed short;         // signed 16-bit number
using int32 = signed long;          // signed 32-bit number
using int64 = signed long long;     // signed 64-bit number

using flo32 = float;                // 32-bit float 
using flo64 = double;               // 64-bit float


// define some minimal and maximal values that the standard types can have
constexpr int8  int8min  = -((uns8 )1 << 7 );       // minimum value for signed 8-bit number
constexpr int16 int16min = -((uns16)1 << 15);       // minimum value for signed 16-bit number
constexpr int32 int32min = -((uns32)1 << 31);       // minimum value for signed 32-bit number
constexpr int64 int64min = -((uns64)1 << 63);       // minimum value for signed 64-bit number

constexpr int8  int8max  =  ((uns8 )1 << 7 ) - 1;   // maximum value for signed 8-bit number
constexpr int16 int16max =  ((uns16)1 << 15) - 1;   // maximum value for signed 16-bit number
constexpr int32 int32max =  ((uns32)1 << 31) - 1;   // maximum value for signed 32-bit number
constexpr int64 int64max =  ((uns64)1 << 63) - 1;   // maximum value for signed 64-bit number

constexpr uns8  uns8min  =  (uns8 )0;               // minimum value for unsigned 8-bit number
constexpr uns16 uns16min =  (uns16)0;               // minimum value for unsigned 16-bit number
constexpr uns32 uns32min =  (uns32)0;               // minimum value for unsigned 32-bit number
constexpr uns64 uns64min =  (uns64)0;               // minimum value for unsigned 64-bit number

constexpr uns8  uns8max  =  ~(uns8 )0;              // maximum value for unsigned 8-bit number
constexpr uns16 uns16max =  ~(uns16)0;              // maximum value for unsigned 16-bit number
constexpr uns32 uns32max =  ~(uns32)0;              // maximum value for unsigned 32-bit number
constexpr uns64 uns64max =  ~(uns64)0;              // maximum value for unsigned 64-bit number


// assembler specific macros
#define MAX_TOKEN_LEN      1024

// separators
#define SEP1    "\t"
#define SEP2    "\t\t\t"
#define SEP3    "\t\t\t\t\t"
#define SEP4    "\t\t\t\t\t\t\t"

#define UNSIGNED_BYTE_MIN  uns8min
#define UNSIGNED_BYTE_MAX  uns8max
#define SIGNED_BYTE_MIN    int8min
#define SIGNED_BYTE_MAX    int8max
#define UNSIGNED_WORD_MIN  uns16min
#define UNSIGNED_WORD_MAX  uns16max
#define SIGNED_WORD_MIN    int16min
#define SIGNED_WORD_MAX    int16max

// special character to use when extracting tokens from expression
#define PLUSMINUS          char(241)

// is sign allowed at the beginning of a symbol or literal
#define SIGN_NOT_ALLOWED false
#define SIGN_ALLOWED     true

// default values for assembly
#define UNKNOWN_CODE          (-1)   // unknown ins/dir code
#define UNKNOWN_COREMNEMONIC    ""
#define UNKNOWN_TYPE           'x'
#define UNKNOWN_DATAWIDTH     (-1)
#define UNKNOWN_OPCNT         (-1)
#define UNKNOWN_DSTPOS        (-1)
#define UNKNOWN_OPTYPE        (-1)   // unknown operand type (in case of instruction: unknown addressing type of operand)
#define UNKNOWN_OPREGNUM      (-1)   // unknown operand register number
#define UNKNOWN_OPREGLOWHIGH  (-1)   // unknown operand register low/high
#define UNKNOWN_VALUE         SIGNED_WORD_MAX

// operand index
#define FIRST       0x0
#define SECOND      0x1
#define OPCNT         2

// position of destination operands (for instruction operands)
#define NO_DST_OPERAND          0x0   // ne postoji "dst" operand
#define DST_OPERAND_FIRST       0x1   // "dst" je prvi operand ("dst, src" ili samo "dst")
#define DST_OPERAND_SECOND      0x2   // "dst" je drugi operand ("src, dst")
#define DST_OPERAND_BOTH        0x3   // "dst" su i prvi i drugi operand ("dst, dst" - npr.u xchg)

// instruction type
#define NO_OPERAND_INSTRUCTION  ' '
#define JUMP_INSTRUCTION        'j'
#define DATA_INSTRUCTION        'd'

// directive type
#define GENERAL_DIRECTIVE         0x1
#define DATA_DEFINING_DIRECTIVE   0x2
#define ANY_DIRECTIVE             0x3

// type of addressing (for instruction operands)
#define ADDR_IMMEDIATE                      0
#define ADDR_REG_DIRECT                     1
#define ADDR_REG_INDIRECT                   2
#define ADDR_REG_INDIRECT_WITH_16BIT_OFFSET 3
#define ADDR_MEMORY                         4

// trimming scope
#define TRIMSCOPE_INSTRUCTION    0x1
#define TRIMSCOPE_OPERANDS       0x2

// type of operand (for directive operands)
#define NONE                    0
#define LITERAL                 1
#define SYMBOL                  2                      
#define SYMBOLLITERAL           (SYMBOL|LITERAL)
#define LITERAL_LIST            (4+1)
#define SYMBOL_LIST             (4+2)
#define SYMBOLLITERAL_LIST      (SYMBOL_LIST|LITERAL_LIST)
#define LVALUE                  8      // used only as 1st operand in EQU, so default action is CALCULATE_THIS_SYMBOL
#define EXPRESSION              16     // used only as 2st operand in EQU, so default action is CALCULATE_ANOTHER_SYMBOL (op[FIRST])
#define LITERAL_POSITIVE        32     // used only in .skip directive
#define SECTIONNAME             64
#define SECTIONTYPE             128

// what to do with this symbol (used in ProcessSymbol())
#define NO_ACTION                  0
#define DEFINE_THIS_SYMBOL         1    // when symbol appears as label
#define CALCULATE_THIS_SYMBOL      2    // when symbol appears as first operand of .equ directive
#define CALCULATE_ANOTHER_SYMBOL   3    // when symbol appears as element of .equ directive, look for symbol value to calculate another symbol (op[FIRST])
#define USE_THIS_SYMBOL            4    // when symbol appears elsewhere, so its value is required
#define DECLARE_THIS_SYMBOL_GLOBAL 5
#define DECLARE_THIS_SYMBOL_EXTERN 6

// section types
#define SECTION_UNKNOWN        0      // artificial section, use for symbols defined before any section, and for calculated .equ symbols (before it is recognized to which section they belong)
#define SECTION_BSS            1
#define SECTION_DATA           2
#define SECTION_TEXT           3
#define SECTION_EXTERN         4      // artificial section, to separate extern symbols from others while resolving expressions (while calculating .equ symbols)

#define SECTION_CNT            5      // total number of sections
#define SECTION_SIZE           65536  // 64KB for 16-bit processor

// symbol origins (used for more detailed error handling)
#define ORIGIN_UNKNOWN     0
#define ORIGIN_LOCAL       1
#define ORIGIN_GLOBAL      2
#define ORIGIN_LVALUE      4
#define ORIGIN_EXTERN      8
#define ORIGIN_SECTION    16

#define FMT_UNSIGNED        0  // same as issigned = false
#define FMT_SIGNED          1  // same as issigned = true
#define FMT_BASED_ON_VALUE  2  // it will be formated as FMT_SIGNED if the value is negative, otherwise as FMT_UNSIGNED

#define MODE_ZERO           0
#define MODE_ABSOLUTE       1 
#define MODE_RELATIVE       2

// allowed origin combinations:
//
//    ORIGIN_UNKNOWN                                                         --> not yet known
//                   ORIGIN_LOCAL                                            --> defined localy
//                                ORIGIN_GLOBAL                              --> not yet defined localy, but declared as global
//                   ORIGIN_LOCAL|ORIGIN_GLOBAL                              --> defined localy, declared as global
//                                               ORIGIN_LVALUE               --> dependent on other symbols (lvalue of .equ directive)
//                                                             ORIGIN_EXTERN --> defined externaly
//                                               ORIGIN_LVALUE|ORIGIN_EXTERN --> dependent on a symbol defined externaly (resolved)
//
// allowed origin transitions
// 
//    ORIGIN_UNKNOWN  -->  ORIGIN_LOCAL, ORIGIN_GLOBAL, ORIGIN_LVALUE, ORIGIN_EXTERN
//    ORIGIN_LOCAL    -->  ORIGIN_GLOBAL
//    ORIGIN_GLOBAL   -->  ORIGIN_LOCAL
// 
//    ORIGIN_LVALUE resolved as --> ORIGIN_EXTERN




