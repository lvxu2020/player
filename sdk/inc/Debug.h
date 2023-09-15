#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <iostream>


#define DEBUG_POS std::cout << "-->" << __FILE__<< ":" << __LINE__ << " [" << __FUNCTION__ << "]" << std::endl;


#define DEBUG_I(format, ...) { printf("\033[32mDEBUG_I: \033[0m"#format, ##__VA_ARGS__); DEBUG_POS }
#define DEBUG_E(format, ...) { printf("\033[31mDEBUG_E: \033[0m"#format, ##__VA_ARGS__); DEBUG_POS }


#endif // DEBUG_H
