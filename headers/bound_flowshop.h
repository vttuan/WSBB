
using namespace std;

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <queue>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include "../headers/problem.h"



#ifndef BOUND_FLOWSHOP_H
#define BOUND_FLOWSHOP_H
class Noeud;
class pbab;

struct bound_flowshop :public bound_abstract
{struct Noeud *courant; 
 
int nbOrdo ;
int nbElem ;
int nbborne ;//Nombre de borne calculer par le programme.
int leMeilleur ;//Nous donne la meilleur valeur trouve.
unsigned long int nbNoeud;//Nombre de noueud parcourrus
long double nbNoeudTotal ;//Nombre de noeud de l'arbre total

int nbJob;//Nombre de jobs
int nbMachines ;//Nombre de machines
int somme ;//5:10, 10:45, 20:190.Nombre de couple de machines possibles avec
int **tempsJob ;//[nbMachines][nbJob]Donne le temps des jobs sur les machines.
int *tempsFin ;//[nbJob]Temps auquels les jobs finissent.
int **tempsFinMachines ;//[nbMachines][nbJob]
int **tempsFinMachinesTrie ;//[nbMachines][nbJob]
int **tempsLag ;//[somme][nbJob]Donne le temps mis par un job entre la machine
int **tempsEntre ;//[somme][nbJob]
int **tempsEntreTrie ;//[somme][nbJob]Donne le temps lag de facon trie pour 
int *nbFois ;//[somme]Donne le nombre de fois ou chaque couple de machines 
int *ordoSomme ;//[somme]Ordonnance pour chaque couple le nbFois
int *tempsFinTrie ;//[nbJob]Tableau contenant les temps de fin des jobs dans 
int **tempsExeTrie ;//[nbJob]Tableau contenant les job trie du plus court au 
int **tabJohnson ;//[somme][nbJob]Donne l'ordonnancement selon l'algorithme 
int *machine[2] ;//[somme]Pour chaque couple nous donne le numero de la machine
int **tempsArriver ;//[nbMachines][nbJob]Donne le temps d'arriver minimale pour
int **tempsDepart ;//[nbMachines][nbJob]Donne le temps de depart minimale pour
int  *minTempsArr ;//[nbMachines]Donne le temps minimale mis pour arriver sur 
int *minTempsDep ;//[nbMachines]Donne le temps minimale mis pour finir a partir
long double *valeurCoupe ;//[nbJob+1]Donne le nombre de noeuds coupes selon 
int **tempsB2 ;//[nbMachines][nbJob]
int **tempsB1 ;//[nbMachines][nbJob]
int lambda1, lambda2 ;//Coefficients pour le makespan et pour le tardiness
int nbPartition ;
int nbRetardNonAffB1 ;
int nbRetardNonAffB2 ;
int nbZero ;
int seuil ;
int nb ;
int intervalleCmax, intervalleRetard ;
int *pluspetit[2] ;
int temps[100]; 

 bool supCmax(const Noeud n1,const Noeud n2);
 void initCmax(Noeud *noeud, int *tmp, int *ma, int ind);
 void cmaxFin(Noeud *noeud,int *tmp, int *ma, int *res, int *retardJob, int ind,int minCmax);
 int initialiserRetardJob(Noeud *noeud, int minCmax, int *retardJob);
 int borneInfMakespan(Noeud *noeud, int *valBorneInf, int retardNonFin, int minCmax);
 int calculTemps(Noeud *noeud, int ind, int entre);
 void initRetard(Noeud *noeud, int *retardJobB1, int *retardJobB2);
 void calculRetard(Noeud *noeud, int *retardJobB1, int *retardJobB2);
 int borneInfRetardNonAff(Noeud *noeud);
 void affecter(Noeud *n1, Noeud *n2);
 void remplirNbJobNbMachines();
 void remplirTempsJob();
 void calculCout(int *ordo, int *res);
 void initSomme();
 void initTempsExeTrie();
 void initTempsEntreTrie();
 void initTempsFinTrie();
 void initTempsFinMachinesTrie();
 void allouerMemoire();
 int calculCmaxTotal(int *ordo);
 long double aFaire(int profondeur);
 void remplirTempsFinMachines();
 void remplirTempsExeTrie();
 void remplirTempsEntreTrie();
 void remplirTempsFinMachinesTrie();
 void remplirValeurCoupe();
 void remplirLag();
 void remplirMachine();
 void remplirTempsArriverDepart();
 void initNbFois();
 void initialiserVar();
 int estInf(int i, int j);
 int estSup(int i, int j);
 int partionner(int *ordo,int deb,int fin);
 void quicksort(int *ordo, int deb, int fin);
 void Johnson(int *ordo, int m1, int m2, int s);
 void trierTableau(int *ordo, int nbElem, int *nbFois, bool croissant);
 int calculCmax(int *ordo, int m1, int m2);
 void allocationNoeud(Noeud *courant); 
 void racine(Noeud *courant);
 void initialiserNoeud(Noeud *pere, Noeud *courant, int jobAff, bool enFin);
 void calculTemps(Noeud *pere, Noeud *courant, int jobAff, bool enFin);
 void calculCout(Noeud *courant); 
 void calculBorne(Noeud *courant, int minCmax);
 void desalouerNoeud(Noeud *noeud);
 int max(int i, int j);
 void set_tempsMachines_retardDebut(Noeud *courant, int permutation[], int  limite1);
 void set_job_jobFin(Noeud *courant, int permutation[], int  limite1, int limite2);
 void set_nombres(Noeud *courant, int limite1, int limite2);
 void set_tempsMachinesFin_tempsJobFin(Noeud *courant);
 void criteres_calculer(int permutation[], int*cmax ,int*tardiness );
 void remplirTabJohnson();
 void heuristiqueCmax(Noeud *noeud, int *tmp, int *ma, int ind);

 void init() ;
 void bornes_calculer(int permutation[], int limite1, int limite2,int*couts, int); 
 void bornes_calculer(raw_bb_problem& p);
 void set_instance(instance_abstract*_instance); 
};

#endif
















