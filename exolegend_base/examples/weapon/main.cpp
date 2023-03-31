#include "gladiator.h"
Gladiator* gladiator;
void reset();
void setup() {
    //instanciation de l'objet gladiator
    gladiator = new Gladiator();
    //enregistrement de la fonction de reset qui s'éxecute à chaque fois avant qu'une partie commence
    gladiator->game->onReset(&reset);

    //initialisation d'une arme en mode SERVO (brancher un servomoteur) sur la pin M1
    gladiator->weapon->initWeapon(WeaponPin::M1, WeaponMode::SERVO);
    //initialisation d'une arme en mode PWM (brancher un moteur avec un transistor) sur la pin M2
    gladiator->weapon->initWeapon(WeaponPin::M2, WeaponMode::PWM);
}

void reset() {
    //fonction de reset:
    //initialisation de toutes vos variables avant le début d'un match
    gladiator->log("Appel de la fonction de reset");

    //on remet le servo moteur à 0°
    gladiator->weapon->setTarget(WeaponPin::M1, 0);
    //on étaint le moteur
    gladiator->weapon->setTarget(WeaponPin::M2, 0);
}

void loop() {
    if(gladiator->game->isStarted()) { //tester si un match à déjà commencer
        //code de votre stratégie
        gladiator->log("Le jeu a commencé");

        //servo moteur à 90°
        gladiator->weapon->setTarget(WeaponPin::M1, 90);
        //moteur à pleine puissance (valeur entre 0 et 255)
        gladiator->weapon->setTarget(WeaponPin::M2, 254);

        delay(1000);

    }
}