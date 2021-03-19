// TD4 INF1015 hiver 2021
// Par Matheo Benoit-Paraschivoiu et Justin St-Arnaud
#pragma once
// Structures mémoires pour une collection d'items, de films, de livres et de FilmLivre.

#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <cassert>
#include "gsl/span"
using gsl::span;
using namespace std;

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class Item 
{
public:
	Item() = default;
	string titre = "";
	int anneeSortie = 0;
	virtual ~Item() = default;
	virtual void afficher(ostream& os) const 
	{
		os << "Titre: " << titre << endl;
		os << "Annee de sortie: " << anneeSortie << endl;
	}
};

class ListeFilms 
{
public:
	ListeFilms() = default;
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	void detruire(bool possedeLesFilms = false);
	Film*& operator[] (int index) { return elements[index]; }
	Film* trouver(const function<bool(const Film&)>& critere)
	{
		for (auto& film : enSpan())
			if (critere(*film))
				return film;
		return nullptr;
	}

private:
	void changeDimension(int nouvelleCapacite);

	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
};

template <typename T>
class Liste 
{
public:
	Liste() = default;
	explicit Liste(int capaciteInitiale) :  // explicit n'est pas matière à ce TD, mais c'est un cas où c'est bon de l'utiliser, pour ne pas qu'il construise implicitement une Liste à partir d'un entier, par exemple "maListe = 4;".
		capacite_(capaciteInitiale),
		elements_(make_unique<shared_ptr<T>[]>(capacite_))
	{
	}
	Liste(const Liste<T>& autre) :
		capacite_(autre.nElements_),
		nElements_(autre.nElements_),
		elements_(make_unique<shared_ptr<T>[]>(nElements_))
	{
		for (int i = 0; i < nElements_; ++i)
			elements_[i] = autre.elements_[i];
	}
	Liste(Liste<T>&&) = default;  // Pas nécessaire, mais puisque c'est si simple avec unique_ptr...
	Liste<T>& operator= (Liste<T>&&) noexcept = default;  // Utilisé pour l'initialisation dans lireFilm.

	void ajouter(shared_ptr<T> element)
	{
		assert(nElements_ < capacite_);  // Comme dans le TD précédent, on ne demande pas la réallocation pour ListeActeurs...
		elements_[nElements_++] = move(element);
	}

	// Noter que ces accesseurs const permettent de modifier les éléments; on pourrait vouloir des versions const qui retournent des const shared_ptr, et des versions non const qui retournent des shared_ptr.
	shared_ptr<T>& operator[] (int index) const { return elements_[index]; }
	span<shared_ptr<T>> enSpan() const { return span(elements_.get(), nElements_); }

private:
	int capacite_ = 0, nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.
};
using ListeActeurs = Liste<Acteur>;

struct Acteur
{
	string nom; int anneeNaissance = 0; char sexe = '\0';
};

ostream& operator<< (ostream& os, const Acteur& acteur)
{
	return os << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

struct Film : virtual public Item
{
	string realisateur; // nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	int recette=0; // recette globale du film en millions de dollars
	ListeActeurs acteurs;
	void afficher(ostream& os) const override 
	{
		Item::afficher(os);
		os << "Réalisateur: " << realisateur << endl;
		os << "Recette: " << recette << "M$" << endl;
		os << "Acteurs:" << endl;
		for (const shared_ptr<Acteur>& acteur : acteurs.enSpan())
			os << *acteur;
	}
};

struct Livre : virtual public Item
{
	string auteur = "";
	int copiesVendues = 0;
	int nPages = 0;
	void afficher(ostream& os) const override 
	{
		Item::afficher(os);
		os << "Auteur: " << auteur << endl;
		os << "Copies vendues: " << copiesVendues << "M" << endl;
		os << "Nombre de pages: " << nPages <<endl;
	}
};

ostream& operator<< (ostream& os, const Item& item)
{
	item.afficher(os);
	return os;
}

class FilmLivre : public Film, public Livre 
{
public:	
	FilmLivre(Film* film, Livre* livre) : acteurs(film->acteurs) 
	{ 
		titre = film->titre;
		anneeSortie = film->anneeSortie;
		realisateur = film->realisateur;
		recette = film->recette;
		auteur = livre->auteur;
		copiesVendues = livre->copiesVendues;
		nPages = livre->nPages;
	}
	ListeActeurs acteurs;
	
	void afficher(ostream& os) const override
	{
		Item::afficher(os);
		//on ne reutilise pas les fonctions afficher de livre et film pour ne pas avoir en triple le tritre et lannee de sortie
		os << "Réalisateur: " << realisateur << endl;
		os << "Recette: " << recette << "M$" << endl;
		os << "Acteurs:" << endl;
		for (const shared_ptr<Acteur>& acteur : acteurs.enSpan())
			os << *acteur;
		os << "Auteur: " << auteur << endl;
		os << "Copies vendues: " << copiesVendues << "M" << endl;
		os << "Nombre de pages: " << nPages << endl;
	}
};