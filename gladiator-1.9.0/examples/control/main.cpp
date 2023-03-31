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
}

void loop() {
    if(gladiator->game->isStarted()) { //tester si un match à déjà commencer
        //code de votre stratégie :
        //appliquer une vitesse de 0.6m/s au deux roue
        gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0.6); //controle de la roue droite
        gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0.6); //control de la roue gauche
        //Lorsque le jeu commencera le robot ira en ligne droite
        delay(100);
    }
    //La consigne en vitesse est forcée à 0 lorsque aucun match n'a débuté.
}