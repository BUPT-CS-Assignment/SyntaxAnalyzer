#include "../../include/grammar.hpp"


void Grammar::init(const char* file_name)
{
    _sc.open(file_name);
    Symbol s(&_sc);
    ++s;
    if(s.content() == "#" && (++s).content() == "S")
        start = (++s).content();
    else
    {
        std::cout << s.content() << std::endl;
        EXIT_ERROR("invalid input");
    }


    if((++s).content() == "#" && (++s).content() == "N")
    {
        while(s.attr() != Symbol::Type::END && (++s).content() != "#")
        {
            if(s.attr() == Symbol::Type::ERROR)
                EXIT_ERROR("invalid terminal symbol");
            N.emplace_back(s.content());
        }
    }
    else
    {
        std::cout << s.content() << std::endl;
        EXIT_ERROR("invalid input");
    }


    if((++s).content() == "T")
    {
        while(s.attr() != Symbol::Type::END && (++s).content() != "#")
        {
            if(s.attr() == Symbol::Type::ERROR)
                EXIT_ERROR("invalid terminal symbol");
            T.emplace_back(s.content());
        }
    }

    bool in = false;
    for(auto it : this->N)
        if(it == start)
        {
            in = true;
            break;
        }
    if(!in)
        EXIT_ERROR("start symbol not found in not_terminal list");
    P.clear();
    P.resize(N.size());



    if((++s).content() != "P")
    {
        std::cout << s.content() << std::endl;
        EXIT_ERROR("invalid input");
    }
    /* Production */

    while(s.attr() != Symbol::Type::END)
    {
        while(s.attr() != Symbol::Type::END && s.content() != PRODUCTION_BEGIN)
            ++s;
        std::string from = (++s).content();
        if((++s).content() != "->")
        {
            std::cout << s.content() << std::endl;
            EXIT_ERROR("invalid production");
        }
        while(s.attr() != Symbol::Type::END && s.content() != PRODUCTION_BEGIN)
        {
            ++s;
            std::vector<Symbol> temp_to;
            while(s.attr() != Symbol::Type::END && s.content() != PRODUCTION_DIVIDE && s.content() != PRODUCTION_BEGIN)
            {
                s.syntaxReset();
                if(_findT(s.content()) != -1){
                    if(s.attr() == Symbol::Type::NOT_TERMINAL)
                        s.setAttr(Symbol::Type::IDENTIFIER);
                }
                temp_to.emplace_back(s);
                ++s;
            }
            insert(from, temp_to);
        }
    }

    _sc.close();

    std::cout << BOLDYELLOW << "[origin]" << RESET << std::endl;
    std::cout << BOLDWHITE << "[N] " << RESET;
    for(auto it : N)
        std::cout << it << " ";
    std::cout << BOLDWHITE << "\n[T] " << RESET;
    for(auto it : T)
        std::cout << it << " ";
    std::cout << std::endl;
    for(auto it : P)
    {
        std::cout << it.toString() << std::endl;
    }
}