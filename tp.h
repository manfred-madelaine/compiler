#ifndef TP_H
#define TP_H

#include <stdlib.h>

#define TRUE 1
#define FALSE 0

typedef unsigned char bool;

#define NEW(howmany, type) (type *) calloc((unsigned) howmany, sizeof(type))
#define NIL(type) (type *) 0

/* Etiquettes additionnelles pour les arbres de syntaxe abstraite.
 * Certains tokens servent directement d'etiquette. Attention ici a ne pas
 * donner des valeurs identiques a celles des tokens.
 */
#define NE	1
#define EQ	2
#define LT	3
#define LE	4
#define GT	5
#define GE	6

#define ECLASS	7
#define EVAR  	8
#define ITE  	9

#define IDVAR 10
#define ESTR 11
#define CONST 20

// #define POINT	12
#define APPEL  13
#define SELECT 14
#define EAFF 15
#define ERETURN 16
#define LTHIS 17
#define LSUPER 18
#define LRESULT 72

#define ECAST 73
#define ECASTCLASS 74

/* OPERATEURS BINAIRES ET UNAIRES */
#define EADD 50
#define EMIN 51
#define EDIV 52
#define EMUL 53
#define UADD 54
#define UMIN 55
#define ECONCAT 56

#define INSTR 19
#define LCLASS 21
#define LPARAM 23
#define PARAM 24
#define NEWIN 30
#define LARG 31
#define LINSTR 32
#define EOVERRIDE 33
#define BLOCK	35
#define EINF 60
#define ESUP 61
#define EEGAL 62
#define EDIFF 63
#define ESUPEG 64
#define EINFEG 65

#define DECL 70
#define IDAPPEL 71



/* Codes d'erreurs */
#define NO_ERROR	0
#define USAGE_ERROR	1
#define LEXICAL_ERROR	2
#define SYNTAX_ERROR    3
#define CONTEXT_ERROR	40	/* default value for this stage */
#define DECL_ERROR	41	/* more precise information */
#define TYPE_ERROR	42
#define EVAL_ERROR	50
#define UNEXPECTED	10O



typedef struct _attribut Attribut, *AttributP;
typedef struct _method Method, *MethodP;
typedef struct _variableLocale VariableLocale, *VariableLocaleP;
typedef struct _heritage Heritage, *HeritageP;

typedef struct _varDecl {
  int rang;
  char *name;
  struct _varDecl *next;
} VarDecl, *VarDeclP;

/*
*
*/
typedef struct _class{
	char* name;
	AttributP paramConstructeur; 	/*attributs passes en parametre */
	struct _class *parentClass;		/* pointeur vers la classe mère */
	AttributP attribut;				/*file des attributs de la classe*/
	MethodP methode;				/*file constructeur + methodes de la classe */
	struct _class* nextClass;

  int tailleAttribut;             /*somme des tailles de chaque arttribut définissant la classe (Integer et String)*/
  int tailleMeth;
  int rang;

} Class, *ClassP;


/* la structure d'un arbre (noeud ou feuille) */
typedef struct _Tree {
  short op;         		/* etiquette de l'operateur courant */
  short nbChildren;			/* nombre de sous-arbres */
  union {
    char *str;      		/* valeur de la feuille si op = Id ou STR */
    int val;        		/* valeur de la feuille si op = Cste */
    AttributP lDecl;
    ClassP classT;
    MethodP meth;
    struct _Tree **children; /* tableau des sous-arbres */
  } u;
  bool isPointerMethod;
} Tree, *TreeP;



/*
*
*/
struct _attribut{
	char* name;
	ClassP type;			/* pointeur vers le type */
	TreeP expression;
	AttributP nextA;
  int rang;
};

/*
*
*/
struct _method{
	char* name;
	AttributP signature; 	/* liste de parametres en entree */
	bool redefinition;		/* booleen implicite */
	TreeP instructions;     /* corps de la fonction */
	ClassP typeRetour;      /* pointeur sur le type de retour */

  int rang;

	struct _method* nextM;
};

/*
*
*/
struct _heritage{
	char* supercName;		/*nom de la super classe*/
	TreeP listArg;		/*liste des arg passés en param a la super classe*/
};


typedef union
{ char *S;
  char C;
  int I;
  bool B;
  ClassP pC;
  MethodP pM;
  AttributP pA;
  HeritageP pH;
  TreeP pT;
  VarDeclP pV;
} YYSTYPE;

#define YYSTYPE YYSTYPE

extern MethodP firstConst;
extern FILE *out;

void copyAttribut(AttributP src, AttributP dst);
bool linkMethSup(MethodP *list, MethodP elem);
/* construction pour les AST */
TreeP makeLeafStr(short op, char *str);       /* feuille (string) */
TreeP makeLeafInt(short op, int val);             /* feuille (int) */
TreeP makeTree(short op, int nbChildren, ...);      /* noeud interne */
TreeP makeLeafDecl(short op, AttributP a);

TreeP makeLeafInstance(short op, char* nom, TreeP lparam);
TreeP makeLeafInstanceVIDE(short op, char* nom);
TreeP getClassParent(TreeP this, char* nom);
TreeP getAttributMethode(TreeP this, TreeP id);

TreeP makeBlock(AttributP lAtt, TreeP t);

/*Alloue de la memoire a une classe et ajoute la clase à
la liste des classe déclarées
*/
ClassP alloueClass(char* nom);


ClassP fillClass(ClassP pClass, AttributP lparam, HeritageP herite,
			TreeP constructeur, AttributP field, MethodP methodList);

AttributP makeAttribut(char *s, ClassP type, TreeP expression);

MethodP makeMethod(bool isOverride, char *nom, AttributP lparam,
			ClassP typeRetour, TreeP expr);
/*recuperer le pointeur vers une clase à parti d'ue chaine de caractere (son nom)
besoin de la liste de toutes les classes qu'on a pour cherche parmi ces dernieres
si jalais c'est trop compliqué on utilise un char* au lieu du classP
faireu var globale
*/
ClassP getClass(char * nom);

/* permet de remonter les informations pour faire le super juste avant
le constructeur de la classe */
HeritageP makeHeritage(char *s, TreeP listArg);

void printTree(TreeP t, int i);
void printAttribut(AttributP attribut);
void printMethod(MethodP methode);
void printClass(ClassP classe);


bool existClass(char * nom, ClassP listClass);

TreeP getChild(TreeP tree, int rank);

/**
 * Les trois methodes "link" relient un element a une liste d'elements
 * uniquement si celui-ci n'existe pas deja dans la liste (test les noms)
  */
bool linkClass(ClassP list, ClassP elem);
bool linkMeth(MethodP list, MethodP elem);
bool linkParam(AttributP list, AttributP elem);
bool linkParamSignature(AttributP list, AttributP elem);

int addTaille(AttributP listAttribut);
int addTailleMeth(MethodP meth);
void gestionTailles(ClassP c);
bool gestionRangMeth(ClassP maClass);
bool setRangMeth(ClassP maClass, MethodP methode, int rang);
int getRang(char* nomMethode , ClassP listParentClass);
int getRangMethodes(char* nomMethode ,MethodP listMethod);

bool classPredefini(ClassP maClass);

/* Methodes pour les verifications à la volée */
int verifHeritage(MethodP listMethod, ClassP herite);
bool verifClassMere(char* nomMethode , AttributP sign, ClassP type, ClassP listParentClass);
bool verifMethodesOverride(char* nomMethode, AttributP sign, ClassP type, MethodP listMethod);
void checkResultConstructor(TreeP t);
bool compareSignature(AttributP s1, AttributP s2);

/* Methodes pour le rang */
bool appartient(AttributP var, char* str);
AttributP chercher(AttributP var, char* str);
bool appartientMethSimple(MethodP m, char* str);
MethodP chercherMethod(MethodP m, char* str);

int rangAttributMere(ClassP classe, AttributP attribut);

int vraieTaille(ClassP classe);
int vraieMethTaille(ClassP classe);

int tailleHerite(ClassP classe);
int tailleMethHerite(ClassP classe);

bool setRangAttribut(ClassP classe, AttributP attribut, int rang);
bool gestionRangMeth(ClassP maClass);
bool setRangClass(ClassP maClass, int rang);
bool setRangParam(AttributP listParam, int rang);

#endif
