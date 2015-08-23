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

#include "../headers/bound_abstract.h"
#include "../headers/instance_abstract.h"
#include "../headers/pbab.h"
#include "../headers/bound_flowshop.h"
#include "../headers/problem.h"

class Noeud {
public:
	int *tempsMachines; //[nbMachines] Nous donne le temps de chaque machines
						//selon les jobs deja ordonnances au debut.
	int *tempsMachinesFin; //[nbMachines]Nous donne le temps de fin des jobs
						   //a partir chaque machines selon les jobs deja
						   //ordonnances a la fin.
	int *job; //[nbJob]Donne pour chaque job sont ordre de passage
	int *jobFin; //[nbJob]Garde en memoire l'ordre de passage des jobs affecter
				 //en fin.
	int nbAffectFin; //Nombre de job affecte a la fin.
	int nbAffectDebut; //Nombre de job affecte au debut
	int nbAffect; //Nombre de job affecte en tout.
				  //On a nbAffect=nbAffectDebut+nbAffectFin
	int **tempsJobFin; //[nbMachines][nbAffectFin]Donne a partir de  chaque
					   //machine le temps qu'il va falloir au job ordonnace a la
					   //fin pour finir leur execution. Sert pour le calcul de
					   //la borne inf du retard.
	int coutCmax; //Donne le cout du makespan
	int coutRetard; //Donne le cout du retard
	int coutTotal; //Donne le cout total en fonction des coefficients.
	int borneInfCmax; //Donne le cout de la borne inf pour le makespan.
	int borneInfRetard; //Donne le cout de la borne inf pour le retard.
	int borneInfTotal; //Donne le cout de la borne inf pour le total
	int retardDebut; //Donne le retard engendre par les jobs places au debut.
	int retardFin; //Donne le retard engendre par les jobs places a la fin.
	int *genotype; //Donne l'ordre de passage des jobs.
	Noeud(bound_flowshop* flowshop, Noeud *avant = NULL, int jobAff = 0,
			bool enFin = false, int minCmax = 0);
};

bool bound_flowshop::supCmax(const Noeud n1, const Noeud n2) {
	return n1.coutCmax > n2.coutCmax;
}

void bound_flowshop::heuristiqueCmax(Noeud *noeud, int *tmp, int *ma, int ind) {
	int jobCour;
	for (int j = 0; j < nbJob; j++) {
		jobCour = tabJohnson[ind][j];
		if (noeud->job[jobCour] == 0) {
			tmp[0] += tempsJob[ma[0]][jobCour];
			if (tmp[1] > tmp[0] + tempsLag[ind][jobCour])
				tmp[1] += tempsJob[ma[1]][jobCour];
			else
				tmp[1] = tmp[0] + tempsLag[ind][jobCour]
						+ tempsJob[ma[1]][jobCour];
		}
	}
}

void bound_flowshop::initCmax(Noeud *noeud, int *tmp, int *ma, int ind) {
	ma[0] = machine[0][ind];
	ma[1] = machine[1][ind];
	/*On verifie si il y a deja un job affecte au debut*/
	if (noeud->nbAffectDebut != 0) {
		tmp[0] = noeud->tempsMachines[ma[0]];
		tmp[1] = noeud->tempsMachines[ma[1]];
	} else {
		tmp[0] = minTempsArr[ma[0]];
		tmp[1] = minTempsArr[ma[1]];
	}
}

void bound_flowshop::cmaxFin(Noeud *noeud, int *tmp, int *ma, int *res,
		int *retardJob, int ind, int minCmax) {
	if (tmp[1] + noeud->tempsMachinesFin[ma[1]]
			> tmp[0] + noeud->tempsMachinesFin[ma[0]])
		res[0] = tmp[1] + noeud->tempsMachinesFin[ma[1]];
	else
		res[0] = tmp[0] + noeud->tempsMachinesFin[ma[0]];

}

int bound_flowshop::initialiserRetardJob(Noeud *noeud, int minCmax,
		int *retardJob) {
	int jobCour, jobAvant, temps = minCmax;
	int retard = 0;
	for (int i = nbJob - 1; i >= nbJob - noeud->nbAffectFin; i--) {
		jobCour = noeud->jobFin[i];
		if ((temps - tempsFin[jobCour]) > retardJob[i]) {
			retard += (temps - tempsFin[jobCour]) - retardJob[i];
			retardJob[i] = temps - tempsFin[jobCour];
		}
		if (i > nbJob - noeud->nbAffectFin) {
			jobAvant = noeud->jobFin[i - 1];
			temps -= (noeud->tempsJobFin[0][jobCour]
					- noeud->tempsJobFin[0][jobAvant]);
		}
	}
	return retard;
}

int bound_flowshop::borneInfMakespan(Noeud *noeud, int *valBorneInf,
		int retardNonFin, int minCmax) {
	int moinsBon = 0;
	int idxMoinsBonCmax = -1, i, tmpDep, retard = retardNonFin;
	int ma[2];/*Contient les rang des deux machines considere.*/
	int tmp[2];/*Contient les temps sur les machines considere*/
	int res[2];/*Contient le cmax est le retard*/
	int retardJob[nbJob];/*Contient les retard engendre par les jobs affectes en fin*/
	//if(nbborne%100000==0)
	//trierTableau(ordoSomme, somme, nbFois, false) ;
	for (int j = 0; j < nbJob; j++)
		retardJob[j] = 0;
	//retard+=initialiserRetardJob(noeud, minCmax, retardJob) ;
	//enleverCouple() ;
	int l;
	for (l = 0; l < nbElem; l++) {
		i = ordoSomme[l];
		initCmax(noeud, tmp, ma, i);
		heuristiqueCmax(noeud, tmp, ma, i);
		if (noeud->nbAffectFin != 0) {
			res[1] = retard;
			cmaxFin(noeud, tmp, ma, res, retardJob, i, minCmax);
			tmpDep = res[0];
			retard = res[1];
			tmp[1] = tmpDep;
		} else {
			tmpDep = minTempsDep[ma[1]];
			tmp[1] += tmpDep;
		}
		if (tmp[1] > moinsBon) {
			idxMoinsBonCmax = i;
			moinsBon = tmp[1];
		}
		if (lambda2 * moinsBon + lambda1 * retard > leMeilleur
				&& leMeilleur != -1) {
			nbborne++;
			nbFois[i]++;
			valBorneInf[0] = moinsBon;
			valBorneInf[1] = retard;
			return 0;
		}
	}
	nbborne++;
	nbFois[idxMoinsBonCmax]++;
	valBorneInf[0] = moinsBon;
	valBorneInf[1] = retard;
	return 0;
}

int bound_flowshop::calculTemps(Noeud *noeud, int ind, int entre) {
	int jobCourB1, jobCourB2, tempsCourB1 = noeud->tempsMachines[ind];
	int leJob, num;
	int numJobB1 = 0;
	int plusCourt[nbMachines], numJobB2 = 0;
	int tempsCourB2 = noeud->tempsMachines[ind];
	for (int k = ind + 1; k < nbMachines; k++) {
		num = 0;
		leJob = tempsEntreTrie[entre][num];
		while (noeud->job[leJob] != 0) {
			num++;
			leJob = tempsEntreTrie[entre][num];
		}
		plusCourt[k] = tempsEntre[entre][leJob];
		entre++;
	}
	for (int j = 0; j < nbJob; j++) {
		jobCourB1 = tempsExeTrie[ind][j];
		if (noeud->job[jobCourB1] == 0) {
			numJobB1++;
			tempsCourB1 += tempsJob[ind][jobCourB1];
			if (tempsCourB1 > tempsB1[ind][numJobB1])
				tempsB1[ind][numJobB1] = tempsCourB1;
			for (int k = ind + 1; k < nbMachines; k++)
				if (tempsB1[ind][numJobB1] + plusCourt[k]
						> tempsB1[k][numJobB1])
					tempsB1[k][numJobB1] = tempsB1[ind][numJobB1]
							+ plusCourt[k];
		}
		jobCourB2 = tempsFinTrie[j];
		if (noeud->job[jobCourB2] == 0) {
			numJobB2++;
			tempsCourB2 += tempsJob[ind][jobCourB2];
			if (tempsCourB2 > tempsB2[ind][numJobB2])
				tempsB2[ind][numJobB2] = tempsCourB2;
			for (int k = ind + 1; k < nbMachines; k++)
				if (tempsB2[ind][numJobB2] + plusCourt[k]
						> tempsB2[k][numJobB2])
					tempsB2[k][numJobB2] = tempsB2[ind][numJobB2]
							+ plusCourt[k];
		}
	}
	return entre;
}

void bound_flowshop::initRetard(Noeud *noeud, int *retardJobB1,
		int *retardJobB2) {
	for (int i = 0; i < nbMachines; i++)
		for (int j = 0; j < nbJob; j++) {
			tempsB1[i][j] = 0;
			tempsB2[i][j] = 0;
		}
	for (int i = 0; i < nbMachines; i++) {
		tempsB1[i][0] = noeud->tempsMachines[i];
		tempsB2[i][0] = noeud->tempsMachines[i];
	}
	for (int i = 0; i < nbJob; i++) {
		retardJobB1[i] = 0;
		retardJobB2[i] = 0;
	}
}

void bound_flowshop::calculRetard(Noeud *noeud, int *retardJobB1,
		int *retardJobB2) {
	int sommeB1 = 0, sommeB2 = 0;
	int retardNonAffB1 = 0, retardNonAffB1Somme = 0;
	int retardB1[nbJob];
	int retardB2[nbJob];
	int jobCour, num, retardNonAffB2 = 0, retardNonAffB2Somme = 0;
	for (int i = 0; i < nbMachines; i++) {
		//int i=nbMachines-1 ;
		retardNonAffB1 = 0;
		retardNonAffB2 = 0;
		retardNonAffB2Somme = 0;
		retardNonAffB1Somme = 0;
		num = 0;
		for (int j = 1; j <= nbJob - noeud->nbAffect; j++) {
			jobCour = tempsFinMachinesTrie[i][num];
			while (noeud->job[jobCour] != 0) {
				num++;
				jobCour = tempsFinMachinesTrie[i][num];
			}
			if (tempsB1[i][j] > tempsFinMachines[i][jobCour]) {
				retardNonAffB1 = (tempsB1[i][j] - tempsFinMachines[i][jobCour]);
				retardNonAffB1Somme += retardNonAffB1;
				//if(retardNonAffB1>retardJobB1[j])
				retardB1[j] = retardNonAffB1;
			} else
				retardB1[j] = 0;
			if (tempsB2[i][j] > tempsFinMachines[i][jobCour]) {
				retardNonAffB2 = (tempsB2[i][j] - tempsFinMachines[i][jobCour]);
				if (retardNonAffB2 > tempsJob[nbMachines - 1][jobCour])
					retardNonAffB2 = tempsJob[nbMachines - 1][jobCour];
				//if(retardNonAffB2>retardJobB2[j])
				retardB2[j] = retardNonAffB2;
				retardNonAffB2Somme += retardNonAffB2;
			} else
				retardB2[j] = 0;
			num++;
		}
		if (retardNonAffB1Somme > sommeB1) {
			sommeB1 = retardNonAffB1Somme;
			for (int j = 1; j <= nbJob - noeud->nbAffect; j++) {
				retardJobB1[j] = retardB1[j];
			}
		}
		if (retardNonAffB2Somme > sommeB1) {
			sommeB2 = retardNonAffB2Somme;
			for (int j = 1; j <= nbJob - noeud->nbAffect; j++) {
				retardJobB2[j] = retardB2[j];
			}
		}
	}
}

int bound_flowshop::borneInfRetardNonAff(Noeud *noeud) {
	int retardJobB1[nbJob];
	int retardJobB2[nbJob];
	int retardNonAffB1 = 0;
	int retardNonAffB2 = 0;
	int entre = 0;

	initRetard(noeud, retardJobB1, retardJobB2);
	for (int i = 0; i < nbMachines; i++)
		entre = calculTemps(noeud, i, entre);

	calculRetard(noeud, retardJobB1, retardJobB2);

	for (int i = 1; i <= nbJob - noeud->nbAffect; i++)
		retardNonAffB1 += retardJobB1[i];
	for (int i = 1; i <= nbJob - noeud->nbAffect; i++)
		retardNonAffB2 += retardJobB2[i];
	if (retardNonAffB1 > retardNonAffB2) {
		nbRetardNonAffB1++;
		return retardNonAffB1;
	} else if (retardNonAffB2 != 0) {
		nbRetardNonAffB2++;
		return retardNonAffB2;
	}
	nbZero++;
	return 0;
}

void bound_flowshop::affecter(Noeud *n1, Noeud *n2) {
	n1->coutCmax = n2->coutCmax;
	n1->coutRetard = n2->coutRetard;
	n1->coutTotal = n2->coutTotal;
	for (int i = 0; i < nbJob; i++)
		n1->genotype[i] = n2->genotype[i];
}
void bound_flowshop::remplirNbJobNbMachines() {
	(pbb->instance->data)->seekg(0);
	(pbb->instance->data)->clear();
	*(pbb->instance->data) >> nbJob;
	*(pbb->instance->data) >> nbMachines;

	int tmp;
	*(pbb->instance->data) >> tmp;

}

void bound_flowshop::remplirTempsJob() {
	for (int j = 0; j < nbMachines; j++)
		for (int i = 0; i < nbJob; i++)
			*(pbb->instance->data) >> tempsJob[j][i];
}

void bound_flowshop::calculCout(int *ordo, int *res) {
	int tmp[nbMachines];
	int jobCour;
	int retard = 0;
	for (int j = 0; j < nbMachines; j++)
		tmp[j] = 0;
	for (int i = 0; i < nbJob; i++) {
		jobCour = ordo[i];
		tmp[0] += tempsJob[0][jobCour];
		for (int j = 1; j < nbMachines; j++) {
			if (tmp[j] > tmp[j - 1])
				tmp[j] += tempsJob[j][jobCour];
			else
				tmp[j] = tmp[j - 1] + tempsJob[j][jobCour];
		}
		if (tmp[nbMachines - 1] > tempsFin[jobCour])
			retard += tmp[nbMachines - 1] - tempsFin[jobCour];
	}
	res[0] = tmp[nbMachines - 1];
	res[1] = retard;
}

void bound_flowshop::initSomme() {
	somme = 0;
	for (int i = 1; i < nbMachines; i++)
		somme += i;
}

void bound_flowshop::initTempsExeTrie() {
	for (int i = 0; i < nbMachines; i++)
		for (int j = 0; j < nbJob; j++)
			tempsExeTrie[i][j] = j;
}

void bound_flowshop::initTempsEntreTrie() {
	for (int i = 0; i < somme; i++)
		for (int j = 0; j < nbJob; j++)
			tempsEntreTrie[i][j] = j;
}

void bound_flowshop::initTempsFinTrie() {
	for (int i = 0; i < nbJob; i++)
		tempsFinTrie[i] = i;
}

void bound_flowshop::initTempsFinMachinesTrie() {
	for (int i = 0; i < nbMachines; i++)
		for (int j = 0; j < nbJob; j++)
			tempsFinMachinesTrie[i][j] = j;
}

void bound_flowshop::allouerMemoire() {
	tempsJob = (int **) malloc(nbMachines * sizeof(int *));
	tempsExeTrie = (int **) malloc(nbMachines * sizeof(int *));
	for (int i = 0; i < nbMachines; i++) {
		tempsJob[i] = (int *) malloc(nbJob * sizeof(int));
		tempsExeTrie[i] = (int *) malloc(nbJob * sizeof(int));
	}
	tempsFin = (int *) malloc(nbJob * sizeof(int));
	tempsB1 = (int **) malloc(nbMachines * sizeof(int *));
	tempsB2 = (int **) malloc(nbMachines * sizeof(int *));
	tempsFinMachines = (int **) malloc(nbMachines * sizeof(int *));
	tempsFinMachinesTrie = (int **) malloc(nbMachines * sizeof(int *));
	for (int i = 0; i < nbMachines; i++) {
		tempsB1[i] = (int *) malloc(nbJob * sizeof(int));
		tempsB2[i] = (int *) malloc(nbJob * sizeof(int));
		tempsFinMachines[i] = (int *) malloc(nbJob * sizeof(int));
		tempsFinMachinesTrie[i] = (int *) malloc(nbJob * sizeof(int));
	}
	tempsLag = (int **) malloc(somme * sizeof(int *));
	tempsEntre = (int **) malloc(somme * sizeof(int *));
	tempsEntreTrie = (int **) malloc(somme * sizeof(int *));
	for (int i = 0; i < somme; i++) {
		tempsLag[i] = (int *) malloc(nbJob * sizeof(int));
		tempsEntre[i] = (int *) malloc(nbJob * sizeof(int));
		tempsEntreTrie[i] = (int *) malloc(nbJob * sizeof(int));
	}
	ordoSomme = (int *) malloc(somme * sizeof(int));
	nbFois = (int *) malloc(somme * sizeof(int));
	tempsFinTrie = (int *) malloc(nbJob * sizeof(int));
	tabJohnson = (int **) malloc(somme * sizeof(int *));
	for (int i = 0; i < somme; i++)
		tabJohnson[i] = (int *) malloc(nbJob * sizeof(int));
	for (int i = 0; i < 2; i++)
		machine[i] = (int *) malloc(somme * sizeof(int));
	tempsDepart = (int **) malloc(nbMachines * sizeof(int *));
	tempsArriver = (int **) malloc(nbMachines * sizeof(int *));
	for (int i = 0; i < nbMachines; i++) {
		tempsArriver[i] = (int *) malloc(nbJob * sizeof(int));
		tempsDepart[i] = (int *) malloc(nbJob * sizeof(int));
	}
	minTempsArr = (int *) malloc(nbMachines * sizeof(int));
	minTempsDep = (int *) malloc(nbMachines * sizeof(int));
	valeurCoupe = (long double *) malloc((nbJob + 1) * sizeof(long double));
}

int bound_flowshop::calculCmaxTotal(int *ordo) {
	int tmp[nbMachines];
	for (int j = 0; j < nbMachines; j++)
		tmp[j] = 0;
	for (int i = 0; i < nbJob; i++) {
		cout << ordo[i] << " ";
		tmp[0] += tempsJob[0][ordo[i]];
		for (int j = 1; j < nbMachines; j++) {
			if (tmp[j] > tmp[j - 1])
				tmp[j] += tempsJob[j][ordo[i]];
			else
				tmp[j] = tmp[j - 1] + tempsJob[j][ordo[i]];
		}
	}
	return tmp[nbMachines - 1];
}

long double bound_flowshop::aFaire(int profondeur) {
	long double nbNoeudAFaire = 1;
	for (int i = 2; i <= profondeur; i++)
		nbNoeudAFaire = (nbNoeudAFaire * i) + i;
	return nbNoeudAFaire;
}

void bound_flowshop::remplirTempsFinMachines() {
	for (int i = 0; i < nbJob; i++)
		tempsFinMachines[nbMachines - 1][i] = tempsFin[i];
	for (int i = nbMachines - 2; i >= 0; i--)
		for (int j = 0; j < nbJob; j++)
			tempsFinMachines[i][j] = tempsFinMachines[i + 1][j]
					- tempsJob[i + 1][j];
}

void bound_flowshop::remplirTempsExeTrie() {
	for (int i = 0; i < nbMachines; i++)
		trierTableau(tempsExeTrie[i], nbJob, tempsJob[i], true);
}

void bound_flowshop::remplirTempsEntreTrie() {
	for (int i = 0; i < somme; i++)
		trierTableau(tempsEntreTrie[i], nbJob, tempsEntre[i], true);
}

void bound_flowshop::remplirTempsFinMachinesTrie() {
	for (int i = 0; i < nbMachines; i++)
		trierTableau(tempsFinMachinesTrie[i], nbJob, tempsFinMachines[i], true);
}

void bound_flowshop::remplirValeurCoupe() {
	for (int i = 0; i < nbJob + 1; i++)
		valeurCoupe[i] = aFaire(i);
}

void bound_flowshop::remplirLag() {
	int m1, m2;
	for (int i = 0; i < somme; i++) {
		m1 = machine[0][i];
		m2 = machine[1][i];
		for (int j = 0; j < nbJob; j++) {
			tempsLag[i][j] = 0;
			for (int k = m1 + 1; k < m2; k++)
				tempsLag[i][j] += tempsJob[k][j];
			tempsEntre[i][j] = tempsLag[i][j] + tempsJob[m2][j];
		}
	}
}

void bound_flowshop::remplirMachine() {
	int cmpt = 0;
	for (int i = 0; i < (nbMachines - 1); i++)
		for (int j = i + 1; j < nbMachines; j++) {
			machine[0][cmpt] = i;
			machine[1][cmpt] = j;
			cmpt++;
		}
}

void bound_flowshop::remplirTempsArriverDepart() {
	bool faitDep[nbMachines];
	bool faitArr[nbMachines];
	int machine;
	for (int k = 1; k < nbMachines; k++) {
		faitArr[k] = false;
		faitDep[k] = false;
	}
	minTempsDep[nbMachines - 1] = 0;
	minTempsArr[0] = 0;
	for (int i = 0; i < nbJob; i++) {
		tempsArriver[0][i] = 0;
		tempsDepart[nbMachines - 1][i] = 0;
		for (int k = 1; k < nbMachines; k++) {
			machine = nbMachines - k - 1;
			tempsArriver[k][i] = tempsArriver[k - 1][i] + tempsJob[k - 1][i];
			tempsDepart[machine][i] = tempsDepart[machine + 1][i]
					+ tempsJob[machine + 1][i];
			if (!faitArr[k] || minTempsArr[k] > tempsArriver[k][i]) {
				faitArr[k] = true;
				minTempsArr[k] = tempsArriver[k][i];
			}
			if (!faitDep[k] || minTempsDep[machine] > tempsDepart[machine][i]) {
				faitDep[k] = true;
				minTempsDep[machine] = tempsDepart[machine][i];
			}
		}
	}
}

void bound_flowshop::remplirTabJohnson() {
	int cmpt = 0;
	for (int i = 0; i < (nbMachines - 1); i++)
		for (int j = i + 1; j < nbMachines; j++) {
			Johnson(tabJohnson[cmpt], i, j, cmpt);
			cmpt++;
		}
}

void bound_flowshop::initNbFois() {
	for (int i = 0; i < somme; i++) {
		ordoSomme[i] = i;
		nbFois[i] = 0;
	}
}

void bound_flowshop::initialiserVar() {
	remplirNbJobNbMachines();
	initSomme();
	allouerMemoire();
	remplirValeurCoupe();
	remplirTempsJob();
	initTempsFinTrie();
	initTempsExeTrie();
	initTempsEntreTrie();
	initTempsFinMachinesTrie();
	trierTableau(tempsFinTrie, nbJob, tempsFin, true);
	remplirTempsExeTrie();
	remplirTempsArriverDepart();
	remplirMachine();
	remplirLag();
	remplirTempsEntreTrie();
	remplirTabJohnson();
	remplirTempsFinMachines();
	remplirTempsFinMachinesTrie();
	initNbFois();
}
/////////////////////////johnson.cpp

int bound_flowshop::estInf(int i, int j) {
	if (pluspetit[0][i] == pluspetit[0][j]) {
		if (pluspetit[0][i] == 1)
			return pluspetit[1][i] < pluspetit[1][j];
		return pluspetit[1][i] > pluspetit[1][j];
	}
	return pluspetit[0][i] < pluspetit[0][j];
}

int bound_flowshop::estSup(int i, int j) {
	if (pluspetit[0][i] == pluspetit[0][j]) {
		if (pluspetit[0][i] == 1)
			return pluspetit[1][i] > pluspetit[1][j];
		return pluspetit[1][i] < pluspetit[1][j];
	}
	return pluspetit[0][i] > pluspetit[0][j];
}

int bound_flowshop::partionner(int *ordo, int deb, int fin) {
	int d = deb - 1;
	int f = fin + 1;
	int mem, pivot = ordo[deb];
	do {
		do {
			f--;
		} while (estSup(ordo[f], pivot));
		do {
			d++;
		} while (estInf(ordo[d], pivot));

		if (d < f) {
			mem = ordo[d];
			ordo[d] = ordo[f];
			ordo[f] = mem;
		}
	} while (d < f);
	return f;
}

void bound_flowshop::quicksort(int *ordo, int deb, int fin) {
	int k;
	if ((fin - deb) > 0) {
		k = partionner(ordo, deb, fin);
		quicksort(ordo, deb, k);
		quicksort(ordo, k + 1, fin);
	}
}

void bound_flowshop::Johnson(int *ordo, int m1, int m2, int s) {
	pluspetit[0] = (int *) malloc((nbJob) * sizeof(int));
	pluspetit[1] = (int *) malloc((nbJob) * sizeof(int));
	for (int i = 0; i < nbJob; i++) {
		ordo[i] = i;
		if (tempsJob[m1][i] < tempsJob[m2][i]) {
			pluspetit[0][i] = 1;
			pluspetit[1][i] = tempsJob[m1][i] + tempsLag[s][i];
		} else {
			pluspetit[0][i] = 2;
			pluspetit[1][i] = tempsJob[m2][i] + tempsLag[s][i];
		}
	}
	quicksort(ordo, 0, (nbJob - 1));
	free(pluspetit[0]);
	free(pluspetit[1]);
}

void bound_flowshop::trierTableau(int *ordo, int nbElem, int *nbFois,
		bool croissant) {
	pluspetit[0] = (int *) malloc((nbElem) * sizeof(int));
	pluspetit[1] = (int *) malloc((nbElem) * sizeof(int));
	for (int i = 0; i < nbElem; i++) {
		if (croissant)
			pluspetit[0][i] = 1;
		else
			pluspetit[0][i] = 2;
		pluspetit[1][i] = nbFois[i];
	}
	quicksort(ordo, 0, (nbElem - 1));
	free(pluspetit[0]);
	free(pluspetit[1]);
}

int bound_flowshop::calculCmax(int *ordo, int m1, int m2) {
	int tmp1 = 0;
	int tmp2 = 0;
	for (int i = 0; i < nbJob; i++) {
		tmp1 += tempsJob[m1][ordo[i]];
		if (tmp2 > tmp1)
			tmp2 += tempsJob[m2][ordo[i]];
		else
			tmp2 = tmp1 + tempsJob[m2][ordo[i]];
	}
	return tmp2;
}

void bound_flowshop::allocationNoeud(Noeud *courant) {
	courant->tempsMachines = (int *) malloc(nbMachines * sizeof(int));
	courant->tempsMachinesFin = (int *) malloc(nbMachines * sizeof(int));
	courant->job = (int *) malloc(nbJob * sizeof(int));
	courant->genotype = (int *) malloc(nbJob * sizeof(int));
	courant->jobFin = (int *) malloc(nbJob * sizeof(int));
	courant->tempsJobFin = (int **) malloc(nbMachines * sizeof(int *));
	for (int i = 0; i < nbMachines; i++)
		courant->tempsJobFin[i] = (int *) malloc(nbJob * sizeof(int));
}

void bound_flowshop::racine(Noeud *courant) {
	for (int k = 0; k < nbJob; k++) {
		courant->job[k] = 0;
		courant->jobFin[k] = 0;
	}
	for (int j = 0; j < nbMachines; j++) {
		courant->tempsMachinesFin[j] = 0;
		courant->tempsMachines[j] = 0;
		for (int k = 0; k < nbJob; k++)
			courant->tempsJobFin[j][k] = 0;
	}
	courant->coutCmax = 0;
	courant->coutRetard = 0;
	courant->coutTotal = 0;
	courant->nbAffect = 0;
	courant->nbAffectFin = 0;
	courant->nbAffectDebut = 0;
	courant->borneInfCmax = 0;
	courant->borneInfRetard = 0;
	courant->borneInfTotal = 0;
	courant->retardDebut = 0;
	courant->retardFin = 0;
}

void bound_flowshop::initialiserNoeud(Noeud *pere, Noeud *courant, int jobAff,
		bool enFin) {
	courant->retardDebut = pere->retardDebut;
	courant->nbAffectFin = pere->nbAffectFin;
	courant->nbAffectDebut = pere->nbAffectDebut;
	courant->nbAffect = (pere->nbAffect) + 1;
	for (int k = 0; k < nbJob; k++) {
		courant->job[k] = pere->job[k];
		courant->genotype[k] = pere->genotype[k];
	}
	for (int k = nbJob - (courant->nbAffectFin); k < nbJob; k++)
		courant->jobFin[k] = pere->jobFin[k];
	if (enFin) {
		courant->job[jobAff] = nbJob - (courant->nbAffectFin);
		courant->nbAffectFin++;
		courant->genotype[nbJob - (courant->nbAffectFin)] = jobAff;
		courant->jobFin[nbJob - (courant->nbAffectFin)] = jobAff;
	} else {
		courant->genotype[(courant->nbAffectDebut)] = jobAff;
		courant->nbAffectDebut++;
		courant->job[jobAff] = (courant->nbAffectDebut);
	}
}

void bound_flowshop::calculTemps(Noeud *pere, Noeud *courant, int jobAff,
		bool enFin) {
	courant->coutCmax = 0;
	if (!enFin) {
		courant->tempsMachines[0] = (pere->tempsMachines[0])
				+ tempsJob[0][jobAff];
		courant->tempsMachinesFin[0] = pere->tempsMachinesFin[0];
		for (int k = 0; k < nbJob; k++)
			courant->tempsJobFin[0][k] = pere->tempsJobFin[0][k];
		for (int j = 1; j < nbMachines; j++) {
			courant->tempsMachinesFin[j] = pere->tempsMachinesFin[j];
			if (courant->tempsMachines[j - 1] > pere->tempsMachines[j])
				courant->tempsMachines[j] = (courant->tempsMachines[j - 1]
						+ tempsJob[j][jobAff]);
			else
				courant->tempsMachines[j] = (pere->tempsMachines[j]
						+ tempsJob[j][jobAff]);
			for (int k = 0; k < nbJob; k++)
				courant->tempsJobFin[j][k] = pere->tempsJobFin[j][k];
		}
		if (courant->tempsMachines[nbMachines - 1] > tempsFin[jobAff])
			courant->retardDebut += (courant->tempsMachines[nbMachines - 1]
					- tempsFin[jobAff]);
	} else {
		int jobCour;
		int tmpMa[nbMachines];
		for (int j = 0; j < nbMachines; j++)
			courant->tempsMachines[j] = pere->tempsMachines[j];
		for (int j = nbMachines - 1; j >= 0; j--) {
			for (int k = j; k < nbMachines; k++)
				tmpMa[k] = 0;
			for (int k = nbJob - courant->nbAffectFin; k < nbJob; k++) {
				jobCour = courant->jobFin[k];
				tmpMa[j] += tempsJob[j][jobCour];
				for (int l = j + 1; l < nbMachines; l++) {
					if (tmpMa[l - 1] > tmpMa[l])
						tmpMa[l] = tmpMa[l - 1] + tempsJob[l][jobCour];
					else
						tmpMa[l] += tempsJob[l][jobCour];
				}
				courant->tempsJobFin[j][jobCour] = tmpMa[nbMachines - 1];
			}
			courant->tempsMachinesFin[j] = tmpMa[nbMachines - 1];
		}
	}
	courant->coutCmax = courant->tempsMachines[nbMachines - 1];
}

void bound_flowshop::calculCout(Noeud *courant) {
	int leJob;
	for (int i = nbJob - (courant->nbAffectFin); i < nbJob; i++) {
		leJob = courant->jobFin[i];
		courant->tempsMachines[0] += tempsJob[0][leJob];
		for (int j = 1; j < nbMachines; j++) {
			if (courant->tempsMachines[j - 1] > courant->tempsMachines[j])
				courant->tempsMachines[j] = (courant->tempsMachines[j - 1]
						+ tempsJob[j][leJob]);
			else
				courant->tempsMachines[j] += tempsJob[j][leJob];
		}
		if (courant->tempsMachines[nbMachines - 1] > tempsFin[leJob])
			courant->retardDebut += (courant->tempsMachines[nbMachines - 1]
					- tempsFin[leJob]);
	}
	courant->coutCmax = courant->tempsMachines[nbMachines - 1];
	courant->retardFin = 0;
	courant->coutRetard = courant->retardDebut;
	courant->coutTotal = lambda1 * (courant->coutRetard)
			+ lambda2 * (courant->coutCmax);
	courant->borneInfCmax = courant->coutCmax;
	courant->borneInfRetard = courant->coutRetard;
	courant->borneInfTotal = courant->coutTotal;
}

void bound_flowshop::calculBorne(Noeud *courant, int minCmax) {
	int retardNonAff = 0; //borneInfRetardNonAff(courant) ;
	int valBorneInf[2];
	int retardNonFin = courant->retardDebut + retardNonAff;
	borneInfMakespan(courant, valBorneInf, retardNonFin, minCmax);
	courant->borneInfCmax = valBorneInf[0];
	courant->borneInfRetard = valBorneInf[1];
	courant->borneInfTotal = (lambda1 * (courant->borneInfRetard)
			+ lambda2 * (courant->borneInfCmax));
	courant->retardFin = (courant->borneInfRetard) - retardNonFin;
}

Noeud::Noeud(bound_flowshop* flowshop, Noeud *pere, int jobAff, bool enFin,
		int minCmax) {
	flowshop->allocationNoeud(this);
	flowshop->racine(this);
}

void bound_flowshop::desalouerNoeud(Noeud *noeud) {
	for (int i = 0; i < nbMachines; i++)
		free(noeud->tempsJobFin[i]);
	free(noeud->tempsJobFin);
	free(noeud->tempsMachines);
	free(noeud->tempsMachinesFin);
	free(noeud->job);
	free(noeud->genotype);
	free(noeud->jobFin);
	free(noeud);
}

int bound_flowshop::max(int i, int j) {
	return (i > j) ? i : j;
}

void bound_flowshop::set_tempsMachines_retardDebut(Noeud *courant,
		int permutation[], int limite1) {
	courant->retardDebut = 0;
	for (int mm = 0; mm < nbMachines; mm++)
		courant->tempsMachines[mm] = 0;
	for (int j = 0; j <= limite1; j++) {
		int job = permutation[j];
		courant->tempsMachines[0] = courant->tempsMachines[0]
				+ tempsJob[0][job];
		for (int m = 1; m < nbMachines; m++)
			courant->tempsMachines[m] = max(courant->tempsMachines[m],
					courant->tempsMachines[m - 1]) + tempsJob[m][job];
		courant->retardDebut += max(0,
				courant->tempsMachines[nbMachines - 1] - tempsFin[job]);
	}
}

void bound_flowshop::set_job_jobFin(Noeud *courant, int permutation[],
		int limite1, int limite2) {
	for (int j = 0; j <= limite1; j++)
		courant->job[permutation[j]] = j + 1;
	for (int j = limite1 + 1; j < limite2; j++)
		courant->job[permutation[j]] = 0;
	for (int j = limite2; j < nbJob; j++) {
		courant->job[permutation[j]] = j + 1;
		courant->jobFin[j] = permutation[j];
	}
}

void bound_flowshop::set_nombres(Noeud *courant, int limite1, int limite2) {
	courant->nbAffectDebut = limite1 + 1;
	courant->nbAffectFin = nbJob - limite2;
	courant->nbAffect = courant->nbAffectDebut + courant->nbAffectFin;
}

void bound_flowshop::set_tempsMachinesFin_tempsJobFin(Noeud *courant) {
	int jobCour;
	int tmpMa[nbMachines];
	for (int j = nbMachines - 1; j >= 0; j--) {
		for (int k = j; k < nbMachines; k++)
			tmpMa[k] = 0;
		for (int k = nbJob - courant->nbAffectFin; k < nbJob; k++) {
			jobCour = courant->jobFin[k];
			tmpMa[j] += tempsJob[j][jobCour];
			for (int l = j + 1; l < nbMachines; l++) {
				if (tmpMa[l - 1] > tmpMa[l])
					tmpMa[l] = tmpMa[l - 1] + tempsJob[l][jobCour];
				else
					tmpMa[l] += tempsJob[l][jobCour];
			}
			courant->tempsJobFin[j][jobCour] = tmpMa[nbMachines - 1];
		}
		courant->tempsMachinesFin[j] = tmpMa[nbMachines - 1];
	}
}

void bound_flowshop::criteres_calculer(int permutation[], int*cmax,
		int*tardiness) {
	for (int mm = 0; mm < nbMachines; mm++)
		temps[mm] = 0;
	*tardiness = 0;
	for (int j = 0; j < nbJob; j++) {
		int job = permutation[j];
		temps[0] = temps[0] + tempsJob[0][job];
		for (int m = 1; m < nbMachines; m++)
			temps[m] = max(temps[m], temps[m - 1]) + tempsJob[m][job];
	}
	*cmax = temps[nbMachines - 1];
}

void bound_flowshop::init() {
	nbOrdo = 1;
	nbborne = 1;
	nbNoeud = 0;
	nbPartition = 10;
	nbRetardNonAffB1 = 0;
	nbRetardNonAffB2 = 0;
	nbZero = 0;
	seuil = 0;
	initialiserVar();
	courant = new Noeud(this);
	nbElem = somme;
	nbNoeudTotal = aFaire(nbJob);
	allocationNoeud(courant);
	lambda1 = 0;
	lambda2 = 1;
}

void bound_flowshop::bornes_calculer(int permutation[], int limite1,
		int limite2, int*couts, int best) {
	//if(first){init();first=false;}
	leMeilleur = best;
	couts[0] = 0;
	if (limite2 - limite1 == 1) {
		criteres_calculer(permutation, &couts[0], &couts[1]);
	} else {
		set_tempsMachines_retardDebut(courant, permutation, limite1);
		set_job_jobFin(courant, permutation, limite1, limite2);
		set_nombres(courant, limite1, limite2);
		set_tempsMachinesFin_tempsJobFin(courant);
		calculBorne(courant, 0);
		couts[0] = courant->borneInfCmax + 0; //-3, pour faire durer 50.10.0 environs 8 minutes
		couts[1] = courant->borneInfRetard;
	}
	couts[1] = 0;
}

void bound_flowshop::bornes_calculer(raw_bb_problem& p) {
	bornes_calculer(p.permutation, p.limite1, p.limite2, p.couts, 999999);
	p.couts_somme = p.couts[0] + p.couts[1];
}

void bound_flowshop::set_instance(instance_abstract*_instance) {
	instance = _instance;
	init();
}

