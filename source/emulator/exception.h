// _____________________________________________________________________________________________________________________________________________
// EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION
// _____________________________________________________________________________________________________________________________________________
// EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION
// _____________________________________________________________________________________________________________________________________________
// EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION...EXCEPTION

#pragma once
#include <stdexcept>


// logic error
using LogicError = std::logic_error;
// runtime error
using RuntimeError = std::runtime_error;

// out of range error (logic error)
using RangeError = std::out_of_range;



// base class for various marshalling and unmarshalling related exceptions
class MarshallError : public std::logic_error
{
    using std::logic_error::logic_error;
};

// class that represents a parse exception
class ParseError : public MarshallError
{
    using MarshallError::MarshallError;
};

// class that represents an instruction/operand serialization exception
class SerialError : public MarshallError
{
    using MarshallError::MarshallError;
};



    // class that represents an access rights violation
    class AccessError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };


    
    // base class for various instruction cycle related exceptions
    class InstrCycleError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    // class that represents an instruction/operand fetch exception
    class FetchError : public InstrCycleError
    {
        using InstrCycleError::InstrCycleError;
    };

    // class that represents a instruction execution exception
    class ExecError : public InstrCycleError
    {
        using InstrCycleError::InstrCycleError;
    };

    // class that represents an operand value store exception
    class StoreError : public InstrCycleError
    {
        using InstrCycleError::InstrCycleError;
    };

    // class that represents an interrupt handle exception
    class InterError : public InstrCycleError
    {
        using InstrCycleError::InstrCycleError;
    };



