%{
#include "syntax.tab.h"
#include "node.h"
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
int yycolumn = 1;
extern int lexError;
%}

%option yylineno

digit [0-9]
letter [_a-zA-Z]
ws [ \t]
INT 0|[1-9]+{digit}*
FLOAT {digit}+"."{digit}+|{digit}*"."{digit}+[eE][+-]?{digit}+|{digit}+"."{digit}*[eE][+-]?{digit}+
ID {letter}({letter}|{digit})*
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS "+"
MINUS -
STAR "*"
DIV "/"
AND &&
OR "||"
DOT "."
NOT !
TYPE int|float
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

%%

{ws}+ {;}
\n|\r {yycolumn = 1;}
{SEMI} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "SEMI", yytext); return SEMI;}
{COMMA} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "COMMA", yytext); return COMMA;}
{ASSIGNOP} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "ASSIGNOP", yytext); return ASSIGNOP;}
{RELOP} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "RELOP", yytext); return RELOP;}
{PLUS} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "PLUS", yytext); return PLUS;}
{MINUS} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "MINUS", yytext); return MINUS;}
{STAR} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "STAR", yytext); return STAR;}
{DIV} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "DIV", yytext); return DIV;}
{AND} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "AND", yytext); return AND;}
{OR} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "OR", yytext); return OR;}
{DOT} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "DOT", yytext); return DOT;}
{NOT} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "NOT", yytext); return NOT;}
{TYPE} {yylval.node = newTokenNode(yylineno, TYPE_TOKEN, "TYPE", yytext); return TYPE;}
{LP} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "LP", yytext); return LP;}
{RP} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "RP", yytext); return RP;}
{LB} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "LB", yytext); return LB;}
{RB} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "RB", yytext); return RB;}
{LC} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "LC", yytext); return LC;}
{RC} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "RC", yytext); return RC;}
{STRUCT} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "STRUCT", yytext); return STRUCT; }
{RETURN} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "RETURN", yytext); return RETURN;}
{IF} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "IF", yytext); return IF;}
{ELSE} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "ELSE", yytext); return ELSE;}
{WHILE} {yylval.node = newTokenNode(yylineno, OTHER_TOKEN, "WHILE", yytext); return WHILE;}

{INT} {yylval.node = newTokenNode(yylineno, INT_TOKEN, "INT", yytext); return INT;}
{FLOAT} {yylval.node = newTokenNode(yylineno, FLOAT_TOKEN, "FLOAT", yytext); return FLOAT;}
{ID} {yylval.node = newTokenNode(yylineno, ID_TOKEN, "ID", yytext); return ID;}

"."{digit}+|{digit}+"."|{digit}*"."{digit}+[eE]|{digit}+"."{digit}*[eE]|{digit}+[eE][+-]?{digit}*|"."[eE][+-]?{digit}+ {lexError = 1; printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext);}
{digit}+{ID} {lexError = 1; printf("Error type A at Line %d: Illegal ID \"%s\".\n", yylineno, yytext);}
. {lexError = 1; printf("Error type A at Line %d: Mysterious character \'%s\'.\n", yylineno, yytext); }

%%

//int main(int argc, char** argv) {
//	if (argc > 1) {
//		if (!(yyin = fopen(argv[1], "r"))) {
//			perror(argv[1]);
//			return 1;
//		}
//	}
//	while (yylex() != 0);
//	return 0;
//}
