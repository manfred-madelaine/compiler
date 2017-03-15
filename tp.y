/* attention: NEW est defini dans tp.h Utilisez un autre token */
/* nous avons noté le mot-clef new par INST */
%token IS CLASS VAR EXTENDS DEF OVERRIDE RETURN AS IF THEN ELSE AFF INST
%token INFEG INFSUP DIFF
%token<S> Id STR NOM
%token<I> Cste
%token<C> RelOp

/* On realise la version simple dans laquelle les declarations sont
 * stockees dans l'AST comme le reste du programme
 */
%type <pT> expression instructionListOpt instructionList instruction block
%type <pT> select appel expressionOperator constructorBodyOpt argList argListOpt
%type <pC> classList classListOpt classDecl returnTypeOpt classHeader
%type <pA> paramListOpt paramList param fieldList field fieldListOpt
%type <pM> methodListOpt methodList method
%type <B>  overrideOpt
%type <pH> heritageOpt


%nonassoc '=' '<' '>' DIFF INFEG INFSUP
%left '+' '-' '&'
%left '/' '*'
%left '.'


%{
    #include "verif.h"
    #include "code.h"
    #include "tp_y.h"

    extern int yylex();
    extern void yyerror(char *);
    %}

%%


Prog : classListOpt block  		{	printClass($1);

						//TreeP tr = $2;
						verifPortee($1, &$2, NIL(Attribut));
						//printTree($2, 0);

						/*printf("DEBUT\n");printTree($2, 0);
						printf("FIN\n");*/
						verifAppelMethod(&$1, &$2);
						/*printf("DEBUT2\n");printTree($2, 0);
						printf("FIN2\n");*/
						/*ClassP cN = NIL(Class);
						verifAppelMethod(&cN, &$2);*/
						/*TreeP cN = NIL(Tree);
						verifAppelMethod(&$1, &cN);*/
                        verifTypage($2, NIL(Class));

                        setRangClass($1,0);
                        
                        coDegeneration($1, $2);
                        }
;

classListOpt:					{ $$ = NIL(Class);}
| classList 					{ $$ = $1;}
;

classList: classDecl 			{ $$ = $1;}
| classDecl classList  			{ $$ = $1;}
;

classHeader: CLASS NOM 			{ $$ = alloueClass($2);
						}
;

classDecl: classHeader '(' paramListOpt ')' heritageOpt
			constructorBodyOpt IS '{' fieldListOpt methodListOpt '}'
								{ $$ = fillClass($1, $3, $5, $6, $9, $10);}
;

paramListOpt:   			{ $$ = NIL(Attribut);}
| paramList 				{ $$ = $1;}
;

paramList: param 			{ $$ = $1;}
| param',' paramList 		{ linkParam($1, $3); $$ = $1; }
;

param: Id ':' NOM   		{ $$ = makeAttribut($1, getClass($3), NIL(Tree));}
;

heritageOpt:				{ $$ = NIL(Heritage);}

| EXTENDS NOM '(' argListOpt ')'
							{ $$ = makeHeritage($2, $4) ;}
;

constructorBodyOpt:			{ $$ = NIL(Tree);}
| block						{ $$ = $1;}
;

argListOpt:					{ $$ = NIL(Tree);}
| argList 					{ $$ = $1;}
;

argList: expression 		{ $$ = makeTree(LARG, 2, $1, NIL(Tree));}
| expression ',' argList    { $$ = makeTree(LARG, 2, $1, $3);}
;

fieldListOpt:				{ $$ = NIL(Attribut);}
| fieldList 				{ $$ = $1;}
;


fieldList: field 			{$$ = $1;}
| field fieldList 			{ linkParam($1, $2); $$ = $1;}
;

field: VAR Id ':' NOM ';'	{$$ = makeAttribut($2, getClass($4), NIL(Tree));}
| VAR Id ':' NOM AFF expression ';'
							{$$ = makeAttribut($2, getClass($4), $6);}
;

methodListOpt:        		{$$ = NIL(Method);}
| methodList 				{$$ = $1;}
;

methodList: method     		{$$ = $1;}
| method methodList    		{linkMeth($1, $2); $$ = $1;}
;

method: overrideOpt DEF Id '(' paramListOpt ')' returnTypeOpt IS block
							{$$ = makeMethod($1,$3,$5,$7,$9);}
| overrideOpt DEF Id '(' paramListOpt ')' ':' NOM AFF expression
							{$$ = makeMethod($1,$3,$5,getClass($8),$10);}

;

overrideOpt: 				{$$ = FALSE;}
| OVERRIDE 					{$$ = TRUE;}
;

returnTypeOpt:				{$$ = NIL(Class);}
| ':' NOM 					{$$ = getClass($2);}
;

expression: Cste			{ $$ = makeLeafInt(CONST, $1);}
| '(' expression ')'		{ $$ = $2;}
| '(' AS NOM ':' expression ')'
							{ $$ = makeTree(ECAST, 2, makeLeafStr(ECASTCLASS, $3), $5); }
| INST NOM '(' argListOpt ')'
							{ $$ = makeLeafInstance(NEWIN,$2,$4);}
| select 					{ $$=$1;}
| expressionOperator		{ $$=$1;}
| expression '.' appel		{ $$ = makeTree(APPEL, 2, $1, $3);}
| STR			        	{ $$ = makeLeafStr(ESTR, $1); }
;


select: expression '.' Id 	{ $$ = makeTree(SELECT, 2, $1, makeLeafStr(IDAPPEL, $3));}
| Id                        { $$ = makeLeafStr(IDVAR, $1); }
;

appel: Id '(' argListOpt ')'
						{ $$ = makeTree(APPEL, 2, makeLeafStr(IDAPPEL, $1), $3);}
;

expressionOperator:
  expression '+' expression 	{ $$ = makeTree(EADD, 2, $1, $3); }
| expression '-' expression 	{ $$ = makeTree(EMIN, 2, $1, $3); }
| expression '/' expression 	{ $$ = makeTree(EDIV, 2, $1, $3); }
| expression '*' expression 	{ $$ = makeTree(EMUL, 2, $1, $3); }

| expression INFEG expression   { $$ = makeTree(EINFEG, 2, $1, $3); }
| expression INFSUP expression  { $$ = makeTree(ESUPEG, 2, $1, $3); }
| expression '=' expression  	{ $$ = makeTree(EEGAL, 2, $1, $3);}
| expression '<' expression  	{ $$ = makeTree(EINF, 2, $1, $3);}
| expression '>' expression  	{ $$ = makeTree(ESUP, 2, $1, $3);}
| expression DIFF expression  	{ $$ = makeTree(EDIFF, 2, $1, $3);}

| expression '&' expression 	{ $$ = makeTree(ECONCAT, 2, $1, $3); }
| '-' expression %prec '*'		{ $$ = makeTree(UMIN, 1, $2); }
| '+' expression %prec '*'		{ $$ = makeTree(UADD, 1, $2); }
;

instructionListOpt:				{ $$ = NIL(Tree); }
| instructionList 				{ $$ = $1; }
;

instructionList: instruction instructionList
								{ $$ = makeTree(LINSTR, 2, $1, $2); }
| instruction					{ $$ = makeTree(LINSTR, 2, $1, NIL(Tree)); }
;

instruction: expression ';'		{ $$ = $1;}
| RETURN ';'					{ $$ = makeTree(ERETURN, 0);}
| block							{ $$ = $1;}
| select AFF expression ';'		{ $$ = makeTree(EAFF, 2, $1, $3);}
| IF expression THEN instruction ELSE instruction
								{ $$ = makeTree(ITE, 3, $2, $4, $6); }
;

block: '{' instructionListOpt '}'		{ $$ = $2;}
| '{' fieldList IS instructionList '}'	{ $$ = makeTree(BLOCK, 2, makeLeafDecl(DECL, $2), $4); }
;
