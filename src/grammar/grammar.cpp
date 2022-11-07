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
    std::set<std::string> first;  // 定义容器
    if(to.size() == 0)            // 传参检测
        return first;
    if(to.size() == 1 && to[0].attr() == Symbol::Type::EPSILON) // 起始空串检测
    {
        first.insert(EPSILON_STR);
        return first;
    }
    bool epsilon_flag = true;   // 空串记号
    for(auto ele:to)
    {
        /* 遇到终结符 */
        if(ele.attr() != Symbol::Type::NOT_TERMINAL)
        {
            first.insert(ele.content());    // 将其加入FIRST
            epsilon_flag = false;           // 最终不推出epsilon
            break;
        }
        else
        {
            /* 遇到非终结符 */
            int pos = _findN(ele.content());    // 定位该非终结符下标
            bool has_epsilon = false;           // 该非终结符是否推出epsilon
            for(auto ele : FIRST[pos])          // 对于该非终结符的每一个FIRST集合元素
            {
                if(ele != EPSILON_STR)
                    first.insert(ele);          // 将其非epsilon元素插入现有容器
                else
                    has_epsilon = true;         // 能推出epsilon，设置该记号
            }
            if(!has_epsilon)                    // 该非终结符不推出epsilon
            {
                epsilon_flag = false;           // 最终不推出epsilon
                break;                          // 停止继续遍历
            }
        }
    }
    if(epsilon_flag)                            // 所有符号都能推出epsilon
        first.insert(EPSILON_STR);              // 将epsilon加入现有容器
    return first;                               // 返回计算结果
}


void Grammar::computeFirstTable()
{
    FIRST.clear();
    FIRST.resize(N.size());                             // 调整为与非终结符列表的大小一致
    bool run_flag = true;
    while(run_flag)
    {
        run_flag = false;                               // 设置循环记号
        for(int i = 0; i < N.size(); i++)               // 对于每一个非终结符
        {
            std::set<std::string>* first = &FIRST[i];   // 通过指针记录
            int pre_size = first->size();               // 记录原大小
            /* p : FROM -> a|bX|cY */
            for(auto& P_right : P[i].to)                // 对于每个非终结符的对应产生式
            {   /* P_right : bX */
                bool epsilon_flag;                      // 标记是否最终推出epsilon
                for(auto& ele : P_right.elements)       // 对于每个产生式右部的元素
                {   /* ele : X */
                    if(ele.attr() != Symbol::Type::NOT_TERMINAL)    // 是非终结符
                    {
                        first->insert(ele.content());               // 将其加入FIRST, 停止向下遍历
                        break;
                    }
                    epsilon_flag = false;
                    // 是非终结符，获取其FIRST集合
                    int loc = _findN(ele.content());
                    LOG_INFO(ele.content() << " " << loc);
                    if(loc == -1)
                        EXIT_ERROR("program error.")
                    std::set<std::string>* set_ele = &FIRST[loc];
                    for(auto& eles : (*set_ele))
                        if(eles != EPSILON_STR)         // 该非终结符的FIRST不含epsilon
                            first->insert(eles);
                        else
                            epsilon_flag = true;
                    if(!epsilon_flag)                   // 如果不包含epsilon, 停止向下遍历
                        break;
                }
                if(epsilon_flag)                        // 所有的产生式右部都能推出epsilon
                    first->insert(EPSILON_STR);         // 将epsilon加入该非终结符的FIRST集
            }
            if(first->size() > pre_size)                // FIRST集大小检测
                run_flag = true;                        // 如果某个FIRST集合的大小有变化，则继续循环
        }
    }
}

void Grammar::computeFollowTable()
{
    FOLLOW.clear();
    FOLLOW.resize(N.size());                        // FOLLOW表大小调整为非终结符列表的大小
    FOLLOW[_findN(start)].insert(ENDING_STR);       // 将$插入文法开始符号的FOLLOW集合
    bool run_flag = true;                           // 循环记号
    while(run_flag)
    {
        run_flag = false;   
        for(int i = 0; i < FOLLOW.size(); i++)      // 对于每个非终结符, 记为A
        {
            std::set<std::string>* follow = &FOLLOW[i]; // 指针记录
            for(auto& P_right : P[i].to)                // 对A的产生式的所有右部
            {
                std::vector<Symbol> ele = P_right.elements;
                for(int j = 0; j < ele.size(); j++)     // 对每个产生式的所有符号遍历
                {
                    if(ele[j].attr() != Symbol::Type::NOT_TERMINAL)     // 遇到非终结符, 跳过
                        continue;
                    /* 遇到非终结符, 计算该终结符对应的FOLLOW, 记为X */
                    int pos = _findN(ele[j].content());         // 定位该终结符X
                    int pre_size = FOLLOW[pos].size();          // 记录初始大小
                    bool end_flag = true;                       // 记录是否X后部符号串能推出epsilon
                    for(int k = j + 1; k < ele.size(); k++)     // 对X后的每个符号
                    {
                        if(ele[k].attr() != Symbol::Type::NOT_TERMINAL) // 遇到终结符
                        {
                            FOLLOW[pos].insert(ele[k].content());       // 将其加入该非终结符X的FOLLOW集
                            end_flag = false;                           // X后部的符号串不推出epsilon
                            break;
                        }
                        bool epsilon_flag = false;
                        /* 遇到非终结符, 记为Y */
                        std::set<std::string> first_temp = findFirst(ele[k].content()); // 找到Y的FIRST集合
                        for(auto& it_str : first_temp)
                        {
                            if(it_str == EPSILON_STR)   // Y能推出epsilon
                                epsilon_flag = true;
                            else
                                FOLLOW[pos].insert(it_str);     // 将非EPSILON元素加入非终结符X的FOLLOW集
                        }
                        if(!epsilon_flag)           // 如果Y不能推出epsilon
                        {
                            end_flag = false;       // X后部的符号串不推出epsilon
                            break;                  // 停止向下遍历
                        }
                    }
                    if(end_flag)                    // X后部的符号串能推出epsilon
                        FOLLOW[pos].insert(follow->begin(), follow->end()); // 将A的FOLLOW集合加入X的FOLLOW集
                    if(FOLLOW[pos].size() > pre_size)   // 如果X的FOLLOW集大小有更新，则进行新一轮循环
                        run_flag = true;
                }
            }
        }
    }
}


void Grammar::eliminateLeftRecursion()
{
    bool run_flag = true;
    while(run_flag) // 循环检测直接左递归, 直到所有产生式都不存在直接左递归
    {
        run_flag = false;   
        for(auto it = P.begin(); it != P.end(); it++)
        {
            if((*it).existLeftRecursion())  // 如果存在左递归
            {
                Production p = (*it).eliminateLeftRecursion();  // 修改原产生式
                insert(p);      //插入消除该直接左递归后的新产生式
                run_flag = true;    // 设置循环标志, 继续检测
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
                // for all common factors
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

void Grammar::runArith(){
    _mode_arith = new Arith();
    std::string input;
    while(true)
    {
        std::cout << "Arith String > ";
        std::getline(std::cin, input);
        if(input == ".exit" || input == ".quit")
            break;
        std::cout << BOLDBLUE << "[Arith Analyzation]" << RESET << std::endl;
        _sc.input(input);
        std::vector<Symbol> stream = _sc.tokenStream();
        _mode_arith->analyze(stream);
    }
    std::cout << "terminated." << std::endl;
}