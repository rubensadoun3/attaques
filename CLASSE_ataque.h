#ifndef CLASSE_H
#define CLASSE_H

#define NB_SORTS 4

typedef struct {
    char nom[20]; // Nom de la classe (ex : Guerrier)
    int pv; // Points de vie
    int pa; // Points d'action
    int pm; // Points de mouvement

    // Sorts
    char noms_sorts[NB_SORTS][30]; // Nom de chaque sort
    int portee_min[NB_SORTS]; // Portée minimale
    int portee_max[NB_SORTS]; // Portée maximale
    int cout_PA[NB_SORTS]; // Coût en PA
    int degats_min[NB_SORTS]; // Dégâts minimum (ou soin)
    int degats_max[NB_SORTS]; // Dégâts maximum (ou soin)
    int proba_echec[NB_SORTS]; // Probabilité d'échec (en %)

    int pos;
} Classe;

// Initialise les 4 classes de base dans le tableau passé en paramètre
void initialiser_classes(Classe classes[]);

#endif