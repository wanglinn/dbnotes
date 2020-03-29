%{

#include <stdio.h>
#define YYSTYPE int 


int yyparse(void);
#define YYDEBUG 1
%}

%token INTEGER PLUS MINUS TIMES DIVIDE LP RP CR

%%


line_list: line
    | line_list line
   ;

line :  exp CR 
            {
              printf("%d\n",$1);
            }
          ;
exp:
    factor
    {  $$=$1; }
  |  factor PLUS factor
     {
       printf("%d %d\n", $1, $3);
       $$ = $1 + $3;
     }
  ;


factor : INTEGER {$$ = $1;}
       ;
%%
int main()
{
   int yydebug = 1;

  extern int yyparse(void);
  extern FILE *yyin;
   yyin = stdin;

  if (yyparse()) {
   fprintf(stderr, "Error ! Error ! Error !\n");
   exit(1);
  }
}

void yyerror(char* s)
{
  extern char *yytext;
   fprintf(stderr,"%s",yytext);
}

