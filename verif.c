#include "verif.h"

/* Global variable that consists in a pile of available constructors */
MethodP firstConst;

/* prend en paramètre une liste de classe et un arbre */
/* va commencer par lancer toutes les vérifications sur les classes, puis va lancer les vérifications sur l'arbre */
/* les arguments sont passés en pointeur car l'arbre va être enrichi */
void verifAppelMethod(ClassP *c, TreeP *t)
{
	if((*c) != NIL(Class)) {
		verifApp(&((*c)->methode));
		verifAppelMethod(&((*c)->nextClass), t);
		return;
	}

	if((*t) == NIL(Tree)) {
		return;
	}

	if((*t)->op == ITE) {
		verifIte((*t));
	}

	if((*t)->op == APPEL || (*t)->op == SELECT) {
		verifAppel(t, NIL(Class));
	}
	else if((*t)->op == EAFF) {
		verifAff((*t));
	}
	else {
		for(int i=0 ; i<(*t)->nbChildren ; i++) {
			verifAppelMethod(c, &((*t)->u.children[i]));
		}
	}
}

/* Prend en argument un arbre d'étiquette ITE et va vérifier que le If then else est correctement utilisé */
void verifIte(TreeP t) {
	ClassP c = verifTypage(t->u.children[0], NIL(Class));
	if(c == getClass("Integer")) {
		return;
	}
	else if(c == NIL(Class)) {
		printf("expression after if must be Integer, undefined type given\n");
		exit(EXIT_FAILURE);
	}
	else {
		printf("expression after if must be Integer, %s given\n", c->name);
		exit(EXIT_FAILURE);
	}
}

/* prend en paramètre un arbre d'étiquette AFF et va vérifier que l'affectation est correcte, modulo héritage */
void verifAff(TreeP t) {
	if(!isSuperClass(verifTypage(t->u.children[1], NIL(Class)), verifAppel(&(t->u.children[0]), NIL(Class)))) {
		printf("Conflicting types for affectation!\n");
		exit(EXIT_FAILURE);
	}
}

/* prend en paramètre une liste de méthodes, et va vérifier que tout est correct (bloc d'instructions de la méthode)*/
void verifApp(MethodP *m) {
	if((*m) == NIL(Method)) {
		return;
	}

	ClassP cl = NIL(Class);
	verifAppelMethod(&cl, &((*m)->instructions));
	verifTypage((*m)->instructions, NIL(Class));
	verifApp(&((*m)->nextM));
}

/* prend en paramètre un arbre et va retourner, si c'est un arbre d'étiquette APPEL, le typage de l'appel , après avoir vérifié tout le typage des éléments */
ClassP verifAppel(TreeP *t, ClassP v) {
	if((*t)==NIL(Tree)) {
		return NIL(Class);
	}

	if(			(*t)->op == IDVAR
			|| 	(*t)->op == LTHIS
			|| 	(*t)->op == LSUPER
			|| 	(*t)->op == LRESULT	) {
		return (*t)->u.lDecl->type;
	}

	if((*t)->op == ECAST) {
		return (*t)->u.children[0]->u.classT;
	}

	if((*t)->op == APPEL) {
		if(			(*t)->u.children[0]->op == EADD
				|| 	(*t)->u.children[0]->op == EMIN
				|| 	(*t)->u.children[0]->op == EDIV
				|| 	(*t)->u.children[0]->op == EMUL
				|| 	(*t)->u.children[0]->op == UADD
				|| 	(*t)->u.children[0]->op == UMIN
				|| 	(*t)->u.children[0]->op == ECONCAT
				|| 	(*t)->u.children[0]->op == EINF
				|| 	(*t)->u.children[0]->op == ESUP
				|| 	(*t)->u.children[0]->op == EEGAL
				|| 	(*t)->u.children[0]->op == EDIFF
				|| 	(*t)->u.children[0]->op == ESUPEG
				|| 	(*t)->u.children[0]->op == EINFEG
				|| 	(*t)->u.children[0]->op == NEWIN	) {
			return verifAppel( &((*t)->u.children[1]), verifTypage((*t)->u.children[0], v));
		}
		if(			(*t)->u.children[0]->op == LTHIS
				||	(*t)->u.children[0]->op == LSUPER
				||	(*t)->u.children[0]->op == ECAST	) {
			return verifAppel( &((*t)->u.children[1]), verifAppel( &((*t)->u.children[0]), v));
		}
		if((*t)->u.children[0]->op == CONST) {
			return verifAppel( &((*t)->u.children[1]), getClass("Integer"));
		}
		if((*t)->u.children[0]->op == ESTR) {
			return verifAppel( &((*t)->u.children[1]), getClass("String"));
		}
		if((*t)->u.children[0]->op == LRESULT) {
			printf("cannot call a function on result keyword\n");
			exit(EXIT_FAILURE);
		}
		if((*t)->u.children[0]->op == ECAST) {
			return verifAppel( &((*t)->u.children[1]), verifAppel( &((*t)->u.children[0]), v));
		}
		if((*t)->u.children[0]->op == IDAPPEL) {
			if(((*t)->u.children[0]->isPointerMethod == FALSE && appartientM(getMethTot(v, NIL(Method)), (*t)->u.children[1], (*t)->u.children[0]->u.str))) {
				(*t)->u.children[0]->isPointerMethod = TRUE;
				char* strTemp = (*t)->u.children[0]->u.str;
				(*t)->u.children[0]->u.meth = chercherMethod(v->methode, strTemp);
				return chercherTypeM(v->methode, strTemp);
			}
			else if((((*t)->u.children[0]->isPointerMethod == TRUE && appartientM(getMethTot(v, NIL(Method)), (*t)->u.children[1], (*t)->u.children[0]->u.meth->name)))) {
				return chercherTypeM(v->methode, (*t)->u.children[0]->u.meth->name);
			}
			else {
				printTree((*t), 0);
				printf("call to undefined method %s()\n", (*t)->u.children[0]->u.str);
				exit(EXIT_FAILURE);
			}
		}
		if((*t)->u.children[1]->op == APPEL) {
			return verifAppel( &((*t)->u.children[1]), verifAppel( &((*t)->u.children[0]), v ) );
		}
	}
	if((*t)->op == SELECT) {
		if((*t)->u.children[1]->isPointerMethod == FALSE && appartient(getEnvTot(verifAppel(&((*t)->u.children[0]), v), NIL(Attribut)), (*t)->u.children[1]->u.str)) {
			char* strTemp = (*t)->u.children[1]->u.str;
			(*t)->u.children[1]->isPointerMethod = TRUE;
			(*t)->u.children[1]->u.lDecl = chercher(verifAppel(&((*t)->u.children[0]), v)->attribut, strTemp);
			return chercher(verifAppel(&((*t)->u.children[0]), v)->attribut, strTemp)->type;
		}
		else if((*t)->u.children[1]->isPointerMethod == TRUE && appartient(getEnvTot(verifAppel(&((*t)->u.children[0]), v), NIL(Attribut)), (*t)->u.children[1]->u.lDecl->name)) {
			return chercher(verifAppel(&((*t)->u.children[0]), v)->attribut, (*t)->u.children[1]->u.lDecl->name)->type;
		}
		else {
			printf("undeclared identifier %s\n", (*t)->u.children[1]->u.str);
			exit(EXIT_FAILURE);
		}
	}
	return NIL(Class);
}

/* prend en paramètre un arbre et un attribut qui doit être initialisé à NIL. Va renvoyer une liste d'attributs constituée des arguments trouvés dans l'arbre, avec leurs types */
AttributP getArguments(TreeP t, AttributP a)
{
		if(t == NIL(Tree)) {
			return NIL(Attribut);
		}
		if(getChild(t, 0)->op == APPEL || getChild(t, 0)->op == SELECT) {
			if(a == NIL(Attribut)) {
				a = makeAttribut("", verifAppel(&(t->u.children[0]), NIL(Class)), NIL(Tree));
			}
			else {
				linkParamSignature(a, makeAttribut("", verifAppel(&(t->u.children[0]), NIL(Class)), NIL(Tree)));
			}
		}
		if(getChild(t, 0)->op == IDVAR) {
			//getChild(t, 0)->u.lDecl->nextA = NIL(Attribut);
			if(a == NIL(Attribut)) {
				a = makeAttribut("", getChild(t, 0)->u.lDecl->type, getChild(t, 0)->u.lDecl->expression);
			}
			else {
				linkParamSignature(a, makeAttribut("", getChild(t, 0)->u.lDecl->type, getChild(t, 0)->u.lDecl->expression));
			}
		}
		if(getChild(t, 0)->op == CONST) {
			if(a == NIL(Attribut)) {
				a = makeAttribut("", getClass("Integer"), makeLeafInt(CONST, getChild(t, 0)->u.val));
			}
			else {
				linkParamSignature(a, makeAttribut("", getClass("Integer"), makeLeafInt(CONST, getChild(t, 0)->u.val)));
			}
		}
		if(			getChild(t, 0)->op == EADD
				|| 	getChild(t, 0)->op == EMIN
				|| 	getChild(t, 0)->op == EDIV
				|| 	getChild(t, 0)->op == EMUL
				|| 	getChild(t, 0)->op == UADD
				|| 	getChild(t, 0)->op == UMIN
				|| 	getChild(t, 0)->op == ECONCAT
				|| 	getChild(t, 0)->op == EINF
				|| 	getChild(t, 0)->op == ESUP
				|| 	getChild(t, 0)->op == EEGAL
				|| 	getChild(t, 0)->op == EDIFF
				|| 	getChild(t, 0)->op == ESUPEG
				|| 	getChild(t, 0)->op == EINFEG
				|| 	getChild(t, 0)->op == NEWIN		) {
			if(a == NIL(Attribut)) {
				a = makeAttribut("", verifTypage(getChild(t, 0), NIL(Class)), NIL(Tree));
			}
			else {
				linkParamSignature(a, makeAttribut("", verifTypage(getChild(t, 0), NIL(Class)), NIL(Tree)));
			}
		}
		if(getChild(t, 0)->op == ESTR) {
			if(a == NIL(Attribut)) {
				a = makeAttribut("", getClass("String"), makeLeafStr(ESTR, getChild(t, 0)->u.str));
			}
			else {
				linkParamSignature(a, makeAttribut("", getClass("String"), makeLeafStr(ESTR, getChild(t, 0)->u.str)));
			}
		}

		if(getChild(t, 1) == NIL(Tree)) {
			return a;
		}
		if(getChild(t, 1)->op == LARG) {
			return getArguments(getChild(t, 1), a);
		}

		return a;
}

MethodP getMethTot(ClassP c, MethodP m)
{
	if(c == NIL(Class)) {
		return m;
	}
	ajouterMethod(&m, c->methode);
	getMethTot(c->parentClass, m);
	return m;
}

/* permet de lier la méthode elem à la dernière méthode de la liste list */
bool linkMethod(MethodP list, MethodP elem)
{
	if(elem != NIL(Method) && strcmp(list->name, elem->name) == 0) {
		return TRUE;
	}
	else if(list->nextM == NIL(Method)) {
		list->nextM = elem;
	}
	else {
		return linkMethod(list->nextM, elem);
	}
	return TRUE;
}


/* permet d'ajouter une méthode v2 à une liste de méthodes v passée en pointeur */
void ajouterMethod(MethodP *v, MethodP v2)
{
	if(v2==NIL(Method)) {
		return;
	}
	if(*v==NIL(Method)) {
		*v = v2;
	}
	else {
		linkMethod(*v, v2);
	}
}


/* prend en paramètre un arbre d'étiquette NEWIN et va retourner le type de l'objet créé, après vérification de l'utilisation du constructeur */
ClassP getNew(TreeP t)
{
	if(getChild(t, 0)->isPointerMethod == FALSE && appartientM(firstConst, getChild(t, 1), getChild(t, 0)->u.str)) {
		char* strTemp = getChild(t, 0)->u.str;
		getChild(t, 0)->isPointerMethod = TRUE;
		getChild(t, 0)->u.meth = chercherMethod(firstConst, strTemp);
		return getClass(strTemp);
	}
	else if(getChild(t, 0)->isPointerMethod == TRUE && appartientM(firstConst, getChild(t, 1), getChild(t, 0)->u.meth->name)) {
		return getClass(getChild(t, 0)->u.meth->name);
	}
	else {
		printf("Constructeur appel incorrect : %s\n", getChild(t, 0)->u.str);
		exit(EXIT_FAILURE);
	}
	return NIL(Class);
}

/* Permet de compter le nombre d'attributs dans une liste a, nb doit être initialisé à 0 */
int compterArg(AttributP a, int nb)
{
	if(a == NIL(Attribut)) {
		return nb;
	}
	return compterArg(a->nextA, nb+1);
}

/* prend en paramètre deux listes d'attributs, renvoie vrai si les deux listes sont strictement égales en terme de nombre et de types, modulo héritage */
int compareArg(AttributP a1, AttributP a2)
{
	if(a1 == NIL(Attribut)) {
		if(a2 == NIL(Attribut)) {
			return 1;
		}
		else {
			printf("incompatible call with method signature, arguments differ\n");
			exit(EXIT_FAILURE);
			return 0;
		}
	}
	if(a2 == NIL(Attribut)) {
		if(a1 == NIL(Attribut)) {
			return 1;
		}
		else {
			printf("incompatible call with method signature, arguments differ\n");
			exit(EXIT_FAILURE);
			return 0;
		}
	}
	if(compterArg(a1, 0) != compterArg(a2, 0)) {
		printf("expecting %d arguments here, %d provided\n", compterArg(a1, 0), compterArg(a2, 0));
		exit(EXIT_FAILURE);
		return 0;
	}
	if(isSuperClass(a2->type, a1->type)) {
		return compareArg(a1->nextA, a2->nextA);
	}
	else {
		printf("incompatible argument type, %s is no super type of %s\n", a2->type->name, a1->type->name);
		exit(EXIT_FAILURE);
	}

	return 0;
}

/* prend en paramètre une liste de méthodes, un nom de méthode au format texte et un arbre représentant les arguments de la méthode, et va comparer toutes les signatures. Renvoie vrai si la méthode $str existe dans la liste */
bool appartientM(MethodP m, TreeP t, char* str)
{
	if(m == NIL(Method)) {
		return FALSE;
	}
	if(strcmp(m->name, str) == 0 ) {
		if(compareArg(m->signature, getArguments(t, NIL(Attribut))) == 1) {
			return TRUE;
		}
	}
	return appartientM(m->nextM, t, str);
}

/* Prend en paramètre un nom de méthode (format texte) et une liste de méthodes, et renvoie le type de retour si la méthode est trouvée dans la liste */
/* Ne prend pas en compte le cas NIL car cette fonction est à utiliser TOUJOURS après avoir vérifié que la méthode appartient bien à la liste avec appartientM */
ClassP chercherTypeM(MethodP m, char* str)
{
	if(strcmp(m->name, str) == 0) {
		return m->typeRetour;
	}
	return chercherTypeM(m->nextM, str);
}


/* prend en paramètre un nom de variable et un environement, et renvoie le type (au format texte) de la variable si elle est trouvée dans cet environnement, NIL(char) sinon. */
char* getType(AttributP env, char* nom)
{
	if(env == NIL(Attribut)) {
		printf("La variable %s n'a jamais ete instanciee.\n", nom);
		exit(EXIT_FAILURE);
		return NIL(char);
	}
	if(strcmp(env->name, nom) == 0) {
		return env->type->name;
	}
	return getType(env->nextA, nom);
}

/*
 * Verifie si une operation d'opérateur binaire est possible
 */
bool verifOpBinary(TreeP bloc, AttributP env, char* type)
{
	if(bloc == NIL(Tree)) {
		printf("binary operator error\n");
		exit(EXIT_FAILURE);
		return FALSE;
	}
	else if (bloc->nbChildren != 2) {
		printf("binary operator expecting 2 operands, %d given\n", bloc->nbChildren);
		exit(EXIT_FAILURE);
		return FALSE;
	}

	for(int i=0 ; i<bloc->nbChildren ; i++)
	{
		if (getChild(bloc, i)->op == CONST && strcmp(type, "Integer") != 0) {
			printf("binary operator incompatible types\n");
			exit(EXIT_FAILURE);
			return FALSE;
		}
		if (getChild(bloc, i)->op == ESTR && strcmp(type, "String") != 0) {
			printf("binary operator incompatible types\n");
			exit(EXIT_FAILURE);
			return FALSE;
		}
		if (getChild(bloc, i)->op == IDVAR) {
			if(strcmp(getChild(bloc, i)->u.lDecl->type->name, type) != 0) {
				printf("binary operator incompatible types\n");
				exit(EXIT_FAILURE);
				return FALSE;
			}
		}
		if (getChild(bloc, i)->op == APPEL) {
			if(strcmp(verifAppel(&((*bloc).u.children[i]), NIL(Class))->name, type) != 0) {
				printf("binary operator incompatible types\n");
				exit(EXIT_FAILURE);
				return FALSE;
			}
		}
	}

	return TRUE;
}


/*
 * Verifie si une operation d'operateur unaire est possible
 */
bool verifOpUnary(TreeP bloc, AttributP env, char* type)
{
	if(bloc == NIL(Tree)) {
		printf("unary operator error\n");
		exit(EXIT_FAILURE);
		return FALSE;
	}
	else if (bloc->nbChildren != 1) {
		printf("unary operator expecting 1 operand, %d given\n", bloc->nbChildren);
		exit(EXIT_FAILURE);
		return FALSE;
	}
	if (getChild(bloc, 0)->op == CONST && strcmp(type, "Integer") == 0)
		return TRUE;
	if (getChild(bloc, 0)->op == ESTR && strcmp(type, "String") == 0)
		return TRUE;
		if (getChild(bloc, 0)->op == IDVAR) {
			if(strcmp(getChild(bloc, 0)->u.lDecl->type->name, type) != 0) {
				printf("unary operator incompatible type\n");
				exit(EXIT_FAILURE);
				return FALSE;
			}
		}
		if (getChild(bloc, 0)->op == APPEL) {
			if(strcmp(verifAppel(&((*bloc).u.children[0]), NIL(Class))->name, type) != 0) {
				printf("binary operator incompatible types\n");
				exit(EXIT_FAILURE);
				return FALSE;
			}
		}

	return TRUE;
}

/* Renvoie VRAI si subclass est bien une sous classe de superclass. VRAI aussi si subclass = superclass */
bool isSuperClass(ClassP subclass, ClassP superclass) {
	if(subclass == NIL(Class)) {
		printf("Bad call to isSuperClass : subclass is NIL\n");
		printf("Other operand is : %s\n", superclass->name);
	}
	if(superclass == NIL(Class)) {
		printf("Bad call to isSuperClass : superclass is NIL\n");
		printf("Other operand is : %s\n", subclass->name);
	}
	if(strcmp(subclass->name, superclass->name) == 0) {
		return TRUE;
	}
	if(subclass->parentClass == NIL(Class)) {
		return FALSE;
	}
	if(strcmp(subclass->parentClass->name, superclass->name) == 0) {
		return TRUE;
	}
	else {
		return isSuperClass(subclass->parentClass, superclass);
	}
	return FALSE;
}

/* prend en paramètre un arbre, un environement, une classe et une méthode, et va effectuer les vérifications contextuelles */
int verifTree(TreeP *t, AttributP *var, int isMethod, ClassP c, MethodP m)
{
	if((*t) == NIL(Tree)) {
		return 1;
	}
	/*if((*t)->op == BLOCK) {

	}*/
	if((*t)->op == DECL) {

		verifAttribut2(&((*t)->u.lDecl), *var, isMethod, c, m, 0);

		ajouter(var, (*t)->u.lDecl);
	}
	if((*t)->op == ERETURN) {
		if(!isMethod) {
			printf("cannot use return outside a method\n");
			exit(EXIT_FAILURE);
		}
	}
	if((*t)->op == EAFF) {
		if(strcmp((*t)->u.children[0]->u.str, "this") == 0) {
			printf("cannot affect something to this\n");
			exit(EXIT_FAILURE);
		}
		if(strcmp((*t)->u.children[0]->u.str, "super") == 0) {
			printf("cannot affect something to super\n");
			exit(EXIT_FAILURE);
		}
	}
	if((*t)->op == ECASTCLASS) {
		if(getClass((*t)->u.str) == NIL(Class)) {
			printf("cannot cast into inexistant type : %s\n", (*t)->u.str);
			exit(EXIT_FAILURE);
		}
		else {
			(*t)->u.classT = getClass((*t)->u.str);
		}
	}

	if((*t)->op == ECAST) {
		verifTree(&((*t)->u.children[1]), var, isMethod, c, m);
		verifTree(&((*t)->u.children[0]), var, isMethod, c, m);
		if(!isSuperClass((*t)->u.children[1]->u.lDecl->type, (*t)->u.children[0]->u.classT)) {
			printf("cannot cast into type (%s) that does not super %s\n", (*t)->u.children[0]->u.classT->name, (*t)->u.children[1]->u.lDecl->type->name);
			exit(EXIT_FAILURE);
		}
		return 1;
	}

	if((*t)->op == IDVAR) {
		if(strcmp((*t)->u.str, "this") == 0) {
			if(isMethod) {
				(*t)->op = LTHIS;
				(*t)->u.lDecl = makeAttribut("this", c, NIL(Tree));
			}
			else {
				printf("this keyword must be in a method \n");
				exit(EXIT_FAILURE);
			}
		}
		else if(strcmp((*t)->u.str, "super") == 0) {
			if(isMethod) {
				if(c->parentClass != NIL(Class)) {
					(*t)->op = LSUPER;
					(*t)->u.lDecl = makeAttribut("super", c->parentClass, NIL(Tree));
				}
				else {
					printf("super can only be used in a sub-class method");
					exit(EXIT_FAILURE);
				}
			}
			else {
				printf("super keyword must be in a method \n");
				exit(EXIT_FAILURE);
			}
		}
		else if(strcmp((*t)->u.str, "result") == 0) {
			if(isMethod) {
				if(m->typeRetour == NIL(Class)) {
					printf("result keyword must be used in a non-void method \n");
					exit(EXIT_FAILURE);
				}
				(*t)->op = LRESULT;
				(*t)->u.lDecl = makeAttribut("result", m->typeRetour, NIL(Tree));
			}
			else {
				printf("result keyword must be in a method \n");
				exit(EXIT_FAILURE);
			}
		}
		else if(appartient(*var, (*t)->u.str))
		{
			(*t)->u.lDecl = chercher(*var, (*t)->u.str);
		}

		else
		{
			if(isMethod) {
				printf("Methode : %s\n", m->name);
			}
			printf("Variable introuvable : %s\n", (*t)->u.str);
			exit(EXIT_FAILURE);
			return FALSE;
		}
	}

	for(int i=0 ; i<(*t)->nbChildren ; i++)
	{
		if((*t)->u.children[i] != NIL(Tree) && (*t)->u.children[i]->op == BLOCK)
		{
			AttributP varTemp;
			if(*var == NIL(Attribut)) {
				varTemp = NIL(Attribut);
			}
			else {
				varTemp = NEW(1, Attribut);
				copyAttribut(*var, varTemp);
			}
			verifTree(&((*t)->u.children[i]), &varTemp, isMethod, c, m);
		}

		else
			verifTree(&((*t)->u.children[i]), var, isMethod, c, m);
	}
	return 1;
}

/* Permet de copier un environement src (ie. une liste d'attributs) vers un autre environement dst */
void copyAttribut(AttributP src, AttributP dst) {
	if(src == NIL(Attribut)) {
		return;
	}

	dst->name = src->name;
	dst->type = src->type;
	dst->expression = src->expression;
	if(src->nextA == NIL(Attribut)) {
		return;
	}

	dst->nextA = NEW(1, Attribut);
	copyAttribut(src->nextA, dst->nextA);
}

/* prend en argument une liste de méthodes, un environement et une classe */
/* cette fonction va copier l'environement dans un environement temporaire, puis va éxécuter dessus la vérification du corps de chaque méthode,
en prennant soin d'ajouter pour chaque méthode les arguments de celle-ci dans l'environnement temporaire */
void verifMethod(MethodP m, AttributP var, ClassP c)
{
	int nbArgsSign = compterArg(m->signature, 0);

	verifArgs(m->signature);

	AttributP varTemp;
	AttributP varSign;

	if(var == NIL(Attribut)) {
		varTemp = NIL(Attribut);
	}
	else {
		varTemp = NEW(1, Attribut);
		copyAttribut(var, varTemp);
	}

	if(m->signature == NIL(Attribut)) {
		varSign = NIL(Attribut);
	}
	else {
		varSign = NEW(nbArgsSign, Attribut);
		copyAttribut(m->signature, varSign);
	}

	ajouter(&varTemp, varSign);
	TreeP tr = m->instructions;
	verifTree(&tr, &varTemp, TRUE, c, m);
	varTemp = NIL(Attribut);
	varSign = NIL(Attribut);

	if(m->nextM != NIL(Method)) {
		verifMethod(m->nextM, var, c);
	}
}

/* prend en paramètre une classe et renvoie l'environnement total (classes mères) */
AttributP getEnvTot(ClassP c, AttributP a)
{
	if(c == NIL(Class)) {
		return a;
	}

	ajouterHerite(&a, c->attribut);
	return getEnvTot(c->parentClass, a);
}

/* prend en paramètre une liste de classes, un arbre et un environement, et va lancer la vérification de portée sur tous ces éléments */
void verifPortee(ClassP c, TreeP *t, AttributP var)
{
	if(c != NIL(Class)) {
		ajouter(&var, getEnvTot(c, NIL(Attribut)));
		verifAttribut(&(c->attribut), var, FALSE, c, NIL(Method));
		verifMethod(c->methode, var, c);
		verifPortee(c->nextClass, t, NIL(Attribut));
	}
	else {
		AttributP va = NIL(Attribut);
		verifTree(t, &va, FALSE, NIL(Class), NIL(Method));
	}
}

/* Cette fonction prend en paramètre une liste d'attributs et vérifie l'utilisation de mots clefs */
void verifArgs(AttributP args) {
	if(args == NIL(Attribut)) {
		return;
	}

	if(			(strcmp(args->name, "this") == 0)
			||	(strcmp(args->name, "super") == 0)
			||	(strcmp(args->name, "result") == 0)	) {
			printf("%s is a reserved keyword, used as argument here\n", args->name);
			exit(EXIT_FAILURE);
	}

	verifArgs(args->nextA);
}

void verifAttribut(AttributP *attr, AttributP var, bool isMethod, ClassP c, MethodP m) {
	ClassP cl = NIL(Class);

	if((*attr) == NIL(Attribut)) {
		return;
	}
	else {
		printf("J'ai vu %s\n", (*attr)->name);
	}

	if(			(strcmp((*attr)->name, "this") == 0)
			||	(strcmp((*attr)->name, "super") == 0)
			||	(strcmp((*attr)->name, "result") == 0)	) {
			printf("%s is a reserved keyword, used in declaration here.\n", (*attr)->name);
			exit(EXIT_FAILURE);
	}

	verifTree(&((*attr)->expression), &var, isMethod, c, m);
	if((*attr)->expression != NIL(Tree) && !isSuperClass(verifTypage((*attr)->expression, NIL(Class)), (*attr)->type)) {
		printf("incompatible types in affectation\n");
		exit(EXIT_FAILURE);
	}
	verifAppelMethod(&cl, &((*attr)->expression));

	verifAttribut(&((*attr)->nextA), var, isMethod, c, m);
}

void verifAttribut2(AttributP *attr, AttributP var, bool isMethod, ClassP c, MethodP m, int rangi) {
	ClassP cl = NIL(Class);

	if((*attr) == NIL(Attribut)) {
		return;
	}
	else {
		printf("J'ai vu %s\n", (*attr)->name);
	}

	if(			(strcmp((*attr)->name, "this") == 0)
			||	(strcmp((*attr)->name, "super") == 0)
			||	(strcmp((*attr)->name, "result") == 0)	) {
			printf("%s is a reserved keyword, used in declaration here.\n", (*attr)->name);
			exit(EXIT_FAILURE);
	}
	(*attr)->rang = rangi;

	verifTree(&((*attr)->expression), &var, isMethod, c, m);
	if((*attr)->expression != NIL(Tree) && !isSuperClass(verifTypage((*attr)->expression, NIL(Class)), (*attr)->type)) {
		printf("incompatible types in affectation\n");
		exit(EXIT_FAILURE);
	}
	verifAppelMethod(&cl, &((*attr)->expression));

	verifAttribut2(&((*attr)->nextA), var, isMethod, c, m, rangi+1);
}

/* prend en argument une liste d'attributs et un attribut, et va ajouter elem à la fin de la liste list, tout en masquant l'attribut si il existait déja (dans une relation d'héritage) */
bool linkParamSupHerite(AttributP list, AttributP elem)
{
  if (elem == NIL(Attribut)) {
		return FALSE;
	}
  if( strcmp(list->name, elem->name) == 0) {
    return TRUE;
  }
  else if(list->nextA == NIL(Attribut)) {
		list->nextA = elem;
	}
  else {
		return linkParamSupHerite(list->nextA, elem);
	}

  return TRUE;
}

/* prend en argument une liste d'attributs et un attribut, et va ajouter elem à la fin de la liste list */
bool linkParamSup(AttributP list, AttributP elem)
{
  if (elem == NIL(Attribut)) {
		return FALSE;
	}

  if( strcmp(list->name, elem->name) == 0) {
		list->type = elem->type;
		list->expression = elem->expression;
    return TRUE;
  }
  else if(list->nextA == NIL(Attribut)) {
		list->nextA = elem;
	}
  else {
		return linkParamSup(list->nextA, elem);
	}

  return TRUE;
}

/* Ajoute l'attribut v2 à l'environement v, passé par pointeur */
void ajouter(AttributP *v, AttributP v2)
{
	if(v2==NIL(Attribut)) {
		return;
	}
	if(*v==NIL(Attribut)) {
		*v = v2;
	}
	else {
		linkParamSup(*v, v2);
	}
}

void ajouterHerite(AttributP *v, AttributP v2)
{
	if(v2==NIL(Attribut)) {
		return;
	}
	if(*v==NIL(Attribut)) {
		*v = v2;
	}
	else {
		linkParamSupHerite(*v, v2);
	}
}


/* Prend en paramètre un arbre et une classe (qui peut être NIL) et renvoie le type de l'expression considérée */
ClassP verifTypage(TreeP bloc, ClassP v)
{
	if(bloc == NIL(Tree)) {
		return NIL(Class);
	}

	ClassP c = NIL(Class);
	switch(bloc->op) {
		case IDVAR:
		return bloc->u.lDecl->type;
		break;

		case APPEL:
		return verifAppel(&bloc, v);
		break;

		case SELECT:
		return verifAppel(&bloc, v);
		break;

		case LTHIS:
		c = verifAppel(&bloc, v);
		break;

		case LSUPER:
		c = verifAppel(&bloc, v);
		break;

		case LRESULT:
		c = verifAppel(&bloc, v);
		break;

		case CONST:
		c = getClass("Integer");
		break;

		case ESTR:
		c = getClass("String");
		break;

		case EADD:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case EMIN:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case EDIV:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case EMUL:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case UADD:
		verifOpUnary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case UMIN:
		verifOpUnary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case ECONCAT:
		verifOpBinary(bloc, NIL(Attribut), "String");
		c = getClass("String");
		break;

		case EEGAL:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case EDIFF:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case EINF:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case ESUP:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case EINFEG:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case ESUPEG:
		verifOpBinary(bloc, NIL(Attribut), "Integer");
		c = getClass("Integer");
		break;

		case NEWIN:
		return getNew(bloc);
		break;

		default:;
	}
	for(int i=0 ; i<bloc->nbChildren ; i++) {
		verifTypage(getChild(bloc, i), v);
	}

	return c;
}
