#include <string.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char*);

extern void setError(int code);


ClassP verifTypage(TreeP bloc, ClassP v);
ClassP verifAppel(TreeP *t, ClassP v);
ClassP chercherTypeM(MethodP m, char* str);
ClassP getNew(TreeP t);

MethodP getMethTot(ClassP c, MethodP m);

AttributP getArguments(TreeP t, AttributP a);
AttributP getEnvTot(ClassP c, AttributP a);

char* getType(AttributP env, char* nom);

int verifTree(TreeP *t, AttributP *var, int isMethod, ClassP c, MethodP m);

bool verifOpBinary(TreeP bloc, AttributP env, char* type);
bool verifOpUnary(TreeP bloc, AttributP env, char* type);

bool appartientM(MethodP m, TreeP t, char* str);

bool isSuperClass(ClassP subclass, ClassP superclass);


void verifAff(TreeP);
void verifIte(TreeP);
void verifMethod(MethodP m, AttributP var, ClassP c);
void verifPortee(ClassP c, TreeP *t, AttributP var);
void verifApp(MethodP *m);
void verifAppelMethod(ClassP *c, TreeP *t);
void verifAttribut(AttributP *attr, AttributP var, bool isMethod, ClassP c, MethodP m);
void verifArgs(AttributP args);
void verifAttribut2(AttributP *attr, AttributP var, bool isMethod, ClassP c, MethodP m, int rangi);

void ajouter(AttributP *v, AttributP v2);
void ajouterHerite(AttributP *v, AttributP v2);
void ajouterMethod(MethodP *v, MethodP v2);

int compterArg(AttributP a, int nb);
int compareArg(AttributP a1, AttributP a2);

bool linkParamSup(AttributP list, AttributP elem);
bool linkParamSupHerite(AttributP list, AttributP elem);
bool linkMethod(MethodP m, MethodP m2);
