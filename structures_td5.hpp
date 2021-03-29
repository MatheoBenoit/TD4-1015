﻿#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <cassert>
#include "gsl/span"
using gsl::span;
using namespace std;  // On le permet, mais j'ai écrit mon .hpp sans, avant de le permettre dans l'énoncé.

class Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
	ListeFilms() = default;
	void ajouterFilm(Film* film);
	//void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	int capacity() const { return capacite; }
	void detruire();
	Film*& operator[] (int index) { return elements[index]; }
	Film* trouver(const function<bool(const Film&)>& critere) {
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

struct Acteur
{
	string nom; int anneeNaissance = 0; char sexe = '\0';
};


class Node {
public:
	Node(const Acteur& acteur)
	{
		acteur_ = acteur;
	}
//private:
	Node* next_ = past_end;
	//Node* previous_ = past_end;
	Acteur acteur_;
	inline static constexpr Node* past_end = nullptr;
	/*friend class Liste<T>;
	friend class Iterator;*/
};

class Iterator {
public:
	Iterator(Node* position = Node::past_end) 
	{
		position_ = position;
	}
	Acteur& operator* () {
		Expects(position_ != Node::past_end);
		return position_->acteur_;
	}
	Iterator& operator++ () {
		Expects(position_ != Node::past_end);
		position_ = position_->next_;
		return *this;
	}
	bool operator== (const Iterator& b) const = default;
	bool operator!= (const Iterator& b) const = default;

	Node* position_;
	//friend class Liste<Acteur>;
};

template <typename T>
class Liste {
public:
	Liste() = default;
	Liste(int capaciteInitiale) :
		capacite(capaciteInitiale),
		elements(make_unique<shared_ptr<T>[]>(capacite))
	{
	}
	Liste(const Liste<T>& autre) :
		capacite(autre.nElements),
		nElements(autre.nElements),
		elements(make_unique<shared_ptr<T>[]>(nElements))
	{
		for (int i = 0; i < nElements; ++i)
			elements[i] = autre.elements[i];
	}
	Liste(Liste<T>&&) = default;  // Pas nécessaire, mais puisque c'est si simpler avec unique_ptr...
	Liste<T>& operator=(Liste<T>&&) = default;  // Nécessaire dans mon cas pour l'initialisation dans lireFilm.

	void ajouter(shared_ptr<T> element)
	{
		assert(nElements < capacite);  // Comme dans le TD1, on ne demande pas la réallocation pour ListeActeurs...
		elements[nElements++] = move(element);
	}

	shared_ptr<T>& operator[] (int index) { return elements[index]; }
	span<shared_ptr<T>> enSpan() const { return span(elements.get(), nElements); }

	Iterator begin() {
		return Iterator(first_);
	}

	Iterator end() {
		return Iterator(Node::past_end);
	}

private:
	int capacite = 0, nElements = 0;
	unique_ptr<shared_ptr<T>[]> elements;
	Node* first_ = Node::past_end;
	Node* last_ = Node::past_end;
};

using ListeActeurs = Liste<Acteur>;

class Affichable {
public:
	virtual void afficherSur(ostream& os) const = 0;
	virtual ~Affichable() = default;
};

class Item : public Affichable {
public:
	void afficherSur(ostream& os) const override;
	void lireDe(istream& is);

	string titre;
	int anneeSortie = 0;
};

class Film : virtual public Item
{
public:
	void afficherSur(ostream& os) const override;
	void afficherSpecifiqueSur(ostream& os) const;  // Affiche la parite de cette classe sans afficher la base virtuelle.

	string realisateur; // (on suppose qu'il n'y a qu'un réalisateur).
	int recette = 0; // Recette globale du film en millions de dollars
	ListeActeurs acteurs;
};

class Livre : virtual public Item
{
public:
	Livre() = default;
	explicit Livre(istream& is);
	void afficherSur(ostream& os) const override;
	void afficherSpecifiqueSur(ostream& os) const;
	void lireDe(istream& is);

	string auteur;
	int copiesVendues=0, nPages=0;
};

class FilmLivre : public Film, public Livre {
public:
	FilmLivre(const Film& film, const Livre& livre) : Item(film), Film(film), Livre(livre) { }

	void afficherSur(ostream& os) const override;
};
