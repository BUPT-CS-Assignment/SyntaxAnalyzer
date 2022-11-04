#ifndef __LL_LR_HPP__
#define __LL_LR_HPP__

#include "grammar.hpp"

class Grammar::GrammarLL1{
const static int ERROR = -1;
const static int SYNCH = -2;

private:
    std::vector<Symbol>* _stream;
    int _stream_pos = -1;
    Symbol _cur_token;
    std::vector<Symbol> _state_stack;
    Symbol& _nextToken();
    std::string _outputStack();
    std::string _outputStream();
    void _outputPair();
    void _analyze();
    void _error();
    Symbol& _topState();

public:
    Grammar* G = nullptr;
    std::vector<std::vector<int>> table;
    
    GrammarLL1(Grammar*);
    bool initTable();
    void printTable();
    void analyze(std::vector<Symbol>& s_table);
};

class Grammar::GrammarLR1{
class Item;
enum class ActionType{
    ERROR,
    SHIFT,
    REDUCTION,
    ACCEPT,
};
const static int ERROR = -1;
private:
    std::vector<Item*> _colletion;
    std::vector<int> _state_stack;
    std::vector<Symbol> _symbol_stack;
    std::vector<Symbol>* _stream;
    int _stream_pos = -1;
    void _error();
    void _outputPair();
    std::string _outputStack();

    Symbol _cur_token;
    //Symbol _next_token;
    Symbol& _nextToken();
    std::string _outputStream();
    int _topState();
    Symbol& _topSymbol();
    void _analyze();

    int _findItem(Item&);
    int _findProduction(Production&);
public:
    Grammar* _G = nullptr;
    std::vector<Production> P;
    std::vector<std::vector<std::pair<ActionType,int>>> action_table;
    std::vector<std::vector<int>> goto_table;

    

    GrammarLR1(Grammar*);
    void init();
    void generateDFA();
    bool initTable();
    void printDFA();
    void printTable();
    void analyze(std::vector<Symbol>& s_table);
};

class Grammar::GrammarLR1::Item{
const static int NOT_FOUND = -3;
const static int AHEAD_CONTAIN = -2;
const static int ALL_EQUAL = -1;
private:
    Grammar::GrammarLR1* _LR1 = nullptr;
    int _find(Production&);
    void _computeAhead(Production&,Production&);

public:
    void clear();
    int _serial;
    std::vector<int> _nextTo;
    std::vector<Symbol> _nextRead;
    std::vector<Production> _productions;
    Item(GrammarLR1* lr = nullptr ,int serial = -1);
    Item(const Item&);
    int insert(Production&);
    void closure();
    void next();

    Production& operator[](int);
    Item& operator=(const Item&);
    bool operator==(const Item&) const;
    bool operator!=(const Item&) const;

    // to-string function
    std::string toString();
};


#endif