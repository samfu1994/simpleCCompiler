/*
* file name : smallc.y
* this file is syntex analizer source code
*/

%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "def.h"

#include <string>
#include <map>
#include <vector>
#include <list>
#include <stdlib.h>
#include <iostream>

extern void yyerror(const char * s);
extern int yylex(void);

// counter for unnamed structure
int unNamedStructureDef = 0;
// enviroment
int ENV	= 0;
int ENVcount = 0;
std::vector<int> ENVstack;

std::vector<std::list<int> > forStartStack;
std::vector<std::list<int> > forNextStack;

extern std::map<std::string, int> funcTable;
// extern std::vector<threeAddCodeNode> code;
// used for print err msg on checking return stmt
string currentFuncname;

std::vector<string> *params;

using namespace std;

// local functionde fine
void expBinExp(struct struct_EXP * &dd,struct struct_EXP *d1,struct struct_EXP *d2,int op);
void expRelExp(struct struct_EXP * &dd,struct struct_EXP *d1,struct struct_EXP *d2,int op);
void opAssignExp(struct struct_EXP * &dd,struct struct_EXP *d1,struct struct_EXP *d2,int op);

%}

%union{
	bool hasReturn;
	int val;
	std::string * sid;
	struct struct_VAR *svar;
	struct struct_EXP *sexp;
	std::vector<struct struct_EXP *> *sarrs;
	std::vector<struct struct_EXP *> *sargs;
	std::vector<std::string> *sstruct;
	std::vector<std::string> * ssextvars;
	std::vector<std::string> * sparas;
	std::string * sbackpatch;
}

%token <val>INT
%token <sid>ID
%token SEMI
%token COMMA

%token TYPE
%token LP
%token RP
%token LB
%token RB
%token LC
%token RC
%token STRUCT
%token RETURN

%token IF
%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

%token BREAK
%token CONT
%token FOR


%right	ASSIGN PLUSASSIGN MINUSASSIGN PRODUCTASSIGN DIVIDEASSIGN ANDASSIGN XORASSIGN ORASSIGN SLEFTASSIGN SRIGHTASSIGN
%left	LOGICALOR
%left	LOGICALAND
%left	BITOR
%left	BITXOR
%left	BITAND
%left	NOTEQUALTO EQUALTO
%left	NOGREATERTHEN LESSTHAN NOLESSTHAN GREATERTHEN
%left	SHIFTLEFT SHIFTRIGHT
%left	PLUS MINUS
%left	MODULU DIVISION PRODUCT
%right	BITNOT PREFIXDEC PREFIXINC LOGICALNOT
%left	DOT

%type <hasReturn>	STMTBLOCK STMT STMTS
%type <svar> 		VAR
%type <sstruct>		SDEFS SDECS
%type <ssextvars>	SEXTVARS
%type <sid>			STSPEC
%type <sparas>		PARAS FUNC
%type <sexp>		EXP EXPS GOTOEXP
%type <sarrs>		ARRS
%type <sarrs>		ARGS INIT
%type <sbackpatch>	BACK_PATCH

%%
PROGRAM		:	EXTDEFS
				{
					if(funcTable.find("@main") == funcTable.end()){
						recErr("ERROR: can not find \"main\" function");
					}
				}
			;
EXTDEFS		:	EXTDEF EXTDEFS
			|	/* empty */
			;
EXTDEF		:	TYPE  EXTVARS SEMI
			|	STSPEC  SEXTVARS SEMI
				{
					addStructVar(ENV,*($1),$2);
				}
			|	TYPE FUNC STMTBLOCK
				{
					// checn if stmtBlock has return
					if(!$3){
						recErr("WARRING: there is no return in function "+currentFuncname);
						genCode(OP_RET,"0","","");
					}
				}
			;
SEXTVARS	:	ID
				{
					$$ = new vector<string>;
					$$->push_back(*($1));
				}
			|	ID COMMA SEXTVARS
				{
					$$ = $3;
					$$->push_back(*($1));
				}

			|	{$$ = NULL;}
			;
EXTVARS		:	VAR
				{
					addVar(ENV,$1);
					delete $1;
				}
			|	VAR ASSIGN INIT
				{
					addVar(ENV,$1);
					addInitVal($1,$3);
					delete $1;
				}
			|	VAR COMMA EXTVARS
				{
					addVar(ENV,$1);
					delete $1;
				}
			|	VAR ASSIGN INIT COMMA EXTVARS
				{
					addVar(ENV,$1);
					addInitVal($1,$3);
					delete $1;
				}
			| /*empty */
			;

STSPEC		:	STRUCT ID LC SDEFS RC
				{
					addStruct(ENV,$2,$4);
					$$ = $2;
				}
			|	STRUCT LC SDEFS RC
				{
					$$ = new string("#"+i2s(unNamedStructureDef++));
					addStruct(ENV,$$,$3);
				}
			|	STRUCT ID
				{
					$$ = $2;
				}
			;
FUNC		:	ID LP PARAS RP
				{
					if(findSymble(ENV, "@" + *($1) ) != -1){
						recErr("ERROR: name already used.");
					}
					if($3 == NULL)
						$$ = new std::vector<std::string>;
					else
						$$ = $3;
					$$->push_back("@" + *($1));
					genLable("@" + *($1));
					params = $$;
					currentFuncname = *$1;
				}
			;
PARAS		:	TYPE ID COMMA PARAS
				{
					if($4){
						$$ = $4;
						$$->push_back(*($2));
					}
				}
			|	TYPE ID
				{
					$$ = new vector<string>;
					$$->push_back(*($2));
				}
			|
				{
					$$ = NULL;
				}
			;
STMTBLOCK	:	LC
				{
					symbleTableNode tmp;
					tmp.parent = ENV;
					tmp.name = "";
					ENVstack.push_back(ENV);
					ENV = newENV();
					symbleTable.push_back(tmp);
					// add parameters for function if any
					if(params){
						struct_VAR var;
						var.name = "__RETVAL__";
						var.type = TYPE_INT;
						addVar(ENV,&var);
						var.name = "__RETADD__";
						addVar(ENV,&var);
						var.name = "__SAVED_FP__";
						addVar(ENV,&var);
						for(int i = 0; i < 8; i++){
							var.name = "__REGVAL_"+i2s(i)+"__";
							addVar(ENV,&var);
						}
						string funcName = params->back();
						params->pop_back();
						addParams(ENV,params);
						symbleTable[ENV].name = funcName;
						funcTable[funcName] = ENV;
						delete params;
						params = NULL;
					}
				}
				DEFS STMTS RC
				{
					ENV = ENVstack.back();
					ENVstack.pop_back();
					$$ = $4;
				}
			;
STMTS		:	STMT STMTS {$$ = ($1 || $2);}
			|	{$$ = false;}
			;
STMT		:	EXP SEMI {$$ = false;}
			|	STMTBLOCK {$$ = $1;}
			|	RETURN EXP SEMI
				{
					if($2 == NULL){
						recErr("ERROR: no return value.");
					}else{
						bool2int($2);
						genCode(OP_RET, $2->addr,"","");
					}
					$$ = true;
				}
			|	IF LP EXP CV_BOOL RP BACK_PATCH STMT %prec LOWER_THEN_ELSE
				{
					backPatch($3->trueList,*$6);
					string tmp = genLable();
					backPatch($3->falseList,tmp);
					$$ = false;
				}
			|	IF LP EXP CV_BOOL RP BACK_PATCH STMT ELSE GOTOEXP BACK_PATCH STMT
				{
					backPatch($9->trueList,genLable());
					backPatch($3->trueList,*$6);
					backPatch($3->falseList,*$10);
					$$ = false;
				}
			|	FOR LP EXP SEMI BACK_PATCH EXP CV_BOOL BACK_PATCH SEMI EXP RP GOTOEXP BACK_PATCH FORSTACK STMT GOTOEXP BACK_PATCH
				{
					// FOR LP EXP SEMI BACK_PATCH EXP CV_BOOL BACK_PATCH SEMI EXP RP GOTOEXP BACK_PATCH FORSTACK STMT GOTOEXP BACK_PATCH
					// 1   2  3   4    5          6   7       8          9    10  11 12      13         14       15   16      17
					// catche err if $3 ot $10 is bool type
					if($3){
						backPatch($3->trueList,  *$5);
						backPatch($3->falseList, *$5);
					}
					if($10){
						backPatch($10->trueList, *$5);
						backPatch($10->falseList,*$5);
					}
					// control flow for for statement
					backPatch($6->trueList,*$13);
					backPatch($6->falseList,*$17);
					backPatch($12->trueList, *$5);
					backPatch($16->trueList, *$8);
					// back patch continue and braak
					backPatch(forStartStack.back(), *$8);
					backPatch(forNextStack.back(), *$17);
					forStartStack.pop_back();
					forNextStack.pop_back();
					$$ = false;
				}
			|	CONT SEMI
				{
					if(forStartStack.size()){
						forStartStack.back().push_back(
							genCode(OP_JMP,"","","CONT_BACK_PATCH") );
					}else{
						recErr("ERROR: continue not in a for loop.\n");
					}
					$$ = false;
				}
			|	BREAK SEMI
				{
					if(forNextStack.size()){
						forNextStack.back().push_back(
							genCode(OP_JMP,"","","BREAK_BACK_PATCH") );
					}else{
						recErr("ERROR: break not in a for loop.\n");
					}
					$$ = false;
				}
			;
DEFS		:	TYPE DECS SEMI DEFS
			|	STSPEC SDECS SEMI DEFS
				{
					addStructVar(ENV,*($1),$2);
				}
			|	/* empty */
			;
SDEFS		:	TYPE SDECS SEMI SDEFS
				{
					if($4 == NULL){
						$4 = new vector<string>;
					}
					for(int i = 0; i < $2->size(); i++){
						$4->push_back((*($2))[i]);
					}
					$$ = $4;
				}
			|
				{
					$$ = NULL;
				}
			;
SDECS		:	ID COMMA SDECS
				{
					$$ = $3;
					$$->push_back(*($1));
				}
			|	ID
				{
					$$ = new vector<string>;
					$$->push_back(*($1));
				}
			;
DECS		:	VAR { addVar(ENV,$1); }
			|	VAR COMMA DECS { addVar(ENV,$1); }
			|	VAR ASSIGN INIT COMMA DECS
				{
					addVar(ENV,$1);
					genCodeForInit(ENV, $1->name, $3);
				}
			|	VAR ASSIGN INIT
				{
					addVar(ENV,$1);
					genCodeForInit(ENV, $1->name, $3);
				}
			;
VAR			:	ID
				{
					$$ = new struct_VAR();
					$$->name = *($1);
					$$->type = TYPE_INT;
				}
			|	VAR LB INT RB
				{
					$$->arrayIndex.push_back($3);
					$$->type = TYPE_ARRAY;
				}
			;
INIT		:	EXP
				{
					$$ = new std::vector<struct_EXP *>;
					$$->push_back($1);
				}
			|	LC ARGS RC { $$ = $2; }
			;

EXP			:	EXPS {$$ = $1;}
			|	{$$ = NULL;}
			;

EXPS		:	LP EXPS RP { $$ = $2; }
			|	ID LP ARGS RP
				{
					genCodeForCall("@" + *$1, $3);
					$$ = new struct_EXP;
					$$->addr = newTemp();
					genCode(OP_ADD,"0","__RETVAL__",$$->addr);
				}
			|	ID ARRS
				{
					$$ = new struct_EXP;
					string last = "0";
					int env;
					std::vector<int> arr;

					env = findVar(ENV,*$1);
					if(env == -1 ){
						recErr("ERROR: var:\"" + *($1) + "\" not defined");
						$$->addr = newTemp();
					}
					if(env>=0 && $2 == NULL){
						if(symbleTable[env].v[*$1].type != TYPE_INT &&
							symbleTable[env].v[*$1].type != TYPE_PARAM &&
							symbleTable[env].v[*$1].type != TYPE_STRUCT){
							recErr("ERROR: error type");
							env = -1;

						}
						if(env != -1){
							$$->addr = *($1) + "." + i2s(env);
						}
					}else if(env != -1){
						arr = symbleTable[env].v[*$1].arrayIndex;
						if(arr.size() != $2->size()){
							recErr("ERROR : error array access, type not match.");
						}else{
							for(int i = arr.size() - 1; i > 0; i--){
								last = genCodeForExp( ((*($2))[i]) , last , OP_ADD);
								last = genCodeForExp(last,i2s(arr[arr.size() - i]),OP_MUL);
							}
							last = genCodeForExp( ((*($2))[0]) ,last,OP_ADD);
							$$->addr = *($1) + "." + i2s(env) + ":" + last;
						}
					}
					// cout << "DEBUG:\t" << $$->addr << endl;
				}
			|	ID DOT ID
				{
					int ret = getStructElementIndex(ENV,$1,$3);
					int env;
					env = findVar(ENV,*$1);
					if(ret == -1){
						recErr("ERROR: var not defined");
					}else if(ret == -2){
						recErr("ERROR: struct element not defined");
					}else if(ret == -3){
						recErr("ERROR: var is not a struct type\n");
					}

					$$ = new struct_EXP;
					if(ret >= 0){
						string tmpval = newTemp();
						$$->addr = *($1) + "." + i2s(env) + ":" + i2s(ret);
					}else{
						// error chapture;
						$$->addr = newTemp();
					}
					// cout << "DEBUG:\t" << $$->addr << endl;
				}
			|	INT
				{
					$$ = new struct_EXP;
					$$->addr = i2s($1);
					$$->isConst = true;
					$$->val = $1;
				}
			|	EXPS PRODUCT EXPS       { expBinExp($$,$1,$3,OP_MUL); }
			|	EXPS DIVISION EXPS      { expBinExp($$,$1,$3,OP_DIV); }
			|	EXPS MODULU EXPS        { expBinExp($$,$1,$3,OP_MOD); }
			|	EXPS PLUS EXPS          { expBinExp($$,$1,$3,OP_ADD); }
			|	EXPS MINUS EXPS         { expBinExp($$,$1,$3,OP_SUB); }
			|	EXPS SHIFTLEFT EXPS     { expBinExp($$,$1,$3,OP_SHL); }
			|	EXPS SHIFTRIGHT EXPS    { expBinExp($$,$1,$3,OP_SHR); }
			|	EXPS BITAND EXPS        { expBinExp($$,$1,$3,OP_BAND);}
			|	EXPS BITOR EXPS         { expBinExp($$,$1,$3,OP_BOR); }
			|	EXPS BITXOR EXPS        { expBinExp($$,$1,$3,OP_BXOR);}
			|	EXPS GREATERTHEN EXPS   { expRelExp($$,$1,$3,OP_BGT); }
			|	EXPS NOLESSTHAN EXPS    { expRelExp($$,$1,$3,OP_BGE); }
			|	EXPS LESSTHAN EXPS      { expRelExp($$,$1,$3,OP_BLT); }
			|	EXPS NOGREATERTHEN EXPS { expRelExp($$,$1,$3,OP_BLE); }
			|	EXPS EQUALTO EXPS       { expRelExp($$,$1,$3,OP_BEQ); }
			|	EXPS NOTEQUALTO EXPS    { expRelExp($$,$1,$3,OP_BNE); }
			|	EXPS PLUSASSIGN EXPS    { opAssignExp($$,$1,$3,OP_ADD); }
			|	EXPS MINUSASSIGN EXPS   { opAssignExp($$,$1,$3,OP_SUB); }
			|	EXPS PRODUCTASSIGN EXPS { opAssignExp($$,$1,$3,OP_MUL); }
			|	EXPS DIVIDEASSIGN EXPS  { opAssignExp($$,$1,$3,OP_DIV); }
			|	EXPS ANDASSIGN EXPS     { opAssignExp($$,$1,$3,OP_BAND);}
			|	EXPS XORASSIGN EXPS     { opAssignExp($$,$1,$3,OP_BXOR);}
			|	EXPS ORASSIGN EXPS      { opAssignExp($$,$1,$3,OP_BOR); }
			|	EXPS SLEFTASSIGN EXPS   { opAssignExp($$,$1,$3,OP_SHL); }
			|	EXPS SRIGHTASSIGN EXPS  { opAssignExp($$,$1,$3,OP_SHR); }
			|	EXPS ASSIGN EXPS
				{
					$$ = $3;
					bool2int($3);
					$$->addr = genCodeForAssign($3->addr,$1->addr);
				}
			|	EXPS LOGICALAND CV_BOOL2 BACK_PATCH EXPS
				{
					// int2bool($1);
					int2bool($5);
					backPatch($1->trueList, *$4);
					$$ = $5;
					$$->isBoolExp = true;
					$$->falseList.merge($1->falseList);
				}
			|	EXPS LOGICALOR CV_BOOL2 BACK_PATCH EXPS
				{
					// int2bool($1);
					int2bool($5);
					backPatch($1->falseList, *$4);
					$$ = $5;
					$$->isBoolExp = true;
					$$->trueList.merge($1->trueList);
				}
			|	LOGICALNOT EXPS
				{
					int2bool($2);
					$$ = new struct_EXP;
					$$->isBoolExp = true;
					$$->falseList = $2->trueList;
					$$->trueList = $2->falseList;
				}
			|	PREFIXINC EXPS
				{
					$$ = new struct_EXP;
					$$->addr = genCodeForExp($2,"1",OP_ADD);
					genCodeForAssign($$->addr,$2->addr);
				}
			|	PREFIXDEC EXPS
				{
					$$ = new struct_EXP;
					$$->addr = genCodeForExp($2,"1",OP_SUB);
					genCodeForAssign($$->addr,$2->addr);
				}
			|	BITNOT EXPS
				{
					bool2int($2);
					$$ = new struct_EXP;
					$$->addr = genCodeForExp($2,"",OP_BNOT);
				}
			|	MINUS EXPS
				{
					bool2int($2);
					if($$->isConst){
						$$ = $2;
						$$->val = -$$->val;
					}else{
						$$ = new struct_EXP;
						$$->addr = genCodeForExp("0",$2,OP_SUB);
					}
				}
			;

ARRS		:	LB EXP RB ARRS
				{
					if($4 == NULL) $$ = new std::vector<struct_EXP *>;
					else $$ = $4;
					$$->push_back(($2));
				}
			|	{ $$ = NULL; }
			;

ARGS		:	EXP COMMA ARGS
				{
					$$ = $3;
					$$->push_back($1);
				}
			|	EXP
				{
					$$ = new std::vector<struct_EXP *>;
					if($1)
						$$->push_back($1);
				}
			;

BACK_PATCH	:	{ $$ = new string(genLable()); }
			;

GOTOEXP		:	{
					$$ = new struct_EXP;
					$$->trueList.push_back(
						genCode(OP_JMP,"","","BACK_PATCH") );
				}
			;

FORSTACK	:	{
					forStartStack.push_back(list<int>());
					forNextStack.push_back(list<int>());
				}
			;
CV_BOOL		:	{
					int2bool($<sexp>0);
				}

CV_BOOL2	:	{
					int2bool($<sexp>-1);
				}
%%


/**
 *		NOTES FOR SYNATEX ANALISER
 *
 *		EXP
 *	for EXP, evry exp derive should garentee that the addr
 *	return to the higher layer is useable. Because error may
 *	occur in the test code, so, to get everything go right
 *	in following codes, exp should detect undecleared vars,
 *	and alway return a tme var for undecleared vars. If not,
 *	following part may find empty pointer or other odd stuff,
 *	this compiler should be robust, using this way, error
 *	check for following code is much simpler.
 *
 *
 */

void expBinExp(struct struct_EXP * &dd,struct struct_EXP *d1,struct struct_EXP *d2,int op){
	dd = new struct_EXP;
	if(d1->isConst && d2->isConst){
		dd->isConst = true;
		switch(op){
			case OP_MUL:  dd->val = d1->val * d2->val; break;
			case OP_DIV:  dd->val = d1->val / d2->val; break;
			case OP_MOD:  dd->val = d1->val % d2->val; break;
			case OP_ADD:  dd->val = d1->val + d2->val; break;
			case OP_SUB:  dd->val = d1->val - d2->val; break;
			case OP_SHL:  dd->val = d1->val <<d2->val; break;
			case OP_SHR:  dd->val = d1->val >>d2->val; break;
			case OP_BAND: dd->val = d1->val & d2->val; break;
			case OP_BOR:  dd->val = d1->val | d2->val; break;
			case OP_BXOR: dd->val = d1->val ^ d2->val; break;
			default:
				recErr("ERROR:expBinExp " + i2s(op));
		}
		dd->addr = i2s(dd->val);
	}else{
		dd->addr = genCodeForExp(d1,d2,op);
	}
}

void expRelExp(struct struct_EXP * &dd,struct struct_EXP *d1,struct struct_EXP *d2,int op){
	bool2int(d2);
	bool2int(d1);
	dd = new struct_EXP;
	dd->trueList.push_back(
		genCode(op,d1->addr,d2->addr,"BACK_PATCH") );
	dd->falseList.push_back(
		genCode(OP_JMP,"","","BACK_PATCH") );
	dd->isBoolExp = true;
}

void opAssignExp(struct struct_EXP * &dd,struct struct_EXP *d1,struct struct_EXP *d2,int op){
	dd = new struct_EXP;
	string tmp = genCodeForExp(d1,d2,op);
	dd->addr = genCodeForAssign(tmp, d1->addr);
}
