#ifndef HANDLER_H 
#define HANDLER_H

#include <csignal>
#include <iostream>

inline bool killed = false;

inline
void sig_handler(int)
{
    std::cerr << "\n --- Called termination of the program, terminating calculation and closing files ---\n\n";
    killed = true;
}


#endif