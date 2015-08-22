#include "def.h"
#include "defCodegen.h"
#include <string>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>


using namespace std;

std::vector<string> asmCode;
std::map<int,string> asmComment;
extern std::vector<threeAddCodeNode> code;

map<string, int> tmpVarUseagerec;

/*
 local functions
*/
void addComment(const string &);
void getTmpVarscop(void);

// generate code for a tree, return register that store value
// TODO .. add code here
string transThreeToCode(int opcode){
	switch(opcode){
	case OP_ADD: return "add";
	case OP_SUB: return "sub";
	case OP_MUL: return "mul";
	case OP_DIV: return "div";
	case OP_MOD: return "FUCK";
	case OP_SHL: return "sll";
	case OP_SHR: return "srav";
	case OP_BAND: return "and";
	case OP_BOR:  return "or";
	case OP_BNOT: return "not";
	case OP_BXOR: return "xor";
	case OP_BGE: return "bge";
	case OP_BGT: return "bgt";
	case OP_BLT: return "blt";
	case OP_BLE: return "ble";
	case OP_BEQ: return "beq";
	case OP_BNE: return "bne";
	default : return "ERROR opcode";
	}
	return "ERROR opcode";
}

// scan and generate code
vector<int>envstack;
// memage dynamic area for
void genEnvforLocalvar(int env){
	// size of stack allocation
	int size = 0;
	if(env == envstack.back())
		return;
	vector<int>tmp;
	while(env != 0){
		tmp.push_back(env);
		env = symbleTable[env].parent;
	}
	while(envstack.size()>1){
		size += symbleTable[envstack.back()].blockSize;
		envstack.pop_back();
	}
	for(int i = tmp.size()-1; i >=0 ; i--){
		size -= symbleTable[tmp[i]].blockSize;
		envstack.push_back(tmp[i]);
	}
	if(size){
		addCode("\tadd\t$sp,\t$sp,\t" + i2s(size*4) );
		addComment("ENV SETUP: " + i2s(envstack.back()));
	}
}

// free space allocated for function
// call this on return
void freeSpace(void){
	int size = 0;
	while(envstack.size()>1)
		envstack.pop_back();
}

// ONLY MEMORY variable is handled here
inline bool	LABLE_IS_TMP (string &s){return (s != "" && s[1] <= '9' && s[1] >= '0');}
inline bool OFFSET_IS_TMP(string &s){return (s != "" && s[0] == '#') ;}
inline bool OFFSET_IS_NUM(string &s){return (s != "" && s[1] <= '9' && s[0] >= '0');}
inline bool VAR_IS_TMP(string &s){return (s != "" && s[0] == '#');}
inline bool VAR_IS_NUM(string &s) {return(s != "" && s[0]>='0' && s[0]<='9');}
inline bool VAR_IS_MEM(string &s) {return (s != "" && !VAR_IS_TMP(s) && !VAR_IS_NUM(s));}
inline string transLable(const string & l){
	return "L_" + l.substr(1,l.size()-1);
}
inline string transGVar(const string & l){
	return "VAR_" + scanName(l);
}

// map<string, expTreeNode*> tmap;
// only allocate space of regmap for temp value;
map<string, int> regmap;
set<int>regfree;
int allocateReg(string s){
	if(s != "" && regmap.find(s)!= regmap.end()){
		return regmap[s];
	}
	if(regfree.size() == 0){
		// todo .. add code for unsufferent register
		return 9;
	}else{
		int reg = *regfree.begin();
		regfree.erase(reg);
		if(s != ""){
			regmap[s] = reg;
		}
		return reg;
	}
}
void freereg(string s){
	if(regmap.find(s)!= regmap.end()){
		regfree.insert(regmap[s]);
		regmap.erase(s);
	}
}
void freereg(int reg){
	if(reg < 0){
		return;
	}
	regfree.insert(reg);
}
void freeNoUseReg(int lineno){
	map<string,int>::iterator t;
	for(map<string,int>::iterator i = regmap.begin();
		i != regmap.end();
		++i){
		// cout << "DEB " << i->first << endl;
		t = tmpVarUseagerec.find(i->first);
		if(t != tmpVarUseagerec.end() && t->second < lineno){
			// cout << i->first << "\t" << i->second << endl;
			regfree.insert(i->second);
			regmap.erase(i);
			if(regmap.size() == 0)
				return;
			i = regmap.begin();
		}
	}
	// cout << "EXIT FUNC" << endl;
}
// this function only use free Reg, do not free reg.
string genVar(string &varname){
	if(varname == ""){
		return "-----";
	}
	// cout << varname << endl;
	if(varname == "__RETVAL__"){
		return "$v0";
	}else if(VAR_IS_NUM(varname)){
		int reg = allocateReg(varname);
		addCode("\tadd\t$t"+i2s(reg)+",\t$0,\t"+varname);
		return "$t"+i2s(reg);
	}else if(VAR_IS_TMP(varname)){
		if(regmap.find(varname)==regmap.end()){
			recErr("ERROR: tmp vars refered before assign");
			return "$t9";
		}else{
			int reg = regmap[varname];
			return "$t"+i2s(reg);
		}
	}else{
		string ldvar = decodeMenVar(varname);
		// cout << varname << "\t" << ldvar << endl;
		int reg = (allocateReg(varname));
		addCode("\tlw\t$t"+i2s(reg)+",\t"+ldvar);
		return "$t"+i2s(reg);
	}
	recErr("ERROR: tmp vars refered before assign");
	return "$t9";
}

string getAddr(string varname){
	if(varname == "" || varname[0] == '@')
		return "";
	int reg = allocateReg("__TMP__");
	addCode("\tla\t$t"+i2s(reg)+",\t"+decodeMenVar(varname));
	freereg(reg);
	return "$t"+i2s(reg);
}

// generate a string that can be used in MIPS CODE
// reserve t9 fot loading address for variables
// FIXME... add code for generate code for dstvar
string decodeMenVar(string &varname){
	// cout << varname << endl;
	if(varname == "" || varname[0] == '@'){
		return "";
	}
	int env = scanEnv(varname);
	// int addoffset = -symbleTable[env].v[scanName(varname)].index * 4;
	int addoffset = 0, ev = symbleTable[env].parent;
	addoffset -= symbleTable[env].v[scanName(varname)].index;
	while(ev != 0 && ev != -1){
		addoffset -= symbleTable[ev].blockSize;
		ev = symbleTable[ev].parent;
	}
	addoffset *= 4;
	string offset = scanOffset(varname);
	if(env>0){
		if(offset == ""){
			return (i2s(addoffset)+"($fp)");
		}else if(VAR_IS_NUM(offset)){
			addoffset -= atoi(offset.data()) * 4;
			return (i2s(addoffset)+"($fp)");
		}else{
			string reg = i2s(allocateReg(offset));
			freereg(offset);
			string r = i2s(allocateReg(varname));
			addCode("\tsll\t$t"+reg+",\t$t"+reg+",\t2");
			addCode("\tsub\t$t"+r+",\t$fp,\t$t"+reg);
			addComment("gen dst var : "+varname);
			return i2s(addoffset) + "($t"+r+")";
		}
	}else{
		if(offset == ""){
			return (transGVar(scanName(varname)));
		}else if(VAR_IS_NUM(offset)){
			addoffset = atoi(offset.data()) * 4;
			return (transGVar(scanName(varname))+"+"+i2s(addoffset));
		}else{
			string reg = i2s(allocateReg(offset));
			addCode("\tsll\t$t"+reg+",\t$t"+reg+",\t2");
			freereg(offset);
			return transGVar(scanName(varname))+"($t"+reg+")";
		}
	}
	return "ERROR";
}

// return 0 for fail, 1 for success
// bool structAssign(string arg1, string arg2){
// 	if(!VAR_IS_MEM(arg1) || !VAR_IS_MEM(arg2))
// 		return false;
// 	int ev1 = scanEnv(arg1);
// 	int ev2 = scanEnv(arg2);
// 	string n1 = scanName(arg1);
// 	string n2 = scanName(arg2);
// 	if(symbleTable[ev1].v[n1].type != TYPE_STRUCT ||
// 		symbleTable[ev2].v[n2].type != TYPE_STRUCT){
// 		return false;
// 	}
// 	if(scanOffset(arg1)!= "" || scanOffset(arg2) != "")
// 		return false;
// 	string sn1 = symbleTable[ev1].v[n1].structname;
// 	string sn2 = symbleTable[ev2].v[n2].structname;
// 	if(sn1 == "" || sn1 != sn2){
// 		recErr(" struct type not match");
// 		return true;
// 	}
// 	int evs = findStruct(ev1,sn1);
// 	int ln = symbleTable[evs].sdef[sn1].size();
// 	// printf("SIZZZ: %d\n", ln);
// 	for(int i = 0; i < ln ; i++){
// 	}
// 	return true;
// }

int scanAndGenerateCode(void){
	int paramCounter = 11;
	int l;
	l = code.size();
	for(int i = 0; i < l; i++){
		// printf("%d\n", i);
		if(code[i].lable != ""){
			addCode(transLable(code[i].lable)+":");
			if(!LABLE_IS_TMP(code[i].lable)){
				freeSpace();
				// store current fp
				addCode("\tsw\t$fp\t-8($sp)");
				addComment("store current fp");
				// adjust fp to current sp;
				addCode("\tadd\t$fp,\t$0,\t$sp");
				addComment("adjust fp to current sp;");
				addCode("\tsw\t$ra,\t-4($fp)");
				addComment("store return address");
			}
		}
		genEnvforLocalvar(code[i].env); // order with lable sensitive
		string arg1 = genVar(code[i].arg1);
		string arg2 = genVar(code[i].arg2);
		freereg(code[i].arg1);
		freereg(code[i].arg2);
		switch(code[i].opcode){
		case OP_ST:
			{
				// if(!structAssign(code[i].arg1,code[i].res) ){
					addCode("\tsw\t"+(arg1)+",\t"+decodeMenVar(code[i].res));
					addComment("ST: line "+i2s(i));
				// }
				break;
			}
		case OP_JMP:
			addCode("\tj\t"+ transLable(code[i].res) );
			break;
		case OP_CALL:
			if(code[i].res == "@read"){
				addCode("\tsw\t"+getAddr(code[i-1].arg1)+",\t-"+i2s(paramCounter*4)+"($sp)");
				addComment("PARAM : " + code[i].arg1);
			}
			{	int t = 3;
				for(map<string,int>::iterator ii = regmap.begin();
					ii != regmap.end();
					++ii){
					addCode("	sw	$t"+i2s(ii->second)+"\t-"+i2s(t*4)+"($fp)");
					++t;
				}
			}
			addCode("\tjal\t"+transLable(code[i].res));
			{	int t = 3;
				for(map<string,int>::iterator ii = regmap.begin();
					ii != regmap.end();
					++ii){
					addCode("	lw	$t"+i2s(ii->second)+"\t-"+i2s(t*4)+"($fp)");
					++t;
				}
			}
			paramCounter = 11;
			break;
		case OP_PARA:
			if(code[i+1].res!="@read"){
				addCode("\tsw\t"+arg1+",\t-"+i2s(paramCounter*4)+"($sp)");
				addComment("PARAM : " + code[i].arg1);
				paramCounter++;
			}
			break;
		case OP_RET:
			// store return value;
			addCode("\tlw\t$ra,\t-4($fp)");
			addComment("restore return addres");
			addCode("\tadd\t$v0,\t$0,\t"+arg1);
			addComment("store return value" + code[i].arg1);
			addCode("\tadd\t$sp,\t$0,\t$fp");
			addComment("freeSpace");
			// restore fp
			addCode("\tlw\t$fp,\t-8($fp)");
			addComment("restore fp");
			addCode("\tjr\t$ra");
			break;
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_BOR:
		case OP_BXOR:
		case OP_SHL:
		case OP_SHR:
		case OP_BAND:
		case OP_DIV:
			allocateReg(code[i].res);
			addCode("\t"+
				transThreeToCode(code[i].opcode)+
				"\t"+genVar(code[i].res)+",\t"+arg1+
				",\t"+arg2);
			addComment(i2s(i)+":"+code[i].arg1+":"+code[i].arg2);
			break;
		case OP_MOD:
			allocateReg(code[i].res);
			addCode("\tdiv\t"+arg1+",\t"+arg2);
			addCode("\tmfhi\t"+genVar(code[i].res));
			addComment(i2s(i)+":"+code[i].arg1+":"+code[i].arg2);
			break;
		case OP_BNOT:
			allocateReg(code[i].res);
			addCode("\t"+
				transThreeToCode(code[i].opcode)+
				"\t"+genVar(code[i].res)+",\t"+arg1);
			addComment(i2s(i)+":"+code[i].arg1+":"+code[i].arg2);
			break;
		case OP_BGE:
		case OP_BGT:
		case OP_BLT:
		case OP_BLE:
		case OP_BEQ:
		case OP_BNE:
			addCode("\t"+transThreeToCode(code[i].opcode)+"\t"+arg1+",\t"+arg2+",\t"+transLable(code[i].res));
			break;
		default:
			addCode("# "+i2s(i)+" not handled");
		}
		freeNoUseReg(i);
	}
}

int genASMCode(void){
	envstack.push_back(0);
	for(int i = 0; i < 8; i++){
		regfree.insert(i);
	}
	getTmpVarscop();
	// cout << endl << "MAPSIZE::::" << tmpVarUseagerec.size() << endl;
	// for(map<string,int>::iterator ii = tmpVarUseagerec.begin();
	// 	ii != tmpVarUseagerec.end();
	// 	++ii){
	// 	cout << ii->first << "\t" << ii->second << endl;
	// }
	genHead();
	// createCFG();
	scanAndGenerateCode();
	// genEnvforLocalvar(0);
	// add code for read and write
	addCode("L_read:");
	addCode("	lw	$t0,	-44($sp)");
	addCode("	li	$v0,	5");
	addCode("	syscall");
	addCode("	sw	$v0,	($t0)");
	addCode("	jr	$ra");
	// add code for write
	addCode("L_write:");
	addCode("	lw	$a0,	-44($sp)");
	addCode("	li	$v0,	1");
	addCode("	syscall");
	// addCode("	sw	$v0,	($t0)");
	addCode("	li $v0, 4");
	addCode("	la $a0, str");
	addCode("	syscall");
	addCode("	jr	$ra");
	return 0;
}

void addComment(const string &t){
	asmComment[asmCode.size()-1] = t;
}
void addCode(const string &c){
	asmCode.push_back(c);
}

void genHead(void){
	addCode("\t.data");
	string s = "";
	int j;
	for(map<string ,symble>::iterator i = symbleTable[0].v.begin();
		i != symbleTable[0].v.end();
		++i){
		s = "";
		addCode(transGVar(i->first) + ":");
		if(i->second.initval.size()){
			for(j = 0; j < i->second.initval.size();j++){
				s += " " + i2s(i->second.initval[j]);
				if(j % 5 == 0 && j != 0){
					addCode("\t.word " + s);
					s = "";
				}
			}
			if(s.size()>0){
					addCode("\t.word " + s);
					s = "";
			}
			if(i->second.size > i->second.initval.size()){
				addCode("\t.space " + i2s((i->second.size - i->second.initval.size())*4));
			}
		}else{
			addCode("\t.space " + i2s(i->second.size * 4) );
		}
	}
	addCode("str:");
	addCode("\t.asciiz \"\\n\"");
	addCode("\t.text");
	addCode("\t.globl main");
	addCode("main:");
	addCode("\tadd	$fp,	$0,	$sp");
	addCode("\tj\tL_main");
}


int scanEnv(string varname){
	int pos = varname.find(".");
	if(pos == string::npos){
		return -1;
	}else{
		return atoi(varname.data() + pos+1);
	}
}

string scanOffset(string varname){
	int pos	= varname.find(":");
	if(pos == string::npos){
		return "";
	}else{
		// cout << "SCAN_OFFSET:\t";
		// cout << varname << "\t" ;
		// cout << varname.substr(pos+1, varname.size()-1-pos) << endl;
		return varname.substr(pos+1, varname.size()-1-pos);
	}
}

string scanName(string varname){
	int pos = varname.find(".");
	if(pos == string::npos){
		return varname;
	}else{
		return varname.substr(0,pos);
	}
}


void writeCodeToFile(const char * filename){
	ofstream fout(filename);
	for(int i = 0; i < asmCode.size(); i++){
		fout << asmCode[i] ;
		if(asmComment.find(i) != asmComment.end()){
			fout << "\t#\t" << asmComment[i];
		}
		fout << endl;
	}
	fout.close();
}

void printCode(void){
	int l = asmCode.size();
	for(int i = 0; i < l; i++){
		cout << asmCode[i] << endl;
	}
}

void getTmpVarscop(void){
	for(int i = code.size()-1; i>=0; i--){
		if(VAR_IS_TMP(code[i].arg1)){
			if(tmpVarUseagerec.find(code[i].arg1)==tmpVarUseagerec.end())
				tmpVarUseagerec[code[i].arg1] = i;
		}
		if(VAR_IS_TMP(code[i].arg2)){
			if(tmpVarUseagerec.find(code[i].arg2)==tmpVarUseagerec.end())
				tmpVarUseagerec[code[i].arg2] = i;
		}
		if(VAR_IS_TMP(code[i].res)){
			if(tmpVarUseagerec.find(code[i].res)==tmpVarUseagerec.end())
				tmpVarUseagerec[code[i].res] = i;
		}
		string offset = scanOffset(code[i].res);
		if(offset == "")
			continue;
		if(VAR_IS_TMP(offset)){
			if(tmpVarUseagerec.find(offset)==tmpVarUseagerec.end())
				tmpVarUseagerec[offset] = i;
		}
	}
}
