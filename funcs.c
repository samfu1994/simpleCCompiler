/**
 *	this file implements all utility functions
 *	used in construct three-address code.
 */

#include <stdio.h>
#include <stdlib.h>
#include "def.h"
#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

extern int ENV;
using namespace std;

/**
 *	local function protype
 */
void printImmCodeNode(const threeAddCodeNode &c,ostream&);


/**
 *	global variables
 */
vector <symbleTableNode> symbleTable;
// map funcname to a symble table
std::map<std::string, int> funcTable;
std::vector<std::string> errlist;

/**
 *	return the enviroment number in symble table
 *	corresbounding to the name and env, -1 for error
 */
int findVar(int env , string name){
	if(env < 0 || env >= symbleTable.size()){
		recErr("ERROR: error env number");
		return -1;
	}
	while(env != -1){
		if(symbleTable[env].v.find(name) != symbleTable[env].v.end()){
			return env;
		}else{
			env = symbleTable[env].parent;
		}
	}
	return -1;
}
int findSymble(int env , string name){
	if(env < 0 || env >= symbleTable.size()){
		recErr("ERROR: error env number");
		return -1;
	}
	if(funcTable.find(name) != funcTable.end()){
		return 0;
	}
	while(env != -1){
		if(symbleTable[env].v.find(name) != symbleTable[env].v.end()){
			return env;
		}
		if(symbleTable[env].s.find(name) != symbleTable[env].s.end()){
			return env;
		}
		env = symbleTable[env].parent;
	}
	return -1;
}

int findStruct(int env , string name){
	if(env < 0 || env >= symbleTable.size()){
		recErr("ERROR: error env number.");
		return -1;
	}
	while(env != -1){
		if(symbleTable[env].s.find(name) != symbleTable[env].s.end()){
			return env;
		}else{
			env = symbleTable[env].parent;
		}
	}
	return -1;
}

void addVar(int env, const struct_VAR * var){
	if(findSymble(env,var->name) == env){
		recErr("ERROR: variable redeclarition:\t"+var->name);
		// cout << env << "\t" << var->name << endl;
	}else{
		symble s;
		int size = 0;
		s.type = var->type;
		s.arrayIndex = var->arrayIndex;
		if(var->type == TYPE_INT)
			size = 1;
		else if(var->type == TYPE_ARRAY){
			size = 1;
			for(int i = 0; i < var->arrayIndex.size();i++){
				size *= var->arrayIndex[i];
			}
		}else{
			recErr("ERROR:addVar type not right");
		}
		s.size = size;
		s.index = symbleTable[env].blockSize+size-1;
		symbleTable[env].v[var->name] = s;
		symbleTable[env].blockSize += size;
	}
}

/**
 *	analyse structs;
 *
 *	map < structName, map < varName , offset > >
 *
 *	since struct can only contain intage numbers,
 *	only offset information are need.
 */

void addStruct(
	int env,
	string *name,
	vector<string> *sstruct){
	if(findSymble(env,*name) == env){
		recErr("ERROR: struct redeclarition.");
	}else{
		map<string, int > m;
		for(int i = 0; i < sstruct->size(); i++){
			if(m.find((*sstruct)[i]) != m.end()){
				recErr("ERROR: struct def redeclarition,"+*name+":"+(*sstruct)[i]);
			}else{
				m[(*sstruct)[i]] = sstruct->size()-i-1;
			}
		}
		symbleTable[env].s[*name] = m;
		symbleTable[env].sdef[*name] = *sstruct;
	}
}

void addParams(int env,vector<string> *params){
	symble tmp;
	if(params == NULL)
		return;
	tmp.type = TYPE_PARAM;
	tmp.size = 1;
	tmp.structname = "";
	for(int i = (*params).size() - 1; i >= 0 ; i--){
		if(findVar(env,(*params)[i]) == env){
			recErr("ERROR: parameters has same name");
		}else{
			tmp.index = symbleTable[env].blockSize;
			symbleTable[env].blockSize++;
			symbleTable[env].v[(*params)[i]] = tmp;
		}
	}
	// symbleTable[env].blockSize = params->size();
	symbleTable[env].params = (*params);
}


void addStructVar(int env,string sname,vector<string>*vars){
	if(vars == NULL)
			return;
	if(env < 0 || env >= symbleTable.size()){
		recErr("ERROR: no such env.");
		return;
	}
	env = findStruct(env,sname);
	if(env == -1){
		recErr("ERROR: struct \"" + sname + "\" not defined");
		return;
	}
	symble tmp;
	tmp.type = TYPE_STRUCT;
	tmp.structname = sname;
	for(int i = 0; i < vars->size(); i++){
		// if(symbleTable[env].v.find((*vars)[i]) != symbleTable[env].v.end()){
		if(findVar(env,(*vars)[i]) == env){
			recErr("ERROR: redeclarition." + sname+ ":"+(*vars)[i]);
		}else{
			tmp.size = symbleTable[env].sdef[sname].size();
			tmp.index = symbleTable[env].blockSize + tmp.size - 1;
			symbleTable[env].v[(*vars)[i]] = tmp;
			symbleTable[env].blockSize += tmp.size;
		}
	}

}

void addInitVal(struct_VAR * var,std::vector<struct struct_EXP *> * exps){
	if(var == NULL || exps == NULL){
		recErr("ERROR: NULL pointrt\n");
		return ;
	}else{
		if(var->arrayIndex.size()>1){
			recErr("WARING: initiating a multi dimentional array");
		}
		std::vector<int> inival;
		for(int i = exps->size() - 1; i >= 0 ; i--){
			if((*exps)[i]->isConst == false){
				recErr("ERROR: init value for global variable is not a const");
				return ;
			}
			inival.push_back( (*exps)[i]->val );
		}
		symbleTable[0].v[var->name].initval = inival;
	}
}

/**
 *	functions and variables for intermediate code
 *	generation all names are strings, In this project,
 *	String are used ad index for the symble table.
 *	Although an integer index is better for performance,
 *	But it is much convenient using a string.
 *
 *	      THREE  ADDRESS  CODE  DEFINE
 *	Use for segments representing a line of code.
 *  Calculations
 *		SEG 1:	int 		operation
 *		SEG 2:  string 		arg1
 *		SEG 3:  string 		arg2(optional)
 *		SEG 4:  string 		result
 *	JMP and BR operation
 *		SEG 1:	int 		operation
 *		SEG 2:	string 		arg1(optional)
 *		SEG 3:  string 		arg2(optional)
 *		SEG 4: 	string 		jmpAddress
 *
 *	JMP address are a integer represented in string.
 *	In actuall
 *	there is a map<string, ieterator> that handle
 *	this stuff.
 *
 *			RESTRICTION FOR THIS CODE
 *	I assume that all ID type varrs is address, and can
 *	be regarded as immerdiate number.
 *	only a tmpvar can be a offset for an ID address.
 *
 *	element lable is the lable for that line of code,
 *	used for jump or brance operation, an inveted
 *	lable table can also be used.
 **/
std::vector<threeAddCodeNode> code;
int tempVarCount = 0;
int tempLableCount = 0;
/**
 *	temp variables are represented like "#%D"
 *	so that normal, user defined identifers cannot
 *	coinside with the compiler defined names.
 */
string newTemp(void){
	return "#"+i2s(tempVarCount++);
}

string newLable(void){
	return "@"+i2s(tempLableCount++);
}

int newENV(void){
	extern int ENVcount;
	return ENVcount++;
}

string _nextLable = "";
int genCode(int opcode,string arg1 , string arg2,string res){
	threeAddCodeNode tmp;
	tmp.opcode = opcode;
	// to do... add error check
	tmp.arg1 = arg1;
	tmp.arg2 = arg2;
	tmp.res = res;
	tmp.env = ENV;
	if(_nextLable != ""){
		tmp.lable = _nextLable;
		_nextLable = "";
	}
	code.push_back(tmp);
	return code.size()-1;
}

/**
 *	generate lable sign for the next line of code
 *	variable string lable are shared data between
 *	genCode and genLable.
 *	that is not a good design, but since this project
 *	run just for one time and no need to maintain,
 *	I choosed this way for comvenient;
 */
void genLable(const string &lable){
	if(_nextLable != ""){
		// printf("%s\n", );("ERROR: generate two lable for the same line of code :");
		// cout << _nextLable << "\t" << lable << endl;
		genCode(OP_RET, "0","","");
	}
	_nextLable = lable;
}

string genLable(void){
	if(_nextLable == ""){
		_nextLable = newLable();
	}else{
		//printf("WARING: two lable share same line of code\n");
	}
	return _nextLable;
}

string genCodeForExp(struct_EXP *addr1, struct_EXP *addr2,int op){
	string ret;
	bool2int(addr1);
	bool2int(addr2);
	ret = newTemp();
	genCode(op,
		addr1->addr,addr2->addr,ret);
	return ret;
}
string genCodeForExp(string addr1, struct_EXP *addr2,int op){
	string ret;
	bool2int(addr2);
	ret = newTemp();
	genCode(op,addr1,addr2->addr,ret);
	return ret;
}

string genCodeForExp(struct_EXP *addr1, string addr2,int op){
	string ret;
	bool2int(addr1);
	ret = newTemp();
	genCode(op,addr1->addr,addr2,ret);
	return ret;
}
string genCodeForExp(string addr1, string addr2,int op){
	string ret = newTemp();
	genCode(op,addr1,addr2,ret);
	return ret;
}

string genCodeForAssign(const string src, const string &dst){
	bool srcIsInt = false, dstIsInt = false;
	if(dst[0] == '#' || IS_NUM(dst[0]) ){
		recErr("ERROR: expression \"" + dst	+ "\"cannot be a left value.");
		return newTemp();
	}
	int envdst = findVar(ENV,scanName(dst));
	int envsrc = -1;
	if(envdst == -1){
		recErr("ERROR: variable "+dst+" not decleared.");
		return newTemp();
	}

	// get dst information
	symble sdst = symbleTable[envdst].v[scanName(dst)];
	symble ssrc;

	if(sdst.type == TYPE_STRUCT && scanOffset(dst) == ""){
		dstIsInt = false;
	}else{
		dstIsInt = true;
	}
	// get src information
	if(src[0] == '#' || IS_NUM(src[0])){
		srcIsInt = true;
	}else{
		envsrc = findVar(ENV, scanName(src));
		ssrc = symbleTable[envsrc].v[scanName(src)];
		if(ssrc.type == TYPE_STRUCT && scanOffset(src) == ""){
			srcIsInt = false;
		}else{
			srcIsInt = true;
		}
	}
	if(srcIsInt != dstIsInt){
		recErr("ERROR: variable type not match " + src + "  " + dst);
		return newTemp();
	}

	// handle init assign and struct assign
	if(srcIsInt){
		genCode(OP_ST, src, "",dst);
	}else{
		// handle struct assign
		if(ssrc.structname != sdst.structname){
			recErr("ERROR struct assign, type not match");
			return newTemp();
		}
		int envstr = findStruct(ENV,ssrc.structname);
		int ln = symbleTable[envstr].sdef[ssrc.structname].size();
		for(int i = 0; i < ln; i++){
			genCode(OP_ST,src+":"+i2s(i),"",dst+":"+i2s(i));
		}
	}
	return src;
}

void genCodeForInit(int env, string name, std::vector<struct struct_EXP *>* init){
	if(findVar(env,name) != env || init == NULL || init->size() == 0){
		recErr("ERROR:init error.");
		return;
	}
	const symble &s = symbleTable[env].v[name];
	if(s.type == TYPE_INT){
		if(init->size() > 1){
			return;
		}else{
			genCodeForAssign((*init)[0]->addr, name + "." + i2s(env));
		}
	}else if(s.type == TYPE_ARRAY){
		if(s.arrayIndex.size() != 1){
			recErr("ERROR: cannot init var:" + name);
		}else if(init->size() <= s.arrayIndex[0]){
			for(int i = init->size() - 1; i >= 0 ; i-- ){
				genCodeForAssign((*init)[i]->addr,
					name + "." + i2s(env) + ":" + i2s(init->size() - 1 - i));
			}
		}else{
			recErr("ERROR: init block larger then arrayIndex.");
		}
	}
}

int genCodeForCall(
	string name,
	std::vector<struct struct_EXP *>* arg){
	if(arg == NULL){
		recErr("ERROR: NULL pointrt");
		return -1;
	}
	if(funcTable.find(name) == funcTable.end()){
		recErr("ERROR: no such function\n");
		return -1;
	}
	int env = funcTable[name];
	if(symbleTable[env].params.size() != arg->size()){
		recErr("ERROR: params size not right, in function " + name);
		return 0;
	}
	for(int i = arg->size()-1; i >= 0; i--){
		genCode(OP_PARA, (*arg)[i]->addr,"","");
	}
	genCode(OP_CALL, "", "", name);
}
/**
 *	Get offset for a struct lelment.
 *
 *	Note that the structure name stored in
 *	symble.structname can always be found in either env
 *	for courent function or "/" env for global. THis is
 *	garenteed in function "addStructVar". Refer to function
 *	addStructVar for more details.
 */
int getStructElementIndex(int env,const string *var,const string *ele){
	if(var == NULL || ele == NULL){
		recErr("ERROR:NULL STRING*.");
		return -1000;
	}
	int envVar = findVar(env , *var);
	int envStruct;
	if(envVar == -1){
		return -1;
	}
	const symble &t = symbleTable[envVar].v[*var];
	if(t.type != TYPE_STRUCT){
		// not a struct
		return -3;
	}
	envStruct = findStruct(envVar,t.structname);
	const std::map<string, int> m = symbleTable[envStruct].s[t.structname];
	if(m.find(*ele) == m.end()){
		// handled by caller
		// struct has no such element
		return -2;
	}else{
		return m.find(*ele)->second;
	}
}

void backPatch(std::list<int> &l, string lable){
	while(l.size()){
		code[l.back()].res = lable;
		l.pop_back();
	}
}

void bool2int(struct struct_EXP * exps){
	if(exps == NULL){
		return;
	}
	if(exps->isBoolExp){
		int i;
		exps->addr = newTemp();
		backPatch(exps->trueList,genLable());
		genCode(OP_ADD,"0","1",exps->addr);
		i = genCode(OP_JMP,"","","backPatch");
		backPatch(exps->falseList,genLable());
		genCode(OP_ADD,"0","0",exps->addr);
		code[i].res = genLable();
		exps->isBoolExp = false;
	}
}

void int2bool(struct struct_EXP * &exps){
	if(exps == NULL){
		exps = new struct_EXP;
		exps->trueList.push_back(
			genCode(OP_JMP,"","","backPatch"));
		exps->isBoolExp = true;
	}else if(! exps->isBoolExp){
		exps->falseList.push_back(
			genCode(OP_BEQ, exps->addr,"0","backPatch"));
		exps->trueList.push_back(
			genCode(OP_JMP,"","","backPatch"));
		exps->isBoolExp = true;
		exps->addr = "";
	}
}

string op2s(int opcode){
	char tmp[50];
	switch(opcode){
		case OP_ST:    return "ST";
		case OP_ADD:   return "ADD";
		case OP_SUB:   return "SUB";
		case OP_MUL:   return "MUL";
		case OP_DIV:   return "DIV";
		case OP_MOD:   return "MOD";
		case OP_SHL:   return "SHL";
		case OP_SHR:   return "SHR";
		case OP_BAND:  return "BAND";
		case OP_BOR:   return "BOR";
		case OP_BNOT:  return "BNOT";
		case OP_BXOR:  return "BXOR";
		case OP_BGE:   return "BGE";
		case OP_BGT:   return "BGT";
		case OP_BLT:   return "BLT";
		case OP_BLE:   return "BLE";
		case OP_BEQ:   return "BEQ";
		case OP_BNE:   return "BNE";
		case OP_JMP:   return "JMP";
		case OP_PARA:  return  "PARA";
		case OP_RET:   return  "RET";
		case OP_CALL:  return  "CALL";
		default:       return "UNKNOWN_OPCODE";
	}
}

void writeIRtofile(const char * name){
	if(name == "")
		return;
	ofstream fout(name);
	for(int i = 0; i < code.size(); i++){
		// printf("%5d:",i );
		printImmCodeNode(code[i],fout);
	}
}

#if __DEBUG__


void printImmCodeNode(const threeAddCodeNode &c, ostream &o = cout){
	o <<left << setw(3) << c.env;
	o <<"  " << left <<setw(5) <<  c.lable << "\t";
	o <<left << setw(8) << op2s(c.opcode);
	o <<left << setw(8) << c.arg1 + ",";
	o <<left << setw(8) << c.arg2 + "," ;
	o <<left << setw(8) << c.res << endl;
}

void printIntermediateCode(void){
	for(int i = 0; i < code.size(); i++){
		printf("%5d:",i );
		printImmCodeNode(code[i]);
	}
}

void printVar(void){
	for(int k = 0; k < symbleTable.size(); k++){
		cout << "\t"
			<< k <<":" << symbleTable[k].name
			<< ":" << symbleTable[k].parent
			<< ":" << symbleTable[k].blockSize << endl;
		for(map<string ,symble>::iterator iit = symbleTable[k].v.begin();
			iit != symbleTable[k].v.end();
			++iit){
			cout << "\t\t";
			if(iit->second.type == TYPE_INT)
				cout << setw(10) << "<INT>";
			else if(iit->second.type == TYPE_ARRAY)
				cout << setw(10) << "<ARRAY>";
			else if(iit->second.type == TYPE_STRUCT){
				cout << setw(10) << "<STRUCT>";
				// cout << iit->second.structname << "\t";
			}else if(iit->second.type == TYPE_PARAM)
				cout << setw(10) << "<PARAM>";
			else
				cout << setw(10) << "ERR TYPR";

			cout<< setw(10) << iit->first << "\t";
			// for(int i = 0; i < iit->second.arrayIndex.size();i++){
			// 	printf("[%d]", iit->second.arrayIndex[i]);
			// }
			// for(int i = 0; i < iit->second.initval.size(); i++){
			// 	printf("%d,", iit->second.initval[i]);
			// }
			cout << setw(10) << iit->second.index << setw(10) << iit->second.size;
			printf("\n");
		}
	}
}

void printStruct(void){
	for(int k = 0; k < symbleTable.size(); k++){
		cout <<"\t" << k<< ":" << symbleTable[k].name << endl;
		for(map< string , map < string,int > >::iterator i = symbleTable[k].s.begin();
			i != symbleTable[k].s.end();
			++i){
			cout << "\t\t" <<  i->first << endl;
			for(map <string,int>::iterator j = i->second.begin();
				j != i->second.end();
				++j){
				cout<< "\t\t\t" << j->first <<"\t" <<  j->second << endl;
			}
		}
	}
}

void printSymbleTable(void){
	printf("STRUCT DEFINE:\n");
	printStruct();
	printf("\nVAR DEFINE\n");
	printVar();
}
#endif /* __DEBUG__ */


/**
 *	INIT system initial function:
 */
void sysInit(void){
	extern int ENV;
	extern std::vector<int> ENVstack;
	symbleTableNode tmp;

	tmp.parent = -1;
	tmp.name = "___GLOBAL___";
	symbleTable.push_back(tmp);
	ENV = newENV();
	ENVstack.push_back(ENV);
	// add read and write function
	int env = newENV();
	tmp.parent = 0;
	tmp.name = "@read";
	symbleTable.push_back(tmp);
	funcTable["@read"] = env;
	vector<string> params;
	params.push_back("x");
	addParams(env,&params);
	env = newENV();
	tmp.parent = 0;
	tmp.name = "@write";
	symbleTable.push_back(tmp);
	funcTable["@write"] = env;
	addParams(env,&params);

	// set tmp var counter
	tempVarCount = 0;
}

/**
 *	other things
 */

string i2s(int val){
	char buf[20];
	sprintf(buf,"%d",val);
	return buf;
}

void recErr(string s){
	errlist.push_back(s);
}

void printErr(void){
	for(int i = 0; i < errlist.size(); i++)
		cerr << errlist[i] << endl;
}
