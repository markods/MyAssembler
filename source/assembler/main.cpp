// _____________________________________________________________________________________________________________________________________________
// MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...M
// _____________________________________________________________________________________________________________________________________________
// MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...M
// _____________________________________________________________________________________________________________________________________________
// MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...M

#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include "assembler.h"

int main( int argc, char* argv[] )
{
    std::unique_ptr<Assembler> assembler { new Assembler() };

    // check if the number of command line arguments is invalid
    if( argc != 4 )
       { std::cout << "ERROR - missing command line argument\n"; return -1; }

    std::string argv1       = argv[1];
    std::string outfilename = argv[2];
    std::string infilename  = argv[3];

    // check the first command line argument
    if( argv1 != "-o" )
       { std::cout << "ERROR - wrong command line argument '" << argv1 << '\n'; return -1; }

    // open output file
    std::ofstream outfile(outfilename);
    if( !outfile.is_open() )
        { std::cout << "ERROR - output file '" << outfilename << "' could not be opened\n"; return -1; }

    // open input file
    std::ifstream infile(infilename);
    if( !infile.is_open() )
        { std::cout << "ERROR - input file '" << infilename << "' could not be opened\n"; return -1; }

    std::cout << "=================================================\n";
    std::cout << "   assembly started\n";
    std::cout << "=================================================\n";
    std::cout << "    input file = '" << infilename << '\n';
    std::cout << "   output file = '" << outfilename << '\n';
    std::cout << "-------------------------------------------------\n";
    std::cout << '\n';

    // read lines from input file
    std::string line;
    while( std::getline(infile, line) )
    {
        if( !assembler->ParseCmd(line) )
            return -1;  // parsing was not successful (cmd has incorrect syntax)

        // do not process any further lines if the end line is hit
        if( assembler->GetCmd() == ".end" )
            { std::cout << "NOTE - found '.end' directive - further lines will be ignored\n"; break; }
    }

    // if the assembler unsuccessfully finished
    if( !assembler->Finish( outfile ) )
        return -1;

    std::cout << '\n';
    std::cout << "=================================================\n";
    std::cout << "   assembly successfully finished\n";
    std::cout << "=================================================\n";
}

