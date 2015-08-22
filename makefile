projct: scc


scc:	y.tab.o lex.yy.o main.o funcs.o codegen.o
	g++ y.tab.o lex.yy.o funcs.o main.o codegen.o -o scc

main.o: main.c def.h y.tab.h defCodegen.h
	g++ -c main.c -o main.o

lex.yy.o:	lex.yy.c y.tab.h
	g++ -c lex.yy.c -o lex.yy.o

lex.yy.c: smallc.l def.h
	flex smallc.l

y.tab.o: y.tab.c
	g++ -c y.tab.c -o y.tab.o

y.tab.c: smallc.y def.h
	yacc -d -v smallc.y

#y.tab.h: smallc.y lex.yy.c
#	yacc -d -v smallc.y

funcs.o: funcs.c def.h
	g++ -c funcs.c -o funcs.o

codegen.o: def.h codegen.c defCodegen.h
	g++ -c codegen.c -o codegen.o

run:	scc
	./scc testcode.c

clean:
	rm lex.yy.c y.tab.c y.tab.h *.output *.o scc *.gv *.xdot *.s InterCode *.exe
