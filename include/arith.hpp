#ifndef __ARITH_HPP__
#define __ARITH_HPP__

#include "lexer/scanner.hpp"

/* 产生式 */
const static std::string ARITH_GRAMMAR[][3] = {
    {"E->E+T","E->E-T","E->T"},
    {"T->T*F","T->T/F","T->F"},
    {"F->(E)","F->num"}
};

class Arith{
/* 非终结符 */
enum class State{
    E,T,F
};
/* 终结符定义 */
const Symbol __ADD = Symbol("+",Symbol::Type::OPERATOR);
const Symbol __SUB = Symbol("-",Symbol::Type::OPERATOR);
const Symbol __MUL = Symbol("*",Symbol::Type::OPERATOR);
const Symbol __DIV = Symbol("/",Symbol::Type::OPERATOR);
const Symbol __LBK = Symbol("(",Symbol::Type::LIMIT);
const Symbol __RBK = Symbol(")",Symbol::Type::LIMIT);
const Symbol ENDING = Symbol("$",Symbol::Type::END);

private:
/* Variable */
    std::vector<Symbol>* _stream;   // 记号流
    int _stream_pos = -1;           
    Symbol _cur_token;              // 当前记号
/* Function */
    Symbol& _nextToken();           // 下一记号
    void _outputPair(State,int);    // 输出分析情况
    std::string _outputStream();    // 输出记号流
    void _error();                  // 错误处理
    void _analyze();                // 分析
    bool _stateE();                 // 处理非终结符E的产生式
    bool _stateT();                 // 处理非终结符T的产生式
    bool _stateF();                 // 处理非终结符F的产生式
public:
    void analyze(std::vector<Symbol>&); // 传入记号流分析
};

#endif