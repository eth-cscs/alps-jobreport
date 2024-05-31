#ifndef JOBREPORT_ERROR_HPP
#define JOBREPORT_ERROR_HPP

#include <string>
#include <iostream>
#include <cstdlib>

// Debugging macro
#ifdef JOBREPORT_DEBUG
    #define DEBUG_LOG(x) std::cerr << "DEBUG: " << x << std::endl;
#else
    #define DEBUG_LOG(x)
#endif


void raise_error(const std::string &msg)
{
    std::cerr << msg << std::endl;
    std::exit(EXIT_FAILURE);
}

#endif // JOBREPORT_ERROR_HPP
