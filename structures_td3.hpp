/*
Définition des structures et des classes utilisées dans le fichier td3.cpp.
\file   structures_td3.hpp
\author Benoit - Paraschivoiu et St - Arnaud
date    24 fevrier 2021
Créé le 17 fevrier 2021
*/
#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include "gsl/span"
#include <memory>
#include <functional>
#include <iostream>
using gsl::span;
using namespace std;

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms
{
public:
	ListeFilms() = default;
	void ajouterFilm(Film* filmPtr);
	void enleverFilm(const Film* filmPtr);
	shared_ptr<Acteur> trouverActeur(const std::string& nomActeur) const;
	span<Film*> enSpan() const { return span(elements, nElements); }
	int size() const { return nElements; }
	void detruire(bool possedeLesFilms = false);
	Film& operator[](int position) const { return *elements[position]; }
	Film* trouverSi(const function<bool(Film*)>& critere) const {
		int nIteration = 0;
		for (auto&& elements : enSpan()) {
			nIteration++;
			if (critere(elements)) return elements;
			else if (nIteration == nElements) {
				cout << "Aucun element existant";
			}
		}
		return nullptr;
	}
private:
	void changeDimension(int nouvelleCapacite);
	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
};

template<typename T>
class Liste
{
public:
	Liste() = default;
	Liste(int capacite) : capacite_(capacite), elements_(make_unique<shared_ptr<T>[]>(capacite_)) {}
	Liste(const Liste<T>& liste) {
		capacite_ = liste.capacite_;
		nElements_ = liste.nElements_;
		elements_ = make_unique<shared_ptr<T>[]>(capacite_);
		{
			for (int i = 0; i < capacite_; ++i)
				elements_[i] = liste.elements_[i]; }
	}
	span<shared_ptr<T>> enSpan() const { return span(elements_.get(), capacite_); }
	int size() const { return nElements_; }
	void initialiserElement(T& objet, int index) { elements_[index] = make_shared<T>(objet); }
	Liste<T>& operator= (Liste<T>&&) = default;
	T& operator[](int position) const { return *elements_[position]; }
private:
	int capacite_ = 0;
	int nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_;
};

using ListeActeurs = Liste<Acteur>;

struct Point { double x, y; };

struct Film
{
	string titre = "";
	string realisateur = "";// Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	int anneeSortie = 0;
	int recette = 0; // Année de sortie et recette globale du film en millions de dollars
	ListeActeurs acteurs;
};

struct Acteur
{
	string nom = ""; 
	int anneeNaissance = 0; 
	char sexe = ' ';
	//ListeFilms joueDans;
};
