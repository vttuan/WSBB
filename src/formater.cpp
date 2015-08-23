#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>

using namespace std;

void supprimer_avant(string& ligne, const char*mot)
{
	string tmp=ligne.substr(ligne.find(mot)+strlen(mot),ligne.length());
	ligne=tmp;
}

void supprimer_apres(string& ligne, const char*mot)
{
	string tmp=ligne.substr(0,ligne.find(mot));
	ligne=tmp;
}

void remplacer_caractere(string& ligne, const char ancien, const char nouveau)
{
	for(int i=0;i<ligne.length();i++)
		if(ligne[i]==ancien)
			ligne[i]=nouveau;
}


float string_to_temps(string& ligne)
{
	stringstream buffer(stringstream::in | stringstream::out);
	buffer<<ligne<<" "<<-1;
	float v1,v2,v3;
	buffer>>v1>>v2>>v3;
	return (v3==-1)?(v1*60+v2):(v1*60*60+v2*60+v3);
}

string nom_reponse(const char*nom)
{
	stringstream buffer(stringstream::in | stringstream::out);
	string _nom(nom);
	supprimer_apres(_nom,".txt");
	buffer<<_nom<<".data";
	return buffer.str();
}

string lire_ligne(ifstream& f)
{
#define TAILLE 5000
char _ligne[TAILLE];
	f.getline(_ligne,TAILLE);
	string ligne(_ligne);
	return ligne;
}

bool contenir(string& ligne, const char* mot)
{
	size_t found = ligne.find (mot);
	return (found != string::npos);
}

void lire_instance(string& ligne, int& instance)
{
	stringstream buffer(stringstream::in | stringstream::out);
	buffer<<ligne;
	string INSTANCE;
	buffer>>INSTANCE>>instance;
} 

void lire_makespan_noeuds(string& ligne, int& makespan, int& noeuds)
{
	stringstream buffer(stringstream::in | stringstream::out);
	buffer<<ligne;
	string EXIT;
	buffer>>EXIT>>makespan>>noeuds;
} 

void lire_temps1(string& ligne, float& temps)
{
	supprimer_avant(ligne,"user");
	remplacer_caractere(ligne,'m',' ');
	remplacer_caractere(ligne,'s',' ');
	temps=string_to_temps(ligne);
} 

void lire_temps2(string& ligne, float& temps)
{
	supprimer_avant(ligne,"system");
	supprimer_apres(ligne,"elapsed");
	remplacer_caractere(ligne,':',' ');
	temps=string_to_temps(ligne);
} 

int main(int argc, char **argv)
{
	ifstream f(argv[1], ifstream::in);
	ofstream o(nom_reponse(argv[1]).c_str());
	int instance, makespan, noeuds; float temps;
	while (f.good ())
	{
	string ligne=lire_ligne(f);
	if(contenir(ligne,"INSTANCE"))
		lire_instance(ligne, instance);
	if(contenir(ligne,"EXIT"))
		lire_makespan_noeuds(ligne, makespan, noeuds);

		if(contenir(ligne,"elapsed")) lire_temps2(ligne, temps);
	else 	if(contenir(ligne,"user")) lire_temps1(ligne, temps);

	if(contenir(ligne,"sys")||contenir(ligne,"inputs"))
		o<<instance<<";"<<makespan<<";"<<noeuds<<";"<<temps<<";"<<endl;
	}
	o.close ();
	f.close ();
	return 0;
}

