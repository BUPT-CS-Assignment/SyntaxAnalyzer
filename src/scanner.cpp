#include "../include/lexer/scanner.hpp"

std::vector<Symbol> Scanner::tokenStream(){
    std::vector<Symbol> symbols;    // 记号流
    bool error_flag = false;
    Symbol s(this);
    /* 生成记号流 */
    while((++s).attr() != Symbol::Type::END){
        s.setContent(s.syntaxSign());               // 设置显示内容
        if(s.attr() == Symbol::Type::ERROR)
            error_flag = true;
        else if(s.attr() != Symbol::Type::COMMENT)  // 跳过注释
            symbols.emplace_back(s);
    }
    if(error_flag){     
        symbols.clear();
        LOG_INFO("invalid input");  // 错误检测  
    }
    symbols.emplace_back(Symbol("end",Symbol::Type::END));  // 插入结束记号
    return symbols;
}

void Scanner::setReport(bool flag){
    _error_output_flag = flag;
}

void Scanner::input(const std::string in_str){
    _ofs.open(__tmp_filename,std::ios::out|std::ios::trunc);    // 写入tmp文件
    if(!_ofs.is_open())
        EXIT_ERROR("system error");
    _ofs << in_str;
    _ofs.close();
    close();    
    open(__tmp_filename.c_str());   // 调用open读取文件流
}

void Scanner::terminate(){
    close();
    _ofs.close();
    remove(__tmp_filename.c_str());
}

bool Scanner::open(const char* file_name)
{
    _ifs.open(file_name, std::ios::in);
    if(!_ifs.is_open())
        EXIT_ERROR("no such file '" << file_name << "'");
    _total_char = 0;
    _ifs_line.open(file_name, std::ios::in);
    _ifs_line.getline(_line_buffer,__buffer_length);
    memset(_file_buffer,EOF,sizeof(_file_buffer));
    _ifs.read(_file_buffer, __buffer_mid);
    _file_name = std::string(file_name);
    return true;
}

void Scanner::close()
{
    memset(_file_buffer, 0, sizeof(_file_buffer));
    _ptr_buffer = -1;
    _ifs.close();
}

void Scanner::_rollBack()
{
    char ch = _file_buffer[_ptr_buffer];
    if(ch != ' ' && ch != '\n' && ch != '\t')
        --_total_char;  // 字符计数
    _cur_ind.lastCol();
    if(_row_flag) _row_flag = false;
    if(_ptr_buffer == 0)
        _ptr_buffer = __buffer_length - 1;
    else
        --_ptr_buffer;
    ++_roll_back_times;
    
}


char Scanner::_nextChar()
{   
    _ptr_buffer = (_ptr_buffer + 1) % __buffer_length;
    if(_roll_back_times != 0){  // 有回退记录, 不进行文件读取
        --_roll_back_times;     // 回退消除            
    }else{
        if(_row_flag){
            _ifs_line.getline(_line_buffer,__buffer_length);    // 行缓冲, 用于错误报告
        }
        if(_ptr_buffer == __buffer_mid - 1 && !_ifs.eof())
        {
            memset(&_file_buffer[__buffer_mid], EOF, __buffer_mid); //段重置
            _ifs.read(&_file_buffer[__buffer_mid], __buffer_mid);
        }
        else if(_ptr_buffer == __buffer_length - 1 && !_ifs.eof())
        {
            memset(&_file_buffer[0], EOF, __buffer_mid);    // 段重置
            _ifs.read(_file_buffer, __buffer_mid);
        }
    }

    if(_row_flag){
        _cur_ind.nextRow();     // 行计数
        _row_flag = false;
    }
    else
        _cur_ind.nextCol();     // 列计数

    char ch = _file_buffer[_ptr_buffer];
    if(ch != ' ' && ch != '\n' && ch != '\t' && ch != EOF)
        ++_total_char;  // 字符计数
    if(ch == '\n')
        _row_flag = true;
    return ch;
}

Symbol Scanner::nextSymbol()
{
    _token.clear();
    
    char ch;
    do
        ch = _nextChar();
    while(ch == ' ' || ch == '\n'); // 跳过开始读取时的空格/换行
    _token.indicator = _cur_ind;
    if(ch == EOF)           // 文件结束
        return _token.end();
    _token.append(ch);      
    if(ch == '.')           // 点号 (->操作符/数字)
        _procDot();
    else if(_isNum(ch))     // 数字
        _procNumber();
    else if(_isLetter(ch))  // 字母/_
        _procIdentifier();
    else if(ch == '/')      // 除号 (->操作符/注释)
        _procDiv();
    else if(ch == '\"' || ch == '\'')   // 字符串
        _procString(ch);
    else if(_isLimit(ch))   // 界限符
        _procLimit();
    else if(_isOperator(ch))    // 操作符
        _procOperator(ch);
    else                        // 非法字符
        _procError(ErrorType::INVALID_WORD);

    ++_type_counter[(int)_token.attr()];    // 类型计数
    if(_token.attr() == Symbol::Type::COMMENT)
        return nextSymbol();    // 跳过注释
    return _token;
}


void Scanner::_procDot()
{
    _token.setAttr(Symbol::Type::OPERATOR);
    char ch = _nextChar();
    if(_isLetter(ch))
        return _rollBack();
    _token.append(ch);
    if(_isNum(ch))
    {
        _rollBack();
        _token.rollBack();
        return _procNumber();
    }
    else if(ch == '.')
    {

        ch = _nextChar();
        if(ch == '.')
            return _token.append(ch);
    }
    return _procError(ErrorType::INVALID_WORD);
}


void Scanner::_procString(char start)
{
    _token.setAttr( Symbol::Type::STRING);
    bool escape = false;
    
    for(;;)
    {
        char ch = _nextChar();
        _token.append(ch);
            
        if(escape){
            escape = (ch == '\\' ? true:false);
            if(ch == '\n'){         // 隐藏token中的转义\n
                _token.rollBack();  
                _token.rollBack();  
            }
        }
        else if(!escape && ch == '\\')
            escape = true;
        else if(ch == EOF || (!escape && ch == '\n'))
        {
            _token.rollBack();
            return _procError(ErrorType::EXPECTED_END);
        }
        else if(!escape && ch == start){
            if(start == '\''){
                int l = _token.content().length();
                if(l > 4 || (l == 4 && _token[1] != '\\')){
                    return _procError(ErrorType::CHAR_OVERFLOW);
                }  
            }
            return;
        }
            
    }
}

void Scanner::_procDiv()
{
    _token.setAttr(Symbol::Type::OPERATOR);
    char ch = _nextChar();
    if(ch == '*' || ch == '/')
        return _procAnnotaion(ch);
    return _rollBack();
}

void Scanner::_procAnnotaion(char type)
{
    _token.setAttr(Symbol::Type::COMMENT);
    _token.append(type);
    if(type == '/')
    {
        for(char ch;;)
        {
            ch = _nextChar();
            if(ch == '\n' || ch == EOF)
                return;
        }
    }

    bool end = false;
    for(char ch;;)
    {
        ch = _nextChar();
        if(ch == '*')
            end = true;
        else if(end){
            if(ch == '/')   return;
            end = false;
        }
        else if(ch == EOF)
            return _procError(ErrorType::EXPECTED_END);
    }
}

void Scanner::_procIdentifier()
{
    _token.setAttr(Symbol::Type::IDENTIFIER);
    for(char ch;;)
    {
        ch = _nextChar();
        if(ch == EOF)
            break;
        if(_isLetter(ch) || _isNum(ch))
            _token.append(ch);
        else{
            _rollBack();
            break;
        } 
    }
    if(__is_keyword(_token.content().c_str()))
        _token.setAttr(Symbol::Type::KEYWORD);
    
}


/* 数字处理 */
void Scanner::_procNumber()
{
    _token.setAttr(Symbol::Type::NUMBER);
    int type = 0;
    int part_int = 1, part_float = 2, part_e = 3, part_end = 4;
    int part = part_int;
    PrefixType ptype = _procPrefix();
    NumType ntype = NumType::INT;
    if(ptype == PrefixType::_NAN)   return;
    for(char ch;;)
    {
        ch = _nextChar();
        if(ch == EOF) return;
        if(_isSuffix(ch)){
            _rollBack();
            return _procSuffix(ntype);
        }
        if(part == part_int)
        {
            if(ptype == PrefixType::BINARY && !(ch == '0' || ch == '1')){
                if(_isNum(ch))
                    _procError(ErrorType::INVALID_BINARY);
                return _rollBack();
            }else if(ptype == PrefixType::HEX && !_isHex(ch)){
                _procError(ErrorType::INVALID_HEX);
                return _rollBack();
            }else if(ptype == PrefixType::OCTAL && !(ch >= '0' && ch <= '7')){
                if(_isNum(ch))
                    _procError(ErrorType::INVALID_OCTAL);
                return _rollBack();
            }
            // Decimal
            if(ch != '.' && ch != 'E' && ch != 'e' && !_isNum(ch))
                return _rollBack();
            part = (ch == '.' ? part_float : ((ch == 'e' || ch == 'E') ? part_e : part_int));
        }
        else if(part == part_float)
        {
            ntype = NumType::FLOAT;
            if(ch != 'E' && ch != 'e' && !_isNum(ch))
                return _rollBack();
            part = (ch == 'E' || ch == 'e' ? part_e : part_float);
        }
        else if(part_e)
        {
            if(ch != '+' && ch != '-' && !_isNum(ch))
                return _rollBack();
            part = part_end;
        }
        else if(part_end){
            if(!_isNum(ch))
                return _rollBack();
        }
        _token.append(ch);
    }
}

Scanner::PrefixType Scanner::_procPrefix(){
    char ch = _token[0]; 
    if(ch == EOF) return PrefixType::_NAN;
    if(ch != '0'){
        _rollBack();
        _token.rollBack();
        return PrefixType::DECIMAL;
    }
    ch = _nextChar();
    _token.append(ch);
    if(_isNum(ch))
        return PrefixType::OCTAL;
    else if(ch == 'x' || ch == 'X')
        return PrefixType::HEX;
    else if(ch == 'b' || ch == 'B')
        return PrefixType::BINARY;

    _token.rollBack();
    _rollBack();
    if(ch == '.')
        return PrefixType::DECIMAL;
    return PrefixType::_NAN;
}

void Scanner::_procSuffix(Scanner::NumType ntype){
    char ch = _nextChar();
    if(ch == EOF)   return;
    if(ntype == NumType::FLOAT){
        if(ch == 'u' || ch == 'U'){
            _procError(ErrorType::INVALID_FLOAT);
            return _rollBack();
        }
        _token.append(ch);
        return;
    }else if(ntype == NumType::INT){
        if(ch == 'f' || ch == 'F')
            return _rollBack();  
        _token.append(ch);
        if(ch == 'l' || ch == 'L'){         //l
            ch = _nextChar();
            if (ch == 'l' || ch == 'L'){    //ll
                _token.append(ch);          
                ch = _nextChar();
                if(ch == 'u' || ch == 'U')  //llu
                    _token.append(ch);
                else
                    return _rollBack();
            }else if(ch == 'u' || ch == 'U')    //lu
                _token.append(ch);
            else
                return _rollBack();
        }else if(ch == 'u' || ch == 'U'){   //u
            ch = _nextChar();
            if (ch == 'l' || ch == 'L'){    //ul
                _token.append(ch);
                ch = _nextChar();
                if(ch == 'l' || ch == 'L')  //ull
                    _token.append(ch);
                else 
                    return _rollBack();
            }else
                return _rollBack();     
        }
    }
}

void Scanner::_procLimit()
{
    _token.setAttr(Symbol::Type::LIMIT);
    return;
}

void Scanner::_procOperator(char start)
{
    _token.setAttr(Symbol::Type::OPERATOR);
    bool append_flag = true;
    char ch = _nextChar();
    if(ch == EOF)   return;
    if(start == '-')
        append_flag = (ch == '-' || ch == '=' || ch == '>');
    else if(start == '*' || start == '/' || start == '!' || start == '%' || start == '^')
        append_flag = (ch == '=');
    else if(start == '>' || start == '<' || start == '=' || start == '|' || start == '&' || start == '+')
        append_flag = (ch == '=' || ch == start);

    if((start == '<' && ch == '<') || (start == '>' && ch == '>'))
    {
        _token.append(ch);
        ch = _nextChar();
        append_flag = (ch == '=');
    }

    return append_flag ? _token.append(ch) : _rollBack();
}

void Scanner::_procError(Scanner::ErrorType type)
{
    if(type == ErrorType::CHAR_OVERFLOW || type == ErrorType::EXPECTED_END || type == ErrorType::INVALID_WORD)
        _token.setAttr(Symbol::Type::ERROR) ;
    _token.indicator.setCol(_cur_ind.col());    // 标明错误所在行列
    if(_error_output_flag)
        _reportError(type);                         // 打印错误
}

void Scanner::_reportError(ErrorType e)
{
    std::string msg = "",type = BOLDYELLOW "warning: " RESET;
    /* 错误类型 & 信息注入 */
    if(e == ErrorType::INVALID_WORD){
        msg = "invalid character '" BOLDWHITE + _token.content() + RESET "'";
        type = BOLDRED "error: " RESET;
    }
    else if(e == ErrorType::EXPECTED_END){
        std::string end = std::string(1,_token[0]);
        msg = "expected '" BOLDWHITE + (end == "*"?"*/":end) + RESET "'";
        type = BOLDRED "error: " RESET;
    }else if(e == ErrorType::INVALID_BINARY){
        msg = "invalid binary number";
    }else if(e == ErrorType::INVALID_FLOAT){
        msg = "invalid float number";
    }else if(e == ErrorType::INVALID_HEX){
        msg = "invalid hex number";
    }else if(e == ErrorType::INVALID_OCTAL){
        msg = "invalid octal number";
    }else if(e == ErrorType::CHAR_OVERFLOW){
        msg = "too many characters";
    }
    /* 高亮显示 */
    printf(BOLDWHITE "%s:%d:%d: " "%s" "%s\n", _file_name.c_str(),_token.indicator.row(), _token.indicator.col(),type.c_str(),  msg.c_str());
    printf("%5d |%s\n",_token.indicator.row(),_line_buffer);
    printf("      |");
    for(int i = 0; i < _token.indicator.col()-1; i++)
        printf(" ");
    printf(BOLDRED "^\n" RESET);
}


bool Scanner::_isLetter(char ch)
{
    return ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool Scanner::_isNum(char ch)
{
    return ch >= '0' && ch <= '9';
}

bool Scanner::_isLimit(char ch)
{
    return ch == '{' || ch == '}' || ch == '(' || ch == ')'
        || ch == '[' || ch == ']' || ch == ',' || ch == ';';
}

bool Scanner::_isOperator(char ch)
{
    return ch == '<' || ch == '>' || ch == '=' || ch == '+' || ch == '-' || ch == '*'
        || ch == '/' || ch == '%' || ch == '&' || ch == '|' || ch == '!' || ch == '^';
}

bool Scanner::_isSpace(char ch)
{
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
}

bool Scanner::_isHex(char ch){
    return _isNum(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

bool Scanner::_isSuffix(char ch){
    return ch == 'L' || ch == 'l' || ch == 'U' || ch == 'u' || ch == 'F' || ch == 'f';
}





/***********Symbol***********/
Symbol& Symbol::operator++(){
    Symbol temp = _sc->nextSymbol();
    indicator = temp.indicator;
    _attr = temp._attr;
    _ptr_content = temp._ptr_content;
    memcpy(&_content,&temp._content,__buffer_length);
    if(_attr == Type::END)
        _sc->terminate();

    return *this;
}
