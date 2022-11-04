#include "../../include/grammar.hpp"
#include "../include/LL_LR.hpp"
#include <string>
static Grammar::Production NULL_PRODUCTION = Grammar::Production("");

/* class Production */

Grammar::Grammar()
{
    _sc.setReport(false);
}


Grammar::Grammar(std::string start, std::vector<std::string> N, std::vector<std::string> T)
{
    if(start.length() == 0)
        EXIT_ERROR("empty start symbol");
    this->start = start;
    this->N = N;
    this->T = T;
    _sc.setReport(false);
    bool in = false;
    for(auto it : this->N)
        if(it == start)
        {
            in = true;
            break;
        }
    if(!in)
        EXIT_ERROR("start symbol not found in N list");
    P.clear();
    P.resize(N.size());
}



Grammar::Production& Grammar::operator[](std::string from_str)
{
    for(auto& it : P)
    {
        if(it.from == from_str)
            return it;
    }
    return NULL_PRODUCTION;
}

Grammar::Production& Grammar::operator[](int pos)
{
    pos = pos < 0 ? pos + P.size() : pos;
    return P[pos];
}

int Grammar::operator()(std::string from_str)
{
    return _findN(from_str);
}

int Grammar::operator()(Symbol& s)
{
    if(s.attr() == Symbol::Type::NOT_TERMINAL)
        return _findN(s.content());
    else
        return _findT(s.content());
}

bool Grammar::setStart(std::string start_str)
{
    if(start == start_str)
        return true;
    for(auto it : N)
    {
        if(it == start_str)
        {
            start = start_str;
            return true;
        }
    }
    return false;
}

bool Grammar::addN(std::string new_n)
{
    for(auto it : N)
    {
        if(it == new_n)
            return false;
    }
    N.insert(N.begin(), new_n);
    P.insert(P.begin(), Production(new_n));
    return true;
}


void Grammar::insert(std::string from_str, std::vector<Symbol> s)
{
    int p = _findN(from_str);
    if(p == -1)
        addN(from_str);
    else
        P[p].from = from_str;
    P[p == -1 ? 0 : p].insert(s);
}

void Grammar::insert(std::string from_str, std::vector<std::string> to_vec)
{
    int p = (*this)(from_str);
    if(p == -1)
        addN(from_str);
    else
        P[p].from = from_str;
    P[p == -1 ? 0 : p].insert(to_vec);
}

void Grammar::insert(Grammar::Production& p)
{
    int pos = _findN(p.from);
    if(pos == -1)
        addN(p.from);
    for(auto& it : p.to)
        P[pos == -1 ? 0 : pos].insert(it.elements);
}


int Grammar::_findN(std::string n)
{
    for(int i = 0; i < N.size(); i++)
        if(n == N[i])
            return i;
    return -1;
}

int Grammar::_findT(std::string t)
{
    for(int i = 0; i < T.size(); i++)
        if(t == T[i])
            return i;
    return -1;
}

static std::set<std::string> EMPTY_STRING_SET;

std::set<std::string>& Grammar::findFirst(std::string N)
{
    int pos = _findN(N);
    if(pos == -1)
        return EMPTY_STRING_SET;
    return FIRST[pos];
}

std::set<std::string>& Grammar::findFollow(std::string N)
{
    int pos = _findN(N);
    if(pos == -1)
        return EMPTY_STRING_SET;
    return FOLLOW[pos];
}

bool Grammar::findFirst(std::string N, std::string c)
{
    std::set<std::string> temp = findFirst(N);
    return temp.find(c) != temp.end();
}

bool Grammar::findFollow(std::string N, std::string c)
{
    std::set<std::string> temp = findFollow(N);
    return temp.find(c) != temp.end();
}

std::set<std::string> Grammar::findFirst(std::vector<Symbol> to)
{
    std::set<std::string> first;
    if(to.size() == 0)
        return first;
    if(to.size() == 1 && to[0].attr() == Symbol::Type::EPSILON)
    {
        first.insert(EPSILON_STR);
        return first;
    }
    bool epsilon_flag = true;
    for(auto it : to)
    {
        /* terminal */
        if(it.attr() != Symbol::Type::NOT_TERMINAL)
        {
            first.insert(it.content());
            epsilon_flag = false;
            break;
        }
        else
        {
            /* not terminal */
            int pos = _findN(it.content());
            bool has_epsilon = false;
            for(auto ele : FIRST[pos])
            {
                if(ele != EPSILON_STR)
                    first.insert(ele);
                else
                    has_epsilon = true;
            }
            if(!has_epsilon)
            {
                epsilon_flag = false;
                break;
            }
        }
    }
    if(epsilon_flag)
        first.insert(EPSILON_STR);
    return first;
}


void Grammar::computeFirstTable()
{
    FIRST.clear();
    FIRST.resize(N.size());
    bool run_flag = true;
    while(run_flag)
    {
        run_flag = false;
        for(int i = 0; i < N.size(); i++)
        {
            std::string from = N[i];
            std::set<std::string>* first = &FIRST[i];
            /* for all production from N[i] */
            int pre_size = first->size();
            /* p : FROM -> a|X|Y */
            for(auto& it : P[i].to)
            {
                /* it : X[bZ] */
                bool epsilon_flag;
                for(auto& temp : it.elements)
                {
                    /* temp : Z */
                    /* meet not_teminal */
                    if(temp.attr() != Symbol::Type::NOT_TERMINAL)
                    {
                        first->insert(temp.content());
                        break;
                    }
                    epsilon_flag = false;
                    std::set<std::string>* set_temp = &FIRST[_findN(temp.content())];
                    /* not include epsilon */
                    for(auto eles : (*set_temp))
                        if(eles != EPSILON_STR)
                            first->insert(eles);
                        else
                            epsilon_flag = true;
                    if(!epsilon_flag)
                        break;
                }
                if(epsilon_flag)
                    first->insert(EPSILON_STR);
            }
            if(first->size() > pre_size)   // stop test
                run_flag = true;
        }
    }
}

void Grammar::computeFollowTable()
{
    FOLLOW.clear();
    FOLLOW.resize(N.size());
    FOLLOW[_findN(start)].insert(ENDING_STR);

    bool run_flag = true;
    while(run_flag)
    {
        run_flag = false;
        for(int i = 0; i < FOLLOW.size(); i++)
        {
            std::string from = N[i];
            std::set<std::string>* follow = &FOLLOW[i];
            /* for all production from N[i] */
            for(auto& it : P[i].to)
            {
                std::vector<Symbol> ele = it.elements;
                for(int j = 0; j < ele.size(); j++)
                {
                    /* terminal symbol, skeep */
                    if(ele[j].attr() != Symbol::Type::NOT_TERMINAL)
                        continue;

                    /* not terminal, compute follow for it */
                    int pos = _findN(ele[j].content());
                    int pre_size = FOLLOW[pos].size();

                    bool end_flag = true;
                    for(int k = j + 1; k < ele.size(); k++)
                    {
                        if(ele[k].attr() != Symbol::Type::NOT_TERMINAL)
                        {
                            FOLLOW[pos].insert(ele[k].content());
                            end_flag = false;
                            break;
                        }
                        bool epsilon_flag = false;
                        std::set<std::string> first_temp = findFirst(ele[k].content());
                        for(auto& it_str : first_temp)
                        {
                            if(it_str == EPSILON_STR)
                                epsilon_flag = true;
                            else
                                FOLLOW[pos].insert(it_str);
                        }
                        if(!epsilon_flag)
                        {
                            end_flag = false;
                            break;
                        }
                    }
                    if(end_flag)
                        FOLLOW[pos].insert(follow->begin(), follow->end());

                    if(FOLLOW[pos].size() > pre_size)
                        run_flag = true;
                }
            }
        }
    }
}


void Grammar::eliminateLeftRecursion()
{
    bool run_flag = true;
    while(run_flag)
    {
        run_flag = false;
        for(auto it = P.begin(); it != P.end(); it++)
        {
            if((*it).existLeftRecursion())
            {
                Production p = (*it).eliminateLeftRecursion();
                insert(p);
                run_flag = true;
                break;
            }

        }
    }
}

void Grammar::extractLeftCommonFactor()
{
    bool run_flag = true;
    while(run_flag){
        run_flag = false;
        for(int i = 0; i < P.size(); i++){
            std::set<Symbol> lefts = P[i].existLeftCommonFactor();
            if(lefts.size() == 0)
                continue;
            // exist left common factor
            run_flag = true;
            std::string from = P[i].from;
            for(auto& it:lefts){
                //for all common factors
                from += "`";
                Production p(from);
                Symbol s(from,Symbol::Type::NOT_TERMINAL);
                // find all left-common productions
                for(auto it1 = P[i].to.begin(); it1 != P[i].to.end();){
                    if((*it1)[0] == it){
                        (*it1).elements.erase((*it1).elements.begin());
                        if((*it1).elements.size() == 0)
                            (*it1).elements.push_back(EPSILON);
                        p.insert((*it1));
                        it1 = P[i].to.erase(it1);
                    }else
                        it1++;
                }
                // find duplicated from->to
                bool exist = false;
                for(auto& ps:P){
                    if(ps.to == p.to){
                        s.setContent(ps.from);
                        exist = false;
                        break;
                    }
                }
                P[i].insert({it,s});
                // insert new not_terminal symbol and new production
                if(!exist){
                    this->N.emplace_back(p.from);
                    this->P.emplace_back(p);
                }
            }
        }
    }
}

void Grammar::expandGrammar()
{
    if((*this)[start].to.size() == 1)
        return;

    /* new start state */
    std::string new_start = start + "'";
    Production new_p(new_start);
    new_p.insert({Symbol(start,Symbol::Type::NOT_TERMINAL)});
    insert(new_p);
    start = new_start;
}

void Grammar::_printSet(std::vector<std::set<std::string>>* list)
{
    for(int i = 0; i < list->size(); i++)
    {
        std::cout << BOLDWHITE << std::left << std::setw(6) << N[i];
        std::cout << "|" << RESET;
        for(auto it : (*list)[i])
            std::cout << " " << it;
        std::cout << std::endl;
    }
}

void Grammar::printFirst()
{
    std::cout << BOLDYELLOW << "[FIRST]" << RESET << std::endl;
    _printSet(&FIRST);
}

void Grammar::printFollow()
{
    std::cout << BOLDYELLOW << "[FOLLOW]" << RESET << std::endl;
    _printSet(&FOLLOW);
}

void Grammar::printGrammar()
{
    std::cout << BOLDWHITE << "[N] " << RESET;
    for(auto it : N)
        std::cout << it << " ";
    std::cout << BOLDWHITE << "\n[T] " << RESET;
    for(auto it : T)
        std::cout << it << " ";
    std::cout << std::endl;
    for(auto it : P)
        std::cout << it.toString() << std::endl;
}

void Grammar::runLL1()
{

    eliminateLeftRecursion();
    extractLeftCommonFactor();
    std::cout << BOLDYELLOW << "[conversed]" << RESET << std::endl;
    printGrammar();
    computeFirstTable();
    computeFollowTable();
    printFirst();
    printFollow();

    _mode_ll1 = new GrammarLL1(this);
    if(!_mode_ll1->initTable())
        EXIT_ERROR("not a LL1 grammar.")
        _mode_ll1->printTable();

    Scanner sc;
    std::string input;
    while(true)
    {
        std::cout << "Symbol String > ";
        std::getline(std::cin, input);
        if(input == ".exit" || input == ".quit")
            break;
        std::cout << BOLDBLUE << "[Syntax Analyzation]" << RESET << std::endl;
        sc.input(input);
        std::vector<Symbol> stream = sc.tokenStream();
        _mode_ll1->analyze(stream);
    }
    std::cout << "terminated." << std::endl;
}

void Grammar::runLR1()
{
    expandGrammar();
    std::cout << BOLDYELLOW << "[expanded]" << RESET << std::endl;
    printGrammar();
    computeFirstTable();
    computeFollowTable();
    printFirst();
    printFollow();

    _mode_lr1 = new GrammarLR1(this);
    _mode_lr1->init();
    _mode_lr1->generateDFA();
    /* output */
    _mode_lr1->printDFA();
    if(!_mode_lr1->initTable())
        EXIT_ERROR("not a LR1 grammar.");
    _mode_lr1->printTable();

    Scanner sc;
    std::string input;
    while(true)
    {
        std::cout << "Symbol String > ";
        std::getline(std::cin, input);
        if(input == ".exit" || input == ".quit")
            break;
        std::cout << BOLDBLUE << "[Syntax Analyzation]" << RESET << std::endl;
        sc.input(input);
        std::vector<Symbol> stream = sc.tokenStream();
        _mode_lr1->analyze(stream);
    }
    std::cout << "terminated." << std::endl;
}