/* 
* file name : smallc.l
* this file is lexical analizer source code
* as well as implementation of main function 
* and yyerror function
*/



%option noyywrap
%option yylineno

%{
#include <string>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <stdlib.h>
#include "y.tab.h"
#include "def.h"

using namespace std;
%}

WHITESPACE	" "|"\t"

DIGIT		[0-9]
LETTER		[a-zA-Z]

INT			("0x"|"0X"|{DIGIT})({DIGIT})*
ID			(_|{LETTER})(_|{LETTER}|{DIGIT})*
SEMI		";"
COMMA		","

DOT			"."

LOGICALNOT	"!"
PREFIXINC	"++"
PREFIXDEC	"--"
BITNOT		"~"

PRODUCT		"*"
DIVISION	"/"
MODULU		"%"

PLUS		"+"
MINUS		"-"

SHIFTLEFT	"<<"
SHIFTRIGHT	">>"

GREATERTHEN		">"
NOLESSTHAN		">="
LESSTHAN		"<"
NOGREATERTHEN	"<="

EQUALTO			"=="
NOTEQUALTO		"!="

BITAND		"&"

BITXOR		"^"

BITOR		"|"

LOGICALAND	"&&"

LOGICALOR	"||"

ASSIGN			"="
PLUSASSIGN		"+="
MINUSASSIGN		"-="
PRODUCTASSIGN	"*="
DIVIDEASSIGN	"/="
ANDASSIGN		"&="
XORASSIGN		"^="
ORASSIGN		"|="
SLEFTASSIGN		"<<="
SRIGHTASSIGN	">>="


TYPE		int
LP			"("
RP			")"
LB			"["
RB			"]"
LC			"{"
RC			"}"
STRUCT		"struct"
RETURN		"return"
IF			"if"
ELSE		"else"
BREAK		"break"
CONT		"continue"
FOR			"for"

COMMENT		"//"(.)*

%%

%{
	/* rules */
%}

{TYPE}			{return TYPE;}
{STRUCT}		{return STRUCT;}
{RETURN}		{return RETURN;}
{IF}			{return IF;}
{ELSE}			{return ELSE;}
{BREAK}			{return BREAK;}
{CONT}			{return CONT;}
{FOR}			{return FOR;}
{DOT}			{return DOT;}

{INT}			{
					yylval.val = atoi(yytext);
					return INT;
				}
{ID}			{
					yylval.sid = new string(yytext);
					return ID;
				}
{SEMI}			{return SEMI;}
{COMMA}			{return COMMA;}


{LP}			{return LP;}
{RP}			{return RP;}
{LB}			{return LB;}
{RB}			{return RB;}
{LC}			{return LC;}
{RC}			{return RC;}


{MINUS}				{return MINUS;}
{LOGICALNOT}		{return LOGICALNOT;}
{PREFIXINC}			{return PREFIXINC;}
{PREFIXDEC}			{return PREFIXDEC;}
{BITNOT}			{return BITNOT;}

{PRODUCT}			{return PRODUCT;}
{DIVISION}			{return DIVISION;}
{MODULU}			{return MODULU;}

{PLUS}				{return PLUS;}

{SHIFTLEFT}			{return SHIFTLEFT;}
{SHIFTRIGHT}		{return SHIFTRIGHT;}

{GREATERTHEN}		{return GREATERTHEN;}
{NOLESSTHAN}		{return NOLESSTHAN;}
{LESSTHAN}			{return LESSTHAN;}
{NOGREATERTHEN}		{return NOGREATERTHEN;}
{EQUALTO}			{return EQUALTO;}
{NOTEQUALTO}		{return NOTEQUALTO;}

{BITAND}			{return BITAND;}

{BITXOR}			{return BITXOR;}

{BITOR}				{return BITOR;}

{LOGICALAND}		{return LOGICALAND;}

{LOGICALOR}			{return LOGICALOR;}

{ASSIGN}			{ return ASSIGN;}
{PLUSASSIGN}		{ return PLUSASSIGN;}
{MINUSASSIGN}		{ return MINUSASSIGN;}
{PRODUCTASSIGN}		{ return PRODUCTASSIGN;}
{DIVIDEASSIGN}		{ return DIVIDEASSIGN;}
{ANDASSIGN}			{ return ANDASSIGN;}
{XORASSIGN}			{ return XORASSIGN;}
{ORASSIGN}			{ return ORASSIGN;}
{SLEFTASSIGN}		{ return SLEFTASSIGN;}
{SRIGHTASSIGN}		{ return SRIGHTASSIGN;}

{COMMENT}			{ /* printf("COMMENT:%s\n",yytext); */}
{WHITESPACE}		{}
.				{     /*printf("not recognized:%s\n" , yytext); */}
\n				{ }
%%

/* user code */



void yyerror(const char * s){
	printf("%d : %s %s\n",yylineno,s,yytext);
	return ;
}



