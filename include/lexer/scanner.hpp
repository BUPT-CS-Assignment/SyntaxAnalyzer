#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__

#include "../basic.h"
#include "notation.h"
#include "indicator.hpp"


class Scanner;

const static std::string __type_str[] = {
    "end",
    "error",
    "keyword",
    "id",
    "num",
    "op",
    "lim",
    "str",
    "comment",
    "N",
    "~"
};

class Symbol
{
    const static size_t __buffer_length = 16;
public:
    enum class Type
    {
        END,            // 结束符
        ERROR,          // 错误符
        KEYWORD,        // 关键字
        IDENTIFIER,     // 标识符
        NUMBER,         // 数字
        OPERATOR,       // 操作符
        LIMIT,          // 界限符
        STRING,         // 字符
        COMMENT,        // 注释
        /* for grammar analyze */
        NOT_TERMINAL,   // 非终结符
        EPSILON,
    };
private:
    Scanner* _sc;
    int _ptr_content = 0;                   // 内容指针
    char _content[__buffer_length] = {0};   // 字符内容
    Type _attr = Type::END;                 // 字符记号
    
public:
    Indicator indicator;    // 指示器

    Symbol(const Symbol& s){
        _sc = s._sc;
        _ptr_content = s._ptr_content;
        _attr = s._attr;
        memcpy(_content,s._content,sizeof(_content));
    }

    Symbol(Scanner* sc = nullptr){
        _sc = sc;
    }
    Symbol(std::string s, Type attr){
        setContent(s);
        _attr = attr;
        _ptr_content = s.length();
    }
    Symbol(std::string s){
        setContent(s);
        _ptr_content = s.length();
        syntaxReset();
    }

    std::string syntaxSign(){
        if(_attr == Type::OPERATOR || _attr == Type::LIMIT || _attr == Type::KEYWORD)
            return content();
        return __type_str[(int)_attr];
    }

    void syntaxReset(){
        if(_attr != Type::END && _attr != Type::IDENTIFIER)
            return;
        std::string s(_content);
        if(s == "num")
            _attr = Type::NUMBER;
        else if(s == "id")
            _attr = Type::IDENTIFIER;
        else if(s == "str")
            _attr = Type::STRING;
        else if(s == "~")
            _attr = Type::EPSILON;
        else if(s == "$")
            _attr = Type::END;
        else
            _attr = Type::NOT_TERMINAL;
    }
    // 清空内容
    void clear(){
        memset(&_content,0,__buffer_length);
        _ptr_content = 0;
        indicator.setRow(0);
        indicator.setCol(0);
        _attr = Type::END;
    }
    // 添加字符
    void append(char ch){
        if(_ptr_content + 1 >= __buffer_length)
        {
            _ptr_content = 0;
            _attr = Type::ERROR;
        }
        _content[_ptr_content ++] = ch;
    }
    // 字符回退
    char rollBack(){
        if(_ptr_content <= 0)
            return EOF;
        char ch = _content[_ptr_content - 1];
        _content[--_ptr_content] = 0;
        return ch;
    }
    // 获取字符内容
    std::string content(){
        return std::string(_content);
    }
    // 设置字符内容
    void setContent(std::string c){
        const char* s = c.c_str();
        strcpy(_content,s);
        // memcpy(&_content,s,c.length() + 1);
    }
    // 获取属性
    Type attr(){
        return _attr;
    }
    // 设置属性
    void setAttr(Type t){
        _attr = t;
    }
    // 达到末尾
    Symbol& end(){
        _attr = Type::END;
        return *this;
    }
    // 错误
    Symbol& error(){
        _attr = Type::ERROR;
        return *this;
    }
    // 重载 []
    char operator[](int pos){
        if(pos >= 0 && pos < _ptr_content)
            return _content[pos];
        if(pos < 0 && (_ptr_content + pos) > 0)
            return _content[_ptr_content];
        return EOF;
    }

    Symbol& operator = (const Symbol& s){
        _sc = s._sc;
        _ptr_content = s._ptr_content;
        _attr = s._attr;
        memcpy(_content,s._content,sizeof(_content));
        return *this;
    }

    bool operator==(const Symbol& s) const{
        if(_attr != s._attr)
            return false;
        return strcmp(_content,s._content) == 0;
    }

    bool operator!=(const Symbol& s) const{
        if(_attr != s._attr)
            return true;
        return strcmp(_content,s._content) != 0;
    }

    bool operator<(const Symbol& s) const{
        return strcmp(_content,s._content) < 0;
    }

    Symbol& operator++();

};

class Scanner
{
    const static int __buffer_length = 64;  // 缓冲区长度
    const static int __buffer_mid = __buffer_length / 2;
    const std::string __tmp_filename = ".lexer_input.tmp";
private:
    enum class ErrorType{   //*错误类型
        INVALID_WORD,       // 非法标识符
        EXPECTED_END,       // 需要字符串/注释结尾
        INVALID_FLOAT,      // 浮点无效
        INVALID_HEX,        // 十六进制无效
        INVALID_OCTAL,      // 八进制无效
        INVALID_BINARY,     // 二进制无效
        CHAR_OVERFLOW,      // 字符常量超限
    };
    enum class PrefixType{  //*前缀类型
        DECIMAL,            // 十进制
        HEX,                // 十六进制
        BINARY,             // 二进制
        OCTAL,              // 八进制
        _NAN,               // 非数字
    };
    enum class NumType{     //*数字类型
        INT,                // 整形
        FLOAT,              // 浮点
    };
    // File stream
    std::string _file_name;             // 文件名
    std::ifstream _ifs, _ifs_line;      // 文件流(字符文件流)(行文件流)
    std::ofstream _ofs;                 // 输出流
    bool _row_flag = false;
    bool _error_output_flag = true;     //
    // String buffer
    char _line_buffer[256] = {0};       // 行缓冲区
    char _file_buffer[__buffer_length] = {0};   // 字符缓冲区
    int _ptr_buffer = -1;               // 字符缓冲区指针
    // Counter
    int _type_counter[9] = {0};         // 记号类型计数器
    int _total_char = 0;                // 字符计数器
    int _roll_back_times = 0;           // 指针回退计数器
    // Current Indicator
    Indicator _cur_ind;                 // 行列指示器
    // Read symbol
    Symbol _token;                      // 记号缓冲

    // 字符类型判断
    static bool _isNum(char);           // 数字
    static bool _isLetter(char);        // 字母/下划线
    static bool _isLimit(char);         // 界限符
    static bool _isOperator(char);      // 操作符
    static bool _isSpace(char);         // 空格/换行
    static bool _isHex(char);           // 十六进制 
    static bool _isSuffix(char);        // 后缀

    void _rollBack();                   // 字符指针回退
    char _nextChar();                   // 字符读取

    void _procDot();                    // 点号处理 (->数字/操作符)
    
    void _procNumber();                 // 数字处理
    PrefixType _procPrefix();           // 前缀处理
    void _procSuffix(NumType);          // 后缀处理
    
    void _procIdentifier();             // 标识符处理
    void _procLimit();                  // 界限符处理

    void _procDiv();                    // 除号处理  (->操作符/注释)
    void _procOperator(char);           // 操作符处理
    void _procString(char start);       // 字符串处理
    void _procAnnotaion(char start);    // 注释处理

    void _procError(ErrorType);         // 错误处理
    void _reportError(ErrorType e);     // 错误报告

public:
    void input(const std::string);
    void terminate();
    bool open(const char* file_name);   // 打开文件
    void close();                       // 关闭文件
    void setReport(bool);

    Symbol nextSymbol();                // 下一记号
    std::vector<Symbol> tokenStream();  // 生成记号流 
};



#endif