Chiffre [0-9]
Entier {Chiffre}+
Lower	[a-z]
Upper	[A-Z]
Lettre	[a-zA-Z]
LC 	({Lettre}|{Chiffre})
Delim   [ \t\n\r]
Symbole [&+\-*/();:,.{}><=]
BS      \\.
PasBSG [^\\"]
Guill		"\""
Star  		"*"
NotStar		[^*]
NotStarPar      [^*/]


%{
#include <string.h>
#include <strings.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char *);
extern YYSTYPE yylval;
extern int yylineno;
extern int yylex();

void lexError(char c) {
  fprintf(stderr, "Error: Skipping invalid character '%c' on line %d\n",
          c, yylineno);
  setError(LEXICAL_ERROR);
}

int keywordOrId() {
  /* a completer */
  if (! strcmp(yytext, "class")) return CLASS;
  if (! strcmp(yytext, "extends")) return EXTENDS;
  if (! strcmp(yytext, "if")) return IF;
  if (! strcmp(yytext, "then")) return THEN;
  if (! strcmp(yytext, "else")) return ELSE;
  if (! strcmp(yytext, "is")) return IS;
  if (! strcmp(yytext, "var")) return VAR;
  if (! strcmp(yytext, "def")) return DEF;
  if (! strcmp(yytext, "override")) return OVERRIDE;
  if (! strcmp(yytext, "return")) return RETURN;
  if (! strcmp(yytext, "new")) return INST;
  if (! strcmp(yytext, "as")) return AS;
  yylval.S = strdup(yytext);
  return(Id);
}
%}

%%
{Entier} 	{ yylval.I = atoi(yytext); return(Cste); }
{Symbole}	{ return(yytext[0]); }
":="		{ return(AFF); }
"<>"		{ return(DIFF); }
"<="		{ return(INFEG); }
">="		{ return(INFSUP); }
{Upper}{LC}*    { yylval.S = strdup(yytext); return(NOM); }
{Lower}{LC}*    { return keywordOrId(); }
{Delim}         { }
{Guill}(({PasBSG}|{BS})*){Guill} { yylval.S = strdup(yytext); return (STR); }
"/*"({NotStar}|"*"+{NotStarPar})*"*"+"/"  { }
.               { lexError(yytext[0]); }

%%
