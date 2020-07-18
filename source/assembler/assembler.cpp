// _____________________________________________________________________________________________________________________________________________
// ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER
// _____________________________________________________________________________________________________________________________________________
// ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER
// _____________________________________________________________________________________________________________________________________________
// ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER...ASSEMBLER

#include <fstream>
#include <algorithm>
#include "assembler.h"
#include "!global.h"


// struct that represents a directive
struct Directive
{
    int32       dc {};         // directive code
    std::string mnemonic {};
    char        dirtype {};    // directive type 
    int32       opcnt {};      // operand count (<symbol list> and <expression> are treated as a single! operand)
    int32       optype[2] {};  // operand type
}
dirdfn[] =
{
    //  dc    mnemonic                    dirtype  opcnt,   op1type               op2type  
    {   0,   ".equ",            GENERAL_DIRECTIVE,     2,  { LVALUE,              EXPRESSION  } },
    {   1,   ".global",         GENERAL_DIRECTIVE,     1,  { SYMBOL_LIST,         NONE        } },
    {   2,   ".extern",         GENERAL_DIRECTIVE,     1,  { SYMBOL_LIST,         NONE        } },
    {   3,   ".section",        GENERAL_DIRECTIVE,     2,  { SECTIONTYPE,         SECTIONNAME } },
    {   4,   ".end",            GENERAL_DIRECTIVE,     0,  { NONE,                NONE        } },
    {   5,   ".skip",     DATA_DEFINING_DIRECTIVE,     1,  { LITERAL_POSITIVE,    NONE        } },
    {   6,   ".byte",     DATA_DEFINING_DIRECTIVE,     1,  { SYMBOLLITERAL_LIST,  NONE        } },
    {   7,   ".word",     DATA_DEFINING_DIRECTIVE,     1,  { SYMBOLLITERAL_LIST,  NONE        } },
};
const int32 dirdfncnt = 8;

// struct that represents an instruction
struct Instruction
{
    int32       oc {};        // instruction code
    std::string mnemonic {};
    char        instype {};   // instruction type
    int32       opcnt {};     // operand count
    int32       dstpos {};    // which of the operands is(are) the destionation operand(s)
}
insdfn[] =
{
    // oc    mnemonic                 instype   opcnt    dstpos
    {   0,   "halt",   NO_OPERAND_INSTRUCTION,      0,   NO_DST_OPERAND      },
    {   1,   "iret",   NO_OPERAND_INSTRUCTION,      0,   NO_DST_OPERAND      },
    {   2,   "ret" ,   NO_OPERAND_INSTRUCTION,      0,   NO_DST_OPERAND      },
    {   3,   "int" ,         JUMP_INSTRUCTION,      1,   NO_DST_OPERAND      },
    {   4,   "call",         JUMP_INSTRUCTION,      1,   NO_DST_OPERAND      },
    {   5,   "jmp" ,         JUMP_INSTRUCTION,      1,   NO_DST_OPERAND      },
    {   6,   "jeq" ,         JUMP_INSTRUCTION,      1,   NO_DST_OPERAND      },
    {   7,   "jne" ,         JUMP_INSTRUCTION,      1,   NO_DST_OPERAND      },
    {   8,   "jgt" ,         JUMP_INSTRUCTION,      1,   NO_DST_OPERAND      },
    {   9,   "push",         DATA_INSTRUCTION,      1,   NO_DST_OPERAND      },
    {  10,   "pop" ,         DATA_INSTRUCTION,      1,   DST_OPERAND_FIRST   },
    {  11,   "xchg",         DATA_INSTRUCTION,      2,   DST_OPERAND_BOTH    },
    {  12,   "mov" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  13,   "add" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  14,   "sub" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  15,   "mul" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  16,   "div" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  17,   "cmp" ,         DATA_INSTRUCTION,      2,   NO_DST_OPERAND      },
    {  18,   "not" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  19,   "and" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  20,   "or"  ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  21,   "xor" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  22,   "test",         DATA_INSTRUCTION,      2,   NO_DST_OPERAND      },
    {  23,   "shl" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_SECOND  },
    {  24,   "shr" ,         DATA_INSTRUCTION,      2,   DST_OPERAND_FIRST   }
};
const int32 insdfncnt = 25;

inline bool iswhitespace(char c) { return c == ' ' || c == '\t'; }  // space and tab are currently the only recognized whitespaces
inline bool isletter(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || '_' == c; }  // underscore is treated as letter
inline bool isdigit(char c) { return '0' <= c && c <= '9'; }
inline bool isregnum(char c) { return '0' <= c && c <= '7'; }

inline bool shouldremovewhitespaceafter(char c) { return c == ',' || c == '+' || c == '-'; }  // whitespaces after these characters are removed (if they exist)
inline bool shouldremovewhitespacebefore(char c, int32 trimscope = TRIMSCOPE_INSTRUCTION) { return c == ',' || c == ':' || (trimscope == TRIMSCOPE_OPERANDS && (c == '+' || c == '-')); }  // whitespaces after these characters are removed (if they exist)

std::string Assembler::GetSectionName( int32 s )
{
    if( 0 <= s && s < SECTION_CNT ) return sectionname[s];
    else                            return "?";
}

std::string Assembler::GetSectionType( int32 s )
{
         if( s == SECTION_BSS    ) return "bss";
    else if( s == SECTION_DATA   ) return "data";
    else if( s == SECTION_TEXT   ) return "text";
    else if( s == SECTION_EXTERN ) return "extern";
    else                           return "?";
}

int32 Assembler::GetSectionByName( std::string s )
{
     if( s.length() > 0 ) 
        for( int32 i = 0; i < SECTION_CNT; i++ )
            if( s == sectionname[i] )
                return i;

    return SECTION_UNKNOWN;
}

int32 Assembler::GetSectionByType( std::string s )
{
         if( s == "bss"    ) return SECTION_BSS;
    else if( s == "data"   ) return SECTION_DATA;
    else if( s == "text"   ) return SECTION_TEXT;
    else if( s == "extern" ) return SECTION_EXTERN;
    else                     return SECTION_UNKNOWN;
}


// removes whitespaces and comments (except inside strings "...")
// important: whitespaces should not be added if the trim result is written over the source string
void Assembler::Trim(std::string& s, int32 trimscope)
{
    int32 cnt = 0;     // consecutive whitespaces count
    int32 j = 0;
    int32 insidestr = 0;
    for( int32 i = 0; i < (int32)s.length() && s[i] != ';'; i++ )  // comments (beginning with ";") are also trimmed out!
    {
        if( !insidestr && iswhitespace(s[i]) )  // whitespaces are skipped only if not inside asm string
            cnt++;
        else
        {
            if( cnt > 0 ) // if at least one whitespace character is preceeding the current character
            {
                if( j > 0 )       // if already at least one character exists in the trimmed string
                    if( insidestr                                                 // always place whitespace if inside string
                       || !(shouldremovewhitespacebefore(s[i], trimscope)         // do not place whitespaces before some characters
                       || shouldremovewhitespaceafter(s[(uns64)j-1])) )           // do not place whitespaces after some characters
                        s[j++] = ' ';   // replace all previous whitespaces with a single space
                cnt = 0;            // start counting consecutive whitespaces again
            }
         // s[j++] = ( insidestr ? s[i] : tolower(s[i]) );   // case-insensitve
            s[j++] = s[i];                                   // case-sensitive
        }

        if( s[i] == '"' )   // character for marking beginning and the end of a asm string
            insidestr = 1 - insidestr;
    }
    s.erase(j);
}

// takes the next token from the given string, which was trimmed previously
// the string should not have:
// +   whitespaces at the beginning! or end! of the string
// +   consecutive (two or more) whitespaces between tokens
// +   whitespaces immediately before the breakchar character
// if a token in the string is also a string (surrounded with "") then it must not contain whitespaces
std::string Assembler::GetToken(std::string& s, char breakchar)
{
    std::string t = s;   // token

    int32 j = 0;
    int32 i = 0;
    int32 insidestr = 0;
    while( i < (int32) s.length() )
    {
        if( s[i] == '"' )   // character for marking the beginning and the end of a asm string
            insidestr = 1 - insidestr;

//      if( !insidestr && (iswhitespace(s[i]) || s[i] == breakchar) )                // stop search if whitespace/breakchar is found
        if( !insidestr && (s[i] == breakchar                                         // stop search if breakchar is found 
           || (breakchar == PLUSMINUS && i > 0 && (s[i] == '+' || s[i] == '-'))) )   // if breakchar is PLUSMINUS then the search will stop if either "+" or "-" or PLUSMINUS is found, except at the beginning of the string
            break;
        else
            i++;
    }

    t.erase(i);

    if( breakchar != PLUSMINUS )
    {
        i++;  // breakchar is never copied, except if it is PLUSMINUS (since it has to represent the sign of the following number)
        if( i < (int32) s.length() && iswhitespace(s[i]) )  // skip the whitespace possibly present after! the breakchar
            i++;
    }

    while( i < (int32) s.length() )
        s[j++] = s[i++];

    s.erase(j);

    return t;
}

bool Assembler::IsNumber(std::string& s)
{
    if( 1 <= s.length() && s.length() <= MAX_TOKEN_LEN )
    {
        for( int32 i = 0; i < (int32) s.length(); i++ )
            if( !isdigit(s[i]) )
                return false;

        return true;
    }
    return false;
}

// a literal can only be made from decimal digits and must be inside the given range [minvalue, maxvalue]
// it can also have a '+' or '-' sign if permitted
bool Assembler::IsLiteral(std::string& s, bool issignallowed, int32 action, int32* value )
{
    int32 v = 0;                                     // local variable for calculating value
    int32 sign = 1;
    if( value != nullptr ) *value = UNKNOWN_VALUE;   // always erase outer *value

    if( 1 <= s.length() && s.length() <= MAX_TOKEN_LEN )
    {
        for( int32 i = 0; i < (int32) s.length(); i++ )
        {
            if( i == 0 && issignallowed == SIGN_ALLOWED && (s[i] == '+' || s[i] == '-') && 1 < s.length() )
            {
                if( s[i] == '-' ) sign = -1;
                continue;
            }

            if( !isdigit(s[i]) ) return false;

            v =  v * 10 + s[i] - '0';
            if( v * sign >  65535 ) { std::cout << "WARNING - literal value is too large\n"; return false; };
            if( v * sign < -65536 ) { std::cout << "WARNING - literal value is too small\n"; return false; };
        }

        v *= sign;
        if( ! ProcessLiteral( v, action ) ) return false;

        if( value != nullptr ) *value = v;  // outer *value is set only if everything is ok
        return true;
    }
    return false;
}

// a symbol must start with a letter or an underscore, followed by letters, underscores or digits
// it can also have a '+' or '-' sign if permitted
bool Assembler::IsSymbol(std::string& s, bool issignallowed, int32 action )
{
 // int32 sign = 1;
    int32 firstcharpos = 0;  // indeks prvog karaktera u stringu (nakon eventualnog "+" ili "-")
    if( 1 <= s.length() && s.length() <= MAX_TOKEN_LEN )
    {
        for( int32 i = 0; i < (int32)s.length(); i++ )
        {
            if( i == 0 && issignallowed == SIGN_ALLOWED && (s[i] == '+' || s[i] == '-') && 1 < s.length() )
            {
                firstcharpos++;
             // if( s[i] == '-' ) sign = -1;
                continue;
            }

            if( i == firstcharpos ) { if( !isletter(s[i]) ) return false; }  // firstchar must be a letter
            else { if( !isletter(s[i]) && !isdigit(s[i]) ) return false; }
        }

        if( ! ProcessSymbol( s, action ) ) return false;

        return true;
    }
    return false;
}

// list items must be of type SYMBOL or LITERAL or both
// the list is assumed to not have whitespaces and must contain at least one token
// list items are separated with commas (','), but the list should not begin or end with a comma, and there should not be empty items (consecutive commas, or ",,")
bool Assembler::IsList(std::string& s, int32 listtype)
{
    std::string lst = s;
    std::string token;

    // the list is not allowed to begin or end with a comma, and it must contain at least one token
    if( lst.length() == 0 || (lst.length() > 0 && (lst[0] == ',' || lst[lst.length()-1] == ',')) )
        return false;

    // check each token in list
    while( !lst.empty() )
    {
        token = GetToken(lst, ',');
     // std::cout << "==> '" << token << '\n';

        if( !((listtype & SYMBOL) && IsSymbol(token)) && !((listtype & LITERAL) && IsLiteral(token)) )
            return false;

        // process current symbol from the list (literals are not allowed in .global and .extern directives)
        if( dir == ".global" && !ProcessSymbol( token, DECLARE_THIS_SYMBOL_GLOBAL ) )
            return false;
        if( dir == ".extern" && !ProcessSymbol( token, DECLARE_THIS_SYMBOL_EXTERN ) )
            return false;
    }

    return true;
}

// an expression is made of symbols and literals separated with '+' or '-' signs
// the expression can begin on '+' or '-' signs but must not end on those characters
bool Assembler::IsExpression(std::string& s)
{
    std::string exp = s;
    std::string token;

    // the expression is not allowed to end with '+' or '-', and it must contain at least one token
    if( exp.length() == 0 || (exp.length() > 0 && (exp[exp.length()-1] == '+' || exp[exp.length()-1] == '-')) )
        return false;

    // check each token in expression
    while( !exp.empty() )
    {
        token = GetToken(exp, PLUSMINUS);   // when extracted in PLUSMINUS mode, token can have "+" or "-" signs at the beginning
     // std::cout << "==> '" << token << '\n';

        if( !IsSymbol(token, SIGN_ALLOWED, CALCULATE_ANOTHER_SYMBOL) && !IsLiteral(token, SIGN_ALLOWED, CALCULATE_ANOTHER_SYMBOL) )
            return false;
    }
    return true;
}

// check if section has a proper name
bool Assembler::IsSectionName(std::string& s)
{
    if( s == "data" || s == "text" || s == "bss" ) { std::cout << "WARNING - section type '" << s << "' cannot be used as a section name\n"; return false; }

    if( ! IsSymbol(s, SIGN_NOT_ALLOWED, DEFINE_THIS_SYMBOL) )
        return false;

    return true;
}

// check if the section type is valid
bool Assembler::IsSectionType(std::string& s)
{
    if( !IsSymbol(s, SIGN_NOT_ALLOWED, DEFINE_THIS_SYMBOL) )
        return false;

    return   s == "data"
          || s == "text"
          || s == "bss";
}

// check if the given string is a label
// a label must be a symbol that ends with a colon ':'
bool Assembler::IsLabel(std::string& s)
{
    if( 2 <= s.length() && s.length() <= MAX_TOKEN_LEN && s[s.length()-1] == ':' )
    {
        std::string l = s.substr(0, s.length()-1);
        if( IsSymbol(l, SIGN_NOT_ALLOWED, DEFINE_THIS_SYMBOL) )
        {
            // check if label is in a valid section
            if( currsection != SECTION_BSS && currsection != SECTION_DATA && currsection != SECTION_TEXT ) { std::cout << "WARNING - label '" << s << "' found outside BSS/DATA/TEXT section\n"; return false; }

            return true;
        }
    }
    return false;
}

// check if the given string is a directive of the specified type
bool Assembler::IsDirective(std::string& s, int32 dirtype)
{
    for( int32 dc = 0; dc < dirdfncnt; dc++ )
    {
        if( (dirdfn[dc].dirtype & dirtype) && s == dirdfn[dc].mnemonic )
        {
            // check if directive is in a valid section
                 if( s == ".skip" &&                    currsection != SECTION_BSS && currsection != SECTION_DATA ) { std::cout << "WARNING - data-defining directive '" << s << "' found outside BSS/DATA section\n"; return false; }
            else if( s != ".skip" && dirdfn[dc].dirtype == DATA_DEFINING_DIRECTIVE && currsection != SECTION_DATA ) { std::cout << "WARNING - data-defining directive '" << s << "' found outside DATA section\n"; return false; }

            code         = dc;
            coremnemonic = s;
            type         = dirdfn[dc].dirtype + '0';  // make a character '1' or '2' from an integer 1 or 2
            opcnt        = dirdfn[dc].opcnt;
            return true;
        }
    }
    return false;
}

// search if the given string is an instruction opcode (instruction opcode modifiers 'b' or 'w' should not be present in the string)
bool Assembler::IsInstructionCore(std::string& s)
{
    for( int32 oc = 0; oc < insdfncnt; oc++ )
    {
        if( s == insdfn[oc].mnemonic )
        {
            // check if instruction is in a valid section
            if( currsection != SECTION_TEXT ) { std::cout << "WARNING - instruction '" << s << "' found outside TEXT section\n"; return false; }

            code         = oc;
            coremnemonic = s;
            type         = insdfn[oc].instype;
            opcnt        = insdfn[oc].opcnt;
            dstpos       = insdfn[oc].dstpos;
            return true;
        }
    }
    return false;
}

// check if the given string is an instruction (includes 'b' or 'w' instruction opcode modifiers, if they exist)
bool Assembler::IsInstruction(std::string& s)
{
    if( IsInstructionCore(s) )
        return true;

    if( 1 < s.length() && (s[s.length()-1] == 'b' || s[s.length()-1] == 'w') )
    {
        std::string mnemonic;
        mnemonic = s.substr(0, s.length()-1);
        if( IsInstructionCore(mnemonic) )
        {
                 if( s[s.length()-1] == 'b' ) { datawidth = 1; return true; }
            else if( s[s.length()-1] == 'w' ) { datawidth = 2; return true; }
            else return false;
        }
    }

    return false;
}

// check if the given string is a directive operand at the given position
bool Assembler::IsDirOperand(std::string& s, int32 opidx)
{
    if( opidx != 0 && opidx != 1 )
        return false;

    int32 type = dirdfn[code].optype[opidx];
    switch( type )
    {
        case LITERAL:            if( IsLiteral(s) )                                          { optype[opidx] = LITERAL;                                                                                     return true; }; break;
        case LITERAL_POSITIVE:   if( IsLiteral(s, SIGN_NOT_ALLOWED) )                        { optype[opidx] = LITERAL_POSITIVE;                                                                            return true; }; break;
        case LVALUE:             if( IsSymbol(s, SIGN_NOT_ALLOWED, CALCULATE_THIS_SYMBOL) )  { optype[opidx] = LVALUE;                                                                                      return true; }; break;
        case SYMBOL_LIST:        if( IsList(s, type) )                                       { optype[opidx] = SYMBOL_LIST;                                                                                 return true; }; break;
        case SYMBOLLITERAL_LIST: if( IsList(s, type) )                                       { optype[opidx] = SYMBOLLITERAL_LIST;                                                                          return true; }; break;
        case SECTIONNAME:        if( IsSectionName(s) )                                      { optype[opidx] = SECTIONNAME;        sectionname[currsection] = s;  symboltable[s].origin |= ORIGIN_SECTION;  return true; }; break;
        case SECTIONTYPE:        if( IsSectionType(s) )                                      { optype[opidx] = SECTIONTYPE;                                                                                 return true; }; break;
        case EXPRESSION:         if( IsExpression(s) )                                       { optype[opidx] = EXPRESSION;                                                                                  return true; }; break;
    }
    return false;
}

// ---------------------------------------------------------------------------------
// POSSIBLE SUB-OPERAND SIZES (subopsize[opidx])
// ---------------------------------------------------------------------------------
// zero bytes -- the subop[i] field is empty
// +   used in register direct addressing                     -- ADDR_REG_DIRECT 
// +   used in register indirect addressing (with no offset)  -- ADDR_REG_INDIRECT 
//
// one byte -- contains a signed number in the range [-128, +127]
// +   used in immediate addressing when the datawidth = 1    -- ADDR_IMMEDIATE
//
// two bytes -- contains a signed number in the range [-32768, +32767]
// +   used in immediate addressing when the datawidth = 2    -- ADDR_IMMEDIATE
// +   used in register direct addressin with a 16-bit offset -- ADDR_REG_INDIRECT_WITH_16BIT_OFFSET
//
// two bytes -- contains an unsigned number in the range [0, 65535]
// +   used in memory direct addressing                       -- ADDR_MEMORY
// ---------------------------------------------------------------------------------

// check if a given token is an instruction operand
bool Assembler::IsInstrOperand(std::string& s, int32 opidx)
{
    if( opidx != 0 && opidx != 1 )
        return false;

    std::string suboperand;  // suboperand

    if( 1 <= s.length() && s.length() <= MAX_TOKEN_LEN )
    {
        if( type == DATA_INSTRUCTION && 1 < s.length() && s[0] == '$' )
        {
            // $<literal> or $<symbol>   // applied only for source! operand
            suboperand = s.substr(1, s.length()-1);
            if( IsLiteral(suboperand, SIGN_ALLOWED, NO_ACTION, &subopvalue[opidx]) || IsSymbol(suboperand) )
            {
                // immediate addressing is not allowed for the destination operand
                if( dstpos & (opidx+1) ) { std::cout << "WARNING - immediate addressing is not allowed in the destination operand '" << s << '\n'; return false; }

                optype[opidx]        = ADDR_IMMEDIATE;

                // the suboperand in immediate addressing is either a signed word or a signed byte, depending on the datawidth
                subop[opidx]         = suboperand;
                subopsize[opidx]     = (datawidth == UNKNOWN_DATAWIDTH ? 2 : datawidth);  // default datawidth is 2 for data instructions
                subopsigned[opidx]   = true;
                return true;
            }
        }
        else if( type == JUMP_INSTRUCTION && 0 < s.length() && s[0] != '*' )
        {
            // <literal> or <symbol>
            if( IsLiteral(s, SIGN_ALLOWED, NO_ACTION, &subopvalue[opidx]) || IsSymbol(s) )
            {
                optype[opidx] = ADDR_IMMEDIATE;

                // the operand in jmp/jxx instructions is either an unsigned word or an unsigned byte, depending on the datawidth
                subop[opidx]         = s;
                subopsize[opidx]     = (datawidth == UNKNOWN_DATAWIDTH ? 2 : datawidth);  // default datawidth is 2 for jmp/jxx instructions
                subopsigned[opidx]   = false;
                return true;
            }
        }
        else if( type == DATA_INSTRUCTION )
        {
            // <nonimmediate>
            return IsInstrOperandNonImmediate(s, opidx);
        }
        else if( type == JUMP_INSTRUCTION && 1 < s.length() && s[0] == '*' )
        {
            // *<nonimmediate>
            suboperand = s.substr(1, s.length()-1);
            return IsInstrOperandNonImmediate(suboperand, opidx);
        }
    }

    return false;
}

bool Assembler::IsInstrOperandNonImmediate(std::string& s, int32 opidx)
{
    if( opidx != 0 && opidx != 1 )
        return false;

    std::string suboperand;

    if( 3 == s.length() && s[0] == '%' && s[1] == 'r' && isregnum(s[2]) )
    {
        if( datawidth == 1 ) { std::cout << "WARNING - inconsistent size of operand '" << s << '\n'; return false; }
        if( datawidth == UNKNOWN_DATAWIDTH ) datawidth = 2;

        // %r<num>
        optype[opidx]   = ADDR_REG_DIRECT;
        opregnum[opidx] = s[2] - '0';
        return true;
    }
    else if( 4 == s.length() && s[0] == '%' && s[1] == 'r' && isregnum(s[2]) && (s[3] == 'h' || s[3] == 'l') )
    {
        if( datawidth == 2 ) { std::cout << "WARNING - inconsistent size of operand '" << s << '\n'; return false; }
        if( datawidth == UNKNOWN_DATAWIDTH ) datawidth = 1;

        // %r<num><h|l>
        optype[opidx]       = ADDR_REG_DIRECT;
        opregnum[opidx]     = s[2] - '0';
        opreglowhigh[opidx] = (s[3] == 'h' ? 1 : 0);
        return true;
    }
    else if( 5 == s.length() && s[0] == '(' && s[1] == '%' && s[2] == 'r' && isregnum(s[3]) && s[4] == ')' )
    {
        // (%r<num>)
        optype[opidx]   = ADDR_REG_INDIRECT;
        opregnum[opidx] = s[3] - '0';
        return true;
    }
    else if( 5 < s.length() && s[s.length()-5] == '(' && s[s.length()-4] == '%' && s[s.length()-3] == 'r' && isregnum(s[s.length()-2]) && s[s.length()-1] == ')' )
    {
        // <literal>(%r<num>) or <symbol>(%r<num>)
        suboperand = s.substr(0, s.length()-5);
        if( IsLiteral(suboperand, SIGN_ALLOWED, NO_ACTION, &subopvalue[opidx]) || IsSymbol(suboperand) ) 
        {
            optype[opidx]        = ADDR_REG_INDIRECT_WITH_16BIT_OFFSET;
            opregnum[opidx]      = s[s.length()-2] - '0';

            // the suboperand in register indirect addressing with 16-bit offset is a signed word
            subop[opidx]         = suboperand;
            subopsize[opidx]     = 2; 
            subopsigned[opidx]   = true;
            return true;
        }
    }
    else if( 5 < s.length() && s[s.length()-5] == '(' && s[s.length()-4] == '%' && s[s.length()-3] == 'p' && s[s.length()-2] == 'c' && s[s.length()-1] == ')' )
    {
        // <symbol>(%pc)
        suboperand = s.substr(0, s.length()-5);
        if( IsSymbol(suboperand) )
        {
            optype[opidx]        = ADDR_REG_INDIRECT_WITH_16BIT_OFFSET;
            opregnum[opidx]      = 7; // this is fixed because the program counter is the sevent register (pc === r7)

            // the suboperand in register indirect addressing with 16-bit offset is a signed word
            subop[opidx]         = suboperand;
            subopsize[opidx]     = 2; 
            subopsigned[opidx]   = true;
            return true;
        }
    }
    else 
    {
        if( IsLiteral(s, SIGN_NOT_ALLOWED, NO_ACTION, &subopvalue[opidx]) || IsSymbol(s) )
        {
            // <literal> or <symbol>
            optype[opidx]        = ADDR_MEMORY;

            // the operand in memory direct addressing is an unsigned word
            subop[opidx]         = s;
            subopsize[opidx]     = 2; 
            subopsigned[opidx]   = false;
            return true;
       }
    }

    return false;
}


// initialize the structures used for parsing lines
void Assembler::EraseCmd()
{
    tokencnt = 0;

    lbl.erase(0);
    dir.erase(0);
    ins.erase(0);
    coremnemonic.erase(0);

    code      = UNKNOWN_CODE;
    datawidth = UNKNOWN_DATAWIDTH;
    type      = UNKNOWN_TYPE;
    opcnt     = UNKNOWN_OPCNT;
    dstpos    = UNKNOWN_DSTPOS;

    for( int32 i = 0; i < OPCNT; i++ )
    {
        op[i].erase(0);

        optype[i]        = UNKNOWN_OPTYPE;
        opregnum[i]      = UNKNOWN_OPREGNUM;
        opreglowhigh[i]  = UNKNOWN_OPREGLOWHIGH;

        subop[i].erase(0);
        subopvalue[i]    = UNKNOWN_VALUE;
        subopsize[i]     = 0;
        subopsigned[i]   = false;
    }
}

int32 Assembler::ChangeSection( std::string s )
{
   int32 newsection = SECTION_UNKNOWN;

        if( s == "bss"  && currsection != SECTION_BSS  ) newsection = SECTION_BSS;
   else if( s == "data" && currsection != SECTION_DATA ) newsection = SECTION_DATA;
   else if( s == "text" && currsection != SECTION_TEXT ) newsection = SECTION_TEXT;

   if( newsection == currsection )
   {
       std::cout << "NOTE - already in section " << s << '\n';
   }
   else if( newsection != SECTION_UNKNOWN )
   {
       currsection = newsection;
       std::cout << "NOTE - switched to section " << s << '\n';
   }
   return currsection;
}

bool Assembler::ParseCmd(std::string line)
{
    linecnt++;  // add the new line to the assembler parsed line count
    std::cout << "   L" << linecnt << " --> '" << line << '\n';

    Trim(line, TRIMSCOPE_INSTRUCTION);
    if( line.empty() )
        return true;   // if the line is emtpy do nothing

    // std::cout << "'" << line << '\n';
    
    // for manual assembly checking
    linedump          = new LineDump; 
    linedump->line    = line; 
    linedump->section = SECTION_UNKNOWN;
    linedump->beg     = 0;
    linedump->end     = 0;

    // initialize the structures used for parsing lines
    EraseCmd();

    // check each token in line
    char breakchar;
    std::string token;
    while( !line.empty() )
    {
        if( !ins.empty() )                                        breakchar = ',';  // if the instruction is already detected, operands will follow, and they must be separated with ","
        else if( !dir.empty() && op[FIRST].empty() && opcnt > 1 ) breakchar = ',';  // if the instruction should have two operands, the first one is separated from the second one with ","
        else                                                      breakchar = ' ';

        token = GetToken(line, breakchar);
        tokencnt++;
     // std::cout << "'" << token << '\n';

        // unknown token
        if( lbl.empty() && ins.empty() && dir.empty() )
        {
            // check if the token is a directive, instruction or label
                 if( IsDirective(token) )   { dir = token; Trim(line, TRIMSCOPE_OPERANDS); }
            else if( IsInstruction(token) ) { ins = token; Trim(line, TRIMSCOPE_OPERANDS); }
            else if( IsLabel(token) )       { lbl = token.substr(0, token.length()-1); }  // remove the colon ':' from the label
            else { std::cout << "ERROR - bad token '" << token << '\n'; return false; }
        }
        // the line contains a label
        else if( !lbl.empty() && ins.empty() && dir.empty() )
        {
            // since the token contains a label, the following tokens must belong to either a data defining directive or an instruction (a label cannot be followed by a label or a non data defining directive)
            // in other words, a label can be placed before an instruction, or a .byte, .word or .skip directive (they are data defining), and there can only be one such label at that position
                 if( IsDirective(token, DATA_DEFINING_DIRECTIVE) ) { dir = token; Trim(line, TRIMSCOPE_OPERANDS); }
            else if( IsInstruction(token) )                        { ins = token; Trim(line, TRIMSCOPE_OPERANDS); }
            else { std::cout << "ERROR - bad token '" << token << '\n'; return false; }
        }
        // the line contains an instruction and the frequired first operand is being parsed
        else if( !ins.empty() && opcnt >= 1 && op[FIRST].empty() )
        {
                 if( IsInstrOperand(token, FIRST) ) op[FIRST] = token;
            else { std::cout << "ERROR - bad operand '" << token << "' in instruction '" << ins << '\n'; return false; }
        }
        // the line contains an instruction and the required second operand is being parsed
        else if( !ins.empty() && !op[FIRST].empty() && opcnt == 2 && op[SECOND].empty() )
        {
                 if( IsInstrOperand(token, SECOND) ) op[SECOND] = token;
            else { std::cout << "ERROR - bad 2nd operand '" << token << "' in instruction '" << ins << '\n'; return false; }
        }
        // the line contains a directive and the required first operand is being parsed
        else if( !dir.empty() && opcnt >= 1 && op[FIRST].empty() )
        {
                 if( IsDirOperand(token, FIRST) ) op[FIRST] = token;
            else { std::cout << "ERROR - bad operand '" << token << "' in directive '" << dir << '\n'; return false; }
        }
        // the line contains a directive and the required second operand is being parsed
        else if( !dir.empty() && !op[FIRST].empty() && opcnt == 2 && op[SECOND].empty() )
        {
            if( IsDirOperand(token, SECOND) ) op[SECOND] = token;
            else { std::cout << "ERROR - bad 2nd operand '" << token << "' in directive '" << dir << '\n'; return false; }
        }
        // excess or invalid token found in line
        else
        {
            { std::cout << "ERROR - excess or invalid token \"" << token << "\" in line " << linecnt << '\n'; return false; }
        }
    }

    // check if all the necessary instruction/directive parts are present
         if( !ins.empty() && opcnt == 1 && op[FIRST].empty()  ) { std::cout << "ERROR - INCOMPLETE INSTRUCTION - missing operand "            << linecnt << '\n'; return false; }
    else if( !ins.empty() && opcnt == 2 && op[FIRST].empty()  ) { std::cout << "ERROR - INCOMPLETE INSTRUCTION - missing the first operand "  << linecnt << '\n'; return false; }
    else if( !ins.empty() && opcnt == 2 && op[SECOND].empty() ) { std::cout << "ERROR - INCOMPLETE INSTRUCTION - missing the second operand " << linecnt << '\n'; return false; }
    else if( !dir.empty() && opcnt == 1 && op[FIRST].empty()  ) { std::cout << "ERROR - INCOMPLETE DIRECTIVE - missing operand "              << linecnt << '\n'; return false; }
    else if( !dir.empty() && opcnt == 2 && op[FIRST].empty()  ) { std::cout << "ERROR - INCOMPLETE DIRECTIVE - missing the first operand "    << linecnt << '\n'; return false; }
    else if( !dir.empty() && opcnt == 2 && op[SECOND].empty() ) { std::cout << "ERROR - INCOMPLETE DIRECTIVE - missing the second operand "   << linecnt << '\n'; return false; }

    if( !ins.empty() )
    {
        // store the instruction code in the current section buffer, starting from the next available location
        if( ! StoreInstrCode() ) return false;
    }
    else if( dir == ".equ" )
    {
        // check if the symbol being defined in the .equ directive is now fully resolbed
        if( ! CheckIfSymbolResolved( op[FIRST] ) ) return false;   
    }

    // cmd syntax was successfully checked so far
    PrintCmd();

    // for manual assembly checking
    dump.push_back( *linedump ); 
    return true;
}

// store the symbol value if the symbol belongs to a data-defining directive
bool Assembler::StoreData( SymItem& symitem )
{
    if( dir != ".byte" && dir != ".word" ) return true;

    int32 size = (dir == ".byte" ? 1 : 2);

    // for manual assembly checking
    if( linedump->section == SECTION_UNKNOWN )
    {
        linedump->section = currsection;
        linedump->beg     = locationcounter[currsection];
    }    

    // store a forwardref for assembler to backpatch when the symbol becomes defined, or for the loader to patch if the symbol is defined as extern
    if( !symitem.isdefined || symitem.section == SECTION_EXTERN )
        symitem.forwardref.push_back( { linecnt, currsection, locationcounter[currsection], size, FMT_BASED_ON_VALUE, MODE_ABSOLUTE } );

    // store a forwardref for the loader to patch when the final location of symbol's section becomes known
 // if( symitem.isdefined && (symitem.section == SECTION_BSS || symitem.section == SECTION_DATA || symitem.section == SECTION_TEXT) )
    if(                       symitem.section == SECTION_BSS || symitem.section == SECTION_DATA || symitem.section == SECTION_TEXT  )
        symboltable[ GetSectionName(symitem.section) ].forwardref.push_back( { linecnt, currsection, locationcounter[currsection], size, FMT_UNSIGNED, MODE_ABSOLUTE } );  

    // if the symbol is defined
    if( symitem.isdefined )
    {
        // store its value in the current section (which must be SECTION_DATA)
        if( ! IsValueStorable( symitem.value, size, FMT_BASED_ON_VALUE ) ) { std::cout << "ERROR - value "   << symitem.value << " cannot be stored because it is out of range\n"; return false; };
        if( ! StoreValue( symitem.value, size, FMT_BASED_ON_VALUE, MODE_ABSOLUTE, currsection, locationcounter[currsection] ) ) return false;
    }
    // otherwise
    else
    {
        // reserve space for later backpatching (initialize it with an invalid value for readability)
        if( ! StoreValue( 0xcccc, size, FMT_UNSIGNED, MODE_ABSOLUTE, currsection, locationcounter[currsection] ) ) return false; 
    }

    // for manual assembly checking
    linedump->end = locationcounter[currsection];
    return true;
}

// store instruction code in the current section buffer starting from the next available location
bool Assembler::StoreInstrCode()
{
    if( ins.empty() ) return true;

    // for manual assembly checking
    linedump->section = currsection;
    linedump->beg     = locationcounter[currsection];

    // the next available location in the current section buffer
    int32 loc = locationcounter[currsection];
    uns8 b;  // instruction byte

    // -------------------------------------------------------------------
    // the first instruction byte is InstrDescr "OC4 OC3 OC2 OC1 OC0 S UN UN" 
    // -------------------------------------------------------------------

    // if( code == UNKNOWN_CODE ) { std::cout << "ERROR - unknown instruction code in line " << linecnt; return false; }
    b = code << 3;                                      // operation code
    if( datawidth == UNKNOWN_DATAWIDTH ) datawidth = 2; // default datawidth is 2 
    b += (datawidth - 1) << 2;                          // actual width of the data

    if( loc > SECTION_SIZE ) { std::cout << "ERROR - text section overflow\n"; return false; }
    section[currsection][ loc++ ] = b;                  // store the instruction byte

    for( int32 i = 0; i < OPCNT && !op[i].empty(); i++ )
    {
        // -------------------------------------------------------------------
        // the first byte of operand is Op<num>Descr "AM2 AM2 AM0 R3 R2 R1 R0 L/H"
        // -------------------------------------------------------------------

     // if( optype[i] == UNKNOWN_OPTYPE ) { std::cout << "ERROR - unknown addressing mode of instruction in line " << linecnt; return false; }
        b = optype[i] << 5;
        if( opregnum[i]     != UNKNOWN_OPREGNUM )     b += opregnum[i] << 1;
        if( opreglowhigh[i] != UNKNOWN_OPREGLOWHIGH ) b += opreglowhigh[i];

        if( loc > SECTION_SIZE ) { std::cout << "ERROR - text section overflow\n"; return false; }
        section[currsection][ loc++ ] = b;  // store the first byte of operand

        // -------------------------------------------------------------------
        // the second and third bytes of the operand are Im/Di/Ad
        // -------------------------------------------------------------------

        if( !subop[i].empty() )   // Im/Di/Ad bytes do not exist if field subop[i] is empty
        {
            // check if <literal> or value of <symbol> can be stored in given number of bytes with a proper sign
            if( ! IsSubopStorable(i) ) return false;

            // STORING AN OPERAND WITH KNOWN VALUE
            // if the subop[i] contains a literal                                 --> write the value in the current section buffer (with range checking)
            // if the subop[i] contains a local symbol which is already defined   --> write the value in the current section buffer (with range checking)
            // if the subop[i] contains an extern symbol which is already defined --> write the value/offset in the current section buffer (with range checking), and make an entry in the forwardref table; the loader must resolve this symbol

            // STORING AN OPERAND WITH UNKNOWN VALUE
            // if the subop[i] contains a symbol which is not yet defined         --> write the value 0xcc or 0xcccc depending on the symbol size, and make an entry in the forwardref table; the assembler must resolve this symbol


            // decide which patching mode (absolute or relative) will be used after storing the operand value
            int32 patchmode = (IsSymbol(subop[i]) && opregnum[i] == 7 && optype[i] == ADDR_REG_INDIRECT_WITH_16BIT_OFFSET ? MODE_RELATIVE : MODE_ABSOLUTE );  // MODE_ABSOLUTE is used if a literal is given

            // if the symbol exists in symboltable
            if( IsSymbol(subop[i]) && symboltable.find( subop[i] ) != symboltable.end() )
            {
                SymItem& symitem = symboltable[subop[i]];

                // store a forwardref for assembler to backpatch (when symbol becomes defined), or for loader to patch (if symbol is already defined as extern)
             // if( subopvalue[i] == UNKNOWN_VALUE || symitem.section == SECTION_EXTERN )
                if( !symitem.isdefined || symitem.section == SECTION_EXTERN )
                   symitem.forwardref.push_back( { linecnt, currsection, loc, subopsize[i], subopsigned[i], patchmode } );  

                // store a forwardref for loader to patch when the final location of symbol's section becomes known
             // if( symitem.isdefined && (symitem.section == SECTION_BSS || symitem.section == SECTION_DATA || symitem.section == SECTION_TEXT ) )
                if(                       symitem.section == SECTION_BSS || symitem.section == SECTION_DATA || symitem.section == SECTION_TEXT   )
                {
                    symboltable[ GetSectionName(symitem.section) ].forwardref.push_back( { linecnt, currsection, loc, subopsize[i], subopsigned[i], patchmode } );  
                }

                // in case when the symbol does not belong to the same "bss", "data" or "text" section, store a zero value (the loader will add to it the symbol's relative address once the section is loaded)
                if( patchmode == MODE_RELATIVE )
                {
                    if( symitem.section == SECTION_UNKNOWN || symitem.section == SECTION_EXTERN )
                        patchmode = MODE_ZERO;
                    else if( symitem.section != currsection )
                        patchmode = MODE_ZERO;
                }    
            }

            // store the symbol's value in current section
            if( ! StoreValue( subopvalue[i], subopsize[i], subopsigned[i], patchmode, currsection, loc ) ) return false;
        }
    }

    // for manual assembly checking
    linedump->end = loc;

    // the instruction is successfully stored
    locationcounter[currsection] = loc; 
    return true;
}

bool Assembler::StoreValue( int32 value, int32 size, int32 /* fmt */, int32 patchmode, int32 StoreInSection, int32& loc )
{
    // for manual assembly checking
    if( linedump->section == SECTION_UNKNOWN )
    {
        linedump->section = currsection;
        linedump->beg     = locationcounter[currsection];
    }    
        


    
    if( patchmode == MODE_RELATIVE )
        value = value - (loc + size);  // in case of relative addressing, adjust the value that will be stored (subtract the address of the first byte after the operand)
    else if( patchmode == MODE_ZERO )
        value = 0;                     // store a zero value



    // little-endian -- lower byte goes to a lower address
    uns8 lo = (value != UNKNOWN_VALUE ?  value & 0x000000ff       : 0xcc);   // low byte of value,  or invalid value (for readability)
    uns8 hi = (value != UNKNOWN_VALUE ? (value & 0x0000ff00) >> 8 : 0xcc);   // high byte of value, or invalid value (for readability)
    if( size >= 1 )
    {
        section[StoreInSection][ loc++ ] = lo;  // store the second byte of operand
        if( loc > SECTION_SIZE ) { std::cout << "ERROR - text section overflow\n"; return false; }
    }
    if( size == 2 )
    {
        section[StoreInSection][ loc++ ] = hi;  // store the third byte of operand
        if( loc > SECTION_SIZE ) { std::cout << "ERROR - text section overflow\n"; return false; }
    }

    // for manual assembly checking
    linedump->end = locationcounter[currsection];
    return true;
}

// checks if a value can be stored in bytesize bytes as signed/unsigned number
bool Assembler::IsValueStorable( int32 value, int32 size, int32 fmt )
{
    bool issigned = (fmt == FMT_BASED_ON_VALUE && value < 0) || fmt == FMT_SIGNED;  // check if the value must be formatted as signed

    return   ( size == 1 && !issigned && UNSIGNED_BYTE_MIN <= value && value <= UNSIGNED_BYTE_MAX )
          || ( size == 1 &&  issigned && SIGNED_BYTE_MIN   <= value && value <= SIGNED_BYTE_MAX   )
          || ( size == 2 && !issigned && UNSIGNED_WORD_MIN <= value && value <= UNSIGNED_WORD_MAX )
          || ( size == 2 &&  issigned && SIGNED_WORD_MIN   <= value && value <= SIGNED_WORD_MAX   );
}

// checks if a suboperand value can be stored in bytesize bytes as signed/unsigned number
bool Assembler::IsSubopStorable( int32 opidx )
{
    int32 value = subopvalue[opidx];
    bool unknownvalue = (value == UNKNOWN_VALUE);

    if( unknownvalue && !symboltable[subop[opidx]].isdefined ) return true; // no value to check

    // if the value of subop is not known, get it from symboltable (otherwise use the given value)
    if( unknownvalue ) 
    { 
       value = symboltable[subop[opidx]].value; 
       subopvalue[opidx] = value;
    }

    // find out if the value is storable
    bool storable = IsValueStorable( value, subopsize[opidx], subopsigned[opidx] );
    if( !storable && unknownvalue) { std::cout << "WARNING - value "   << value << " of sybmol '" << subop[opidx] << "' is out of range\n"; return false; }
    if( !storable                ) { std::cout << "WARNING - literal " << value                                   <<  " is out of range\n"; return false; }
    return true;
}

bool Assembler::Backpatch( std::string _symbol )
{
    if( !symboltable[_symbol].isdefined ) return true;  // backpatching can't be done if the symbol isn't defined

    SymItem& symitem = symboltable[_symbol];

    // go through all forward references of a symbol and try to patch them where specified
    for( FwdRef& fwref : symitem.forwardref )
    {
        // check if value is storable
        if( !IsValueStorable( symitem.value, fwref.patchsize, fwref.patchfmt ) ) { std::cout << "ERROR - value "   << symitem.value << " of sybmol '" << symitem.symbol << "' can't be patched in line " << fwref.patchline << " because it is out of range\n"; return false; }

        // value of fwref.patchoffset shouldn't be changed during backpatching (because it might be kept for later use by loader)
        // therefore a temporary variable is created (since StoreValue() will update it)
        int32 loc = fwref.patchoffset;  

        // store a value
        int32 patchmode = ( symitem.section == SECTION_EXTERN ? MODE_ABSOLUTE : fwref.patchmode );
        StoreValue( symitem.value, fwref.patchsize, fwref.patchfmt, patchmode, fwref.patchsection, loc );
    }

//  // if the symbol is extern, keep all forward references to it (which will be used later by loader)
//  // otherwise erase the forwardref table, since all forward references of a local symbol are successfully patched
//  if( symitem.section != SECTION_EXTERN )
//      symitem.forwardref.clear();

    return true;
}

void Assembler::PrintCmd()
{
    // show what was recognized
    if( !ins.empty() )
    {
        // show details just for existing parts of the instruction
        if( !lbl.empty() ) std::cout << SEP3 << "lbl=" << lbl << '\n';
                           
        std::cout << SEP3 << "ins=" << ins << " " << "[oc=" << code << " core=" << coremnemonic << " wid=" << datawidth << " type=" << type << " opcnt=" << opcnt << "]\n";

        for( int32 i = 0; i < OPCNT && !op[i].empty(); i++ )
        {
            std::cout << SEP3 << "op" << (i+1) << "="  << op[i]  << " " << "[addrtype=" << optype[i] << " regnum=" << opregnum[i] << " low/high=" << opreglowhigh[i] << "]";
            if( !subop[i].empty() )
            {
                std::cout << " [subop=" << subop[i] << " value=";
                if( subopvalue[i] != UNKNOWN_VALUE ) std::cout << subopvalue[i];
                std::cout << " size=" << subopsize[i] << " signed=" << subopsigned[i] << "]";
            }
            std::cout << '\n';
        }
    }
    else if( !dir.empty() )
    {
        // show details just for existing parts of the directive
        if( !lbl.empty() ) std::cout << SEP3 << "lbl=" << lbl << '\n';
        
        std::cout << SEP3 << "dir=" << dir << " " << "[dc=" << code << " core=" << coremnemonic << " wid=" << datawidth << " type=" << type << " opcnt=" << opcnt << "]\n";

        for( int32 i = 0; i < OPCNT && !op[i].empty(); i++ )
            std::cout << SEP3 << "op" << (i+1) << "="  << op[i]  << " " << "[type=" << optype[i] << "]\n";
    }
    else if( !lbl.empty() )
       std::cout << SEP3 << "lbl=" << lbl << '\n';
}

// execute the given action using the literal value over the one in the symboltable
bool Assembler::ProcessLiteral( int32 literalval, int32 action )
{
    if( dir == ".byte" || dir == ".word" )
    {
        // if the literal belongs to a data-defining directive, store its value in the current section (which must be SECTION_DATA)
        int32 size = (dir == ".byte" ? 1 : 2);
        if( ! IsValueStorable( literalval, size, FMT_BASED_ON_VALUE ) ) { std::cout << "ERROR - value "   << literalval << " can't be stored because it is out of range\n"; return false; };
        if( ! StoreValue( literalval, size, FMT_BASED_ON_VALUE, MODE_ABSOLUTE, currsection, locationcounter[currsection] ) ) return false;
    }
    else if( dir == ".skip" )
    {
        // if the literal belongs to a skip directive, skip the given number of bytes in the current section (which must be either SECTION_DATA or SECTION_BSS)
        if( currsection == SECTION_BSS )
        {
            // for manual assembly checking
            linedump->section = currsection;
            linedump->beg     = locationcounter[currsection];
            linedump->end     = locationcounter[currsection];

            if( (locationcounter[currsection] += literalval) > SECTION_SIZE ) { std::cout << "ERROR - bss/data section overflow\n"; return false; }
        }

        if( currsection == SECTION_DATA )
            for( int32 i = 0; i < literalval; i++ )   // in .skip directive only positive values are allowed 
                if( ! StoreValue( 0xcc, 1, false, MODE_ABSOLUTE, currsection, locationcounter[currsection] ) ) return false;   // in case of SECTION_DATA write 0xcc for readability
    }

    if( action == NO_ACTION )
        return true;

    if( action == CALCULATE_ANOTHER_SYMBOL )
    {
        // this literal value is used to calculate another symbol (op[FIRST])
        if( op[FIRST].empty() ) return false;
        SymItem& symitem = symboltable[op[FIRST]];
        symitem.value += literalval;   // add its value to another symbol's (op[FIRST]) value
    }

    return true;
}

// execute the given action on a symbol in symboltable
// add the symbol to the symbol table if it doesn't exist
bool Assembler::ProcessSymbol( std::string symbol, int32 action )
{
    if( action == NO_ACTION )
        return true;

    // create _symbol by removing the '+' or '-' sign from the beginning of the symbol
    std::string _symbol;
    int32 sign = 1;
    if( symbol.length() > 1 && (symbol[0] == '+' || symbol[0] == '-') )
    {
        if( symbol[0] == '-' ) sign = -1;
        _symbol = symbol.substr(1, symbol.length()-1);
    }
    else
        _symbol = symbol;

    // find the _symbol in symboltable / prepare to insert _symbol in symboltable
    bool issymbolintable = true;
    SymItem si;
    SymItem* symitem =& si;
    auto e = symboltable.find( _symbol ); // e = element
    if( e != symboltable.end() )   // symbol already exists in symboltable
        symitem = &e->second;  
    else                          // symbol does not exist in symboltable
    {
        // prepare to add the symbol to the symboltable
        issymbolintable = false;
        symitem->rn            = (int32) symboltable.size();
        symitem->symbol        = _symbol;
     // symitem->alias         =
        symitem->section       = SECTION_UNKNOWN;
        symitem->value         = 0;
        symitem->origin        = ORIGIN_UNKNOWN;
        symitem->isdefined     = false;
        symitem->depcnt        = 0;
     // symitem->depends       =
     // symitem->forwardref    =
     // symitem->resolves      = 
    }

    if( action == DEFINE_THIS_SYMBOL )
    {
        if( symitem->isdefined )              { std::cout << "WARNING - symbol " << symitem->symbol << " cannot be defined more than once\n"; return false; }
        if( symitem->origin & ORIGIN_EXTERN ) { std::cout << "WARNING - symbol " << symitem->symbol << " cannot be declared both as extern and local\n"; return false; }

        // swap to the new section (so this symbol will link to it)
        if( dir == ".section" ) ChangeSection( _symbol );

        // define the symbol in symboltable
        symitem->section   = currsection;
        symitem->value     = locationcounter[currsection]; // the value of the symbol is equal to its location in the current section
        symitem->origin    |= ORIGIN_LOCAL;
        symitem->isdefined = true;
        std::cout << "NOTE - symbol '" << symitem->symbol << "' defined (value=" << symitem->value << ")\n";
    }
    else if( action == DECLARE_THIS_SYMBOL_EXTERN )
    {
        if( symitem->isdefined )              { std::cout << "WARNING - symbol " << symitem->symbol << " cannot be declared as extern since it is already defined\n"; return false; }
        if( symitem->origin & ORIGIN_LOCAL 
         || symitem->origin & ORIGIN_GLOBAL ) { std::cout << "WARNING - symbol " << symitem->symbol << " cannot be declared as extern - it is already declared as local/global\n"; return false; }
        if( symitem->origin & ORIGIN_EXTERN ) { std::cout << "NOTE - symbol "    << symitem->symbol << " already declared as extern\n"; return true; }

        // declare this symbol as extern
        symitem->section = SECTION_EXTERN;  // the symbol is placed in an artificial section SECTION_EXTERN to separate it from other internal symbols while resolving .equ expressions
        symitem->origin  |= ORIGIN_EXTERN;
        symitem->isdefined = true;          // extern symbols are treated as defined
        std::cout << "NOTE - symbol '" << symitem->symbol << "' declared as extern\n";
    }
    else if( action == DECLARE_THIS_SYMBOL_GLOBAL )
    {
        if( symitem->origin & ORIGIN_EXTERN ) { std::cout << "WARNING - symbol " << symitem->symbol << " can't be declared as global - it is already declared as extern\n"; return false; }
        if( symitem->origin & ORIGIN_GLOBAL ) { std::cout << "NOTE - symbol "    << symitem->symbol << " already declared as global\n"; return true; }

        // declare this symbol as global
        symitem->origin |= ORIGIN_GLOBAL;
        std::cout << "NOTE - symbol '" << symitem->symbol << "' declared as global\n";
    }
    else if( action == CALCULATE_THIS_SYMBOL )   
    {
        if( symitem->isdefined )               { std::cout << "WARNING - symbol " << symitem->symbol << " can't be calculated since it is already defined\n"; return false; }
        if( symitem->origin & ORIGIN_EXTERN )  { std::cout << "WARNING - symbol " << symitem->symbol << " can't be calculated - it is already declared as extern\n"; return false; }
        if( symitem->origin & ORIGIN_LOCAL )   { std::cout << "WARNING - symbol " << symitem->symbol << " can't be calculated - it is already declared as local\n"; return false; }
        if( symitem->origin & ORIGIN_LVALUE )  { std::cout << "WARNING - symbol " << symitem->symbol << " can't be calculated more than once\n"; return false; }

        // declare that this symbol will be calculated later on
        symitem->origin |= ORIGIN_LVALUE;
        std::cout << "NOTE - symbol '" << symitem->symbol << "' declared as dependent\n";
    }
    else if( action == CALCULATE_ANOTHER_SYMBOL )   
    {
        if( symitem->isdefined )
        {
            // this symbol is already defined, so it can immediately be used to resolve other symbols (op[FIRST])
            if( ! ResolveDependency( symboltable[op[FIRST]], *symitem, sign ) ) return false;   // resolve other symbols' (op[FIRST]) dependency to this symbol
        }
        else
        {
            // remember which symbol (op[FIRST]) is calculated based on the current symbol, and if the current symbol is added or subtracted in the calculation
            std::string reference;
            reference = (sign > 0 ? "+" : "-") + op[FIRST];  // reference always begin on "+" or "-" sign
            symitem->resolves.push_back( reference );        // this symbol will resolve op[FIRST] symbol
            symboltable[op[FIRST]].depcnt ++;                // count this dependency

            // remember that the other symbol (op[FIRST]) depends on this symbol (but only if op[FIRST] doesn't already depend on it)
            std::vector<std::string>& d = symboltable[op[FIRST]].depends;

            if( std::find( d.begin(), d.end(), _symbol ) == d.end() )
                d.push_back( _symbol ); // remember that the op[FIRST] symbol depends on this symbol
        }
    }

    // add the symbolitem to the symbol table (if it doesn't exist in the sumbol table)
    if( !issymbolintable )
    {
        symboltable.insert( { _symbol, *symitem } ); // insert symitem in table
        symitem =& symboltable[ _symbol];           // from now on, symitem points to an item in symbol table!
    }

    // store the symbol value if the symbol belongs to a data-defining directive
    if( dir == ".byte" || dir == ".word" )
        if( ! StoreData( *symitem ) ) return false;
     
    // if the symbol is has just been defined
    if( action == DEFINE_THIS_SYMBOL || action == DECLARE_THIS_SYMBOL_EXTERN )
    {
        // try to backpatch all forward references
        if( !Backpatch( symitem->symbol ) ) return false;

        // check if dependent symbols have been resolved
        if( !ResolveDependentSymbols( symitem->symbol ) ) return false;
    }
    return true;
}

bool Assembler::ResolveDependentSymbols( std::string _symbol )
{
    // get the symbol item from the symboltable
    auto e = symboltable.find( _symbol );       // e = element
    if( e == symboltable.end() ) return true;   // skip operation if _symbol does not exist in symboltable
    SymItem& symitem = e->second;  
    if( !symitem.isdefined ) return true;       // _symbol must be defined to be able to resolve dependent symbols

    // go through all symbols where the current symbol is refered to, and resolve their dependency on the current symbol
    auto i = symitem.resolves.begin();
    while( i != symitem.resolves.end() )
    {
        // get the dependent symbol from the symbol table, excluding its '+' or '-' sign (at the begining of dependent symbol)
        std::string& _depsymbol = *i;                // dependent symbol
        if( _depsymbol.length() < 2 || (_depsymbol[0] != '+' && _depsymbol[0] != '-')) return false;  // error -- if the dependent symbol is given without '+' or '-' sign
        int32 sign = (_depsymbol[0] == '-' ? -1 : 1); 
        std::string depsymbol = _depsymbol.substr( 1, _depsymbol.length()-1 );  
        auto de = symboltable.find( depsymbol );          // de = dependent element
        if( de == symboltable.end() ) { i++; continue; }  // continue if _depsymbol does not exist in symboltable
        SymItem& depsymitem = de->second;
        if( depsymitem.isdefined ) { i++; continue; }     // _depsymbol must not be defined

        if( ! ResolveDependency( depsymitem, symitem, sign ) ) return false;  // resolve the dependent symbol's dependency on the given symbol
        i = symitem.resolves.erase(i);                                        // remove the dependency which has now been resolved
        depsymitem.depcnt --;                                                 // decrease the number of the dependent symbol's dependencies

        // if there are no other mentions of depsymitem.symbol in the symitem.resolves list,
        // remove the dependency of depsymitem on the symitem (which is now resolved)
        std::vector<std::string>& r = symitem.resolves;
        std::string posdepsymbol, negdepsymbol;
        posdepsymbol = "+" + depsymbol;
        negdepsymbol = "-" + depsymbol;
        if(   std::find( r.begin(), r.end(), posdepsymbol ) == r.end() 
           && std::find( r.begin(), r.end(), negdepsymbol ) == r.end() )
        {
            std::vector<std::string>& x = depsymitem.depends;
            auto j = std::find( x.begin(), x.end(), _symbol );
            if( j != x.end() )
                x.erase( j );
        }

        // check if the dependent symbol has now been resolved
        if( ! CheckIfSymbolResolved( depsymitem.symbol ) )
            return false;
    }
    return true;
}

// resolve dependent symbol dependency on another symbol
bool Assembler::ResolveDependency( SymItem& depsymitem, SymItem& symitem, int32 sign )
{
    // extern symbols must have positive sign in expressions (both those declared as extern, and those resolved as extern)
    if( symitem.section == SECTION_EXTERN && sign < 0 )
        { std::cout << "WARNING - extern symbol '" << symitem.symbol << "' can't have negative sign in expression for resolving dependent symbol '" << depsymitem.symbol << '\n'; return false; }

    if( symitem.section == SECTION_EXTERN && depsymitem.origin & ORIGIN_GLOBAL )
        { std::cout << "WARNING - extern symbol '" << symitem.symbol << "' can't resolve dependent symbol '" << depsymitem.symbol << "' which is declared as global\n"; return false; }

    depsymitem.value += sign * symitem.value;               // add value of this element to the dependent symbol's value
    depsymitem.depcntinsection[ symitem.section ] += sign;  // increase/decrease number of symbol's dependencies in corresponding sections

    // if this is en extern symbol, save symitem->symbol as alias of op1symitem.symbol
    if( symitem.section == SECTION_EXTERN )
    {
        depsymitem.alias = ( symitem.alias.length() > 0 ? symitem.alias : symitem.symbol );
        depsymitem.origin |= ORIGIN_EXTERN;  // treat dependent symbol as ORIGIN_EXTERN from now on
        std::cout << "NOTE - symbol '" << symitem.symbol << "' saved as alias of symbol '" << depsymitem.symbol << '\n';
    }
    return true;
}

bool Assembler::CheckIfSymbolResolved( std::string _symbol )
{
    auto e = symboltable.find( _symbol );      // e = element
    if( e == symboltable.end() ) return true;  // skip operation if _symbol does not exist in symboltable
    SymItem& symitem = e->second;  
    if( symitem.isdefined ) return true;       // if the symbol is defined then it is certainly resolved

    // if the given symbol does not depend on any other symbol, but is not yet marked as defined
    if( symitem.depcnt == 0 && !symitem.isdefined )
    {
        // symbol can be resolved using the same number of positive and negative symbols from each section, 
        // except in one section, where a single positive symbol may suffice

        if( abs( symitem.depcntinsection[ SECTION_BSS    ] ) > 1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on more than one BSS symbol\n"; return false; }
        if( abs( symitem.depcntinsection[ SECTION_DATA   ] ) > 1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on more than one DATA symbol\n"; return false; }
        if( abs( symitem.depcntinsection[ SECTION_TEXT   ] ) > 1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on more than one TEXT symbol\n"; return false; }
        if( abs( symitem.depcntinsection[ SECTION_EXTERN ] ) > 1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on more than one EXTERN symbol\n"; return false; }

        if(   abs( symitem.depcntinsection[ SECTION_BSS    ] ) 
            + abs( symitem.depcntinsection[ SECTION_DATA   ] ) 
            + abs( symitem.depcntinsection[ SECTION_TEXT   ] ) 
            + abs( symitem.depcntinsection[ SECTION_EXTERN ] ) > 1 )
            { std::cout << "WARNING - symbol '" << symitem.symbol << "' can't be resolved since it depends on symbols from different sections\n"; return false; }

        if( symitem.depcntinsection[ SECTION_BSS    ] == -1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on negative BSS symbol\n"; return false; }
        if( symitem.depcntinsection[ SECTION_DATA   ] == -1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on negative DATA symbol\n"; return false; }
        if( symitem.depcntinsection[ SECTION_TEXT   ] == -1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on negative TEXT symbol\n"; return false; }
        if( symitem.depcntinsection[ SECTION_EXTERN ] == -1 ) { std::cout << "WARNING - symbol '" << symitem.symbol << "' cannot be resolved since it depends on negative EXTERN symbol\n"; return false; }

        // from now on, this symbol will be treated as if it belongs to one of the following sections
             if( symitem.depcntinsection[ SECTION_BSS    ] == 1 ) { symitem.section = SECTION_BSS;  }
        else if( symitem.depcntinsection[ SECTION_DATA   ] == 1 ) { symitem.section = SECTION_DATA; }
        else if( symitem.depcntinsection[ SECTION_TEXT   ] == 1 ) { symitem.section = SECTION_TEXT; }
        else if( symitem.depcntinsection[ SECTION_EXTERN ] == 1 ) { symitem.section = SECTION_EXTERN;  }

        // mark that this symbol is now resolved -- defined
        symitem.isdefined = true;  
        std::cout << "NOTE - symbol '" << symitem.symbol << "' has been resolved (" << (symitem.section == SECTION_EXTERN ? "extern, offset=" : "value=") << symitem.value << ")\n";

        // since the symbol is now resolved, try to backpatch all forward references
        if( ! Backpatch( symitem.symbol ) ) return false;

        // check if it would be possible to resolve some other symbols
        return ResolveDependentSymbols( symitem.symbol );
    }
    return true;
}

void Assembler::PrintSymbolTable()
{
    std::cout << "========================= SYMBOL RESOLVING TABLE =========================\n\n";
    std::cout << "------------------------------------------------------------------------------------------------------------------------------------\n";
    std::cout << "    rn\tsymbol\talias\tsection\tvalue\torigin\tdefined\t bss:data:text:ext \tdepcnt\tdepends / resolves / forwardref\n";
    std::cout << "------------------------------------------------------------------------------------------------------------------------------------\n";

    for( auto e: symboltable )
    {
        SymItem& symitem = e.second;

        std::cout << std::setw(6) << symitem.rn << '\t';
        std::cout << symitem.symbol << "\t";
        std::cout << symitem.alias << "\t";

             if( symitem.section == SECTION_UNKNOWN ) std::cout << "?\t";
        else if( symitem.section == SECTION_BSS     ) std::cout << "bss\t";
        else if( symitem.section == SECTION_DATA    ) std::cout << "data\t";
        else if( symitem.section == SECTION_TEXT    ) std::cout << "text\t";
        else if( symitem.section == SECTION_EXTERN  ) std::cout << "extern\t";
        else                                          std::cout << symitem.section << '\t';

        std::cout << symitem.value << '\t';

        std::cout << (symitem.origin & ORIGIN_LOCAL   ? "L" : "");
        std::cout << (symitem.origin & ORIGIN_GLOBAL  ? "G" : "");
        std::cout << (symitem.origin & ORIGIN_EXTERN  ? "E" : "");
        std::cout << (symitem.origin & ORIGIN_LVALUE  ? "C" : ""); // C means CALCULATED
        std::cout << (symitem.origin & ORIGIN_SECTION ? "S" : "");
        std::cout << '\t';

        std::cout << (symitem.isdefined ? "defined" : "?") << '\t';

        for( int32 i = SECTION_BSS; i <= SECTION_EXTERN; i++ )
        {
            std::cout << std::setw(4);
            if( symitem.depcntinsection[i] == 0 ) std::cout << "    "; 
            else                                  std::cout << symitem.depcntinsection[i]; 
            if( i != SECTION_EXTERN )
                std::cout << ":";
        }
        std::cout << "\t";

        std::cout << symitem.depcnt << '\t';

        for( std::string d: symitem.depends )
            std::cout << "(" << d << ")";
        std::cout << " / ";
        for( std::string r: symitem.resolves )
            std::cout << "[" << r << "]";


        std::cout << " / ";
        for( const auto& f: symitem.forwardref )
            std::cout << "{S" << f.patchsection << "." << f.patchoffset << ":" << ( f.patchmode == MODE_ABSOLUTE ? "A" : "R" ) << f.patchsize << (f.patchfmt == FMT_BASED_ON_VALUE ? "?" : (f.patchfmt == FMT_SIGNED ? "s" : "u")) << "@L" << f.patchline <<"}";

        std::cout << '\n';
    }

    std::cout << "------------------------------------------------------------------------------------------------------------------------------------\n";
    std::cout << '\n';
}

void Assembler::PrintSection( int32 s )
{
    std::cout << '\n';
    std::cout << "========================= section S" << s << " - ";
         if( s == SECTION_BSS )  std::cout << "BSS SECTION";
    else if( s == SECTION_DATA ) std::cout << "DATA SECTION";
    else if( s == SECTION_TEXT ) std::cout << "TEXT SECTION";
    else return;

    std::cout << " [size=" << locationcounter[s] << "] =========================\n";

    // print the section content as a table of hex values
    std::cout << '\n';
    std::cout << "         +0 +1 +2 +3 +4  +5 +6 +7 +8 +9\n";
    std::cout << "         --------------  --------------\n";
    if( s == SECTION_BSS ) // content of SECTION_BSS is not printed
    {
        // just signal that SECTION_BSS section size is > 0
        if( locationcounter[s] > 0 )
        {
            std::cout << "         ";
            for( int32 i = 0; i < 10 && i < locationcounter[s]; i++ )
            {
                if( i == (i /  5) *  5 && i > 0 ) std::cout << " ";
                std::cout << "?? ";
            }
            std::cout << '\n';
        }
    }
    else
    {
        for( int32 i = 0; i < locationcounter[s]; i++ )
        {
            if( i == (i /  5) *  5 && i > 0 ) std::cout << " ";
            if( i == (i / 10) * 10 && i > 0 ) std::cout << '\n';
            if( i == (i / 10) * 10          ) std::cout << std::setw(6) << i << "   ";
            std::cout << std::setfill('0') << std::right << std::hex << std::setw(2) << (uns32) section[s][i] << std::setfill(' ') << std::dec << std::left << " ";
        }
        if( locationcounter[s] > 0 ) std::cout << '\n';
    }
    std::cout << "         --------------  --------------\n";
}

void Assembler::PrintDump()
{
    std::cout << "========================= LINE ASSEMBLY =========================\n\n";
    std::cout << " ofset   hex                              command  assembly\n";
    std::cout << "------  ----  -----------------------------------  -----------------------------------\n";
    for( auto d : dump )
    {
        // print offset
        if( d.section == SECTION_BSS || d.section == SECTION_DATA || d.section == SECTION_TEXT )
        {
            std::cout <<                                                std::setw(6) << (uns32) d.beg                                               << "  ";
            std::cout << std::right << std::setfill('0') << std::hex << std::setw(4) << (uns32) d.beg << std::setfill(' ') << std::dec << std::left << "  ";
        }
        else
            std::cout << "              ";

        // print line
        std::cout << std::right << std::setw(35) << d.line << std::left;

        if( d.section == SECTION_DATA || d.section == SECTION_TEXT )
        {
            std::cout << "  ";

            // print relevant bytes from the corresponding section
            for( int32 i = d.beg; i < d.end; i++ )
                std::cout << std::setfill('0') << std::right << std::hex << std::setw(2) << (uns32) section[d.section][i] << std::setfill(' ') << std::dec << std::left << " ";
        }
        std::cout << '\n';
    }
    std::cout << "------  ----  -----------------------------------  -----------------------------------\n";
}

bool Assembler::Finish( std::ofstream& outfile )
{
    // print what was assembled so far
    std::cout << '\n';
    PrintSymbolTable();
    PrintSection( SECTION_BSS );
    PrintSection( SECTION_DATA );
    PrintSection( SECTION_TEXT );

    // check if all symbols are defined
    // TODO: but only if the symbols are used somewhere in the assembly
    // print all symbols which are not defined (not just the first one)
    bool allsymbolsdefined = true;
    for( auto e : symboltable )
    {
        SymItem& symitem = e.second;  
        if( ! symitem.isdefined )
        {
            allsymbolsdefined = false;
            std::cout << "ERROR - symbol '" << symitem.symbol << "' couldn't be resolved\n";
        }
    }
    if( ! allsymbolsdefined )
       return false;

    PrintDump();

    // export all data to the output file
    ExportSectionTable( outfile );
    ExportSymbolTable( outfile );
    ExportRelocationTable( SECTION_TEXT, outfile );
    ExportRelocationTable( SECTION_DATA, outfile );
    ExportRelocationTable( SECTION_BSS,  outfile );
    ExportSection( SECTION_TEXT, outfile );
    ExportSection( SECTION_DATA, outfile );
    return true;
}

bool Assembler::IsSymbolExportable( SymItem& symitem )
{
    return symitem.origin & (ORIGIN_EXTERN | ORIGIN_GLOBAL | ORIGIN_SECTION );  // only these symbols are exported
}

// export the section table
void Assembler::ExportSectionTable( std::ofstream& outfile )
{
    int32 s = SECTION_UNKNOWN;
    outfile << ".sections\n";
    outfile << ";name\ttype\tsize\n";
    for( auto e: symboltable )
    {
        SymItem& symitem = e.second;
        if( symitem.symbol == GetSectionName(symitem.section) )  // symbol representing a section
        {
            s = GetSectionByName(symitem.symbol);
            outfile << symitem.symbol << "\t" << GetSectionType(s) << "\t" << locationcounter[s] << '\n';
        }
    }    
}

// export the symbol table to the given file
void Assembler::ExportSymbolTable( std::ofstream& outfile )
{
    // symboltable is not exported if it is empty
    if( symboltable.size() == 0 ) return;

    bool headerexported = false;
    for( auto e: symboltable )
    {
        SymItem& symitem = e.second;
        if(   symitem.section == SECTION_UNKNOWN ) continue;      // only known sections can be exported
        if(  !symitem.alias.empty() ) continue;                   // symbols based on other symbols (.equ resolved) are not exported

     // if( !(    (symitem.origin & ORIGIN_GLOBAL)                       // only global symbols 
     //        || !symitem.forwardref.empty()                            // ... or symbols with forward references 
     //        || symitem.symbol == GetSectionName(symitem.section) ) )  // ... or symbols representing sections
     //       continue;                                                  // ... are exported

        if( !IsSymbolExportable(symitem) ) continue;              // only some symbols are exported

        if( !headerexported ) // the export of the symbol table header is postponed until the first exportable symbol is found
        {
            outfile << ".symbols\n";
            outfile << ";symbol\tsection\toffset\ttype\n";
            headerexported = true;
        }

     // outfile << symitem.symbol << "\t" << GetSectionName( symitem.section ) << "\t" << symitem.value << "\t" << (symitem.origin & ORIGIN_EXTERN ? "E" : (symitem.origin & ORIGIN_GLOBAL ? "G" :                                                              "L" )) << '\n';
     // outfile << symitem.symbol << "\t" << GetSectionName( symitem.section ) << "\t" << symitem.value << "\t" << (symitem.origin & ORIGIN_EXTERN ? "E" : (symitem.origin & ORIGIN_GLOBAL ? "G" : (symitem.symbol == GetSectionName( symitem.section ) ? "S" : "L"))) << '\n';
        outfile << symitem.symbol << "\t" << GetSectionName( symitem.section ) << "\t" << symitem.value << "\t" << (symitem.origin & ORIGIN_EXTERN ? "E" : (symitem.origin & ORIGIN_GLOBAL ? "G" : (symitem.origin & ORIGIN_SECTION                     ? "S" : "L"))) << '\n';
    }
}

// export the relocation table for the given section to the given file
void Assembler::ExportRelocationTable( int32 s, std::ofstream& outfile )
{
    if( s != SECTION_BSS && s != SECTION_DATA && s != SECTION_TEXT ) return;   // not all sections should be exported
    if( symboltable.find( GetSectionName(s) ) == symboltable.end() ) return;   // sections are not exported if they are not defined
    
    bool headerexported = false;
    for( auto e : symboltable )
    {
        SymItem& symitem = e.second;
        if( symitem.section == SECTION_UNKNOWN ) continue;
            
        for( FwdRef& fwref : symitem.forwardref )
        {
            if( fwref.patchsection == s )
            {
                if( !headerexported ) // the export of the relocation table header is postponed until the first symbol is found
                {
                    outfile << ".rel." << GetSectionName(s) << '\n';
                    outfile << ";symbol\toffset\ttype\n";
                    headerexported = true;
                }

             // outfile << ( !symitem.alias.empty() ? symitem.alias : symitem.symbol) << "\t";  // alias has precedance

                if( !symitem.alias.empty() ) 
                    outfile << symitem.alias << "\t";                     // the symbol's alias has precedence over the symbol's name
                else if( !IsSymbolExportable( symitem ) )
                    outfile << GetSectionName( symitem.section ) << "\t"; // if the symbol should not be exported (for example local symbols), then the section name is exported instead
                else
                    outfile << symitem.symbol << "\t";                    // export the symbol name


             // outfile << std::setfill('0') << std::right << std::hex << std::setw(4) << (uns32) fwref.patchoffset << std::setfill(' ') << std::dec << std::left << "\t";
                outfile <<                                                std::setw(6) << (uns32) fwref.patchoffset                                  << "\t";
             // outfile << (fwref.patchfmt  == FMT_SIGNED    ? "sgn" : "uns") << (fwref.patchsize == 2 ? "16" : "8") << '\n';
                outfile << (fwref.patchmode == MODE_ABSOLUTE ? "abs" : "rel") << (fwref.patchsize == 2 ? "16" : "8") << '\n';
            }
        }
    }
}

// export the content of the given section to the given file
void Assembler::ExportSection( int32 s, std::ofstream& outfile )
{
    if( s != SECTION_DATA && s != SECTION_TEXT ) return;                       // not all sections should be exported
    if( symboltable.find( GetSectionName(s) ) == symboltable.end() ) return;   // sections are not exported if they are not defined
    if( locationcounter[s] == 0 ) return;                                      // sections are not exported if their size is zero

    outfile << "." << GetSectionName(s) << '\n';
    for( int32 i = 0; i < locationcounter[s]; i++ )
    {
        if( i%16 == 0 && i != 0 ) outfile << '\n';
        outfile << std::setfill('0') << std::right << std::hex << std::setw(2) << (uns32) section[s][i] << std::setfill(' ') << std::dec << std::left << " ";
    }
    if( locationcounter[s] > 0 ) outfile << '\n';
}

