/**
 *	this file contains structures used in
 *	code generation phase
 */

#ifndef	__COMPILER_PROJ2_DEF_CODEGEN__
#define __COMPILER_PROJ2_DEF_CODEGEN__

#ifndef __DEBUG__
#define __DEBUG__ 1
#endif

#include <string>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <stdlib.h>
#include <set>
using namespace std;


/*********************************************
 *	Code genetator
 ********************************************/

struct cfgnode{
	int begin;
	int end;
	int next[2];
	std::set<string> livevar;
	std::set<string> needvar;

	cfgnode(void){
		next[0] = -1;
		next[1] = -1;
	}
};

struct expTreeNode{
	expTreeNode *left;
	expTreeNode *right;
	string resname;
	int ershourNum;
	int opcode;
	int regNum;
	expTreeNode(){
		left = NULL;
		right = NULL;
	}
	expTreeNode(string name){
		resname = name;
		left = NULL;
		right = NULL;
	}
};

int genASMCode(void);

void addCode(const string & c);
void addCode(const string &op,
	const string &arg1,
	const string &arg2,
	const string &arg3);
void genHead(void);
void writeCodeToFile(const char * filename);

void createCFG(void);
int scanAndGenerateCode(void);

// CFG node analysis to generate code
map<string, expTreeNode *> genExpTree(int begin, int end);

// generate control flow graph
void genCFG(void);


// code generate
string decodeMenVar(string &varname);

#if __DEBUG__
void printCode(void);
void printCFG(void);
#endif /* __DEBUG__ */

#endif /* __COMPILER_PROJ2_DEF_CODEGEN__ */
