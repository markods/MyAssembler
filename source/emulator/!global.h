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


// emulator macros
#define ADDR_UNKNOWN  -1
#define SIZE_UNKNOWN  -1

// section statuses
#define SEC_UNKNOWN         0
#define SEC_PLACED          1   // section is placed using "-place" command-line argument
#define SEC_DECLARED        2   // section is declared (name, type, size,...)
#define SEC_RELDEFINED      4   // relocation table for a section is defined
#define SEC_CONTENTDEFINED  8   // content of a section is defined
#define SEC_LOADED         16   // section content is loaded in memory

// file segments currently being read
#define READING_BEG         0    // no file segment is being read yet
#define READING_SECTBL      1    // section table is currently being read
#define READING_SYMTBL      2    // table of symbols is currently being read
#define READING_RELTBL      3    // relocation table of "CurrSecName" section is currently being read
#define READING_SECCONTENT  4    // content of "CurrSecName" section is currently being read

// modes of extracting column from a line
#define EXTRACT_DEFAULT    0x00
#define EXTRACT_LASTCOLUMN 0x01
#define EXTRACT_EMPTY      0x02



