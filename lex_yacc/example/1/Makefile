cal: cal_yacc.c cal_lex.c
	gcc -o $@ $^

cal_yacc.c: cal.y
	yacc -d -v -o $@ $<

cal_lex.c : cal.l
	lex -o $@  $<

clean:
	rm -rf cal cal_yacc.c cal_yacc.h cal_yacc.output cal_lex.c
