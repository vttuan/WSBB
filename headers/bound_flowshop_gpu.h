#include "../headers/tree.h"

#ifndef BOUND_FLOWSHOP_GPU_H_
#define BOUND_FLOWSHOP_GPU_H_

int time_seed_g;

int nbJob_g;//Nombre de jobss
int nbMachines_g;//Nombre de machines

int *tempsJob_g;//[nbMachines][nbJob] Donne le temps d'exécution des jobs sur les machines.
int **tempsJob_T;//[nbMachines][nbJob] Donne le temps d'exécution des jobs sur les machines.

int *tempsFin_g;//[nbJob]Temps auquels les jobs finissent.

int nbOrdo_g;
int nbElem_g;
int nbborne_g;//Nombre de bornes calculées par le programme.
unsigned long int nbNoeud_g;//Nombre de noueud parcourrus
long double nbNoeudTotal_g;//Nombre de noeud de l'arbre total
int somme_g;//5:10, 10:45, 20:190.Nombre de couple de machines possibles avec

int **tempsFinMachines_g;//[nbMachines][nbJob] temps de fin des jobs sur les machines
int **tempsFinMachinesTrie_T ;
int *tempsFinMachinesTrie_g;//[nbMachines][nbJob] temps de fin des jobs sur les machines triés

int *tempsLag_g;//[somme][nbJob]Donne le temps mis par un job entre la machine

int **tempsEntre_g;//[somme][nbJob]
int *tempsEntreTrie_g;//[somme][nbJob]Donne le temps lag de facon trie
int **tempsEntreTrie_T ;

int **tempsExeTrie_T;
int *tempsExeTrie_g;//[nbMachines][nbJob]Tableau contenant les job trie du plus court au

int *tabJohnson_g;//[somme][nbJob]Donne l'ordonnancement selon l'algorithme
int **tabJohnson_T;

int *tempsArriver_g;//[nbMachines][nbJob]Donne le temps d'arriver minimale pour
int *tempsDepart_g;//[nbMachines][nbJob]Donne le temps de depart minimale pour
int *pluspetit_g[2];

int *nbFois_g;//[somme]Donne le nombre de fois ou chaque couple de machines
int *ordoSomme_g;//[somme]Ordonnance pour chaque couple le nbFois
int *tempsFinTrie_g;//[nbJob]Tableau contenant les temps de fin des jobs dans
int *minTempsArr_g;//[nbMachines]Donne le temps minimale mis pour arriver sur
int *minTempsDep_g;//[nbMachines]Donne le temps minimale mis pour finir a partir
int *machine_g;//[somme]Pour chaque couple nous donne le numero de la machine

int lambda1_g, lambda2_g;//Coefficients pour le makespan et pour le tardiness
int nbPartition_g;
int nbRetardNonAffB1_g;
int nbRetardNonAffB2_g;
int nbZero_g;
int seuil_g;
int nb_g;
int intervalleCmax_g, intervalleRetard_g;

__constant__ int tempsJob_dd[MAX_NBJOBS*MAX_NB_MACHINES];

int *tempsJob_d;
int *tempsFin_d;
int *tempsFinTrie_d;
int *tempsFinMachinesTrie_d;
int *tempsLag_d;
int *tempsEntreTrie_d;
int *tempsExeTrie_d;
int *tabJohnson_d;
int *tempsArriver_d;
int *tempsDepart_d;
int *pluspetit_d;

int *nbFois_d;
int *ordoSomme_d;
int *minTempsArr_d;
int *minTempsDep_d;
int *machine_d;

#endif /* BOUND_FLOWSHOP_GPU_H_ */
