/*
Programme qui, à partir d'un fichier binaire, créé une ListeFilm, la modifie, l'affiche et la détruit.
\file   td3.cpp
\author Benoit - Paraschivoiu et St - Arnaud
date    24 fevrier 2021
Créé le 17 fevrier 2021
*/
#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures_td3.hpp"      // Structures de données pour la collection de films en mémoire.

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include "cppitertools/range.hpp"
#include "gsl/span"
#include "debogage_memoire.hpp" 
#include <memory>// Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).
#include <sstream>
using namespace std;
using namespace iter;
using namespace gsl;
using ListeActeurs = Liste<Acteur>;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}

// Fonctions pour ajouter un Film à une ListeFilms.
//[
void ListeFilms::changeDimension(int nouvelleCapacite)
{
	Film** nouvelleListePtr = new Film*[nouvelleCapacite];
	
	if (elements != nullptr) {  // Noter que ce test n'est pas nécessaire puique nElements sera zéro si elements est nul, donc la boucle ne tentera pas de faire de copie, et on a le droit de faire delete sur un pointeur nul (ça ne fait rien).
		nElements = min(nouvelleCapacite, nElements);
		for (int i : range(nElements))
			nouvelleListePtr[i] = elements[i];
		delete[] elements;
	}
	
	elements = nouvelleListePtr;
	capacite = nouvelleCapacite;
}

void ListeFilms::ajouterFilm(Film* filmPtr)
{
	if (nElements == capacite)
		changeDimension(max(1, capacite * 2));
	elements[nElements++] = filmPtr;
}
//]

// Fonction pour enlever un Film d'une ListeFilms (enlever le pointeur) sans effacer le film; la fonction prenant en paramètre un pointeur vers le film à enlever.  L'ordre des films dans la liste n'a pas à être conservé.
//[
// On a juste fait une version const qui retourne un span non const. C'est valide puisque c'est la struct qui est const et non ce qu'elle pointe.  Ça ne va peut-être pas bien dans l'idée qu'on ne devrait pas pouvoir modifier une liste const, mais il y aurais alors plusieurs fonctions à écrire en version const et non-const pour que ça fonctionne bien, et ce n'est pas le but du TD (il n'a pas encore vraiment de manière propre en C++ de définir les deux d'un coup).

void ListeFilms::enleverFilm(const Film* filmPtr)
{
	for (Film*& elementPtr : enSpan()) {  // Doit être une référence au pointeur pour pouvoir le modifier.
		if (elementPtr == filmPtr) {
			if (nElements > 1)
				elementPtr = elements[nElements - 1];
			nElements--;
			return;
		}
	} 
}
//]

// Fonction pour trouver un Acteur par son nom dans une ListeFilms, qui retourne un pointeur vers l'acteur, ou nullptr si l'acteur n'est pas trouvé.  Devrait utiliser span.
//[
// Voir la NOTE ci-dessous pourquoi Acteur* n'est pas const.  Noter que c'est valide puisque c'est la struct uniquement qui est const dans le paramètre, et non ce qui est pointé par la struct.
//span<shared_ptr<Acteur>> spanListeActeurs(const ListeActeurs& liste) { return span(liste.elements_.get(), liste.nElements_); }

//NOTE: Doit retourner un Acteur modifiable, sinon on ne peut pas l'utiliser pour modifier l'acteur tel que demandé dans le main, et on ne veut pas faire écrire deux versions aux étudiants dans le TD2.
shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const
{
	for (const Film* filmPtr : enSpan()) {
		for (shared_ptr<Acteur>& acteurPtr : filmPtr->acteurs.enSpan()) {
			if (acteurPtr->nom == nomActeur)
				return acteurPtr;
		}
	}
	return nullptr;
}
//]

// Les fonctions pour lire le fichier et créer/allouer une ListeFilms.

shared_ptr<Acteur> lireActeur(istream& fichier, ListeFilms& listeFilms)
{
	shared_ptr<Acteur> acteurPtr = make_shared<Acteur>();
	acteurPtr->nom            = lireString(fichier);
	acteurPtr->anneeNaissance = lireUint16 (fichier);
	acteurPtr->sexe           = lireUint8  (fichier);

	shared_ptr<Acteur> acteurExistantPtr = listeFilms.trouverActeur(acteurPtr->nom);
	if (acteurExistantPtr != nullptr)
		return acteurExistantPtr;
	else {
		cout << "Création Acteur " << acteurPtr->nom << endl;
		return acteurPtr;
	}
}

Film* lireFilm(istream& fichier, ListeFilms& listeFilms)
{
	Film* filmPtr = new Film;
	filmPtr->titre = lireString(fichier);
	filmPtr->realisateur = lireString(fichier);
	filmPtr->anneeSortie = lireUint16(fichier);
	filmPtr->recette = lireUint16(fichier);
	filmPtr->acteurs = ListeActeurs(lireUint8(fichier)); 

	cout << "Création Film " << filmPtr->titre << endl;
	for (shared_ptr<Acteur>& acteurPtr : filmPtr->acteurs.enSpan()) {
		acteurPtr = lireActeur(fichier, listeFilms);
		//acteur->joueDans.ajouterFilm(filmp); 
	}
	return filmPtr;
}

ListeFilms creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);
	
	int nElements = lireUint16(fichier);

	ListeFilms listeFilms;
	for ([[maybe_unused]] int i : range(nElements)) { //NOTE: On ne peut pas faire un span simple avec ListeFilms::enSpan car la liste est vide et on ajoute des éléments à mesure.
		listeFilms.ajouterFilm(lireFilm(fichier, listeFilms));
	}
	
	return listeFilms;
}

// Fonctions pour détruire un film (relâcher toute la mémoire associée à ce film, et les acteurs qui ne jouent plus dans aucun films de la collection).  Enlève aussi le film détruit des films dans lesquels jouent les acteurs.  Pour fins de débogage, les noms des acteurs sont affichés lors de leur destruction.
//[
/*void detruireActeur(Acteur* acteur)
{
	cout << "Destruction Acteur " << acteur->nom << endl;
	acteur->joueDans.detruire();
	delete acteur;
}
*/
/*bool joueEncore(const Acteur* acteur)
{
	return acteur->joueDans.size() != 0;
}*/

void detruireFilm(Film* filmPtr)
{
	/*for (Acteur* acteur : spanListeActeurs(film->acteurs)) {
		acteur->joueDans.enleverFilm(film);
		if (!joueEncore(acteur))
			detruireActeur(acteur);
	}*/
	cout << "Destruction Film " << filmPtr->titre << endl;
	delete filmPtr;
}
//]

// Fonction pour détruire une ListeFilms et tous les films qu'elle contient.
//[
//NOTE: La bonne manière serait que la liste sache si elle possède, plutôt qu'on le dise au moment de la destruction, et que ceci soit le destructeur.  Mais ça aurait complexifié le TD2 de demander une solution de ce genre, d'où le fait qu'on a dit de le mettre dans une méthode.
void ListeFilms::detruire(bool possedeLesFilms)
{
	if (possedeLesFilms)
		for (Film* filmPtr : enSpan())
			detruireFilm(filmPtr);
	delete[] elements;
}
//]

// Fonctions de surcharge de l'operateur "<<" pour "cout" un acteur ou un film

ostream& operator<< (ostream& os, const Acteur& acteur)
{
	os << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
	return os;
}

ostream& operator<< (ostream& os, const Film& film)
{
	os << "Titre: " << film.titre << endl;
	os << "  Réalisateur: " << film.realisateur << "  Année :" << film.anneeSortie << endl;
	os << "  Recette: " << film.recette << "M$" << endl;

	os << "Acteurs:" << endl;
	for (const shared_ptr<Acteur> acteur : film.acteurs.enSpan())
		os << *acteur;
	return os;
}

void afficherListeFilms(const ListeFilms& listeFilms)
{
	static const string ligneDeSeparation = //[
		"\033[32m────────────────────────────────────────\033[0m\n";
	cout << ligneDeSeparation;
	for (const Film* filmPtr : listeFilms.enSpan()) {
		cout << *filmPtr;
		cout << ligneDeSeparation;
	}
}

/*void afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur)
{
	const Acteur* acteur = listeFilms.trouverActeur(nomActeur);
	if (acteur == nullptr)
		cout << "Aucun acteur de ce nom" << endl;
	else
		afficherListeFilms(acteur->joueDans);
}*/

int main()
{
	#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
	#endif
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	ListeFilms listeFilms = creerListe("films.bin");
	
	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
	// Le premier film de la liste.  Devrait être Alien.
	cout << *listeFilms.enSpan()[0];

	cout << ligneDeSeparation << "Les films sont:" << endl;
	// Affiche la liste des films.  Il devrait y en avoir 7.
	afficherListeFilms(listeFilms);

	listeFilms.trouverActeur("Benedict Cumberbatch")->anneeNaissance = 1976;

	//cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;
	// Affiche la liste des films où Benedict Cumberbatch joue.  Il devrait y avoir Le Hobbit et Le jeu de l'imitation.
	//afficherFilmographieActeur(listeFilms, "Benedict Cumberbatch");

	//Chp 7-8
	cout << ligneDeSeparation << "Chp 7-8:" << endl;
	Film skylien = listeFilms[0];  //ici par reference mais on veut par valeur
	skylien.titre = "Skylien";
	skylien.acteurs.enSpan()[0] = listeFilms[1].acteurs.enSpan()[0];   //ici cest par valeur, mais on veut par reference
	skylien.acteurs[0].nom = "Daniel Wroughton Craig";
	cout << skylien;
	cout << endl;
	cout << listeFilms[0];
	cout << endl;
	cout << listeFilms[1];

	// Détruit et enlève le premier film de la liste (Alien).
	detruireFilm(listeFilms.enSpan()[0]);
	listeFilms.enleverFilm(listeFilms.enSpan()[0]);

	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
	afficherListeFilms(listeFilms);

	//Chp7 test avec ostringstream
	cout << ligneDeSeparation << "Test affichage avec ostringstream:" << endl;
	ostringstream tamponStringStream;
	tamponStringStream << listeFilms[0]; 
	string filmEnString = tamponStringStream.str();
	cout << filmEnString;

	//Chp10
	cout << ligneDeSeparation << "Chp 10:" << endl;
	Film* filmPtr = listeFilms.trouverSi([](Film* film) {return film->recette == 955; });
	if (filmPtr != nullptr) cout << *filmPtr;
	cout << ligneDeSeparation;

	// Pour une couverture avec 0% de lignes non exécutées:
	listeFilms.enleverFilm(nullptr); // Enlever un film qui n'est pas dans la liste (clairement que nullptr n'y est pas).
	//afficherFilmographieActeur(listeFilms, "N'existe pas"); // Afficher les films d'un acteur qui n'existe pas.

	// Détruire tout avant de terminer le programme.
	listeFilms.detruire(true);

	//Chp9
	int nStrings = 2;
	Liste<string> listeTextes = Liste<string>(nStrings);

	int indexDebut = 0;
	int indexUn = 1;
	string str1 = "string 1";
	string str2 = "string 2";
	string str3 = "string 3";

	listeTextes.initialiserElement(str1, indexDebut);
	listeTextes.initialiserElement(str2, indexUn);
	Liste<string> listeTextes2 = listeTextes;

	listeTextes.initialiserElement(str3, indexDebut);
	listeTextes[1] = "string 4";

	cout << ligneDeSeparation << "Affichage des strings (chp9):" << endl;
	cout << "ListeTextes position 0: " << *listeTextes.enSpan()[0].get() << endl; 
	cout << "ListeTextes position 1: " <<  *listeTextes.enSpan()[1].get() << endl;
	cout << "ListeTextes2 position 0: " << *listeTextes2.enSpan()[0].get() << endl;
	cout << "ListeTextes2 position 1: " << *listeTextes2.enSpan()[1].get() << endl;
	
}
