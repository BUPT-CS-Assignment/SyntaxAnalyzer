#include "../include/LL_LR.hpp"

Grammar::GrammarLL1::GrammarLL1(Grammar* g)
{
    G = g;
    table.resize(G->N.size());
    for(auto& it : table)
    {
        it.resize(G->T.size() + 1);
        for(auto& ele : it)
            ele = GrammarLL1::ERROR;
    }
}

bool Grammar::GrammarLL1::initTable()
{
    for(int p = 0; p < G->N.size(); p++)    // 对每个非终结符
    {
        Production* prod = &(*G)[p];
        for(int to_serial = 0; to_serial < prod->to.size(); to_serial++)
        {
            /* A -> alpha */
            /* FIRST(alpha) */
            std::set<std::string> first = G->findFirst(prod->to[to_serial].elements);
            for(auto fi : first)
            {
                if(fi == EPSILON_STR)  // FIRST集合中含有epsilon
                {
                    for(auto fo : G->FOLLOW[p]) // 对于该非终结符的每个FOLLOW符号
                    {/* 定位终结符, 将该非终结符所推出的产生式的序号填入分析表 */
                        if(fo == ENDING_STR)
                        {
                            int* num = &table[p][G->T.size()];
                            if(*num == GrammarLL1::ERROR)
                                *num = to_serial;
                            else
                                return false;
                        }
                        else
                        {
                            int t = G->_findT(fo);
                            if(t != -1 && table[p][t] == GrammarLL1::ERROR)
                                table[p][t] = to_serial;
                            else
                                return false;
                        }
                    }
                }
                else
                {
                    int t = G->_findT(fi);
                    /* locate terminal symbol */
                    if(t != -1 && table[p][t] == GrammarLL1::ERROR)    // not duplicate
                        table[p][t] = to_serial;
                    else
                        return false;
                }
            }
        }
        /* insert synch for error process */
        for(auto f : G->FOLLOW[p])
        {
            int* num;
            if(f == ENDING_STR)
                num = &table[p][G->T.size()];
            else
                num = &table[p][G->_findT(f)];
            if(*num == GrammarLL1::ERROR)
                *num = GrammarLL1::SYNCH;
        }
    }
    return true;
}


Symbol& Grammar::GrammarLL1::_nextToken()
{
    if(_stream == nullptr || _stream_pos >= (int)_stream->size())
        _cur_token = ENDING;
    else
        _cur_token = (*_stream)[++_stream_pos];
    return _cur_token;
}

Symbol& Grammar::GrammarLL1::_topState()
{
    if(_state_stack.size() == 0)
        return ENDING;
    return _state_stack[_state_stack.size() - 1];
}

void Grammar::GrammarLL1::analyze(std::vector<Symbol>& s_table)
{
    _stream = &s_table;
    _stream_pos = -1;
    _nextToken();
    while(_cur_token.attr() != Symbol::Type::END)
    {
        _state_stack.clear();
        _state_stack.emplace_back(ENDING);
        _state_stack.emplace_back(Symbol(G->start, Symbol::Type::NOT_TERMINAL));
        _analyze();
        if(_cur_token.content() == ";")
            _nextToken();
    }
}


void Grammar::GrammarLL1::_analyze()
{
    std::cout << BOLDWHITE << "[input]" << RESET << _outputStream() << std::endl;
    bool res = true;
    while(_state_stack.size() != 0)
    {
        /* terminal */
        /* message output */
        _outputPair();
        if(_topState().attr() == Symbol::Type::END)
        {
            if(_cur_token.content() != ";" && _cur_token.attr() != Symbol::Type::END)
            {
                _error();
                _nextToken();
                res = false;
            }
            else
            {
                if(res)
                    std::cout << BOLDGREEN << "ACC" << RESET;
                std::cout << std::endl;
                _state_stack.pop_back();
            }
        }
        else if(_topState().attr() != Symbol::Type::NOT_TERMINAL)
        {
            if(_topState() == _cur_token)
            {
                _state_stack.pop_back();
                _nextToken();
                std::cout << "\n";
            }
            else
            {
                _error();
                res = false;
                if(_cur_token.attr() == Symbol::Type::END){
                    break;
                }else{
                    _nextToken();
                }
            }
        }
        else
        {
            /* not terminal , change */
            int n_pos = (*G)(_topState().content());
            int t_pos = _cur_token.attr() == Symbol::Type::END ? G->T.size() :
                (_cur_token.content() == ";" ? G->T.size() : G->_findT(_cur_token.content()));
            if(t_pos == -1)
            {
                /* empty */
                _nextToken();
                _error();
                res = false;
            }
            else
            {
                int to_num = table[n_pos][t_pos];
                if(to_num == -2)
                {
                    /* synch */
                    _state_stack.pop_back();
                    std::cout << BOLDMAGENTA << "SYNCH" << RESET << std::endl;
                    //_error();
                    res = false;
                }
                else if(to_num == -1)
                {
                    /* empty */
                    if(_cur_token.attr() == Symbol::Type::END || _cur_token.content() == ";")
                    {
                        _error();
                        break;
                    }
                    _nextToken();
                    _error();
                    res = false;

                }
                else
                {
                    std::cout << (*G)[n_pos].toString(to_num) << std::endl;
                    std::vector<Symbol>* from_to = &(*G)[n_pos][to_num].elements;
                    _state_stack.pop_back();
                    for(int i = from_to->size() - 1; i >= 0; i--)
                    {
                        if((*from_to)[i].content() != EPSILON_STR)
                            _state_stack.emplace_back((*from_to)[i]);
                    }

                }
            }

        }
    }
    std::cout << BOLDWHITE << "[result]" << (res ? BOLDGREEN : BOLDRED) << (res ? " success" : " error") << RESET << std::endl;

}

void Grammar::GrammarLL1::_error()
{
    std::cout << BOLDRED << "ERROR" << RESET << std::endl;
}

void Grammar::GrammarLL1::_outputPair()
{
    LEFT_PRINT(20, _outputStack());
    LEFT_PRINT(20, _outputStream());
}

std::string Grammar::GrammarLL1::_outputStack()
{
    std::string out = "";
    for(int i = 0; i < _state_stack.size(); i++)
        out += _state_stack[i].content();
    return out;
}

std::string Grammar::GrammarLL1::_outputStream()
{
    std::string out = "";
    int p = _stream_pos;
    if(p == -1)
        return out;
    Symbol s = (*_stream)[p++];
    while(s.attr() != Symbol::Type::END && s.content() != ";")
    {
        out += s.syntaxSign();
        s = (*_stream)[p++];
    }
    out += "$";
    return out;
}

void Grammar::GrammarLL1::printTable()
{
    /* output */
    std::cout << BOLDYELLOW << "[Analyze Table]" << RESET << std::endl;

    /* --------- divide ---------*/
    std::cout << "--------";
    for(int i = 0; i <= G->T.size(); i++)
        std::cout << "------------";
    std::cout << std::endl;

    /* header */
    std::cout << "\t";
    for(auto it : G->T)
        std::cout << BOLDWHITE << std::left << std::setw(12) << it << RESET;
    std::cout << BOLDWHITE << std::setw(12) << "$" << RESET << std::endl;

    /* --------- divide ---------*/
    std::cout << "--------";
    for(int i = 0; i <= G->T.size(); i++)
        std::cout << "------------";
    std::cout << std::endl;

    /* body */
    for(int i = 0; i < G->N.size(); i++)
    {
        std::cout << BOLDWHITE << G->N[i] << "\t" << RESET;
        for(int j = 0; j <= G->T.size(); j++)
        {
            int to = table[i][j];
            if(to >= 0)
                std::cout << std::left << std::setw(12) << (*G)[i].toString(to);
            else if(to == -1)
                std::cout << std::setw(12) << "";
            else
                std::cout << std::left << std::setw(12) << "synch";
        }
        std::cout << std::endl;
    }

    /* end */
    std::cout << "--------";
    for(int i = 0; i <= G->T.size(); i++)
        std::cout << "------------";
    std::cout << std::endl;
}