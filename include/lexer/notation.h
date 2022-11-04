#ifndef __NOTATION_H__
#define __NOTATION_H__
#include <iostream>

static std::string LIST_KEYWORD[] = {
    "auto",
    "break",
    "case","char","const","continue",
    "default","do","double",
    "else","enum","extern",
    "float","for",
    "goto",
    "if","int","inline",
    "long",
    "register","return","restrict"
    "short","signed","sizeof","static","struct","switch",
    "typedef",
    "union","unsigned",
    "void","volatile",
    "while",
    "_Bool","_Complex","_Imaginary","_Alignas","_Alignof","_Atomic","_Static_assert","_Noreturn","_Thread_local","_Generic"
};

static bool __is_keyword(const char* input){
    std::string temp(input);
    for(auto str:LIST_KEYWORD){
        if(temp == str)
            return true;
    }
    return false;
}

static std::string LIST_OPERATOR[] = {
    "=","+","-","*","/","%","&","|","!","^",
    "==","+=","-=","*=","/=","%=","&=","|=","!=","^=",
    "++","--","&&","||",
    "->"
};

static bool __is_operator(const char* input){
    std::string temp(input);
    for(int i = 0; i < LIST_OPERATOR->length(); i++){
        if(temp == LIST_OPERATOR[i])
            return true;
    }
    return false;
}

static std::string LIST_LIMIT[] = {
    ";","(",")","[","]","{","}",","
};

static bool __is_limit(const char* input){
    std::string temp(input);
    for(auto it:LIST_LIMIT){
        if(temp == it)
            return true;
    }
    return false;
}

#endif