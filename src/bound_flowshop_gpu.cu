void initSomme_g() {
	somme_g = 0;

	for (int i = 1; i < nbMachines_g; i++)
		somme_g += i;
}

void free_memories() {
	for (int i = 0; i < nbMachines_g; i++)
		free(tempsFinMachines_g[i]);

	free(tempsFinMachines_g);

	for (int i = 0; i < somme_g; i++)
		free(tempsEntre_g[i]);

	free(tempsEntre_g);
	free(tempsFin_g);
	free(tempsExeTrie_g);
	free(tempsEntreTrie_g);
	free(tempsFinMachinesTrie_g);
	free(tempsLag_g);
	free(ordoSomme_g);
	free(nbFois_g);
	free(tempsFinTrie_g);
	free(tabJohnson_g);
	free(machine_g);
	free(tempsArriver_g);
	free(tempsDepart_g);
	free(minTempsArr_g);
	free(minTempsDep_g);
	free(tempsJob_g);
}

void allouerMemoire_g() {
	tempsFin_g = (int *) malloc(nbJob_g * sizeof(int));

	tempsExeTrie_g = (int *) malloc(nbMachines_g * nbJob_g * sizeof(int));

	tempsFinMachines_g = (int **) malloc(nbMachines_g * sizeof(int *));

	for (int i = 0; i < nbMachines_g; i++)
		tempsFinMachines_g[i] = (int *) malloc(nbJob_g * sizeof(int));

	tempsEntre_g = (int **) malloc(somme_g * sizeof(int *));

	for (int i = 0; i < somme_g; i++)
		tempsEntre_g[i] = (int *) malloc(nbJob_g * sizeof(int));

	tempsEntreTrie_g = (int *) malloc(somme_g * nbJob_g * sizeof(int));
	tempsFinMachinesTrie_g = (int *) malloc(
			nbMachines_g * nbJob_g * sizeof(int));
	tempsLag_g = (int *) malloc(somme_g * nbJob_g * sizeof(int));

	ordoSomme_g = (int *) malloc(somme_g * sizeof(int));
	nbFois_g = (int *) malloc(somme_g * sizeof(int));

	tempsFinTrie_g = (int *) malloc(nbJob_g * sizeof(int));

	tabJohnson_g = (int *) malloc(somme_g * nbJob_g * sizeof(int));

	machine_g = (int *) malloc(2 * somme_g * sizeof(int));

	tempsArriver_g = (int *) malloc(nbMachines_g * nbJob_g * sizeof(int));
	tempsDepart_g = (int *) malloc(nbMachines_g * nbJob_g * sizeof(int));

	minTempsArr_g = (int *) malloc(nbMachines_g * sizeof(int));
	minTempsDep_g = (int *) malloc(nbMachines_g * sizeof(int));
}

long double aFaire_g(int profondeur) {
	long double nbNoeudAFaire = 1;

	for (int i = 2; i <= profondeur; i++)
		nbNoeudAFaire = (nbNoeudAFaire * i) + i;

	return nbNoeudAFaire;
}

void initTempsExeTrie_g() {
	for (int i = 0; i < nbMachines_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tempsExeTrie_g[i * nbJob_g + j] = j;
}

void initTempsEntreTrie_g() {
	for (int i = 0; i < somme_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tempsEntreTrie_g[i * nbJob_g + j] = j;
}

void initTempArrays_g() {
	tempsExeTrie_T = (int **) malloc(nbMachines_g * sizeof(int *));
	tempsFinMachinesTrie_T = (int **) malloc(nbMachines_g * sizeof(int *));

	for (int i = 0; i < nbMachines_g; i++) {
		tempsExeTrie_T[i] = (int *) malloc(nbJob_g * sizeof(int));
		tempsFinMachinesTrie_T[i] = (int *) malloc(nbJob_g * sizeof(int));
	}

	tempsEntreTrie_T = (int **) malloc(somme_g * sizeof(int *));
	tabJohnson_T = (int **) malloc(somme_g * sizeof(int *));

	for (int i = 0; i < somme_g; i++) {
		tempsEntreTrie_T[i] = (int *) malloc(nbJob_g * sizeof(int));
		tabJohnson_T[i] = (int *) malloc(nbJob_g * sizeof(int));
	}

	for (int i = 0; i < somme_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tempsEntreTrie_T[i][j] = j;

	for (int i = 0; i < nbMachines_g; i++)
		for (int j = 0; j < nbJob_g; j++) {
			tempsExeTrie_T[i][j] = j;
			tempsFinMachinesTrie_T[i][j] = j;
		}
}

void initTempsFinTrie_g() {
	for (int i = 0; i < nbJob_g; i++)
		tempsFinTrie_g[i] = i;
}

void initTempsFinMachinesTrie_g() {
	for (int i = 0; i < nbMachines_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tempsFinMachinesTrie_g[i * nbJob_g + j] = j;
}

int estInf_g(int i, int j) {
	if (pluspetit_g[0][i] == pluspetit_g[0][j]) {
		if (pluspetit_g[0][i] == 1)
			return pluspetit_g[1][i] < pluspetit_g[1][j];

		return pluspetit_g[1][i] > pluspetit_g[1][j];
	}

	return pluspetit_g[0][i] < pluspetit_g[0][j];
}

int estSup_g(int i, int j) {
	if (pluspetit_g[0][i] == pluspetit_g[0][j]) {
		if (pluspetit_g[0][i] == 1)
			return pluspetit_g[1][i] > pluspetit_g[1][j];

		return pluspetit_g[1][i] < pluspetit_g[1][j];
	}

	return pluspetit_g[0][i] > pluspetit_g[0][j];
}

int partionner_g(int *ordo, int deb, int fin) {
	int d = deb - 1;
	int f = fin + 1;
	int mem, pivot = ordo[deb];

	do {

		do {
			f--;
		} while (estSup_g(ordo[f], pivot));

		do {
			d++;
		} while (estInf_g(ordo[d], pivot));

		if (d < f) {
			mem = ordo[d];
			ordo[d] = ordo[f];
			ordo[f] = mem;
		}

	} while (d < f);

	return f;
}

void quicksort_g(int *ordo, int deb, int fin) {
	int k;

	if ((fin - deb) > 0) {
		k = partionner_g(ordo, deb, fin);
		quicksort_g(ordo, deb, k);
		quicksort_g(ordo, k + 1, fin);
	}
}

void trierTableau_g(int *ordo, int nbElem_g, int *nbFois_g, bool croissant) {
	pluspetit_g[0] = (int *) malloc((nbElem_g) * sizeof(int));
	pluspetit_g[1] = (int *) malloc((nbElem_g) * sizeof(int));

	for (int i = 0; i < nbElem_g; i++) {
		if (croissant)
			pluspetit_g[0][i] = 1;
		else
			pluspetit_g[0][i] = 2;

		pluspetit_g[1][i] = nbFois_g[i];
	}

	quicksort_g(ordo, 0, (nbElem_g - 1));

	free(pluspetit_g[0]);
	free(pluspetit_g[1]);
}

void remplirTempsExeTrie_g() {
	for (int i = 0; i < nbMachines_g; i++)
		trierTableau_g(tempsExeTrie_T[i], nbJob_g, tempsJob_T[i], true);

	for (int i = 0; i < nbMachines_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tempsExeTrie_g[i * nbJob_g + j] = tempsExeTrie_T[i][j];

	for (int i = 0; i < nbMachines_g; i++)
		free(tempsExeTrie_T[i]);

	free(tempsExeTrie_T);
}

void remplirTempsArriverDepart_g() {
	bool faitDep[nbMachines_g];
	bool faitArr[nbMachines_g];

	int machine_g;

	for (int k = 1; k < nbMachines_g; k++) {
		faitArr[k] = false;
		faitDep[k] = false;
	}

	minTempsDep_g[nbMachines_g - 1] = 0;
	minTempsArr_g[0] = 0;

	for (int i = 0; i < nbJob_g; i++) {
		tempsArriver_g[i] = 0;
		tempsDepart_g[(nbMachines_g - 1) * nbJob_g + i] = 0;

		for (int k = 1; k < nbMachines_g; k++) {
			machine_g = nbMachines_g - k - 1;
			tempsArriver_g[k * nbJob_g + i] = tempsArriver_g[(k - 1) * nbJob_g
					+ i] + tempsJob_g[(k - 1) * nbJob_g + i];
			tempsDepart_g[machine_g * nbJob_g + i] = tempsDepart_g[(machine_g
					+ 1) * nbJob_g + i]
					+ tempsJob_g[(machine_g + 1) * nbJob_g + i];

			if (!faitArr[k]
					|| minTempsArr_g[k] > tempsArriver_g[k * nbJob_g + i]) {
				faitArr[k] = true;
				minTempsArr_g[k] = tempsArriver_g[k * nbJob_g + i];
			}

			if (!faitDep[k]
					|| minTempsDep_g[machine_g]
							> tempsDepart_g[machine_g * nbJob_g + i]) {
				faitDep[k] = true;
				minTempsDep_g[machine_g] =
						tempsDepart_g[machine_g * nbJob_g + i];
			}
		}
	}
}

void remplirMachine_g() {
	int cmpt = 0;

	for (int i = 0; i < (nbMachines_g - 1); i++)
		for (int j = i + 1; j < nbMachines_g; j++) {
			machine_g[cmpt] = i;
			cmpt++;
		}

	int taille = cmpt;
	cmpt = 0;

	for (int i = 0; i < (nbMachines_g - 1); i++)
		for (int j = i + 1; j < nbMachines_g; j++) {
			machine_g[1 * taille + cmpt] = j;
			cmpt++;
		}
}

void remplirLag_g() {
	int m1, m2;

	for (int i = 0; i < somme_g; i++) {
		m1 = machine_g[0 * somme_g + i];
		m2 = machine_g[1 * somme_g + i];

		for (int j = 0; j < nbJob_g; j++) {
			tempsLag_g[i * nbJob_g + j] = 0;

			for (int k = m1 + 1; k < m2; k++)
				tempsLag_g[i * nbJob_g + j] += tempsJob_g[k * nbJob_g + j];

			tempsEntre_g[i][j] = tempsLag_g[i * nbJob_g + j]
					+ tempsJob_g[m2 * nbJob_g + j];
		}
	}
}

void remplirTempsEntreTrie_g() {
	for (int i = 0; i < somme_g; i++)
		trierTableau_g(tempsEntreTrie_T[i], nbJob_g, tempsEntre_g[i], true);

	for (int i = 0; i < somme_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tempsEntreTrie_g[i * nbJob_g + j] = tempsEntreTrie_T[i][j];

	for (int i = 0; i < somme_g; i++)
		free(tempsEntreTrie_T[i]);

	free(tempsEntreTrie_T);
}

void Johnson_g(int *ordo, int m1, int m2, int s) {
	pluspetit_g[0] = (int *) malloc(nbJob_g * sizeof(int));
	pluspetit_g[1] = (int *) malloc(nbJob_g * sizeof(int));

	for (int i = 0; i < nbJob_g; i++) {
		ordo[i] = i;

		if (tempsJob_g[m1 * nbJob_g + i] < tempsJob_g[m2 * nbJob_g + i]) {
			pluspetit_g[0][i] = 1;
			pluspetit_g[1][i] = tempsJob_g[m1 * nbJob_g + i]
					+ tempsLag_g[s * nbJob_g + i];
		} else {
			pluspetit_g[0][i] = 2;
			pluspetit_g[1][i] = tempsJob_g[m2 * nbJob_g + i]
					+ tempsLag_g[s * nbJob_g + i];
		}
	}

	quicksort_g(ordo, 0, (nbJob_g - 1));

	free(pluspetit_g[0]);
	free(pluspetit_g[1]);
}

void remplirTabJohnson_g() {
	int cmpt = 0;

	for (int i = 0; i < (nbMachines_g - 1); i++)
		for (int j = i + 1; j < nbMachines_g; j++) {
			Johnson_g(tabJohnson_T[cmpt], i, j, cmpt);
			cmpt++;
		}

	for (int i = 0; i < somme_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tabJohnson_g[i * nbJob_g + j] = tabJohnson_T[i][j];

	for (int i = 0; i < somme_g; i++)
		free(tabJohnson_T[i]);

	free(tabJohnson_T);
}

void remplirTempsFinMachines_g() {
	for (int i = 0; i < nbJob_g; i++)
		tempsFinMachines_g[nbMachines_g - 1][i] = tempsFin_g[i];

	for (int i = nbMachines_g - 2; i >= 0; i--)
		for (int j = 0; j < nbJob_g; j++)
			tempsFinMachines_g[i][j] = tempsFinMachines_g[i + 1][j]
					- tempsJob_T[i + 1][j];

	for (int i = 0; i < nbMachines_g; i++)
		free(tempsJob_T[i]);

	free(tempsJob_T);
}

void remplirTempsFinMachinesTrie_g() {
	for (int i = 0; i < nbMachines_g; i++)
		trierTableau_g(tempsFinMachinesTrie_T[i], nbJob_g,
				tempsFinMachines_g[i], true);

	for (int i = 0; i < nbMachines_g; i++)
		for (int j = 0; j < nbJob_g; j++)
			tempsFinMachinesTrie_g[i * nbJob_g + j] =
					tempsFinMachinesTrie_T[i][j];

	for (int i = 0; i < nbMachines_g; i++)
		free(tempsFinMachinesTrie_T[i]);

	free(tempsFinMachinesTrie_T);
}

void initNbFois_g() {
	for (int i = 0; i < somme_g; i++) {
		ordoSomme_g[i] = i;
		nbFois_g[i] = 0;
	}
}

void initialiserVar_g() {
	initSomme_g();
	allouerMemoire_g();
	initTempArrays_g();
	initTempsFinTrie_g();
	initTempsExeTrie_g();
	initTempsEntreTrie_g();
	initTempsFinMachinesTrie_g();
	trierTableau_g(tempsFinTrie_g, nbJob_g, tempsFin_g, true);
	remplirTempsExeTrie_g();
	remplirTempsArriverDepart_g();
	remplirMachine_g();
	remplirLag_g();
	remplirTempsEntreTrie_g();
	remplirTabJohnson_g();
	remplirTempsFinMachines_g();
	remplirTempsFinMachinesTrie_g();
	initNbFois_g();
}

void init_bound() {
	nbOrdo_g = 1;
	nbborne_g = 1;
	nbNoeud_g = 0;
	nbPartition_g = 10;
	nbRetardNonAffB1_g = 0;
	nbRetardNonAffB2_g = 0;
	nbZero_g = 0;
	seuil_g = 0;

	initialiserVar_g();

	nbElem_g = somme_g;

	nbNoeudTotal_g = aFaire_g(nbJob_g);
}

void init_problem_device_g(problem_d &p) {
	p.limite1 = -1;
	p.limite2 = nbJob_g;

	for (int j = 0; j < nbJob_g; j++)
		p.permutation[j] = j;

	p.couts_somme = 0;

	p.depth = 0;
}

inline __device__ void set_job_jobFin_g(int* job, int* jobFin, int* permutation, int limite1, int limite2,int nbJob_g)
{
	int j = 0;

	for(j = 0;j <= limite1;j++) job[permutation[j]] = j + 1;

	for(j = limite1 + 1;j < limite2;j++) job[permutation[j]] = 0;

	for(j = limite2; j < nbJob_g;j++)
	{
		job[permutation[j]] = j + 1;
		jobFin[j] = permutation[j];
	}
}

inline __device__ int cmaxFin_g(int *tempsMachinesFin, int *tmp, int *ma)
{
	return max(tmp[1] + tempsMachinesFin[ma[1]],tmp[0] + tempsMachinesFin[ma[0]]);
}

inline __device__ void initCmax_g(int* tempsMachines,int nbAffectDebut, int *tmp, int *ma, int ind,int * machine_g, int somme_g, int * minTempsArr_g)
{
	ma[0] = machine_g[ind];
	ma[1] = machine_g[1 * somme_g + ind];

	int coeff = __cosf(nbAffectDebut);
	tmp[0] = (1 - coeff) * tempsMachines[ma[0]] + coeff * minTempsArr_g[ma[0]];
	tmp[1] = (1 - coeff) * tempsMachines[ma[1]] + coeff * minTempsArr_g[ma[1]];
}

inline __device__ void heuristiqueCmax_g(int* job, int *tmp, int *ma, int ind, int nbJob_g, int * tabJohnson_g, unsigned char* tempsJob_g,int * tempsLag_g)
{
	int jobCour;

	for(int j= 0; j < nbJob_g; j++)
	{
		jobCour = tabJohnson_g[ind * nbJob_g + j];

		if( job[jobCour] == 0 )
		{
			tmp[0] = tmp[0] + tempsJob_g[ma[0] * nbJob_g + jobCour];
			tmp[1] = max (tmp[1], tmp[0] + tempsLag_g[ind * nbJob_g + jobCour]) + tempsJob_g[ma[1] * nbJob_g + jobCour];
		}
	}
}

inline __device__ int criteres_calculer_g(int* permutation,int nbMachines_g,int nbJob_g,unsigned char* tempsJob_g)
{
	int temps[MAX_NB_MACHINES];

	for(int mm = 0; mm < nbMachines_g; mm++) temps[mm] = 0;

	for(int j = 0; j < nbJob_g;j++)
	{
		int job = permutation[j];

		temps[0] = temps[0] + tempsJob_g[job];

		for(int m = 1; m < nbMachines_g;m++)
		temps[m] = max(temps[m],temps[m-1]) + tempsJob_g[m * nbJob_g + job];
	}

	return temps[nbMachines_g-1];
}

inline __device__ void set_tempsMachines_retardDebut_g(int *tempsMachines, int* permutation, int limite1,int nbMachines_g,int nbJob_g,unsigned char* tempsJob_g)
{
	int m = 0;

	for(m = 0; m < nbMachines_g; m++) tempsMachines[m] = 0;

	for(int j = 0; j <= limite1; j++)
	{
		int job = permutation[j];

		tempsMachines[0] = tempsMachines[0] + tempsJob_g[job];

		for(m = 1; m < nbMachines_g;m++)
		tempsMachines[m] = max(tempsMachines[m],tempsMachines[m-1]) + tempsJob_g[m * nbJob_g + job];
	}
}

inline __device__ void set_tempsMachinesFin_tempsJobFin_g(int* jobFin, int * tempsMachinesFin,int nbAffectFin,int nbJob_g, int nbMachines_g,unsigned char* tempsJob_g)
{
	int jobCour;

	int tmpMa[MAX_NB_MACHINES];

	for(int j = 0; j < nbMachines_g; j++)
	{
		for(int k = j; k < nbMachines_g; k++) tmpMa[k] = 0;

		for(int k = nbJob_g - nbAffectFin; k < nbJob_g; k++)
		{
			jobCour = jobFin[k];

			tmpMa[j] = tmpMa[j] + tempsJob_g[j * nbJob_g + jobCour];

			for(int l = j + 1; l < nbMachines_g; l++)
			{
				tmpMa[l] = max (tmpMa[l-1],tmpMa[l]);
				tmpMa[l] = tmpMa[l] + tempsJob_g[l * nbJob_g + jobCour];
			}
		}

		tempsMachinesFin[j] = tmpMa[nbMachines_g-1];
	}
}

inline __device__ int borneInfMakespan_g(int* job,int *tempsMachinesFin,int* tempsMachines,
		int nbAffectDebut,int nbAffectFin,int *valBorneInf, int retardNonFin, int minCmax,int nbJob_g, int nbElem_g,
		int leMeilleur_g, int nbborne_g, int somme_g, int * minTempsArr_g, int*nbFois_g, int *machine_g, int * tabJohnson_g,
		unsigned char* tempsJob_g, int* tempsLag_g, int* ordoSomme_g, int* minTempsDep_g)
{
	int moinsBon = 0;

	int idxMoinsBonCmax, i, tmpDep, retard = retardNonFin;

	int ma[2] = {0,0};
	int tmp[2] = {0,0};
	int res[2] = {0,0};

	int l;

	int coeffReturn=1;

	for(l = 0; l < nbElem_g; l++)
	{
		i = ordoSomme_g[l];
		initCmax_g(tempsMachines,nbAffectDebut,tmp,ma,i,machine_g,somme_g,minTempsArr_g);
		heuristiqueCmax_g(job, tmp, ma, i, nbJob_g,tabJohnson_g, tempsJob_g, tempsLag_g);

		if( nbAffectFin != 0 ) tmp[1] = cmaxFin_g(tempsMachinesFin, tmp, ma);
		else tmp[1] = tmp[1] + minTempsDep_g[ma[1]];

		float un = 1;
		int coeff2 = min(un, __expf(tmp[1] - moinsBon - 1));
		idxMoinsBonCmax = coeff2 * i;
		moinsBon = max(moinsBon,tmp[1]);
		int coeff3 = min(un,__expf(moinsBon - leMeilleur_g - 1));
		int coeff4 = min(1,(leMeilleur_g + 2) ^ 1);
		int coeff5 = coeff3 * coeff4;

		nbborne_g = nbborne_g + coeff5;

		valBorneInf[0] = valBorneInf[0] + coeff5 * moinsBon;
		coeffReturn = coeff5;
	}

	nbborne_g++;
	nbFois_g[idxMoinsBonCmax]++;
	valBorneInf[0] = moinsBon;

	return 0;
}

inline __device__ int calculBorne_g(int* job,int *tempsMachinesFin,int *tempsMachines,int nbAffectDebut,int nbAffectFin,int nbJob_g,int leMeilleur_g, int nbborne_g,int somme_g,int nbElem_g,unsigned char* tempsJob_g,int* nBfois,int* machine_g,int* tabJohnson_g,int* tempsLag_g,int * minTempsArr_g,int* ordoSomme_g,int* minTempsDep_g)
{
	int retardNonAff = 0;

	int minCmax = 0;

	int valBorneInf[2];

	int retardNonFin = retardNonAff;

	borneInfMakespan_g(job,tempsMachinesFin,tempsMachines,nbAffectDebut,nbAffectFin,valBorneInf,retardNonFin,minCmax,nbJob_g,nbElem_g,leMeilleur_g,nbborne_g,somme_g,minTempsArr_g,
			nBfois,machine_g,tabJohnson_g,tempsJob_g,tempsLag_g,ordoSomme_g,minTempsDep_g);

	return valBorneInf[0];
}

__global__ void Evaluate_ON_GPU(raw_bb_problem* pool_to_evaluate, int* bounds,
		int nbJob_g, int nbMachines_g, int nbborne_g, int somme_g, int nbElem_g,
		int *nbFois_g, int *machine_g, int *tabJohnson_g, int *tempsJob_g,
		int *tempsLag_g, int *minTempsArr_g, int *ordoSomme_g,
		int *minTempsDep_g, int todo, int time_seed_g) {
	int thread_idx = blockIdx.x * blockDim.x + threadIdx.x;

	__shared__
	unsigned char tempsJob_g_shared[MAX_NB_MACHINES * MAX_NBJOBS];

	if (thread_idx < todo) {
		if (threadIdx.x == 0) {
			for (int i = 0; i < nbMachines_g; i++)
				for (int j = 0; j < nbJob_g; j++)
					tempsJob_g_shared[i * nbJob_g + j] = tempsJob_g[i * nbJob_g
							+ j];
		}

		__syncthreads();

		int tempsMachines[MAX_NB_MACHINES];
		int tempsMachinesFin[MAX_NB_MACHINES];

		int job[MAX_NBJOBS];
		int jobFin[MAX_NBJOBS];
		int nbAffectFin = nbJob_g - pool_to_evaluate[thread_idx].limite2;
		int nbAffectDebut = pool_to_evaluate[thread_idx].limite1 + 1;
		int leMeilleur_g = 999999;
		int borneInf = 0;

		if (pool_to_evaluate[thread_idx].limite2
				- pool_to_evaluate[thread_idx].limite1 == 1)
			borneInf = criteres_calculer_g(
					pool_to_evaluate[thread_idx].permutation, nbMachines_g,
					nbJob_g, tempsJob_g_shared);
		else {
			set_tempsMachines_retardDebut_g(tempsMachines,
					pool_to_evaluate[thread_idx].permutation,
					pool_to_evaluate[thread_idx].limite1, nbMachines_g, nbJob_g,
					tempsJob_g_shared);
			set_job_jobFin_g(job, jobFin,
					pool_to_evaluate[thread_idx].permutation,
					pool_to_evaluate[thread_idx].limite1,
					pool_to_evaluate[thread_idx].limite2, nbJob_g);
			set_tempsMachinesFin_tempsJobFin_g(jobFin, tempsMachinesFin,
					nbAffectFin, nbJob_g, nbMachines_g, tempsJob_g_shared);
			borneInf = calculBorne_g(job, tempsMachinesFin, tempsMachines,
					nbAffectDebut, nbAffectFin, nbJob_g, leMeilleur_g,
					nbborne_g, somme_g, nbElem_g, tempsJob_g_shared, nbFois_g,
					machine_g, tabJohnson_g, tempsLag_g, minTempsArr_g,
					ordoSomme_g, minTempsDep_g);
		}

		bounds[thread_idx] = borneInf;
	}
}
