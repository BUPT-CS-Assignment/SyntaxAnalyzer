%{
#include <stdio.h>
#include <ctype.h>
int yylex(void);
void yyerror(char *);
%}

%token digit ADD SUB MUL DIV LBK RBK LINE

%%
S : E LINE      {printf("end\n");}
  ;
E : E ADD T     {printf("E->E+T\n");}
  | E SUB T     {printf("E->E-T\n");}
  | T           {printf("E->T\n");}
  ;
T : T MUL F     {printf("T->T*F\n");}
  | T DIV F     {printf("T->T/F\n");}
  | F           {printf("T->F\n");}
  ;
F : LBK E RBK   {printf("F->(E)\n");}
  | digit       {printf("F->digit\n");}
  ;
;
%%
int main(){
    return yyparse();
}

int yywrap(){
    return 1;
}

void yyerror(char* s){
    printf("[result] %s\n",s);
}

