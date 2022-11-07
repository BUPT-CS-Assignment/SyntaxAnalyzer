#ifndef __Grammar_HPP__
#define __Grammar_HPP__

#include <iostream>
#include <vector>
#include <set>
#include "lexer/scanner.hpp"
#include "arith.hpp"

static std::string EMPTY_STR = "";                              // 空字符串
static std::string EPSILON_STR = "~";                           // 空串字符串
static std::string ENDING_STR = "$";                            // 结束符字符串
static std::string PRODUCTION_DIVIDE = "#";                     // 文法分割字符串
static std::string PRODUCTION_BEGIN = "@";                      // 文法起始字符串
static Symbol EPSILON = Symbol("~",Symbol::Type::EPSILON);      // 空串记号
static Symbol ENDING = Symbol("$",Symbol::Type::END);           // 结束符记号
static Symbol EMPTY = Symbol("",Symbol::Type::END);             // 空记号

/* 文法 */
class Grammar{
/* LL1/LR1 处理程序子结构 */
class GrammarLL1;
class GrammarLR1;
public:
    /* 产生式子结构 */
    class Production;
private:
    Scanner _sc;                            // 词法分析扫描仪
    GrammarLL1* _mode_ll1 = nullptr;        // LL1分析器指针
    GrammarLR1* _mode_lr1 = nullptr;        // LR1分析器指针
    Arith*      _mode_arith = nullptr;      // ARITH分析
    void _printSet(std::vector<std::set<std::string>>*);    // 集合输出

public:
/* Variable */
    std::string start;                              // 开始符号
    std::vector<std::string> T;                     // 终结符
    std::vector<std::string> N;                     // 非终结符
    std::vector<Production> P;                      // 产生式
    std::vector<std::set<std::string>> FIRST;       // FIRST集
    std::vector<std::set<std::string>> FOLLOW;      // FOLLOW集

/* Function */
    /* 构造及初始化 */
    Grammar();
    Grammar(std::string start,std::vector<std::string> N,std::vector<std::string> T);
    void init(const char* file_name);

    /* 产生式/产生式下标获取 */
    Production& operator [](std::string from);
    Production& operator [](int);
    int operator()(std::string from);
    int operator()(Symbol&);

    /* 终结符/非终结符下标获取 */
    int _findT(std::string);
    int _findN(std::string);

    /* 非终结符操作 */
    bool setStart(std::string);
    bool addN(std::string);

    /* 插入新产生式 */
    void insert(std::string from, std::vector<std::string> to);
    void insert(std::string from, std::vector<Symbol> to);          // 插入新产生式
    void insert(Production& p);

    /* FIRST FOLLOW集计算*/
    void computeFirstTable();
    void computeFollowTable();
    std::set<std::string>& findFirst(std::string N);        // 通过字符串
    std::set<std::string>& findFollow(std::string N);       // 通过字符串
    std::set<std::string> findFirst(std::vector<Symbol>);   // 通过记符号串
    
    /* 判断是否属于FIRST/FOLLOW集 */
    bool findFirst(std::string N, std::string c);
    bool findFollow(std::string N, std::string c);

    void printFirst();              // 输出FIRST集
    void printFollow();             // 输出FOLLOW集
    void printGrammar();            // 输出文法
    
    /* LL1 */
    void eliminateLeftRecursion();  // 消除左递归
    void extractLeftCommonFactor(); // 提取左公因子

    /* LR1 */
    void expandGrammar();           // 拓展文法  

    /* main */
    void runLL1();                  // 运行LL1分析程序
    void runLR1();                  // 运行LR1分析程序
    void runArith();                // 运行算数表达式分析
};


/* 产生式子结构 */
class Grammar::Production{
public:
    /* 产生式右部子结构 */
    class To;
private:
    Scanner* __sc = nullptr;        // 词法分析扫描仪指针
    void _clear();                  // 清空信息
    bool _exist(To&);               // 判断产生式右部重复
public:
/* Variable */
    std::string from;               // 产生式左部
    std::vector<To> to;             // 产生式右部集合
    
/* Function */
    /* 构造 */
    Production(std::string from = "");
    Production(const Production&);
    
    /* 产生式获取, []运算符重载 */
    To& operator [](int pos);

    /* 插入新产生式右部 */
    bool insert(std::vector<std::string> to);
    bool insert(std::vector<Symbol> to);
    bool insert(To);

    /* 产生式处理 */    
    bool existLeftRecursion();                  // 左递归判断
    Production eliminateLeftRecursion();        // 消除左递归, 返回新产生式
    std::set<Symbol> existLeftCommonFactor();   // 左公因子判断, 返回重复因子集合

    /* 产生式相等判断, ==/!=运算符重载 */
    bool operator==(const Production&) const;
    bool operator!=(const Production&) const;

    /* 字符串化 */
    std::string toString(int to);
    std::string toString();
    std::string toStringLR1();
};


/* 产生式右部结构 */
class Grammar::Production::To{
public:
/* LR1分析，项目状态定义 */
enum class State{
    SHIFT,                  // 移进
    REDUCTION_EXPECTING,    // 待约
    REDUCTION,              // 归约
};
private:
    int _dot = 0;                           // 点号位置
public:
/* Variable */
    State state;                            // 项目状态
    std::vector<Symbol> elements;           // 产生式右部符号串
    std::set<std::string> ahead;            // 向前看符号集合

/* Function */
    /* 构造及初始化 */
    To();
    To(const To&);
    void clear();                           // 清空

    /* 插入新符号 */
    void append(std::string);
    void append(Symbol&);

    /* LR1 操作 */
    int getDot();                           // 获取点号位置
    State getState();                       // 获取当前项目状态
    State next();                           // 向前移动点号
    Symbol getDotSymbol();                  // 获取点号后符号
    std::vector<Symbol> getAheadStream();   // 获取向前看符号计算的符号流

    /* 操作及重载 */
    Symbol& operator[](int pos);            // 符号定位获取, []运算符重载
    bool operator == (const To&) const;     // 判断相等, ==符号重载

    /* 字符串化 */
    std::string toString();
    std::string toStringLR1();
};

#endif