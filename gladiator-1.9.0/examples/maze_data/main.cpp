#include "gladiator.h"
Gladiator* gladiator;
void reset();
void setup() {
    //instanciation de l'objet gladiator
    gladiator = new Gladiator();
    //enregistrement de la fonction de reset qui s'éxecute à chaque fois avant qu'une partie commence
    gladiator->game->onReset(&reset);
}

void reset() {
    //fonction de reset:
    //initialisation de toutes vos variables avant le début d'un match
    gladiator->log("Appel de la fonction de reset");
}

void loop() {
    if(gladiator->game->isStarted()) { //tester si un match à déjà commencer
        //code de votre stratégie
        gladiator->log("Le jeu a commencé");

        // A savoir avant de lire le code ci-sessous
        // - La méthode des graphes est utilisés pour représenter le labyrinthe
        // - Cette représentation est expliquée en détails dans le GFA chapitre 4.1.6
        // - Il faut être à l'aise avec les pointeurs ;)

        //obtenir la case ou se trouve le robot
        MazeSquare nearestSquare = gladiator->maze->getNearestSquare();
        //vérifier s'il a un mur au dessus
        if(nearestSquare.northSquare == NULL ) { 
            //SI le pointeur pointant vers la case nord est nul, cela veut dire qu'il y a un mur qui empêche de passer
            gladiator->log("Il y a un mur au dessus de la case où se trouve le robot");
        }else {
            // Le chemin est ouvert par dessus, obtenons les données de la case
            MazeSquare* topSquare = nearestSquare.northSquare;
            // calculons les coordonnées du centre de cette case
            float squareSize = gladiator->maze->getSquareSize(); //largeur d'une case
            Position centerCoor;
            // pour calculer les coordonnées x et y il faut récupérer les index i et j de la case
            centerCoor.x = (topSquare->i + 0.5) * squareSize;
            centerCoor.y = (topSquare->j + 0.5) * squareSize;

            gladiator->log("Le chemin est ouvert par dessus, les coordonnées du centre de la case du dessus sont : ");
            gladiator->log("( " + String(centerCoor.x) + "; " + String(centerCoor.y) + ")");
        }

        // Il est possible de réccupérer les données d'une case en sachant les index i et j de celle-ci
        unsigned char i = 5, j = 12;
        MazeSquare indexedSquare = gladiator->maze->getSquare(i, j);
        //Obtenir les données de pièce de la case
        Coin coin = indexedSquare.coin;
        //véridifier s'il y a une pièce
        if(coin.value > 0) {
            //Il y'a bien une pièce
            Position posCoin = coin.p;
            gladiator->log("position pièce : ( " + String(posCoin.y)  + ";" + String(posCoin.y) +" )");
        }

        delay(1500);
    }
}
