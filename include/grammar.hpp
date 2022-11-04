#ifndef __Grammar_HPP__
#define __Grammar_HPP__

#include <iostream>
#include <vector>
#include <set>
#include "lexer/scanner.hpp"

static std::string EMPTY_STR = "";
static std::string EPSILON_STR = "~";
static std::string ENDING_STR = "$";
static std::string PRODUCTION_DIVIDE = "#";
static std::string PRODUCTION_BEGIN = "@";
static Symbol EPSILON = Symbol("~",Symbol::Type::EPSILON);
static Symbol ENDING = Symbol("$",Symbol::Type::END); 
static Symbol EMPTY = Symbol("",Symbol::Type::END);

class Grammar{
class GrammarLL1;
class GrammarLR1;
public:
    class Production;

private:
    Scanner _sc;
    GrammarLL1* _mode_ll1 = nullptr;
    GrammarLR1* _mode_lr1 = nullptr;
    void _printSet(std::vector<std::set<std::string>>*);

public:
    std::string start;
    std::vector<std::string> T;
    std::vector<std::string> N;
    std::vector<Production> P;
    std::vector<std::set<std::string>> FIRST;
    std::vector<std::set<std::string>> FOLLOW;

    Grammar();
    Grammar(std::string start,std::vector<std::string> N,std::vector<std::string> T);
    void init(const char* file_name);

    /* get */
    Production& operator [](std::string from);
    Production& operator [](int);
    int operator()(std::string from);
    int operator()(Symbol&);

    int _findT(std::string);
    int _findN(std::string);

    /* set */
    bool setStart(std::string);
    bool addN(std::string);
    // bool removeN(std::string);

    // insert production
    void insert(std::string from, std::vector<std::string> to);
    void insert(std::string from, std::vector<Symbol> to);
    void insert(Production& p);

    // first & follow
    void computeFirstTable();
    void computeFollowTable();
    

    std::set<std::string>& findFirst(std::string N);
    std::set<std::string> findFirst(std::vector<Symbol>);
    std::set<std::string>& findFollow(std::string N);
    
    bool findFirst(std::string N, std::string c);
    bool findFollow(std::string N, std::string c);

    void printFirst();
    void printFollow();
    void printGrammar();
    
    /* LL1 */
    // eliminate left recursion
    void eliminateLeftRecursion();
    // extract left common factor
    void extractLeftCommonFactor();

    /* LR1 */
    void expandGrammar();
    void initLR1StateNumber();

    /* main */
    void runLL1();
    void runLR1();
};

class Grammar::Production{
public:
    class To;
private:
    Scanner* __sc = nullptr;
    void _clear();
    bool _exist(To&);
public:
    /* construction and initialize */
    Production(std::string from = "");
    Production(const Production&);
    
    /* get element */
    std::string from;
    std::vector<To> to;
    To& operator [](int pos);
    bool operator==(const Production&) const;
    bool operator!=(const Production&) const;

    /* operate element */
    bool insert(std::vector<std::string> to);
    bool insert(std::vector<Symbol> to);
    bool insert(To);
    
    /* checking */
    bool existLeftRecursion();
    std::set<Symbol> existLeftCommonFactor();

    /* option */
    Production eliminateLeftRecursion();

    /* to string */
    std::string toString(int to);
    std::string toString();
    std::string toStringLR1();
};


class Grammar::Production::To{
public:
enum class State{
    SHIFT,
    REDUCTION_EXPECTING,
    REDUCTION,
};
private:
    int _dot = 0;
public:
    State state;
    std::vector<Symbol> elements;
    std::set<std::string> ahead;
    To();
    To(const To&);
    void clear();

    /* option */
    void append(std::string);
    void append(Symbol&);

    /* for LR1 */
    int getDot();
    State getState();
    State next();
    Symbol getDotSymbol();
    std::vector<Symbol> getAheadStream();


    /* operator */
    Symbol& operator[](int pos);
    bool operator == (const To&) const;

    /* to string */
    std::string toString();
    std::string toStringLR1();
};


#endif