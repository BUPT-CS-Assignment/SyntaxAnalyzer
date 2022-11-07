#ifndef __BASIC_H__
#define __BASIC_H__

#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <iomanip>
#include <vector>
#include <set>

#define MODE_LL1    0
#define MODE_LR1    1
#define MODE_ARITH  2

#define EXIT_ERROR(X) {std::cout << "[error] " << X << std::endl;exit(-1);}
#define LOG_INFO(X) {std::cout << "[info] "<< X << std::endl;}
#define LEFT_PRINT(X,OUT){std::cout << std::left << std::setw(X) << OUT;}
#define LEFT_COLOR_PRINT(COLOR,X,OUT){std::cout << COLOR << std::left << std::setw(X) << OUT << RESET;}

#define RESET       "\033[0m"
#define BLACK       "\033[30m"      /* Black */
#define RED         "\033[31m"      /* Red */
#define GREEN       "\033[32m"      /* Green */
#define YELLOW      "\033[33m"      /* Yellow */
#define BLUE        "\033[34m"      /* Blue */
#define MAGENTA     "\033[35m"      /* Magenta */
#define CYAN        "\033[36m"      /* Cyan */
#define WHITE       "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

static void readArgs(int argc, char** argv, char** path, int* mode){
    /* read args */
    for(int n = 1; n < argc; n++)
    {
        if(strcmp(argv[n], "-I") == 0)
        {
            if(n == argc - 1)
                EXIT_ERROR("empty filepath.")
            *path = argv[n + 1];
            n++;
        }
        else if(strcmp(argv[n], "-m") == 0)
        {
            if(n == argc - 1)
                EXIT_ERROR("empty mode type.")
            if(strcmp(argv[n + 1], "LL1") == 0)
                *mode = MODE_LL1;
            else if(strcmp(argv[n + 1], "LR1") == 0)
                *mode = MODE_LR1;
            else if(strcmp(argv[n + 1], "Arith") == 0)
                *mode = MODE_ARITH;
            else
                EXIT_ERROR("undefined mode.")
            n++;
        }
        else
            EXIT_ERROR("undefine argument " << argv[n] << ".");
    }
}

template<class T>
static inline bool __equalSet(const std::set<T>& s1, const std::set<T>& s2)
{
    if(s1.size() != s2.size())
        return false;
    for(auto it1 = s1.begin(), it2 = s2.begin(); it1 != s1.end(); it1++, it2++)
    {
        if((*it1) != (*it2))
            return false;
    }
    return true;
};

template<class T>
static inline int __containSet(const std::set<T>& check, const std::set<T>& base)
{
    if(check.size() > base.size())
        return -1;
    for(auto it : check)
        if(base.find(it) == base.end())
            return -1;

    return (check.size() == base.size()) ? 0 : 1;
};

template<class T>
static inline bool __equalList(const std::vector<T>& v1, const std::vector<T>& v2)
{
    if(v1.size() != v2.size())
        return false;
    for(int i = 0; i < v1.size(); i++)
    {
        if(v1[i] != v2[i])
            return false;
    }
    return true;
};


#endif