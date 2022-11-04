#include "../../include/grammar.hpp"


Grammar::Production::To::To(){
    clear();
}

Grammar::Production::To::To(const Grammar::Production::To& to){
    _dot = to._dot;
    state = to.state;
    elements = std::vector<Symbol>(to.elements);
    ahead = std::set<std::string>(to.ahead);
}

/* init & clear */
void Grammar::Production::To::clear(){
    elements.clear();
    _dot = 0;
    state = State::REDUCTION;
}

Grammar::Production::To::State Grammar::Production::To::getState(){
    if(_dot == 0 && elements.size() == 0)
        state = State::REDUCTION;
    else if(_dot == elements.size())
        state = State::REDUCTION;
    else{
        if(elements[_dot].attr() == Symbol::Type::NOT_TERMINAL)
            state = State::REDUCTION_EXPECTING;
        else
            state = State::SHIFT;
    }
    return state;
}

/* append at the end */
void Grammar::Production::To::append(std::string ele){
    if(elements.size() == 1 && elements[0].attr() == Symbol::Type::EPSILON)
        elements[0] = Symbol(ele);
    else
        elements.emplace_back(Symbol(ele));
    
    // reset _dot;
    _dot = 0;
    getState();
    
}

void Grammar::Production::To::append(Symbol& s){
    elements.emplace_back(s);

    // reset _dot;
    _dot = 0;
    getState();
}


/* move dot */
Grammar::Production::To::State Grammar::Production::To::next(){
    if(_dot == elements.size())
        return State::REDUCTION;
    ++_dot;
    return getState();
}

/* operator [] */
Symbol& Grammar::Production::To::operator[](int pos){
    pos = pos < 0 ? elements.size() + pos : pos;
    return elements[pos];
}


/* judgement */
bool Grammar::Production::To::operator==(const Grammar::Production::To& t) const{
    return _dot == t._dot &&
           __equalList(elements,t.elements) &&
           __equalSet<std::string>(ahead,t.ahead);
}

/* to-string function */
std::string Grammar::Production::To::toString(){
    std::string out = "";
    for(auto& it:elements)
        out += it.content();
    return out;
}

std::string Grammar::Production::To::toStringLR1(){
    std::string out = "";
    for(int i = 0; i < elements.size(); i++){
        if(_dot == i)
            out += ".";
        out += elements[i].content();
    }
    if(_dot == elements.size())
        out += ".";
    return out;
}

/* for LR1 */
int Grammar::Production::To::getDot(){
    return _dot;
}

Symbol Grammar::Production::To::getDotSymbol(){
    if(_dot == elements.size())
        return EMPTY;
    return elements[_dot];
}

std::vector<Symbol> Grammar::Production::To::getAheadStream(){
    std::vector<Symbol> out;
    for(int i = _dot + 1; i < elements.size(); i++)
        out.emplace_back(elements[i]);
    
    return out;
}