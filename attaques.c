#include <allegro.h>
#include "attaques.h"
#include <string.h>
#include "CLASSE_ataque.h"
#include <stdbool.h>
#include <math.h>
#define ATTAQUE_CORPS_A_CORPS 0
#define ATTAQUE_SORT 1



bool attaque_corps_a_corps(Classe* joueur, Classe* ennemi, Equipement* equip, CaracteristiquesEnnemi* ennemi_carac, int joueur_x, int joueur_y) {
    // 1. Vérification adjacence
    int dx = abs(joueur_x - ennemi_carac->pos_x);
    int dy = abs(joueur_y - ennemi_carac->pos_y);
    if (!(dx <= 1 && dy <= 1 && (dx + dy) > 0)) return false;

    // 2. Vérification PA
    if (joueur->pa < 2) return false;
    joueur->pa -= 2;

    // 3. Calcul dégâts de base
    int degats_min = equip->a_une_arme ? equip->degats_arme_min : 1;
    int degats_max = equip->a_une_arme ? equip->degats_arme_max : 3;

    // 4. Application modificateurs
    degats_min += equip->bonus_force - equip->malus_faiblesse;
    degats_max += equip->bonus_force - equip->malus_faiblesse;
    degats_min = degats_min < 1 ? 1 : degats_min;
    degats_max = degats_max < degats_min ? degats_min : degats_max;

    // 5. Gestion résistance
    if (ennemi_carac->resistance_physique > 0) {
        int reduction = ennemi_carac->resistance_physique * 2;
        degats_min = degats_min * (100 - reduction) / 100;
        degats_max = degats_max * (100 - reduction) / 100;
        degats_min = degats_min < 1 ? 1 : degats_min;
        degats_max = degats_max < degats_min ? degats_min : degats_max;
    }

    // 6. Attaque spéciale
    if (equip->attaque_speciale) {
        degats_min = 2;
        degats_max = 5;
    }

    // 7. Calcul final et échec
    if ((rand() % 10) != 0) { // 90% de succès
        int degats = (rand() % (degats_max - degats_min + 1)) + degats_min;
        ennemi->pv -= degats;
        if (ennemi->pv < 0) ennemi->pv = 0;
    }

    return true;
}
int calculer_distance(Position a, Position b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}
void reinitialiser_lancers_par_tour(Joueur* joueur, int nb_lancers_par_sort) {
    for (int i = 0; i < NB_SORTS; i++) {
        joueur->etats_sorts[i].lancers_restants_ce_tour = nb_lancers_par_sort;
    }
}
void initialiser_joueur(Joueur* joueur) {
    // ...
    for (int i = 0; i < NB_SORTS; i++) {
        joueur->etats_sorts[i].lancers_restants_ce_tour = 1; // 1 lancer par défaut
        joueur->etats_sorts[i].dernier_tour_utilise = -3; // Prêt à l'emploi
    }
}
void nouveau_tour(Joueur* joueurs, int nb_joueurs, int tour_actuel) {
    for (int i = 0; i < nb_joueurs; i++) {
        reinitialiser_lancers_par_tour(&joueurs[i], 1); // Réinitialise les lancers
        joueurs[i].a_lance_un_sort_ce_tour = false; // Réinitialise le flag
    }
}
void appliquer_sort_zone(Joueur* lanceur, Position centre, Sort sort, int rayon, bool affecte_ennemis) {
    for (int x = centre.x - rayon; x <= centre.x + rayon; x++) {
        for (int y = centre.y - rayon; y <= centre.y + rayon; y++) {
            if (calculer_distance(centre, (Position){x,y}) <= rayon) {
                // Logique d'application sur les cases adjacentes
            }
        }
    }
}
bool attaque_par_sort(Joueur* lanceur, Classe* cible, CaracteristiquesEnnemi* ennemi_carac, Sort sort, int sort_index, bool est_ennemi, int tour_actuel) {
    if (lanceur->est_stun || lanceur->est_silence) return false;
    if (lanceur->a_lance_un_sort_ce_tour) return false;
    if (lanceur->classe->pa < sort.cout_pa) return false;

    Position pos_cible = {ennemi_carac->pos_x, ennemi_carac->pos_y};
    int distance = calculer_distance(lanceur->pos, pos_cible);

    if (distance < sort.portee_min || distance > sort.portee_max) return false;

    EtatSort* etat = &lanceur->etats_sorts[sort_index];
    if (tour_actuel - etat->dernier_tour_utilise < sort.cooldown) return false;
    if (etat->lancers_restants_ce_tour <= 0) return false;

    lanceur->classe->pa -= sort.cout_pa;
    lanceur->a_lance_un_sort_ce_tour = true;
    etat->lancers_restants_ce_tour--;
    etat->dernier_tour_utilise = tour_actuel;

    if ((rand() % 100) < sort.proba_echec) return true;

    int effet = (sort.degats_min == sort.degats_max)
        ? sort.degats_min
        : (rand() % (sort.degats_max - sort.degats_min + 1)) + sort.degats_min;

    if (effet > 0 && est_ennemi) {
        cible->pv = MAX(0, cible->pv - effet);
    } else if (effet < 0 && !est_ennemi) {
        cible->pv -= effet;
    }

    return true;
}

bool effectuer_attaque(Joueur* attaquant, Classe* cible, CaracteristiquesEnnemi* ennemi_carac, Equipement* equip, int type_attaque, int sort_index, int tour_actuel) {
    // Déterminer si la cible est un ennemi (true) ou un allié (false)
    bool est_ennemi = (cible != attaquant->classe);

    // Vérifier si l'attaquant peut agir
    if (attaquant->est_stun) return false;

    // Choisir le type d'attaque
    switch (type_attaque) {
        case ATTAQUE_CORPS_A_CORPS:
            return attaque_corps_a_corps(attaquant->classe, cible, equip, ennemi_carac,
                                      attaquant->pos.x, attaquant->pos.y);

        case ATTAQUE_SORT: {
            // Création d'un sort temporaire à partir des données de la classe
            Sort sort;
            strcpy(sort.nom, attaquant->classe->noms_sorts[sort_index]);
            sort.portee_min = attaquant->classe->portee_min[sort_index];
            sort.portee_max = attaquant->classe->portee_max[sort_index];
            sort.cout_pa = attaquant->classe->cout_PA[sort_index];
            sort.degats_min = attaquant->classe->degats_min[sort_index];
            sort.degats_max = attaquant->classe->degats_max[sort_index];
            sort.cooldown = 3; // Valeur par défaut
            sort.proba_echec = attaquant->classe->proba_echec[sort_index];

            return attaque_par_sort(attaquant, cible, ennemi_carac,
                                 sort, sort_index, est_ennemi, tour_actuel);
        }

        default:
            return false;
    }
}
/*bool attaque_par_sort(Classe classe, Classe* ennemi, Equipement* equip, CaracteristiquesEnnemi* ennemi_carac, int joueur_x, int joueur_y, Sort sort, EtatSort etat_du_sort, Joueur joueur ) {
    int dx = abs(joueur_x - ennemi_carac->pos_x);
    int dy = abs(joueur_y - ennemi_carac->pos_y);
    int tours=0;
    int lancers=10;
    joueur.est_stun = false;
    joueur.est_silence = false;
    joueur.a_lance_un_sort_ce_tour = false;
    if (dx==dx-1 || dx ==dx+1 || dy==dy-1 || dy==dy+1 ) return false;
    if (classe.pa< sort. cout_pa )return false;
    int distance =calculer_distance(classe .pos , ennemi->pos);
    if (distance <sort.portee_min || distance > sort.portee_max) return false;
    while(tours>= 3){
     if (tours - etat_du_sort. dernier_tour_utilise>= sort. cooldown){
         lancers--;
         etat_du_sort.lancers_restants_ce_tour --;
     }
    }
    int lancers_executes=lancers -1;
    while (lancers>0) {
        if (lancers_executes) {
            etat_du_sort.lancers_restants_ce_tour=etat_du_sort.lancers_restants_ce_tour-1;
        }
    }
    // 1. Le joueur ne doit pas être stun ou silence
    if (joueur.est_stun || joueur.est_silence) return false;


    // 3. Si un sort a déjà été lancé ce tour (selon ta logique de jeu)
    if (joueur.a_lance_un_sort_ce_tour) return false;

    // 4. Vérifie zone d’effet si besoin (si ce sort est de zone)
    if (!est_dans_zone(classe.pos, ennemi->pos, sort.portee_max)) return false;



    return true;
}
*/
void initialiser_classes(Classe classes[]) {
    // ======================
    // === GUERRIER ===
    // ======================
    strcpy(classes[0].nom, "Guerrier");
    classes[0].pv = 120;  // PV très élevés
    classes[0].pa = 6;     // PA standards
    classes[0].pm = 3;     // PM limités

    // Sort 1 : Coup de Hache (Dégâts physiques)
    strcpy(classes[0].noms_sorts[0], "Coup de Hache");
    classes[0].portee_min[0] = 1;
    classes[0].portee_max[0] = 1;
    classes[0].cout_PA[0] = 4;
    classes[0].degats_min[0] = 15;  // Dégâts élevés
    classes[0].degats_max[0] = 25;
    classes[0].proba_echec[0] = 10;

    // Sort 2 : Tempête de Lames (Zone)
    strcpy(classes[0].noms_sorts[1], "Tempête de Lames");
    classes[0].portee_min[1] = 1;
    classes[0].portee_max[1] = 1;
    classes[0].cout_PA[1] = 6;
    classes[0].degats_min[1] = 10;  // Dégâts en zone
    classes[0].degats_max[1] = 18;
    classes[0].proba_echec[1] = 15;

    // Sort 3 : Cri de Défi (Buff/Débuff)
    strcpy(classes[0].noms_sorts[2], "Cri de Défi");
    classes[0].portee_min[2] = 0;
    classes[0].portee_max[2] = 3;
    classes[0].cout_PA[2] = 5;
    classes[0].degats_min[2] = -15; // Boost alliés
    classes[0].degats_max[2] = -15;
    classes[0].proba_echec[2] = 5;

    // Sort 4 : Fureur Berserk (Ultime)
    strcpy(classes[0].noms_sorts[3], "Fureur Berserk");
    classes[0].portee_min[3] = 1;
    classes[0].portee_max[3] = 2;
    classes[0].cout_PA[3] = 7;
    classes[0].degats_min[3] = 25;  // Dégâts massifs
    classes[0].degats_max[3] = 40;
    classes[0].proba_echec[3] = 30;

    // ======================
    // === MAGE ===
    // ======================
    strcpy(classes[1].nom, "Mage");
    classes[1].pv = 80;   // PV faibles
    classes[1].pa = 7;     // PA élevés
    classes[1].pm = 4;     // Bonne mobilité

    // Sort 1 : Rayon Arcanique (Base)
    strcpy(classes[1].noms_sorts[0], "Rayon Arcanique");
    classes[1].portee_min[0] = 3;
    classes[1].portee_max[0] = 7;
    classes[1].cout_PA[0] = 4;
    classes[1].degats_min[0] = 12;  // Dégâts stables
    classes[1].degats_max[0] = 20;
    classes[1].proba_echec[0] = 15;

    // Sort 2 : Nova de Givre (Contrôle)
    strcpy(classes[1].noms_sorts[1], "Nova de Givre");
    classes[1].portee_min[1] = 1;
    classes[1].portee_max[1] = 4;
    classes[1].cout_PA[1] = 6;
    classes[1].degats_min[1] = 8;   // Dégâts + gel
    classes[1].degats_max[1] = 14;
    classes[1].proba_echec[1] = 20;

    // Sort 3 : Éruption Tellurique (Zone)
    strcpy(classes[1].noms_sorts[2], "Éruption Tellurique");
    classes[1].portee_min[2] = 4;
    classes[1].portee_max[2] = 6;
    classes[1].cout_PA[2] = 7;
    classes[1].degats_min[2] = 15;  // Gros dégâts zone
    classes[1].degats_max[2] = 25;
    classes[1].proba_echec[2] = 25;

    // Sort 4 : Transfert Vital (Support)
    strcpy(classes[1].noms_sorts[3], "Transfert Vital");
    classes[1].portee_min[3] = 1;
    classes[1].portee_max[3] = 5;
    classes[1].cout_PA[3] = 5;
    classes[1].degats_min[3] = -30; // Soin important
    classes[1].degats_max[3] = -40;
    classes[1].proba_echec[3] = 10;

    // ======================
    // === ARCHER ===
    // ======================
    strcpy(classes[2].nom, "Archer");
    classes[2].pv = 90;   // PV moyens
    classes[2].pa = 8;     // PA très élevés
    classes[2].pm = 5;     // Mobilité excellente

    // Sort 1 : Flèche Empennée (Base)
    strcpy(classes[2].noms_sorts[0], "Flèche Empennée");
    classes[2].portee_min[0] = 2;
    classes[2].portee_max[0] = 8;   // Très longue portée
    classes[2].cout_PA[0] = 3;
    classes[2].degats_min[0] = 10;  // Dégâts précis
    classes[2].degats_max[0] = 18;
    classes[2].proba_echec[0] = 10;

    // Sort 2 : Pluie de Flèches (Zone)
    strcpy(classes[2].noms_sorts[1], "Pluie de Flèches");
    classes[2].portee_min[1] = 3;
    classes[2].portee_max[1] = 6;
    classes[2].cout_PA[1] = 6;
    classes[2].degats_min[1] = 8;   // Multi-cibles
    classes[2].degats_max[1] = 15;
    classes[2].proba_echec[1] = 20;

    // Sort 3 : Piège Venimeux (DOT)
    strcpy(classes[2].noms_sorts[2], "Piège Venimeux");
    classes[2].portee_min[2] = 1;
    classes[2].portee_max[2] = 3;
    classes[2].cout_PA[2] = 4;
    classes[2].degats_min[2] = 5;   // Dégâts initiaux
    classes[2].degats_max[2] = 8;    // + poison ensuite
    classes[2].proba_echec[2] = 15;

    // Sort 4 : Tir Perforant (Perce armure)
    strcpy(classes[2].noms_sorts[3], "Tir Perforant");
    classes[2].portee_min[3] = 2;
    classes[2].portee_max[3] = 5;
    classes[2].cout_PA[3] = 7;
    classes[2].degats_min[3] = 20;  // Ignore partie défense
    classes[2].degats_max[3] = 35;
    classes[2].proba_echec[3] = 25;

    // ======================
    // === SOIGNEUR ===
    // ======================
    strcpy(classes[3].nom, "Soigneur");
    classes[3].pv = 100;  // PV élevés
    classes[3].pa = 6;     // PA standards
    classes[3].pm = 3;     // Mobilité limitée

    // Sort 1 : Soin Lumineux (Base)
    strcpy(classes[3].noms_sorts[0], "Soin Lumineux");
    classes[3].portee_min[0] = 1;
    classes[3].portee_max[0] = 4;
    classes[3].cout_PA[0] = 4;
    classes[3].degats_min[0] = -20; // Soin moyen
    classes[3].degats_max[0] = -30;
    classes[3].proba_echec[0] = 5;

    // Sort 2 : Purification (Débuff)
    strcpy(classes[3].noms_sorts[1], "Purification");
    classes[3].portee_min[1] = 1;
    classes[3].portee_max[1] = 3;
    classes[3].cout_PA[1] = 3;
    classes[3].degats_min[1] = 0;   // Nettoie effets
    classes[3].degats_max[1] = 0;
    classes[3].proba_echec[1] = 10;

    // Sort 3 : Barrière Sacrée (Protection)
    strcpy(classes[3].noms_sorts[2], "Barrière Sacrée");
    classes[3].portee_min[2] = 1;
    classes[3].portee_max[2] = 2;
    classes[3].cout_PA[2] = 5;
    classes[3].degats_min[2] = -30; // Bouclier puissant
    classes[3].degats_max[2] = -30;
    classes[3].proba_echec[2] = 0;

    // Sort 4 : Résurrection (Ultime)
    strcpy(classes[3].noms_sorts[3], "Résurrection");
    classes[3].portee_min[3] = 1;
    classes[3].portee_max[3] = 1;
    classes[3].cout_PA[3] = 10;     // Coût très élevé
    classes[3].degats_min[3] = -999; // Soin complet
    classes[3].degats_max[3] = -999;
    classes[3].proba_echec[3] = 40;
}