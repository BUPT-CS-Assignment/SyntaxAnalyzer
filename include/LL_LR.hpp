#ifndef __LL_LR_HPP__
#define __LL_LR_HPP__

#include "grammar.hpp"

/* LL1 分析结构 */
class Grammar::GrammarLL1{
const static int ERROR = -1;            // 错误代码
const static int SYNCH = -2;            // 同步代码
private:
/* Variable */
    std::vector<Symbol>* _stream;       // 记号流
    int _stream_pos = -1;               // 记号流指针
    Symbol _cur_token;                  // 当前记号
    std::vector<Symbol> _state_stack;   // 分析栈

/* Function */
    /* 分析操作 */
    Symbol& _topState();                // 获取栈顶符号
    Symbol& _nextToken();               // 下一记号
    void _analyze();                    // 预测分析
    void _error();                      // 错误处理

    /* 输出 */
    std::string _outputStack();         // 栈输出
    std::string _outputStream();        // 记号流输出
    void _outputPair();                 // 匹配栈与记号流输出
    
public:
/* Variable */
    Grammar* G = nullptr;               // 文法结构指针
    std::vector<std::vector<int>> table;// 预测分析表

/* Fucntion*/
    GrammarLL1(Grammar*);               // 构造
    bool initTable();                   // 构建分析表
    void printTable();                  // 输出分析表
    void analyze(std::vector<Symbol>& s_strem); // 传入记号流分析
};

class Grammar::GrammarLR1{
/* 项目集规范组子结构 */
class Item;
/* Action类型 */
enum class ActionType{
    ERROR,      // 错误
    SHIFT,      // 移进
    REDUCTION,  // 归约
    ACCEPT,     // 接受
};
const static int ERROR = -1;                // 错误代码
private:
/* Variable */
    /* LR1分析结构 */
    std::vector<Item*> _colletion;          // 项目集规范组
    std::vector<int> _state_stack;          // 状态分析栈
    std::vector<Symbol> _symbol_stack;      // 符号分析栈

    /* 输入流操作 */
    std::vector<Symbol>* _stream;           // 记号流
    int _stream_pos = -1;                   // 记号流指针
    Symbol _cur_token;                      // 当前记号

/* Function */
    /* 分析处理 */
    Symbol& _nextToken();                   // 下一记号
    int _topState();                        // 获取栈顶状态
    Symbol& _topSymbol();                   // 获取栈顶符号
    void _analyze();                        // 预测分析
    void _error();                          // 错误处理

    /* 输出 */
    std::string _outputStack();             // 状态栈和符号栈输出
    std::string _outputStream();            // 记号流输出
    void _outputPair();                     // 匹配栈和记号流输出

    /* 定位 */
    int _findItem(Item&);                   // 定位项目集规范组
    int _findProduction(Production&);       // 定位带标号的文法

public:
/* Variable */
    Grammar* _G = nullptr;                                              // 文法结构指针
    std::vector<Production> P;                                          // 带标号的单一文法
    std::vector<std::vector<std::pair<ActionType,int>>> action_table;   // Action分析表
    std::vector<std::vector<int>> goto_table;                           // Goto分析表

/* Function */
    /* 构造及初始化 */
    GrammarLR1(Grammar*);
    void init();

    /* 操作 */
    void generateDFA();     // 生成DFA  
    bool initTable();       // 生成分析表
    void printDFA();        // 输出DFA
    void printTable();      // 输出分析表
    void analyze(std::vector<Symbol>& s_table); // 传入记号流分析
};

/* 项目集规范组子结构 */
class Grammar::GrammarLR1::Item{
const static int NOT_FOUND = -3;        // 文法未找到
const static int AHEAD_CONTAIN = -2;    // 文法相同，向前看符号集合不同
const static int ALL_EQUAL = -1;        // 全相同
private:
    Grammar::GrammarLR1* _LR1 = nullptr;            // LR1结构指针
    int _find(Production&);                         // 重复文法搜索
    void _computeAhead(Production&,Production&);    // 向前看符号计算

public:
/* Variable */
    int _serial;                                            // 标号
    std::vector<Production> _productions;                   // 当前产生式列表
    std::vector<Symbol> _nextRead;                          // 识别活前缀列表
    std::vector<int> _nextTo;                               // 识别后下一规范组列表
    
/* Function */
    /* 构造及初始化 */
    Item(GrammarLR1* lr = nullptr ,int serial = -1);
    Item(const Item&);
    void clear();

    /* 操作 */
    int insert(Production&);    // 插入产生式
    void closure();             // 计算闭包
    void next();                // 活前缀识别及添加新规范组

    /* 数据获取及判断 */
    Production& operator[](int);            // 产生式获取
    Item& operator=(const Item&);           // 赋值运算
    bool operator==(const Item&) const;     // 相等判断, ==重载
    bool operator!=(const Item&) const;     // 不等判断, !=重载

    /* 字符串化 */
    std::string toString();
};


#endif