#include <stdio.h>
#include <stdlib.h>
#include "def.h"
#include "defCodegen.h"
#include <iostream>

using namespace std;

extern FILE* yyin;
extern int yyparse(void);

void testFunc();

int main(int argc , char ** argv){
	testFunc();
	if(argc >1)
		yyin = fopen( argv[1], "r" );
	else{
		yyin = stdin;
	}
	sysInit();
	int res = yyparse();
	writeIRtofile("InterCode");
	res |= genASMCode();
	if(res == 0){
		writeCodeToFile("MIPSCode.s");
	printErr();
#if __DEBUG__
	// printSymbleTable();
	// printf("\n\n");
	// printIntermediateCode();
	// printf("\n\n");
	// printCode();
	// printCFG();
#endif /* #ifdef __DEBUG__ */
	}
	return 0;
}


void testFunc(){
	// for(int i = 0; i < 20; i++){
	// 	cout << newTemp() << endl;
	// }
	for(int i = 0; i < 10; i++){
		//printf("%d\n", newENV());
	}
}
