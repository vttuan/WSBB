using namespace std;

#include "../headers/tree.h"
#include "../headers/arguments.h"

char arguments::directory[50];
char arguments::worker_name[50];
char arguments::file[50];
int arguments::worker_port;
char arguments::type = 'g';
char arguments::gpu_mode;
char arguments::index_name[50];
int arguments::index_port;
bool arguments::worker = true;
bool arguments::root = false;
int arguments::sizev;
int arguments::instancev;
int arguments::costv;
int arguments::checkpointv;
int arguments::balancingv;
bool arguments::divisionv;
int arguments::cpu_number = 0;
int arguments::gpu_number = 0;
int arguments::explorer_number = 0;
int arguments::pool_number = 0;
int arguments::pool_size = 0;
int arguments::sleep_number = 0;
int arguments::period = 0;
bool arguments::file_exists = false;
char arguments::johnson = 'g';
char arguments::temps = 'p';
char arguments::divergence = 'v';
char arguments::strategy = STRATEGY_DEPTH;
int arguments::devices = -1;

int arguments::cpu_operation = CPU_NOT_DEFINED;
void arguments::initialization(int instance, bool optimal) {
	string tmp = "";
	int jobs = 0, machines = 0, valeur = 0;
	stringstream rubrique;
	rubrique << "instance" << instance << "i";
	ifstream infile("../parameters/instances.data");

	while (!infile.eof()) {
		string str;
		getline(infile, str);

		if (str.substr(0, rubrique.str().length()).compare(rubrique.str())
				== 0) {
			stringstream buffer;
			buffer << str << endl;
			buffer >> tmp >> jobs >> machines >> valeur;
			break;
		}
	}

	infile.close();

	sizev = jobs;
	costv = (optimal) ? valeur : 9999999;
	checkpointv = 9999999;
	balancingv = 10;
	divisionv = 0;
	instancev = instance;
}

#define OPTIONS "aiowcdpugsfjtrlynezh"
void arguments::parse_arguments(int argc, char **argv) {
	strcpy(directory, "../parameters/");

	strcpy(index_name, "localhost");
	strcpy(worker_name, "localhost");

	struct option long_options[] = { { "instance", 0, 0, 'i' }, //numéro de l'instance sans initialisation (vaut mieux regrouper le numéro de l'instance et la valeur initiale en une propriété)
			{ "optimal", 0, 0, 'o' }, //numéro de l'instance avec initialisation
			{ "worker", 0, 0, 'w' }, //worker et en donnant le host de la machine où se trouve le coordinateur
			{ "coordinator", 0, 0, 'c' }, //coordinateur
			{ "coordinator_port", 0, 0, 'p' }, //numéro du port du coordinateur
			{ "index_por", 0, 0, 'd' }, //numéro du port du coordinateur
			{ "worker_por", 0, 0, 'n' }, //numéro du port du worker
			{ "cpu", 0, 0, 'u' }, //processus cpu suivant du nombre de coeurs de calcul
			{ "explorer", 0, 0, 'e' }, //processus cpu suivant du nombre de coeurs de calcul
			{ "gpu", 0, 0, 'g' }, //processus gpu suivant du nombre de processeurs gpu
			{ "mesure", 0, 0, 's' }, //processus pour mesure suivi du temps en secondes
			{ "pool", 0, 0, 'l' }, //processus pour mesurer un pool par gpu suivi de la taille du pool à l'origine
			{ "file", 0, 0, 'f' }, //processus pour mesure suivi du temps en secondes
			{ "johnson", 0, 0, 'j' }, // g=la matrice johnson est dans la mémoire globale,  p=la matrice johnson est dans la mémoire partagée
			{ "temps", 0, 0, 't' }, //g=la matrice temps est dans la mémoire globale,  p=la matrice temps est dans la mémoire partagée
			{ "divergence", 0, 0, 'r' }, //v=la divergence de threads est traitée, f=sinon
			{ "strategy", 0, 0, 'y' }, //donne la stratégie d'exploration
			{ "devices", 0, 0, 'a' },
			{ "cpu_operation", 0, 0, 'z' },
			{ "pool_size", 0, 0, 'h' }, { 0, 0, 0, 0 } };

	int c = getopt_long(argc, argv, OPTIONS, long_options, NULL);

	while (c != -1) {
		switch (c) {
		case 'i':
			initialization(strtol(argv[optind], 0, 10), false);
			break;
		case 'o':
			initialization(strtol(argv[optind], 0, 10), true);
			break;
		case 'w':
			worker = true;
			strcpy(index_name, argv[optind]);
			std::cout << index_name << endl;
			break;
		case 'c':
			root = true;
			worker = false;
			strcpy(index_name, argv[optind]);
			break;
		case 'p':
			index_port = strtol(argv[optind], 0, 10);
			break;
		case 'd':
			worker_port = index_port = strtol(argv[optind], 0, 10);
			break;
		case 'n':
			worker_port = strtol(argv[optind], 0, 10);
			break;

		case 'u':
			cpu_number = strtol(argv[optind], 0, 10);
			break;
		case 'g':
			gpu_number = strtol(argv[optind], 0, 10);
			break;

		case 'f':
			file_exists = true;
			strcpy(file, argv[optind]);
			break;
		case 't':
			temps = argv[optind][0];
			break;
		case 'j':
			johnson = argv[optind][0];
			break;
		case 'r':
			divergence = argv[optind][0];
			break;

		case 'a':
			devices = strtol(argv[optind], 0, 10);
			break;

		case 'y':
			strategy = argv[optind][0];
			break;

		case 'z':
			cpu_operation = strtol(argv[optind], 0, 10);
			cout << "xxxx: " << cpu_operation << endl;
			break;

		case 'h':
				pool_size = strtol(argv[optind], 0, 10);
				break;
		}
		c = getopt_long(argc, argv, OPTIONS, long_options, NULL);
	}
}

