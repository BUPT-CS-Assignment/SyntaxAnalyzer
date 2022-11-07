#include "../include/arith.hpp"

/* 处理非终结符E */
bool Arith::_stateE(){
    if(!_stateT())
        return false;
    /* 判断 + - */
    if(_cur_token == __ADD || _cur_token == __SUB){
        _outputPair(State::E,_cur_token == __ADD ? 0 : 1);
        _nextToken();    // 移动指针
        return _stateE();
    }
    _outputPair(State::E,2);    // 输出匹配项
    return true;
}

/* 处理非终结符T */
bool Arith::_stateT(){
    if(!_stateF()){
        //_error();
        return false;
    }
    /* 判断 * / */
    if(_cur_token == __MUL || _cur_token == __DIV){
        _outputPair(State::T,_cur_token == __MUL ? 0 : 1);
        _nextToken();   // 移动指针
        return _stateT();
    }
    _outputPair(State::T,2);
    return true;
}
/* 处理非终结符F */
bool Arith::_stateF(){
    /* 判断数字 */
    if(_cur_token.attr() == Symbol::Type::NUMBER){
        _outputPair(State::F,1);
        _nextToken();
        return true;
    }
    /* 判断左右括号 */
    if(_cur_token == __LBK){
        _outputPair(State::F,0);
        _nextToken();
        if(!_stateE())
            return false;
        if(_cur_token == __RBK){
            _outputPair(State::F,0);
            _nextToken();
            return true;
        }
    }
    _error();
    return false;
}

void Arith::_error(){
    if(_cur_token.attr() == Symbol::Type::END || _cur_token.content() == ";")
        std::cout << BOLDRED << "[error] " << RESET << "incomplete stream." << std::endl;
    else
        std::cout << BOLDRED << "[error] " << RESET << "unknown symbol '" << _cur_token.content() << "'" << std::endl;
    while(_cur_token.attr() != Symbol::Type::END && _cur_token.content() != ";")
        _nextToken();
    //_nextToken();
}

Symbol& Arith::_nextToken(){
    if(_stream == nullptr || _stream_pos >= (int)_stream->size())
        _cur_token = ENDING;
    else
        _cur_token = (*_stream)[++_stream_pos];
    return _cur_token;
}

void Arith::analyze(std::vector<Symbol>& s_stream){
    _stream = &s_stream;
    _stream_pos = -1;
    _nextToken();
    while(_cur_token.attr() != Symbol::Type::END)
    {
        _analyze();
        if(_cur_token.content() == ";")
            _nextToken();
    }
}

void Arith::_analyze(){
    std::cout << BOLDWHITE << "[input]" << RESET << _outputStream() << std::endl;
    bool res = _stateE();
    std::cout << BOLDWHITE << "[result]" << (res ? BOLDGREEN : BOLDRED) << (res ? " success" : " error") << RESET << std::endl;
}

std::string Arith::_outputStream(){
    std::string out = "";
    int p = _stream_pos;
    if(p == -1 || p >= _stream->size())
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

void Arith::_outputPair(State s,int pos){
    LEFT_PRINT(30,_outputStream());
    std::cout << ARITH_GRAMMAR[(int)s][pos];
    std::cout << std::endl;
}
