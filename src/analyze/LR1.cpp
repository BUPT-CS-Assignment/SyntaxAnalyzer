#include "../../include/LL_LR.hpp"

Grammar::GrammarLR1::Item::Item(GrammarLR1* lr, int serial)
{
    _LR1 = lr;
    _serial = serial;
}

Grammar::GrammarLR1::Item::Item(const Grammar::GrammarLR1::Item& item)
{
    _LR1 = item._LR1;
    _serial = item._serial;
    _nextTo = std::vector<int>(item._nextTo);
    _nextRead = std::vector<Symbol>(item._nextRead);
    _productions = std::vector<Production>(item._productions);
}

Grammar::GrammarLR1::Item& Grammar::GrammarLR1::Item::operator=(const Grammar::GrammarLR1::Item& item)
{
    _LR1 = item._LR1;
    _serial = item._serial;
    _nextTo = item._nextTo;
    _nextRead = item._nextRead;
    _productions = item._productions;
    return *this;
}

int Grammar::GrammarLR1::Item::_find(Production& p)
{
    for(int i = 0; i < _productions.size(); i++)
    {
        Production* base = &_productions[i];
        if(base->from == p.from && (*base)[0].getDot() == p[0].getDot())
        {
            if(__equalList((*base)[0].elements, p[0].elements))
            {
                int res = __containSet(p[0].ahead, (*base)[0].ahead);
                if(res == 0)
                    return ALL_EQUAL;
                else if(res == 1)
                    return AHEAD_CONTAIN;
                else
                    return i;
            }
        }
    }
    return NOT_FOUND;  // not found
}

/* insert new production */
int Grammar::GrammarLR1::Item::insert(Production& p)
{
    if(p.to.size() != 1)
        return -1;
    int pos = _find(p);

    if(pos == ALL_EQUAL || pos == AHEAD_CONTAIN)
        return 0;
    if(pos == NOT_FOUND)
        _productions.emplace_back(p);
    else
        _productions[pos][0].ahead.insert(p[0].ahead.begin(), p[0].ahead.end());

    return 1;
}

/* compute ahead symbols for new production */
void Grammar::GrammarLR1::Item::_computeAhead(Grammar::Production& cur, Grammar::Production& base)
{
    Grammar* g = _LR1->_G;
    //cur[0].ahead.clear();

    std::vector<Symbol> ahead_list = base[0].getAheadStream();
    if(ahead_list.size() == 0)
        cur[0].ahead = std::set<std::string>(base[0].ahead);
    else
    {
        std::set<std::string> end = g->findFirst(ahead_list);
        for(auto& it : end)
        {
            if(it == EPSILON_STR)   // count epsilon , copy from origin
                cur[0].ahead.insert(base[0].ahead.begin(), base[0].ahead.end());
            else
                cur[0].ahead.insert(it);
        }
    }
}


/* compute closure */
void Grammar::GrammarLR1::Item::closure()
{
    bool run_flag = true;
    while(run_flag)
    {
        run_flag = false;
        for(int i = 0; i < _productions.size(); i++)
        {
            Production* p = &_productions[i];
            if((*p)[0].state == Production::To::State::REDUCTION_EXPECTING)
            {
                Symbol s = (*p)[0].getDotSymbol();
                /* Add new State */
                for(auto& t : (*(_LR1->_G))[s.content()].to)
                {

                    Production temp(s.content());
                    temp.insert(t);

                    /* compute ahead symbol */
                    _computeAhead(temp, _productions[i]);

                    /* try insert */
                    int res = insert(temp);

                    if(res == -1)
                        EXIT_ERROR("closure compute error.")
                    else if(res == 1)
                    {
                        run_flag = true;
                    }
                }
            }
        }
    }
}

bool Grammar::GrammarLR1::Item::operator==(const Item& item) const
{
    if(_productions.size() != item._productions.size())
        return false;

    for(auto& it : _productions)
    {
        bool find = false;
        for(auto& p : item._productions)
            if(p == it)
            {
                find = true;
                break;
            }
        if(!find)
            return false;
    }
    return true;
}

bool Grammar::GrammarLR1::Item::operator!=(const Item& item) const
{
    return !((*this) == item);
}

void Grammar::GrammarLR1::Item::clear()
{
    _nextTo.clear();
    _nextRead.clear();
    _productions.clear();
}


void Grammar::GrammarLR1::Item::next()
{
    /* find all REDUCTION_EXPECTING */
    for(auto& it : _productions)
    {
        if(it[0].state != Production::To::State::REDUCTION)
        {
            bool found = false;
            Symbol s = it[0].getDotSymbol();
            for(auto& ele : _nextRead)
            {
                if(ele == s)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                _nextRead.emplace_back(s);
        }
    }
    _nextTo.resize(_nextRead.size());

    /* find all next item */
    Item new_item(_LR1);
    for(int i = 0; i < _nextRead.size(); i++)
    {
        new_item.clear();
        // find all reducing by current symbol
        for(auto& temp_p : _productions)
            if(temp_p[0].getDotSymbol() == _nextRead[i])
                new_item.insert(temp_p);

        for(auto& temp_p : new_item._productions)
            temp_p[0].next();

        // generate closure
        new_item.closure();

        // find equal item
        int pos = _LR1->_findItem(new_item);
        if(pos == -1)
        {
            pos = _LR1->_colletion.size();
            new_item._serial = pos;
            Item* insert_item = new Item(new_item);
            _LR1->_colletion.emplace_back(insert_item);
        }
        // add pointer for current item
        _nextTo[i] = pos;
    }
}

std::string Grammar::GrammarLR1::Item::toString()
{
    std::string out = "";
    out += BOLDBLUE"[I]" + std::to_string(_serial) + RESET + "\n";

    for(auto& it : _productions)
        out += it.toStringLR1() + "\n";

    if(_nextRead.size() == 0)
        return out;
    out += "\n";
    for(int i = 0; i < _nextRead.size(); i++)
    {
        out += "read:" + _nextRead[i].content() + "\t";
        out += "  next:[I]" + std::to_string(_nextTo[i]);
        out += "\n";
    }

    return out;
}

Grammar::Production& Grammar::GrammarLR1::Item::operator[](int pos)
{
    pos = pos < 0 ? _productions.size() + pos : pos;
    return _productions[pos];
}

Grammar::GrammarLR1::GrammarLR1(Grammar* g)
{
    _G = g;
    _colletion.clear();
}

int Grammar::GrammarLR1::_findItem(Grammar::GrammarLR1::Item& item)
{
    for(int i = 0; i < _colletion.size(); i++)
    {
        if(*_colletion[i] == item)
            return i;
    }
    return -1;
}

/* divide grammar into singles */
void Grammar::GrammarLR1::init()
{
    if(_G == nullptr)
        return;
    for(auto& from : _G->P)
    {
        for(auto& to : from.to)
        {
            Production p(from.from);
            p.insert(to);
            this->P.emplace_back(p);
        }
    }
}

int Grammar::GrammarLR1::_findProduction(Grammar::Production& p)
{
    if(p.to.size() != 1)
        return -1;
    // matching
    for(int i = 0; i < P.size(); i++)
    {
        if(__equalList(P[i][0].elements, p[0].elements))
            return i;
    }

    return -1;
}

void Grammar::GrammarLR1::generateDFA()
{
    /* start item setting */
    Item* start_item = new Item(this, 0);
    start_item->insert((*_G)[_G->start]);// insert start production into start_item
    start_item->_productions[0][0].ahead.insert(ENDING_STR);

    start_item->closure();           // calculate closure
    _colletion.emplace_back(start_item);

    /* generate DFA and Table */
    for(int i = 0; i < _colletion.size(); i++)
    {
        _colletion[i]->next();
    }
}


bool Grammar::GrammarLR1::initTable()
{
    int item_size = _colletion.size();
    /* init action table */
    action_table.resize(item_size);
    for(auto& it : action_table)
        it.resize(_G->T.size() + 1, std::pair<ActionType, int>(ActionType::ERROR, -1));

    /* init goto table */
    goto_table.resize(item_size);
    for(auto& it : goto_table)
        it.resize(_G->N.size(), -1);

    /* generate action & goto table */
    for(int i = 0; i < _colletion.size(); i++)
    {
        Item* item = _colletion[i];
        // compute SHIFT & GOTO
        for(int j = 0; j < item->_nextRead.size(); j++)
        {
            Symbol* s = &item->_nextRead[j];
            int to_num = item->_nextTo[j];
            // not-terminal symbol, add to goto table
            if(s->attr() == Symbol::Type::NOT_TERMINAL)
            {
                int pos = (*_G)(*s);
                goto_table[i][pos] = to_num;
            }
            else
            {
                // teminal symbol, add to action table
                int pos = _G->_findT(s->content());
                action_table[i][pos].first = ActionType::SHIFT;
                action_table[i][pos].second = to_num;
            }
        }

        // compute reduction
        for(auto& p : item->_productions)
        {
            if(p[0].state == Production::To::State::REDUCTION)
            {
                int p_num = _findProduction(p);
                for(auto& str : p[0].ahead)
                {
                    int pos = str == ENDING_STR ? _G->T.size() : _G->_findT(str);

                    // check conflict
                    if(action_table[i][pos].first != ActionType::ERROR)
                    {
                        action_table.clear();
                        goto_table.clear();
                        return false;
                    }

                    if(str == ENDING_STR && p.from == _G->start)
                        action_table[i][pos].first = ActionType::ACCEPT;
                    else
                    {
                        action_table[i][pos].first = ActionType::REDUCTION;
                        action_table[i][pos].second = p_num;
                    }
                }
            }
        }
    }
    return true;
}


void Grammar::GrammarLR1::printDFA()
{
    /* output */
    std::cout << BOLDYELLOW << "[DFA]" << RESET << std::endl;

    for(auto& item : _colletion)
    {
        std::cout << item->toString() << std::endl;
    }
}


void Grammar::GrammarLR1::printTable()
{
    /* output */
    std::cout << BOLDYELLOW << "[Analyze Table]" << RESET << std::endl;

    /* --------- divide ---------*/
    std::cout << "-----";
    for(int i = 0; i < _G->T.size() + _G->N.size(); i++)
        std::cout << "-----";
    std::cout << std::endl;

    LEFT_PRINT(3, "");
    std::cout << "| ";
    LEFT_COLOR_PRINT(BOLDWHITE, (_G->T.size() + 1) * 5, "action");
    std::cout << "| ";
    std::cout << BOLDWHITE << "goto" << RESET << std::endl;

    /* header */
    LEFT_PRINT(3, "");
    std::cout << "| ";
    for(auto& it : _G->T)
        LEFT_COLOR_PRINT(BOLDCYAN, 5, it);
    LEFT_COLOR_PRINT(BOLDCYAN, 5, "$");
    std::cout << "| ";
    for(int i = 1; i < _G->N.size(); i++)
        LEFT_COLOR_PRINT(BOLDBLUE, 5, _G->N[i]);
    std::cout << std::endl;

    /* --------- divide ---------*/
    std::cout << "---+-";
    for(int i = 0; i < _G->T.size() + _G->N.size(); i++)
        std::cout << "-----";
    std::cout << std::endl;

    /* body */
    for(int i = 0; i < _colletion.size(); i++)
    {
        LEFT_COLOR_PRINT(BOLDWHITE, 3, i);
        std::cout << "| ";
        for(int j = 0; j <= _G->T.size(); j++)
        {
            if(action_table[i][j].first == ActionType::ERROR)
            {
                LEFT_PRINT(5, "");
            }
            else if(action_table[i][j].first == ActionType::ACCEPT)
            {
                LEFT_PRINT(5, "ACC");
            }
            else
            {
                LEFT_PRINT(5, (action_table[i][j].first == ActionType::SHIFT ? "S" : "R") + std::to_string(action_table[i][j].second))
            }
        }
        std::cout << "| ";
        for(int j = 1; j < _G->N.size(); j++)
        {
            if(goto_table[i][j] == -1)
            {
                LEFT_PRINT(5, "");
            }
            else
            {
                LEFT_PRINT(5, goto_table[i][j]);
            }
        }
        std::cout << std::endl;
    }

    /* ending */
    std::cout << "-----";
    for(int i = 0; i < _G->T.size() + _G->N.size(); i++)
        std::cout << "-----";
    std::cout << std::endl;
}


Symbol& Grammar::GrammarLR1::_nextToken()
{
    // if(_stream == nullptr || _stream_pos >= (int)_stream->size())
    //     _cur_token = _next_token = ENDING;
    // else
    // {
    //     _cur_token = (*_stream)[++_stream_pos];
    //     if(_stream_pos >= (int)_stream->size())
    //         _next_token = ENDING;
    //     else if((*_stream)[_stream_pos].content() == ";")
    //         _next_token = ENDING;
    //     else
    //         _next_token = (*_stream)[_stream_pos];
    // }
    // return _cur_token;

    if(_stream == nullptr || _stream_pos >= (int)_stream->size())
        _cur_token = ENDING;
    else
        _cur_token = (*_stream)[++_stream_pos];
    return _cur_token;
}

Symbol& Grammar::GrammarLR1::_topSymbol()
{
    if(_symbol_stack.size() == 0)
        return ENDING;
    return _symbol_stack[_symbol_stack.size() - 1];
}

int Grammar::GrammarLR1::_topState()
{
    if(_state_stack.size() == 0)
        return -1;
    return _state_stack[_state_stack.size() - 1];
}

std::string Grammar::GrammarLR1::_outputStack()
{
    std::string out = "States:";
    for(int i = 0; i < _state_stack.size(); i++)
    {
        out += std::to_string(_state_stack[i])
            + (_state_stack[i] > 10 ? "  " : "   ");
    }
    out += "\nSymbol:";
    for(int i = 0; i < _symbol_stack.size(); i++)
    {
        // out += "[" + std::to_string(_state_stack[i])
        //     + "," + _symbol_stack[i].content() + "]";
        out += _symbol_stack[i].content();
        for(int j = 0; j < 4 - _symbol_stack[i].content().length(); j++)
            out += " ";
    }
    return out;
}

std::string Grammar::GrammarLR1::_outputStream()
{
    std::string out = "";
    int p = _stream_pos;
    if(p == -1)
        return out;
    Symbol* s = &(*_stream)[p++];
    while(s->attr() != Symbol::Type::END && s->content() != ";")
    {
        out += s->syntaxSign();
        s = &(*_stream)[p++];
    }
    out += "$";
    return out;
}

void Grammar::GrammarLR1::_outputPair()
{
    std::cout << _outputStack() << std::endl;
    std::cout << "Stream:";
    LEFT_PRINT(20, _outputStream());
}

void Grammar::GrammarLR1::analyze(std::vector<Symbol>& s_table)
{
    _stream = &s_table;
    _stream_pos = -1;
    _nextToken();
    while(_cur_token.attr() != Symbol::Type::END)
    {
        _state_stack.clear();
        _symbol_stack.clear();
        _state_stack.emplace_back(0);
        _symbol_stack.emplace_back(Symbol("-", Symbol::Type::END));
        _analyze();
        if(_cur_token.content() == ";")
            _nextToken();
    }
}

void Grammar::GrammarLR1::_analyze()
{
    std::cout << BOLDWHITE << "[input]" << RESET << _outputStream() << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    while(true)
    {
        _outputPair();
        int pos = _cur_token.attr() == Symbol::Type::END ? _G->T.size() : 
                                    ( _cur_token.content() == ";" ? _G->T.size() : _G->_findT(_cur_token.content()));
        if(pos == -1){
            Symbol error_token = _cur_token;
            _error();
            std::cout << BOLDRED << "[error] " << RESET << "unknown symbol '" << error_token.content() << "'" << std::endl;
            break;
        }
        std::pair<ActionType, int> res = action_table[_topState()][pos];
        if(res.first == ActionType::ERROR)
        {
            // error process
            _error();
            break;
        }

        if(res.first == ActionType::ACCEPT)
        {
            std::cout << BOLDGREEN << " ACC" << RESET << std::endl;
            std::cout << "--------------------------------------------------" << std::endl;
            break;
        }
        else if(res.first == ActionType::SHIFT)
        {
            std::cout << BOLDWHITE << " shift " << res.second <<RESET << std::endl;
            _symbol_stack.emplace_back(_cur_token);
            _state_stack.emplace_back(res.second);
            _nextToken();
        }
        else if(res.first == ActionType::REDUCTION)
        {
            std::cout << BOLDWHITE << " reduce by " + P[res.second].toString() << RESET << std::endl;
            Production* p = &P[res.second];
            // pop reduction
            for(int i = 0; i < p->to[0].elements.size(); i++)
            {
                _state_stack.pop_back();
                _symbol_stack.pop_back();
            }
            // push reduce-by
            _symbol_stack.push_back(Symbol(p->from, Symbol::Type::NOT_TERMINAL));
            int pos_n = (*_G)(p->from);
            int goto_num = goto_table[_topState()][pos_n];
            if(goto_num == -1)
            {
                _error();
                break;
            }
            // push back new state;;
            _state_stack.emplace_back(goto_num);
        }
        std::cout << "--------------------------------------------------" << std::endl;
    }
}

void Grammar::GrammarLR1::_error(){
    std::cout << BOLDRED << "ERROR" << RESET << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    while(_cur_token.attr() != Symbol::Type::END && _cur_token.content() != ";")
        _nextToken();
    
}