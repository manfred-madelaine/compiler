#include "code.h"
#include "verif.h"

char* startLabel = "main";
int unique = 0;
FILE *out;

void coDegeneration(ClassP class, TreeP tree)
{
	commenter("Debut de la GCI\n\n", 0);

	commenter("Génération des Tables virtuelles pour chacune des classes\n", 0);
	codeClassPrimitive();
	codeClass(class);

	codeBlocks(class, tree);
}


bool codeClassPrimitive()
{
	int indentlevel = 1;

	newLabel("Integer");
	fprintf(out, "ALLOC 2");
	commenter("allocation de la TV d'une classe (Integer)", 0);

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "PUSHA ");
	methodLabel("Integer", "toString");
	commenter("ajout meth", 0);

	indent(indentlevel);
	fprintf(out, "STORE 1\t");
	commenter("stockage du champ dans la TV", 0);


	newLabel("String");
	fprintf(out, "ALLOC 3");
	commenter("allocation de la TV d'une classe (String)", 0);

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "PUSHA ");
	methodLabel("String", "println");
	commenter("ajout meth", 0);

	indent(indentlevel);
	fprintf(out, "STORE 1\t");
	commenter("stockage du champ dans la TV", 0);

	indent(indentlevel);
	dupliquer(1);
	fprintf(out, "\n");

	indent(indentlevel);
	fprintf(out, "PUSHA ");
	methodLabel("String", "print");
	commenter("ajout meth", 0);

	indent(indentlevel);
	fprintf(out, "STORE 2\t");
	commenter("stockage du champ dans la TV", 0);

	return TRUE;
}

bool codeMethodPrimitive()
{
	int indentlevel = 1;

	methodLabel("Integer", "toString");
	fprintf(out, ":");
	fprintf(out, "NOP\n");

	indent(indentlevel);
	fprintf(out, "LOAD 1\n");
	
	indent(indentlevel);
	fprintf(out, "STR\n");
	fprintf(out, "RETURN\n\n");

	methodLabel("String", "print");
	fprintf(out, ":");
	fprintf(out, "NOP\n");

	indent(indentlevel);
	fprintf(out, "LOAD 1\n");

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "WRITES\n");
	fprintf(out, "RETURN\n\n");

	methodLabel("String", "println");
	fprintf(out, ":");
	fprintf(out, "NOP\n");

	indent(indentlevel);
	fprintf(out, "LOAD 1\n");

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "WRITES\n");
	// retour à la ligne
	indent(indentlevel);
	fprintf(out, "PUSHS %s\n", "\"\\n\"");
	indent(indentlevel);
	fprintf(out, "WRITES\n");

	fprintf(out, "RETURN\n\n");

	return TRUE;
}

/*
 * Cree une table virtuelle avec n methodes
 */
bool codeClass(ClassP c)
{
	if (c == NIL(Class))
		return FALSE;

	if(c->name != NIL(char))
		newLabel(c->name);

	fprintf(out, "ALLOC %d", c->tailleMeth);
	commenter("allocation de la TV d'une classe", 0);

	// On alloue l'espace memoire du parent
	codeParent(c->parentClass, 1);

	allocMeths(c->methode, c->name, 1);

	return codeClass(c->nextClass);
}

/*
 * On alloue l'espace memoire du parent
 */
bool codeParent(ClassP c, int indentlevel)
{
	if (c == NIL(Class))
		return FALSE;

	commenter("allocation parent", indentlevel);

	codeParent(c->parentClass, indentlevel);

	allocMeths(c->methode, c->name, indentlevel);

	return TRUE;
}

void indent(int level)
{
	for (int i=0; i<level; i++)
		fprintf(out, "\t");
}

/*
 * Gere l'alocation des methodes
 */
bool allocMeths(MethodP m, char* nomClass, int indentlevel)
{
	if (m == NIL(Method))
		return FALSE;

	indent(indentlevel);
	dupliquer(1);

	if(m->rang == 0)
		commenter("Définition du Constructeur", indentlevel);
	if(m->redefinition)
		commenter("Redéfinition de méthode mère", indentlevel);

	indent(indentlevel);
	fprintf(out, "PUSHA ");
	methodLabel(nomClass, m->name);

	if(m->redefinition)
		fprintf(out, "%d", m->rang);
	
	commenter("champ: nomObjet_nomMethode", indentlevel);

	indent(indentlevel);
	fprintf(out, "STORE %d\t", m->rang);
	commenter("stockage du champ dans la TV", indentlevel);

	if(m->nextM != NIL(Method))
		fprintf(out, "\n");

	return allocMeths(m->nextM, nomClass, indentlevel);
}


void methodLabel(char* nomClass, char* nomMeth)
{
	fprintf(out, "%s_%s", nomClass, nomMeth);
}


void codeBlocks(ClassP c, TreeP tree)
{
	fprintf(out, "* JUMP %s\n\n\n", startLabel);

	commenter("Blocs de methode :\n", 0);

	// génération des blocs de méthode des classes prédéfinies
	codeMethodPrimitive();

	// génération des blocs de méthode des classes
	blockMethode(c);
	fprintf(out, "\n\n\n");

	// génération des appels de fonction
	blockFonction(c);
	fprintf(out, "\n\n\n");

	//debut du main
	setDebut();
	blocMain(tree);
	viderPile(c);
	setFin();
}


bool viderPile(ClassP c)
{
	if (c == NIL(Class))
		return FALSE;

	fprintf(out, "POPN 1\n");
	return viderPile(c->nextClass);
}


bool blockMethode(ClassP class)
{
	if(class == NIL(Class))
		return FALSE;

	codeMeth(class->name, class->methode);

	return blockMethode(class->nextClass);
}


bool codeMeth(char* nomClass, MethodP methode)
{
	if(methode == NIL(Method))
		return FALSE;

	commenter("bloc d'une methode", -1);
	methodLabel(nomClass, methode->name);
	if(methode->redefinition){
		fprintf(out, "%d", methode->rang);
	}
	fprintf(out, ": NOP\n");

	int indentlevel = 0;


	fprintf(out, "\n");
	commenter("Allocation des parametres: verifier que l'alocation se fait au bon endroit", indentlevel);
	allocChamps(methode->signature, indentlevel);

	fprintf(out, "\n");
	codeTree(methode->instructions, -1);

	fprintf(out, "RETURN\n");

	if(methode->nextM != NIL(Method))
		fprintf(out, "\n");

	return codeMeth(nomClass, methode->nextM);
}


bool blockFonction(ClassP class)
{
	if(class == NIL(Class))
		return FALSE; 

	invoke2(class->methode,class->rang, 0);

	return blockFonction(class->nextClass);
}


/*
 * Renvoie la valeur de la plus longue chaine de méthode parmis une liste de classes
 */
int maxMeth(ClassP listC)
{
	if(listC == NIL(Class))
		return 0;

	int tailleMeth = listC->tailleMeth;
	if(listC->nextClass == NIL(Class))
		return tailleMeth;

	int tailleNextMeth = maxMeth(listC->nextClass);

	return (tailleMeth < tailleNextMeth) ? tailleNextMeth : tailleMeth;
}


bool invoke2(MethodP m, int offsetTV, int indentlevel)
{
	if (m == NIL(Method))
		return FALSE;

	indent(indentlevel);
	fprintf(out, "call%d: NOP\t-- rang = %d\n", m->rang+1, m->rang);

	commenter("Emplacement du résultat", indentlevel);
	gestionResultat(m->typeRetour, indentlevel);

	commenter("charger les Param!!", indentlevel);

	fprintf(out, "\n");
	commenter("Allocation des parametres: 'a faire)'", indentlevel);

	fprintf(out, "PUSHL -1");
	commenter("Récupère le recepteur", 0);

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "LOAD %d", 0);
	commenter("charger Table Virtuelle", 0);

	indent(indentlevel);
	fprintf(out, "LOAD %d", m->rang+1);
	commenter("charger @methode", 0);

	indent(indentlevel);
	fprintf(out, "CALL\n");
	indent(indentlevel);

	fprintf(out, "-- STORE %d\t\tle résultat !!\n",m->rang);

	fprintf(out, "RETURN\n\n\n");

	return invoke2(m->nextM, offsetTV, indentlevel);
}


void newLabel(char* nomEtiquette)
{
	fprintf(out, "%s_%d:", nomEtiquette, unique);
	unique++;
}


void setDebut()
{
	fprintf(out, "%s:", startLabel);
	fprintf(out, "START\n");
}

void setFin()
{
	newLabel("finMain");
	fprintf(out, "STOP\n\n");
}


void endMeth()
{
	fprintf(out, "RETURN\n");
}


/*
 * charge des objets via PUSHL
 */
void loadObjectL(int offset, int offsetChamp)
{
	fprintf(out, "PUSHL %d\n", offset);
	fprintf(out, "LOAD %d\n", offsetChamp );
}



void blocMain(TreeP tree)
{
	codeTree(tree, -2);
}


void pushI(int val)
{
    fprintf(out, "PUSHI %d\n", val);
}

void pushS(char* txt)
{
    fprintf(out, "PUSHS %s\n", txt);
}


void commenter(char* texte, int indentlevel)
{
	indent(indentlevel+2);
	fprintf(out, "-- %s\n", texte);
}

void concatenation(TreeP t, int indentlevel)
{
	codeTreeBoucle(t, indentlevel);

	fprintf(out, "CONCAT\n");
}


void codeTreeBoucle(TreeP bloc, int indentlevel)
{
	for(int j=0; j<bloc->nbChildren; j++)
		codeTree(bloc->u.children[j], indentlevel);
}


void binaryOp(TreeP bloc, int indentlevel)
{
	codeTreeBoucle(bloc, indentlevel);

	switch(bloc->op)
	{
		case EADD:
			indent(indentlevel);
        	fprintf(out, "ADD\n");
		    break;

		case EMIN:
			indent(indentlevel);
        	fprintf(out, "SUB\n");
		    break;

		case EDIV:
			indent(indentlevel);
        	fprintf(out, "DIV\n");
		    break;

		case EMUL:
			indent(indentlevel);
        	fprintf(out, "MUL\n");
		    break;

		default:
			commenter("operation non prise en compte",indentlevel);
	}
}


void comparator(TreeP bloc, int indentlevel)
{
	codeTreeBoucle(bloc, indentlevel);

	switch(bloc->op)
	{
		case EINF:
			indent(indentlevel);
        	fprintf(out, "INF\n");
		    break;

		case ESUP:
			indent(indentlevel);
        	fprintf(out, "SUP\n");
		    break;

		case EINFEG:
			indent(indentlevel);
        	fprintf(out, "INFEQ\n");
		    break;

		case ESUPEG:
			indent(indentlevel);
        	fprintf(out, "SUPEQ\n");
		    break;

		case EEGAL:
			indent(indentlevel);
        	fprintf(out, "EQUAL\n");

		    break;

		case EDIFF:
			indent(indentlevel);
        	fprintf(out, "SUB\n");
			indent(indentlevel);
        	fprintf(out, "NOT\n");
		    break;

		default:
			commenter("comparaison non prise en compte", indentlevel);
	}
}


void condition(TreeP tree, int indentlevel)
{
	char* elseLabel = "else";

	codeTree(getChild(tree, 0), indentlevel);
	fprintf(out, "JZ %s_%d\n", elseLabel, unique);

	codeTree(getChild(tree, 1), indentlevel);

	fprintf(out, "%s_%d:", elseLabel, unique);
	fprintf(out, "NOP\n");
	unique++;
	codeTree(getChild(tree, 2), indentlevel);
}

void dupliquer(int nPremieresVal)
{
	fprintf(out, "DUPN %d\n", nPremieresVal);
}

void affectation(TreeP tree, int indentlevel)
{
	// Génère le code de la partie gauche -> adresse de stockage
	codeTree(getChild(tree, 0), indentlevel);

	// Génère le code de l'expression pour obtenir une valeur en sommet de pile
	codeTree(getChild(tree, 1), indentlevel);

	// Stock la valeur dans l'adresse: adresse de stockage + offset
	indent(indentlevel);
	fprintf(out, "STORE %d\n", 0); //mettre le rang pour obtenir le bon offset
	// fprintf(out, "POPN 1\n");
}


void afficherTexte(char* texte)
{
	fprintf(out, "PUSHS \"%s\" \n", texte);
	fprintf(out, "WRITES\n");
	// afficherTexte(tree->u.str);
}


void afficherVal(int offset, int offsetChamp)
{
	loadObjectL(offset, offsetChamp);
	fprintf(out, "WRITEI\n");
}


/*
 * Cree une Instance de classe avec n champs
 */
bool allocObjet(char* etiquette, ClassP c, int indentlevel)
{
	if (c == NIL(Class))
		return FALSE;

	// On n'alloue pas de place pour les types prédef
	if(typePredefini(c))
		return TRUE;

	if(etiquette != NIL(char))
		newLabel(etiquette);

	indent(indentlevel);
	fprintf(out, "ALLOC %d\t-- allocation d'un objet : %s\n", c->tailleAttribut, etiquette);

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "PUSHG %d", c->rang);
	commenter("charge la TV de la bonne classe", 0);
	indent(indentlevel);
	fprintf(out, "STORE %d\n", 0);

	return TRUE;
}


bool typePredefini(ClassP maClass)
{
  if(strcmp(maClass->name, "Integer") == 0 || strcmp(maClass->name, "String") == 0 )
    return TRUE;

  return FALSE;
}


void appel(MethodP methode, AttributP listParam, int indentlevel)
{
	commenter("appel de methode via un Id", -1);
	// on charge l'adresse ou on stockera sa valeur signature->rang
	boucleAppel(methode->signature, listParam);
	// codeTreeBoucle(listParam, indentlevel);
}


	// printAttribut(getArguments(getChild(tree, 1), NIL(Attribut)));
	// appel(getChild(tree, 0)->u.meth, tree, indentlevel);

bool boucleAppel(AttributP paramMethode, AttributP listParam)
{
	int indentlevel = 0;
	if(paramMethode == NIL(Attribut))
		return FALSE;

	fprintf(out, "\n");
	commenter("Creation des paramètres", indentlevel);

	dupliquer(1);

	// codeTree(getChild(valExpression, 0), indentlevel);
	codeTree(listParam->expression, indentlevel);

	// fprintf(out, "STORE %d \t-- nom du paramètre: %s\n", paramMethode->rang+1, paramMethode->name);

	return boucleAppel(paramMethode->nextA, listParam->nextA);
}


bool declaration(AttributP attribut, int indentlevel)
{
	if(attribut == NIL(Attribut))
	    return FALSE;

	// Alloue la place
    if(typePredefini(attribut->type))
	{
  			fprintf(out, "\t\t-- attribut: %s\n", attribut->name);
  		if(strcmp(attribut->type->name, "Integer") == 0)
  		{
			allocINT(attribut->expression->u.val, indentlevel);
  		}

  		else
  		{
			allocSTR(attribut->expression->u.str, indentlevel);
  		}
	}
	else if(attribut->expression != NIL(Tree))
	{
        allocObjet(attribut->name, attribut->type, indentlevel);
		// Rempli ses champs
		fprintf(out, "\n");
		commenter("acceder au constructeur pour faire l'affectation", 0);
		// fprintf(out, "%s -> rang = %d\n", attribut->name, attribut->rang);
		// appel de la bonne methode pour procéder à la construction
		//une fois qu'on a l'addresse du constructeur

		commenter("charge l'objet par rapport à FP", 0);
		// storeParam(attribut->rang, attribut->name);

		commenter("param par défaut:", 0);
		setDefaultParam(attribut->type->attribut);

		boucleDeConstruction(attribut->type->methode->signature,
				getChild(attribut->expression, 1));

		fprintf(out, "POPN 1\n");
	}

	fprintf(out, "\n");
	return declaration(attribut->nextA, indentlevel);
}


bool setDefaultParam(AttributP defaultParam)
{
	if(defaultParam == NIL(Attribut))
		return FALSE;

	if(defaultParam->expression != NIL(Tree))
	{
		dupliquer(1);
		codeTree(defaultParam->expression, 0);
		storeParam(defaultParam->rang-1, defaultParam->name);
	}

	return setDefaultParam(defaultParam->nextA);
}

void storeParam(int rang, char* name)
{
	fprintf(out, "STORE %d \t-- nom du paramètre: %s\n", rang+1, name);
}

void allocSTR(char* text, int indentlevel)
{
	indent(indentlevel);
	fprintf(out, "ALLOC %d", 2);
	commenter("allocation d'un objet", 0);

	indent(indentlevel);
	dupliquer(1);
	stockTV(1);

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "PUSHS %s\n", text);
	indent(indentlevel);
	fprintf(out, "STORE %d", 1);
	commenter("stockage de la valeur du champ de l'obj", 0);
}

void allocINT(int val, int indentlevel)
{
	indent(indentlevel);
	fprintf(out, "ALLOC %d \t\t--allocation d'un objet\n", 2);

	indent(indentlevel);
	dupliquer(1);
	stockTV(0);

	indent(indentlevel);
	dupliquer(1);

	indent(indentlevel);
	fprintf(out, "PUSHI %d\n", val);
	indent(indentlevel);
	fprintf(out, "STORE %d", 1);
	commenter("stockage de la valeur du champ de l'obj", 0);
}

void stockTV(int offsetTV)
{
	fprintf(out, "PUSHG %d\t\t\t-- charge la TV de la bonne classe\n", offsetTV);
	fprintf(out, "STORE %d\n\n", 0);
}

/*
 * Gérer le constructeur des parents
 */
bool boucleDeConstruction(AttributP paramConstructeur, TreeP valExpression)
{
	int indentlevel = 0;
	if(paramConstructeur == NIL(Attribut))
		return FALSE;

	fprintf(out, "\n\n");
	commenter("Boucle de construction !", indentlevel);

	dupliquer(1);
	// commenter("appel la méthode", indentlevel);
	// appelMethode(0, indentlevel );// offsetConstructeur
	codeTree(getChild(valExpression, 0), indentlevel);
	storeParam(paramConstructeur->rang, paramConstructeur->name);

	return boucleDeConstruction(paramConstructeur->nextA, getChild(valExpression, 1));
}


/*
 * appelConstructeur (methodeConstructeur, listParamConstructeur)
 */
void appelConstructeur(MethodP methodeConstr, TreeP listParam)
{
	commenter("appel Constructeur", 0);
	// on charge l'adresse ou on stockera sa valeur signature->rang
	boucleDeConstruction(methodeConstr->signature, listParam);
}


void appelFonction(MethodP meth, int indentlevel)
{
	// Appel de methode
	commenter("Appel de methode\n", indentlevel);

	commenter("Appel: on charge le code de la methode", indentlevel);
	appelMethode(meth->rang, indentlevel); // rang de la méthode: meth->rang

	indent(indentlevel);

	fprintf(out, "POPN %d", getSizeParam(meth->signature)); //taille de la liste d'arguments
	commenter("nettoyage la pile: supprimer les parametres", 0);
}


void gestionResultat(ClassP type, int indentlevel)
{
	if(type == NIL(Class))
		commenter("VOID TYPE", indentlevel);

	else if(typePredefini(type))
		fprintf(out, "ALLOC 1\t-- type de retour : %s\n", type->name);

	else
		allocObjet("result", type, indentlevel);
}


/*
 * Gere l'alocation des champs
 */
bool allocChamps(AttributP a, int indentlevel)
{
	if (a == NIL(Attribut))
		return FALSE;

	if(typePredefini(a->type)){
		fprintf(out, "\t-- parametre %s : %s\n", a->name, a->type->name);
		// codeTree(a->expression, 0);
		fprintf(out, "ALLOC 1\t-- parametre %s : %s\n", a->name, a->type->name);
	}

	else
		fprintf(out, "ALLOC %d\t-- type: %s.taille = %d\n", a->type->tailleAttribut, a->type->name, a->type->tailleAttribut);

	return allocChamps(a->nextA, indentlevel);
}


int getSizeParam(AttributP listAttribut)
{
  if(listAttribut == NIL(Attribut))
    return 0;

  return 1 + addTaille(listAttribut->nextA);
}


void appelMethode(int offsetMeth, int indentlevel)
{
	indent(indentlevel);
	fprintf(out, "PUSHA call%d", offsetMeth);
	commenter("adresse de la fonction", 0);

	indent(indentlevel);
	fprintf(out, "CALL");
	commenter("appel", 0);
}


void appelAttribut(AttributP attr, int indentlevel)
{
	// récupérer l'adresse de l'attribut
	dupliquer(1);

	fprintf(out, "LOAD %d\t-- attribut : %s.rang \n", attr->rang, attr->name);
}


void appelVarLocal(AttributP attr, int indentlevel)
{
	indent(indentlevel);
	fprintf(out, "PUSHL %d\t-- Charger une variable locale: %s\n", attr->rang, attr->name);
}

/*
 * generation de code a partir d'un arbre
 */
bool codeTree(TreeP tree, int indentlevel)
{
	if(tree == NIL(Tree))
	{
	    commenter("SKIP", indentlevel);
		return TRUE;
	}

	switch(tree->op) {

		case BLOCK:
	    	commenter("BLOCK", indentlevel);
			codeTreeBoucle(tree, indentlevel);
		    break;

		case DECL:
	    	commenter("DECL", indentlevel);
	    	commenter("Déclaration de variable: Allocation puis Affectation des valeurs\n", 0);
        	declaration(tree->u.lDecl, indentlevel);

        	fprintf(out, "\n\n");
	    	commenter("IS", indentlevel);
		    break;

	    case CONST:
			indent(indentlevel);
		    fprintf(out, "-- CONST : %d\n", tree->u.val);
		    pushI(tree->u.val);
		    break;

        case ESTR:
			indent(indentlevel);
		    fprintf(out, "-- ESTR : %s\n", tree->u.str);
		    pushS(tree->u.str);
		    break;

        case LINSTR:
	    	commenter("LINSTR", indentlevel);
			codeTreeBoucle(tree, indentlevel);

			indent(indentlevel);
		    fprintf(out, "POPN 1\n");  //mauvaise gestion des POPN -> "### erreur : pile vide"
		    break;


		// Appels
        case APPEL:
	    	commenter("APPEL", indentlevel);

			if(getChild(tree, 0)->op == IDAPPEL) {
				fprintf(out, "--nom methode: %s, %d\n",getChild(tree, 0)->u.meth->name, getChild(tree, 0)->u.meth->rang );
			    //    printf("Propager la liste des attributs ici ! : \n" );
				// printAttribut(getArguments(getChild(tree, 1), NIL(Attribut)));

		    	// appel(getChild(tree, 0)->u.meth, getArguments(getChild(tree, 1), NIL(Attribut)), indentlevel);
				codeTree(getChild(tree, 0), indentlevel); //getChild(tree, 1)
			}
			else {
				codeTree(getChild(tree, 0), indentlevel);
				codeTree(getChild(tree, 1), indentlevel);
			}
		    break;

	    case IDAPPEL:
			indent(indentlevel);
		    fprintf(out, "-- IDAPPEL : %s\n", tree->u.meth->name);
		    appelFonction(tree->u.meth, indentlevel+2);
		    break;


		// Instructions
        case ITE:
	    	commenter("ITE", indentlevel);
		    condition(tree, indentlevel);
		    break;

       	case EAFF:
	    	commenter("EAFF", indentlevel);
		    affectation(tree, indentlevel);
		    break;

        case LARG:
	    	commenter("LARG", indentlevel);
			codeTreeBoucle(tree, indentlevel);
		    break;

        case SELECT:
	    	commenter("SELECT", indentlevel);
	    	// on gere le fils gauche -> adresse de l'expression
			codeTree(getChild(tree, 0), indentlevel);
			// récupérer la variable qui se trouve du coté droit
			appelAttribut(getChild(tree, 1)->u.lDecl, indentlevel);
		    break;

		case IDVAR:
			indent(indentlevel);
		    fprintf(out, "-- IDVAR : %s\n", tree->u.lDecl->name);
		    appelVarLocal(tree->u.lDecl, indentlevel);
		    break;


        case ECAST:
	    	commenter("ECAST, appel des bonnes methodes\n", indentlevel);
			codeTreeBoucle(tree, indentlevel);
	    	break;


        case NEWIN:
	    	commenter("NEW", indentlevel);
	    	appelConstructeur(getChild(tree, 0)->u.meth, getChild(tree, 1));
 		    break;


	    // Opérateurs
		case EADD:
		    binaryOp(tree, indentlevel);
		    break;

        case EMIN:
		    binaryOp(tree, indentlevel);
		    break;
        case EDIV:
		    binaryOp(tree, indentlevel);
		    break;
        case EMUL:
		    comparator(tree, indentlevel);
		    break;

		case UADD:
	    	commenter("UADD", indentlevel);
			codeTreeBoucle(tree, indentlevel);
		    break;

        case UMIN:
	    	commenter("UMIN", indentlevel);
			pushI(-getChild(tree, 0)->u.val);
		    break;

        case ECONCAT:
	    	commenter("ECONCAT", indentlevel);
		    concatenation(tree, indentlevel);
		    break;

        case EEGAL:
		    comparator(tree, indentlevel);
		    break;

        case EDIFF:
		    comparator(tree, indentlevel);
		    break;

        case EINF:
		    comparator(tree, indentlevel);
		    break;

        case ESUP:
		    comparator(tree, indentlevel);
		    break;

        case EINFEG:
		    comparator(tree, indentlevel);
		    break;

        case ESUPEG:
		    comparator(tree, indentlevel);
		    break;

	        case LTHIS:
	    fprintf(out, "PUSHL -1");
	    commenter("LTHIS", indentlevel);
	    break;



        case ERETURN:
			indent(indentlevel);
		    fprintf(out, "RETURN\n");
		    commenter("et pxtt autre chose ?", indentlevel);
		    break;


        case ECASTCLASS:
			indent(indentlevel);
	    	fprintf(out, "-- ECASTCLASS : %s", tree->u.classT->name);
		    break;

	        case 18:
	    fprintf(out, "-- LSUPER");
	    break;



		default:
	    	commenter("AUTRE", indentlevel);
    }
    fprintf(out, "\n");

	return TRUE;
}
