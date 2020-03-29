%{ 

#include <stdio.h>
#include<string.h>
#include "cal_yacc.h" 

extern int yylval; 

int
yywrap(void)
{
    return 1;
}

%} 

%% 

"+" return PLUS;
"-" return MINUS;
"*" return TIMES;
 "\n"  return CR;
"\r" return CR;


([1-9][0-9]*)|0|([0-9]+\.[0-9]*) {
    sscanf(yytext, "%d", &yylval);
    return INTEGER;
}

[ \t] ;

. {
    printf("------%s\n", yylval);
    printf("Error");
  } 


%% 
