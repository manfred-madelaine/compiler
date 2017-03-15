#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"

extern int yyparse();
extern int yylineno;

/* Niveau de 'verbosite'.
 * Par defaut, n'imprime que le resultat et les messages d'erreur
 */
bool verbose = FALSE;

/* Generation de code ou pas. Par defaut, on produit le code */
bool noCode = FALSE;

/* Pose de points d'arret ou pas dans le code produit */
bool debug = FALSE;

/* code d'erreur a retourner */
int errorCode = NO_ERROR;

/**variable globale contenant le pointeur vers la premiere classe déclarée */
ClassP firstClass;

int main(int argc, char **argv) {
  int fi;
  int i, res;
  firstConst = NIL(Method);

  out = stdout;
  for(i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'd': case 'D':
  debug = TRUE; continue;
      case 'v': case 'V':
  verbose = TRUE; continue;
      case 'e': case 'E':
  noCode = TRUE; continue;
      case '?': case 'h': case 'H':
  fprintf(stderr, "Appel: tp -v -e -d -o file.out programme.txt\n");
  exit(USAGE_ERROR);
      case'o':
    if ((out= fopen(argv[++i], "w")) == NULL) {
      fprintf(stderr, "erreur: Cannot open %s\n", argv[i]);
      exit(USAGE_ERROR);
    }
  break;
      default:
  fprintf(stderr, "Option inconnue: %c\n", argv[i][1]);
  exit(USAGE_ERROR);
      }
    } else break;
  }

  if (i == argc) {
    fprintf(stderr, "Fichier programme manquant\n");
    exit(USAGE_ERROR);
  }

  if ((fi = open(argv[i++], O_RDONLY)) == -1) {
    fprintf(stderr, "erreur: Cannot open %s\n", argv[i-1]);
    exit(USAGE_ERROR);
  }

  /* redirige l'entree standard sur le fichier... */
  close(0); dup(fi); close(fi);

  ClassP Integer = NEW(1, Class);
  Integer->name= "Integer";
  firstClass = Integer;
  ClassP String = NEW(1, Class);
  String->name = "String";

	MethodP IntegerToString = NEW(1, Method);
	MethodP StringPrintln = NEW(1, Method);
	MethodP StringPrint = NEW(1, Method);

	IntegerToString = makeMethod(FALSE, "toString", NIL(Attribut), String, NIL(Tree));

	StringPrintln = makeMethod(FALSE, "println", NIL(Attribut), String, NIL(Tree));
	StringPrint = makeMethod(FALSE, "print", NIL(Attribut), String, NIL(Tree));
	linkMeth(StringPrintln, StringPrint);

  fillClass(firstClass, NIL(Attribut), NIL(Heritage), NIL(Tree), NIL(Attribut), IntegerToString);
  fillClass(String, NIL(Attribut), NIL(Heritage), NIL(Tree), NIL(Attribut), StringPrintln);

  linkClass(firstClass,String);
  res = yyparse();
  if (out != NIL(FILE) && out != stdout) fclose(out);
  return res ? SYNTAX_ERROR : errorCode;
}


void setError(int code) {
  errorCode = code;
  if (code != NO_ERROR) { noCode = TRUE; /*  abort(); */}
}


/* yyerror:  fonction importee par Bison et a fournir explicitement. Elle
 * est appelee quand Bison detecte une erreur syntaxique.
 * Ici on se contente d'un message minimal.
 */
void yyerror(char *ignore) {
  printf("erreur de syntaxe: Ligne %d\n", yylineno);
  setError(SYNTAX_ERROR);
}


/* Tronc commun pour la construction d'arbre */
TreeP makeNode(int nbChildren, short op) {
  TreeP tree = NEW(1, Tree);
  tree->op = op;
  tree->nbChildren = nbChildren;
  tree->u.children = nbChildren > 0 ? NEW(nbChildren, TreeP) : NIL(TreeP);
  return(tree);
}


/* Construction d'un arbre a nbChildren branches, passees en parametres */
TreeP makeTree(short op, int nbChildren, ...) {
  va_list args;
  int i;
  TreeP tree = makeNode(nbChildren, op);
  va_start(args, nbChildren);
  for (i = 0; i < nbChildren; i++) {
    tree->u.children[i] = va_arg(args, TreeP);
  }
  va_end(args);
  tree->isPointerMethod = FALSE;
  return(tree);
}


/* Retourne le rankieme fils d'un arbre (de 0 a n-1) */
TreeP getChild(TreeP tree, int rank) {
  if (tree->nbChildren < rank -1) {
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort();
  }
  return tree->u.children[rank];
}


void setChild(TreeP tree, int rank, TreeP arg) {
  if (tree->nbChildren < rank -1) {
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort();
  }
  tree->u.children[rank] = arg;
}


/* Constructeur de feuille dont la valeur est une chaine de caracteres */
TreeP makeLeafStr(short op, char *str) {
  TreeP tree = makeNode(0, op);
  tree->u.str = str;
  tree->isPointerMethod = FALSE;
  return tree;
}


/* Constructeur de feuille dont la valeur est un entier */
TreeP makeLeafInt(short op, int val) {
  TreeP tree = makeNode(0, op);
  tree->u.val = val;
  return(tree);
}

/*
 * Dans le bloc des intructions, chaque fois ou on veut contruire un instance,
 * on parcourt la liste de class pour trouver la bonne classe. On fait une
 * copie et les initialise avec des parametres qu'on veut.
 */
TreeP makeLeafInstance(short op, char* nom, TreeP lparam){
  TreeP tree = makeNode(2, op);
  setChild(tree, 0, makeLeafStr(IDAPPEL, nom));
  setChild(tree,1,lparam);
  return(tree);
}

/*
 * Pour l'expression la definition d'un variable. Il peut etre le
 */
TreeP makeLeafInstanceVIDE(short op, char* nom){
  /* A faire*/
  TreeP tree = makeNode(0, op);
  return(tree);
}

/*
 * Prendre le classe parent d'une classe
 */
TreeP getClassParent(TreeP this, char* nom){
  /* A faire*/
  TreeP tree = makeNode(0, 0);
  return(tree);
}


/* relie une classe à la classe suivante c2,
 * ou à la dernière classe sur laquelle on débouche en suivant la file
 */
bool linkClass(ClassP list, ClassP elem)
{
  if(list->nextClass == NIL(Class))
  {
    list->nextClass = elem;
    return TRUE;
  }
  return linkClass(list->nextClass, elem);
}

bool linkMeth(MethodP list, MethodP elem)
{
  if(elem != NIL(Method) && strcmp(list->name, elem->name) == 0)
  {
    printf("Le nom %s est deja utilise par l'une des methodes!\n", elem->name);
    exit(EXIT_FAILURE);
    return FALSE;
  }

  else if(list->nextM == NIL(Method))
    list->nextM = elem;

  else
    return linkMeth(list->nextM, elem);

  return TRUE;
}

bool linkMethSup(MethodP *list, MethodP elem)
{
  if(*list == NIL(Method))
  {
	*list = elem;
	return TRUE;
  }
  if(elem != NIL(Method) && strcmp((*list)->name, elem->name) == 0)
  {
    printf("Le nom %s est deja utilise par l'une des methodes!\n", elem->name);
    exit(EXIT_FAILURE);
    return FALSE;
  }

  else if((*list)->nextM == NIL(Method))
    (*list)->nextM = elem;

  else
    return linkMeth((*list)->nextM, elem);

  return TRUE;
}

bool linkParam(AttributP list, AttributP elem)
{

  if (elem == NIL(Attribut))
    return FALSE;


  if( strcmp(list->name, elem->name) == 0)
  {
    printf("Le nom %s est deja utilise par l'un des attributs!\n", elem->name);
    exit(EXIT_FAILURE);
    return FALSE;
  }

  else if(list->nextA == NIL(Attribut))
    list->nextA = elem;

  else
    return linkParam(list->nextA, elem);

  return TRUE;
}



bool linkParamSignature(AttributP list, AttributP elem)
{

  if (elem == NIL(Attribut))
    return FALSE;



  if(list->nextA == NIL(Attribut))
    list->nextA = elem;

  else
    return linkParamSignature(list->nextA, elem);

  return TRUE;
}

/**
 * verifie qu'un nom n'est pas deja utilise pour un classe et les classes qui
 * lui sont liées
 */
bool existClass(char * nom, ClassP listClass)
{
  if(listClass == NIL(Class))
    return FALSE;

  //si le nom est déja utilisé
  else if(strcmp(listClass->name, nom) == 0){
    printf("Le nom %s est deja utilise par une classe !\n", nom);
    exit(EXIT_FAILURE);
    return TRUE;
  }

  return existClass(nom, listClass->nextClass);
}


bool classPredefini(ClassP maClass)
{
  if(strcmp(maClass->name, "Integer") == 0 || strcmp(maClass->name, "String") == 0 )
    return TRUE;

  return FALSE;
}


/* rajoute le nombres d'attributs d''une classe - calcul naif */
int addTaille(AttributP listAttribut)
{
  if(listAttribut == NIL(Attribut))
    return 0;

  return 1 + addTaille(listAttribut->nextA);
}

/*
 * Compte le nombre méthodes dans la liste en ignorant les redéfinitions
 * return size(methodes) + constructeur - redéfinition(s)
 */
int addTailleMeth(MethodP meth)
{
  if(meth == NIL(Method))
    return 0;

  int res = 1;

  // On ne compte pas les redéfinitions comme de nouvelles méthodes de classe
  if(meth->redefinition)
  {
    //+1 redéfinition!
    res--;
  }

  return res + addTailleMeth(meth->nextM);
}


ClassP alloueClass(char* nom)
{
  //on verifie si ce nom n'est pas deja utilisé
  if(existClass(nom, firstClass))
    return NIL(Class);


  ClassP class = NEW(1, Class);

  class->name = nom;

  linkClass(class, NIL(Class));

  //chercher la dernière classe dans la fille de classes globale.
  //Faire cette dernière classe pointer vers class
  linkClass(firstClass, class);

  return class;
}

void checkResultConstructor(TreeP t)
{
	if(t == NIL(Tree))
		return;
	if(t->op == IDVAR)
	{
		if(strcmp(t->u.str, "result") == 0)
		{
			printf("Error : Result can't be used in a constructor\n");
			exit(EXIT_FAILURE);
		}

	}
	for(int i=0 ; i<t->nbChildren ; i++)
	{
		checkResultConstructor(getChild(t, i));
	}
}

/*remplir les champs d'une classe
faire att a convertir tout ce qui est constructeur & herite
das ce qu'il faut (constBodyOpt..)
conversion faire la pethode
*/
ClassP fillClass(ClassP maClass, AttributP lparam, HeritageP herite,
        TreeP constr, AttributP fields, MethodP methods)
{
  //on renvoie NIL si maClass est nulle
  if(maClass == NIL(Class))
    return NIL(Class);

		if(!classPredefini(maClass)) {
			maClass->paramConstructeur = lparam;

		  //creation de la methode : constructeur
		   checkResultConstructor(constr);

		   maClass->methode = makeMethod(FALSE, maClass->name, lparam, maClass, constr);

		   linkMethSup(&firstConst, makeMethod(FALSE, maClass->name, lparam, maClass, constr));

		}
		else {
			maClass->methode =  makeMethod(FALSE, "__primitive_constructor__", NIL(Attribut), NIL(Class), NIL(Tree));
		}

		linkMeth(maClass->methode, methods);


  if(herite == NIL(Heritage)) {
    printf("%s N A PAS DE SUPER CLASSE\n", maClass->name);
    maClass->parentClass = NIL(Class);
  }

  else
  {
    printf("%s A UNE SUPER CLASSE\n", maClass->name);
		if(strcmp(herite->supercName, "Integer") == 0 || strcmp(herite->supercName, "String") == 0)
    {
			printf("cannot extend primitive type %s\n", herite->supercName);
			exit(EXIT_FAILURE);
		}
    maClass->parentClass = getClass(herite->supercName);

    verifHeritage(methods, maClass->parentClass);

    if(maClass->methode->instructions == NIL(Tree))
    {
    	TreeP t = makeNode(2, LINSTR);
    	setChild(t, 0, makeNode(2, APPEL));
    	setChild(t, 1, NIL(Tree));

    	setChild(getChild(t, 0), 0, makeLeafStr(IDVAR, "super"));
    	setChild(getChild(t, 0), 1, makeNode(2, APPEL));
    	setChild(getChild(getChild(t, 0), 1), 0, makeLeafStr(IDAPPEL, herite->supercName));
    	setChild(getChild(getChild(t, 0), 1), 1, herite->listArg);
    	maClass->methode->instructions = t;
    }
    else
    {
    	TreeP t = makeNode(2, LINSTR);
    	setChild(t, 0, makeNode(2, APPEL));
    	setChild(t, 1, maClass->methode->instructions);

    	setChild(getChild(t, 0), 0, makeLeafStr(IDVAR, "super"));
    	setChild(getChild(t, 0), 1, makeNode(2, APPEL));
    	setChild(getChild(getChild(t, 0), 1), 0, makeLeafStr(IDAPPEL, herite->supercName));
    	setChild(getChild(getChild(t, 0), 1), 1, herite->listArg);
    	maClass->methode->instructions = t;
    }
  }

  maClass->attribut = fields;

  // initialise la taille des attributs et méthodes de la classe
  gestionRangMeth(maClass);
  gestionTailles(maClass);

  // initialisation des rangs des param du constructeur
  setRangParam(maClass->methode->signature, 0);

  // initialisation des rangs des attributs
  int rang;
  rang = tailleHerite(maClass);
  setRangAttribut(maClass,maClass->attribut,rang);

  printf("Taille des attributs de %s: %d\n", maClass->name, maClass->tailleAttribut);
  printf("Taille des méthodes de %s: %d\n", maClass->name, maClass->tailleMeth);

  return maClass;
}


void gestionTailles(ClassP maClass)
{
  maClass->tailleMeth = addTailleMeth(maClass->methode);
  maClass->tailleAttribut = vraieTaille(maClass)+tailleHerite(maClass);

  if(classPredefini(maClass))
  {
    maClass->tailleMeth--; // les classe prédef n'ont pas de constructeur
  }

  if(maClass->parentClass != NIL(Class)){
    // Un seul constructeur par classe
    maClass->tailleMeth  += maClass->parentClass->tailleMeth -1;
  }
}


bool setRangClass(ClassP maClass, int rang)
{
  if( maClass == NIL(Class))
    return TRUE;

  maClass->rang = rang + 2;

  printf("%s().rang =  %d\n",maClass->name, maClass->rang);
  return setRangClass(maClass->nextClass, ++rang);
}


bool gestionRangMeth(ClassP maClass)
{
  // le constructeur est toujours en position 0
  maClass->methode->rang = 0;

  if(maClass->parentClass == NIL(Class))
    return setRangMeth(maClass, maClass->methode->nextM, 1);

  // passer le dernier rang de l'attribut mère
  return setRangMeth(maClass, maClass->methode->nextM, maClass->parentClass->tailleMeth);
}


bool setRangMeth(ClassP maClass, MethodP methode, int rang)
{
  if( methode == NIL(Method))
    return FALSE;

  else if(methode->redefinition)
  {
    rang--;
    //affecte le bon rang à la méthode
    methode->rang = getRang(methode->name, maClass);
  }

  else
    methode->rang = rang;

  // on défini un rang pour la liste de paramètres passé à la fonction
  setRangParam(methode->signature, 5);

  printf("%s().rang =  %d\n",methode->name, methode->rang);
  return setRangMeth(maClass, methode->nextM, ++rang);
}


bool setRangParam(AttributP listParam, int rang)
{
  if(listParam == NIL(Attribut))
    return FALSE;

  listParam->rang = rang;
  return setRangParam(listParam->nextA, ++rang);
}
/*
 * on verifie qu'aucunes methodes de la liste ne soient deja utilisees dans
 * la classe mère "herite" lorsque le mot clef override n'est pas specifie
 */
int verifHeritage(MethodP listMethod, ClassP herite)
{
  if(listMethod == NIL(Method))
    return FALSE;

  bool isUsed  = verifClassMere(listMethod->name, listMethod->signature, listMethod->typeRetour, herite);
  bool isRedef = listMethod->redefinition;

  if(isRedef && !isUsed)
  {
    printf("La methode '%s' n'existe pas dans toutes les classes dont hérite notre classe actuelle.\n",
     listMethod->name);
    exit(EXIT_FAILURE);
  }

  else if(!isRedef && isUsed)
  {
    printf("La surcharge de methode est interdite et");
    printf(" la redefinition de la methode '%s' doit se faire de façon explicite.\n",
     listMethod->name);
    exit(EXIT_FAILURE);
  }
  if(isRedef && isUsed)
  {
    listMethod->rang = getRang(listMethod->name, herite);
  }

  // Pas de probleme de redefinition, on passe a la methode suivante
  return verifHeritage(listMethod->nextM, herite);
}


int getRang(char* nomMethode , ClassP listParentClass)
{
  if(listParentClass == NIL(Class))
  {
    printf("ERREUR: fin de la liste de classe et toujours pas de match entre les noms.\n");
    return -1;
  }

  int res = getRangMethodes(nomMethode, listParentClass->methode);

  if(res == -1)
  {
    res = getRang(nomMethode , listParentClass->parentClass);
  }

  return res;
}


int getRangMethodes(char* nomMethode ,MethodP listMethod)
{
  if(listMethod == NIL(Method))
    return -1;

  else if(strcmp(nomMethode, listMethod->name) == 0)
  {
    //printf("trouvé !: %s.rang = %d\n", nomMethode, listMethod->rang);
    return listMethod->rang;
  }

  return getRangMethodes(nomMethode, listMethod->nextM);
}


/*
 * Verifie que le nom n'est pas déjà utilisé par l'une des methodes de la liste
 * de classe
 * TRUE s'il l'est, FALSE sinon
 */
bool verifClassMere(char* nomMethode , AttributP sign, ClassP type, ClassP listParentClass)
{
  if(listParentClass == NIL(Class))
    return FALSE;

  else if(verifMethodesOverride(nomMethode, sign, type, listParentClass->methode))
    return TRUE;

  return verifClassMere(nomMethode, sign, type, listParentClass->parentClass);
}

bool verifMethodesOverride(char* nomMethode, AttributP sign, ClassP type, MethodP listMethod)
{
  if(listMethod == NIL(Method))
    return FALSE;

  else if(strcmp(nomMethode, listMethod->name) == 0 && compareSignature(sign, listMethod->signature) && type == listMethod->typeRetour)
    return TRUE;

  return verifMethodesOverride(nomMethode, sign, type, listMethod->nextM);
}


bool compareSignature(AttributP s1, AttributP s2)
{
	if(s1 == NIL(Attribut) && s2 != NIL(Attribut))
		return FALSE;
	if(s2 == NIL(Attribut) && s1 != NIL(Attribut))
		return FALSE;
	if(s1 == NIL(Attribut) && s2 == NIL(Attribut))
		return TRUE;
	if(strcmp(s1->type->name, s2->type->name) == 0)
		return compareSignature(s1->nextA, s2->nextA);
	else
		return FALSE;
}


/*
* crée un attribut et l'ajoute à la liste d'attributs de la classe
*/
AttributP makeAttribut(char *s, ClassP type, TreeP expression)
{
  AttributP attribut = NEW(1, Attribut);
  attribut->name = s;
  attribut->type =  type;
  attribut->expression = expression;
  linkParam(attribut, NIL(Attribut));

  return attribut;
}

/*
 * créé un attribut et l'ajoute à la liste des méthodes de la classe
*/
MethodP makeMethod(bool isRedef, char *s, AttributP lparam,
        ClassP typeRetour, TreeP expr)
{
  MethodP meth = NEW(1, Method);
  meth->redefinition = isRedef;
  meth->name = s;
  meth->signature = lparam;
  meth->typeRetour = typeRetour;
  meth->instructions = expr;
  linkMeth(meth, NIL(Method));

  return meth;
}

/*
* renvoie on pointeur sur la classe
*/
ClassP getClass(char * nom)
{
  ClassP class = firstClass;

  while(class != NIL(Class))
  {
    //si on a trouvé la bonne classe, on renvoie la classe
    if(strcmp(class->name, nom) == 0)
      return class;
    class = class->nextClass;
  }
  printf("\n\nCETTE CLASSE N A PAS ETE TROUVEE : %s\n", nom);
  exit(EXIT_FAILURE);
  return NIL(Class);
}
/*permet de remonter les informations pour faire le super juste avant
le constructeur de la classe*/
HeritageP makeHeritage(char *nomSClass, TreeP listArg)
{
  HeritageP superClass = NEW(1, Heritage);
  superClass->supercName = nomSClass;
  superClass->listArg = listArg;

  return superClass;
}

/*
 * On parcour l'AST d'une classe(intance) pour trouver une attribut ou methode
 */
TreeP getAttributMethode(TreeP this, TreeP id){
  /* A faire*/
  TreeP tree = makeNode(0, 0);
  return(tree);
}

TreeP makeLeafDecl(short op, AttributP a)
{
    TreeP tree = makeNode(0, op);
    tree->u.lDecl = a;
    return(tree);
}

void printTree(TreeP t, int i)
{
  printf("\t\t\t[%d]", i);

  if(t != NIL(Tree) && t->op == STR)
    printf("%s", t->u.str);
  else if(t != NIL(Tree) && t->op == Cste)
    printf("%d", t->u.val);
  else if(t != NIL(Tree) && t->op == DECL)
    {
      printf("DECL");
      printAttribut(t->u.lDecl);
      printf("\n\t\t\tIS\n\n");
    }
  else
  {
    int k;
    for(k=0; k<i; k++)
    {
      printf("  ");
    }
  if(t == NIL(Tree))
  {
    printf("SKIP\n");
    return;
  }
      switch(t->op) {
        case 10:
    printf("IDVAR : %s", t->u.str);
    break;

      case IDAPPEL:
    printf("IDAPPEL : %s", t->u.str);
    break;
        case ECAST:
    printf("ECAST");
    break;
        case ECASTCLASS:
    printf("ECASTCLASS : %s", t->u.str);
    break;
        case 17:
    printf("LTHIS");
    break;
        case 18:
    printf("LSUPER");
    break;
        case 13:
    printf("APPEL");
    break;
        case 31:
    printf("LARG");
    break;
        case 11:
    printf("ESTR : %s", t->u.str);
    break;
        case 32:
    printf("LINSTR");
    break;
        case 35:
    printf("BLOCK");
    break;
        case 50:
    printf("EADD");
    break;
        case 51:
    printf("EMIN");
    break;
        case 52:
    printf("EDIV");
    break;
        case 53:
    printf("EMUL");
    break;
        case 54:
    printf("UADD");
    break;
        case 55:
    printf("UMIN");
    break;
        case 56:
    printf("ECONCAT");
    break;
        case 62:
    printf("EGG");
    break;
        case 63:
    printf("DIFF");
    break;
        case 60:
    printf("INF");
    break;
        case 61:
    printf("SUP");
    break;
        case 65:
    printf("INFEG");
    break;
        case 64:
    printf("SUPEG");
    break;
        case 30:
    printf("NEW");
    break;
        case SELECT:
    printf("SELECT");
    break;
        case ITE:
    printf("ITE");
    break;
        case EAFF:
    printf("EAFF");
    break;

        case 20:
    printf("CONST : %d", t->u.val);
    break;
        case ERETURN:
    printf("RETURN");
    break;
        case LRESULT:
    printf("RESULT");
    break;
        default:
    printf("%d", t->op);
      }
    printf("\n");
    int j;
    for(j=0; j<t->nbChildren; j++)
    {
            printTree(t->u.children[j], i+1);
    }
  }
}

void printAttribut(AttributP attribut){
    if(attribut != NULL)
    {
        if(attribut->type != NULL)
            printf("\n\t\t\tAttribut: %s\n\t\t\tType: %s\tRang: %d\n",attribut->name,attribut->type->name,attribut->rang);
        else
            printf("\n\t\t\tAttribut: %s\n\t\t\tType: undefined type\n",attribut->name);
        if(attribut->expression != NULL){
            printf("\t\t\tExpression de l'attribut :\n");
            printTree(attribut->expression,0);
        }
        if(attribut->nextA != NULL)
                printAttribut(attribut->nextA);
    }
    else
    {
        printf("\n\n\t\\tATTRIBUT == NULL\n\t\tprintAttribut() IMPOSSIBLE\n\n\n");
    }
}

void printMethod(MethodP methode){
    if(methode != NULL)
    {
            if(methode->redefinition == 1)
                printf("\n\n\t\tredefinition: ");

            if(methode->typeRetour == NULL){
                printf("\t\t%s(",methode->name);
            }
            else
            {
                printf("\t\t%s\t%s(",methode->typeRetour->name,methode->name);
            }

            /* on veut afficher la signature de la methode */
            if( methode->signature != NULL){
                AttributP temp;
                temp = methode->signature;
                while(temp != NULL)
                {
                    if(temp->type != NULL)
                        printf("%s  %s",temp->type->name,temp->name);
                    else
                        printf("undefined type  %s",temp->name);

                    if(temp->nextA != NULL)
                    {
                        printf(", ");
                    }
                    temp = temp->nextA;
                }
                free(temp);
            }
            printf(") Rang:%d\n\t\t{\n",methode->rang);
            if(methode->instructions != NULL)
                printTree(methode->instructions,0);
            printf("\t\t}\n\n\n");
            if(methode->nextM != NULL)
                printMethod(methode->nextM);
    }
    else
    {
        printf("\n\n\t\tMETHODE == NULL\n\t\tprintMethod() IMPOSSIBLE\n\n\n");
    }
}

void printClass(ClassP classe){
    if(classe != NULL)
    {
        printf("\tTAILLE classe suivante: %d",classe->tailleAttribut);
        printf("\tTAILLE herite suivante: %d",tailleHerite(classe));
        /* affichage type C++ */
        printf("\nClass\t%s(",classe->name);
        if( classe->paramConstructeur != NULL)
        {
            AttributP temp;
            temp = classe->paramConstructeur;
            while(temp != NULL)
            {
                if(temp->type != NULL)
                    printf("%s  %s",temp->type->name,temp->name);
                else
                    printf("undefined type  %s",temp->name);

                if(temp->nextA != NULL)
                {
                    printf(", ");
                }
                temp = temp->nextA;
            }
            free(temp);
        }
        printf(")");

        /* on a print : Class foo(constructeur) */

        if(classe->parentClass != NULL)
        {
            printf("  extends %s(",classe->parentClass->name);
            if(classe->parentClass->paramConstructeur != NULL)
            {
                AttributP tempAtt;
                tempAtt = classe->parentClass->paramConstructeur;
                while(tempAtt != NULL)
                {
                    if(tempAtt->type != NULL)
                        printf("%s  %s",tempAtt->type->name,tempAtt->name);
                    else
                        printf("undefined type  %s",tempAtt->name);

                    if(tempAtt->nextA != NULL)
                    {
                        printf(", ");
                    }
                    tempAtt = tempAtt->nextA;
                }
                free(tempAtt);
            }
            printf(")");
        }

        printf(" is {\n");

        /* on a print : Class foo(constructeur) extends of bar(constructeur) is {*/

        if(classe->attribut != NULL)
        {
            printf("\nListe des attributs de %s:\n",classe->name);
            printAttribut(classe->attribut);
        }
        if(classe->methode != NULL)
        {
            printf("\nListe des methodes de %s:\n\n",classe->name);
            printMethod(classe->methode);
        }
        printf("\n}\n");

        if(classe->nextClass != NULL)
            printClass(classe->nextClass);
    }
    else
    {
        printf("\n\n\t\tCLASSE == NULL\n\t\tprintClasse() IMPOSSIBLE\n\n\n");
    }
}

MethodP chercherMethod(MethodP m, char* str)
{
	if(strcmp(m->name, str) == 0) {
		return m;
	}
	return chercherMethod(m->nextM, str);
}

/* renvoie si vrai si la méthode est contenue dans la liste */
bool appartient(AttributP var, char* str)
{
	if(var == NIL(Attribut)) {
		return FALSE;
	}
	if(strcmp(var->name, str) == 0) {
		return TRUE;
	}
	return appartient(var->nextA, str);
}

AttributP chercher(AttributP var, char* str)
{
	if(strcmp(var->name, str) == 0) {
		return var;
	}
	return chercher(var->nextA, str);
}



/*
 * class pris en entrée ne doit pas être NULL !
 * On va chercher si l'attribut est "une redefinition" et renvoyer son rang
 * si on ne trouve rien, on retourne -1
 */
int rangAttributMere(ClassP classe, AttributP attribut)
{
    if(classe->parentClass != NIL(Class))
    {
        ClassP classeTemp;
        classeTemp=classe->parentClass;
        do{
            if(appartient(classeTemp->attribut,attribut->name) == TRUE)
            {
                return chercher(classeTemp->attribut, attribut->name)->rang;
            }
            classeTemp = classeTemp->parentClass;
        }while(classeTemp != NIL(Class));
    }

    /* cette variable n'existe pas dans les classes meres, on renvoie -1 */
    return -1;
}

/* renvoie le nombre des arguments d'une classe sans les arguments redefinies */
int vraieTaille(ClassP classe)
{
    if(classe->attribut != NIL(Attribut))
    {
        int taille = addTaille(classe->attribut);
        AttributP attributTemp;
        attributTemp = classe->attribut;

        while(attributTemp != NIL(Attribut))
        {
            /* si un attribut est redefinie, on enleve 1 à la taille de base */
            if(rangAttributMere(classe,attributTemp) != -1)
                taille--;
            attributTemp = attributTemp->nextA;
        }
        return taille;
    }

    return 0;
}

/* renvoie le nombre d'attributs herites
 * i.e : la vraie taille de chaque classe mere
 */
int tailleHerite(ClassP classe){
    int tailleHerite=0;
    ClassP classeTemp;
    classeTemp = classe;

    if(classe->parentClass == NIL(Class))
        return 1;

    while(classeTemp->parentClass != NIL(Class))
    {
        classeTemp=classeTemp->parentClass;
        tailleHerite += vraieTaille(classeTemp);
    }

    return tailleHerite;
}


/* la fonction qui va attribuer les rangs à une classe */
bool setRangAttribut(ClassP classe, AttributP attribut, int rang){
    if(classe == NIL(Class) || attribut == NIL(Attribut))
        return FALSE;

    if(rangAttributMere(classe,attribut) != -1)
    {
        attribut->rang=rangAttributMere(classe,attribut);
    }
    else
    {
        attribut->rang=rang;
        rang++;
    }
    return setRangAttribut(classe,attribut->nextA,rang);
}
