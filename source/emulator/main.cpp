// _____________________________________________________________________________________________________________________________________________
// MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...M
// _____________________________________________________________________________________________________________________________________________
// MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...M
// _____________________________________________________________________________________________________________________________________________
// MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...MAIN...M

#include <memory>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include "emulator.h"

// set the desired file descriptor attributes, get the old attributes
int setattr(int fileno, termios& attr_old)
{
    termios attr_new;

    if( tcgetattr( fileno, &attr_old ) != 0 ) { std::cout << "failed to copy cin attributes\n"; return -1; }
    attr_new = attr_old;

    attr_new.c_lflag &= ~(ECHO|ECHONL|ICANON);
    attr_new.c_cc[VMIN ] = 0;
    attr_new.c_cc[VTIME] = 0;

    if( tcsetattr( fileno, TCSANOW, &attr_new ) != 0 ) { std::cout << "failed to adjust cin attributes\n"; return -1; }
    return 0;
}

// restore the file descriptor attributes
int resattr(int fileno, termios& attr_old)
{
    if( tcsetattr( fileno, TCSANOW, &attr_old ) != 0 ) { std::cout << "failed to adjust cin attributes\n"; return -1; }
    return 0;
}

// empty input stream
void empty(int fileno)
{
    char c;
    while( read(fileno, &c, 1) > 0 );
}


int main( int argc, char* argv[] )
{
    // initialize the emulator
    std::unique_ptr<Emulator> emulator { new Emulator() };
    if( ! emulator->Init( argc, argv ) ) { std::cout << "failed to initialize emulator\n"; return -1; }
    emulator->PrintHw();

    // previous cin attributes
    termios attr_old;
    try
    {
        // set the desired cin attributes
        if( setattr(STDIN_FILENO, attr_old) < 0 ) throw -1;

        // start the emulation
        emulator->StartEmulation();
        emulator->PrintHw();

        // reset the cin attributes to their previous values
        empty(STDIN_FILENO);
        if( resattr(STDIN_FILENO, attr_old) < 0 ) throw -1;
    }
    catch( ... )
    {
        // reset the cin attributes to their previous values
        empty(STDIN_FILENO);
        resattr(STDIN_FILENO, attr_old);

        return -1;
    }

}

