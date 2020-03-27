以下内容来自《Lex与Yacc从入门到精通》

[wl@host122 yacc]$ cat frame.l
%{
int yywrap(void); 
%}

%%
%%
int yywrap(void) 
{ 
 return 1; 
}


[wl@host122 yacc]$ cat frame.y
%{
void yyerror(const char *s); 
%}
%debug

%%

program:
 ;
%%
void yyerror(const char *s) 
{}
int main() 
{
 yydebug=1;
 yyparse(); 
 return 0; 
}

[wl@host122 yacc]$ cat Makefile 
cc = gcc
lex = flex

test1: frame_lex.c frame_yacc.c frame_yacc.h
        cc -o $@ $^
frame_lex.c: frame.l
        lex -d -o $@ $<
frame_yacc.c: frame.y
        bison -t -o $@  -d $<

clean:
        rm -rf frame_lex.c
        rm -rf frame_yacc.c frame_yacc.h
        rm -rf test1

其中 bison -t , lex -d 都是打开debug开关用的
