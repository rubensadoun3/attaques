#ifndef ATTAQUES_H
#define ATTAQUES_H
#include <stdbool.h>
#include <allegro.h>
#include "CLASSE_ataque.h"

#define NB_SORTS 6

typedef struct {
    char nom[30];
    int element;      // 0=neutre 1=feu 2=eau 3=terre 4=air
    int portee_min;
    int portee_max;
    int zone;         // 0=cible unique, 1=cercle 1, 2=cercle 2, etc.
    int cout_pa;
    int degats_min;
    int degats_max;
    int cooldown;
    int proba_echec;
    BITMAP *icone;
} Sort;

void init_sorts(Sort sorts[]);

typedef struct {
    bool a_une_arme;
    int degats_arme_min;
    int degats_arme_max;
    int bonus_force;
    int malus_faiblesse;
    bool attaque_speciale;
} Equipement;

typedef struct {
    int resistance_physique;
    bool est_boss;
    int pos_x;
    int pos_y;
} CaracteristiquesEnnemi;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int dernier_tour_utilise;
    int lancers_restants_ce_tour;
} EtatSort;

typedef struct {
    Classe* classe; // Référence à la classe et aux sorts de base
    Position pos;
    EtatSort etats_sorts[NB_SORTS];
    bool est_stun;
    bool est_silence;
    bool a_lance_un_sort_ce_tour;
} Joueur;



#endif //ATTAQUES_H
