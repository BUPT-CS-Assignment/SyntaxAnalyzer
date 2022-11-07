%{/*2022-11-07*/
#include <stdlib.h>
#include <stdio.h>
#include "y.tab.h"
void yyerror(char *);
%}

NUM         [0-9]
digit       ({NUM}*(".")?{NUM}*([Ee][+-]|[Ee])?{NUM}+)|({NUM}+)

ADD         "+"
SUB         "-"
MUL         "*"
DIV         "/"
LBK         "("
RBK         ")"
LINE        [\r\n]

%%
{digit}     {return digit;}
{LBK}       {return LBK;}
{RBK}       {return RBK;}
{ADD}       {return ADD;}
{SUB}       {return SUB;}
{MUL}       {return MUL;}
{DIV}       {return DIV;}
{LINE}      {return LINE;}
.           {}

%%
