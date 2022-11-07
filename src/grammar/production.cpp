#include "../../include/grammar.hpp"

Grammar::Production::Production(std::string from){
    this->from = from;
}

Grammar::Production::Production(const Grammar::Production& p){
    __sc = p.__sc;
    from = p.from;
    to = std::vector<To>(p.to);
}

void Grammar::Production::_clear(){
    from = "";
    to.clear();
}

bool Grammar::Production::_exist(Grammar::Production::To& cmp){
    for(auto& it:to)
        if(it == cmp)
            return true;
    return false;
}

bool Grammar::Production::insert(std::vector<Symbol> to_s){
    To temp_to;
    for(auto& it:to_s)
        temp_to.append(it);
    if(!_exist(temp_to))
        to.emplace_back(temp_to);
    return true;
}

bool Grammar::Production::insert(std::vector<std::string> to_vec){
    To temp_to;
    for(auto& it:to_vec)
        temp_to.append(it);

    if(!_exist(temp_to))
        to.emplace_back(temp_to);
    return true;
}

bool Grammar::Production::insert(Grammar::Production::To t){
    if(!_exist(t))
        to.emplace_back(t);
    return true;
}


bool Grammar::Production::existLeftRecursion(){
    for(auto& it:to){
        if(it[0].content() == from)
            return true;
    }
    return false;
}

std::set<Symbol> Grammar::Production::existLeftCommonFactor(){
    std::set<Symbol> all;
    std::set<Symbol> ret;
    for(auto& it:to){
        if(all.find(it[0]) == all.end())
            all.insert(it[0]);
        else
            ret.insert(it[0]);
    }
    return ret;
}

/* 消除直接左递归 */
Grammar::Production Grammar::Production::eliminateLeftRecursion(){
    Production new_p(from + "'");                   // 定义新产生式
    for(auto it = to.begin();it != to.end();){      // 对所有产生式右部
        if((*it)[0].content() == from){
            std::vector<Symbol> s = (*it).elements; // 复制该条产生式右部内容
            it = to.erase(it);      // 从原产生式重删除该条产生式右部
            /* add new_satate -> xx */
            s.erase(s.begin());     // 删除左部直接左递归符号
            s.emplace_back(Symbol(new_p.from)); // 在最后插入新符号
            new_p.insert(s);        // 掺入新的产生式
            new_p.insert({Symbol(EPSILON_STR,Symbol::Type::EPSILON)});
        }else{
            (*it).append(new_p.from);   // 修改不含直接左递归的产生式右部
            it++;
        }
    }
    return new_p;
}



Grammar::Production::To& Grammar::Production::operator[](int pos){
    pos = pos < 0 ? to.size() + pos : pos;
    return to[pos];
}

/* to-string function  P->a */
std::string Grammar::Production::toString(int to_num){
    to_num = to_num < 0 ? to_num + to.size() : to_num;
    std::string out = from;
    out += "->";
    out += to[to_num].toString();
    return out;
}

/* to-string function P->a|b */
std::string Grammar::Production::toString(){
    std::string out = from;
    out += "->";
    for(int i = 0; i < to.size(); i++){
        out += to[i].toString();
        if(i != to.size() - 1)
            out += "|";
    }
    return out;
}

std::string Grammar::Production::toStringLR1(){
    std::string out = from;
    out += "->";
    if(to.size() >= 1)
        out += to[0].toStringLR1() + ", ";
    
    for(auto it:to[0].ahead)
        out += it + "/";
    out = out.substr(0,out.length() - 1);
    return out;
}



bool Grammar::Production::operator==(const Grammar::Production& p) const{
    if(to.size() != p.to.size())
        return false;
    
    for(auto& it:to){
        bool find = false;
        for(auto& t:p.to)
            if(t == it){
                find = true;
                break;
            }
        if(!find)
            return false;
    }
    return true;
}

bool Grammar::Production::operator!=(const Grammar::Production& p) const{
    return !((*this)==p);
}