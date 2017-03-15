#ifndef CODE_H
#define CODE_H

#include <string.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"


void coDegeneration(ClassP class, TreeP tree);
bool codeClass(ClassP class);
bool codeClassPrimitive();
bool codeMethodPrimitive();
bool codeParent(ClassP c, int indentlevel);

void newLabel(char* nomEtiquette);
void setDebut();
void setFin();
void endMeth();

void codeBlocks(ClassP c, TreeP tree);
void blocMain(TreeP tree);
bool codeTree(TreeP tree, int indentlevel);

bool blockMethode(ClassP class);
bool codeMeth(char* nomClass, MethodP methode);
void blockFoudre(ClassP class);
void invoke(int offsetTV, int offsetMeth, int indentlevel);
int maxMeth(ClassP listC);

void commenter(char* texte, int indentlevel);
void codeTreeBoucle(TreeP bloc, int indentlevel);
void dupliquer(int nPremieresVal);

bool allocObjet(char* etiquette, ClassP c, int indentlevel);
bool allocChamps(AttributP a, int indentlevel);
bool allocMeths(MethodP m, char* nomClass, int indentlevel);
void appelFonction(MethodP meth, int indentlevel);
void appelMethode(int offsetMeth, int indentlevel);

void loadObjectL(int offset, int offsetChamp);
void afficherTexte(char* texte);
void afficherVal(int offset, int offsetChamp);

int getSizeParam(AttributP listAttribut);
bool typePredefini(ClassP maClass);

void indent(int level);
void methodLabel(char* nomClass, char* nomMeth);
void affectation(TreeP tree, int indentlevel);
bool declaration(AttributP attribut, int indentlevel);

void appelConstructeur(MethodP methode, TreeP listParam);
void gestionResultat(ClassP type, int indentlevel);
bool viderPile(ClassP c);
bool boucleDeConstruction(AttributP paramConstructeur, TreeP valExpression);
bool boucleAppel(AttributP paramConstructeur, AttributP valExpression);
void appel(MethodP methode, AttributP listParam, int indentlevel);

void allocSTR(char* text, int indentlevel);
void allocINT(int val, int indentlevel);
bool setDefaultParam(AttributP defaultParam);
void storeParam(int rang, char* name);
void stockTV(int offsetTV);


bool invoke2(MethodP m, int offsetTV, int indentlevel);
bool blockFonction(ClassP class);

#endif // CODE_H
