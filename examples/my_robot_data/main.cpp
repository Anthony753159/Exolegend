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
    if(gladiator->game->isStarted()) { //tester si un match à déjà commencé
        //code de votre stratégie
        gladiator->log("Le jeu a commencé");

        //Obtenir les données du robot
        RobotData data = gladiator->robot->getData();
        // position du robot (filtré)
        Position position = data.position;
        // position du robot (brut venant de la caméra)
        Position cposition = data.cposition;
        // vitesse du robot
        double vl = data.vl; // vitesse roue gauche
        double vr = data.vr; // vitesse roue droite
        //vie du robot
        unsigned char lifes = data.lifes;
        //limite en vitesse
        double speedLimit = data.speedLimit;
        //Id du robto 
        unsigned char robotId = data.id;
        //score du robot
        unsigned char score = data.score;
        //address du robot
        String macAddress = data.macAddress;


        //obtenir les constante du robot
        //Demi diamètre du robot en m (demi entreaxe)
        float robotRadius = gladiator->robot->getRobotRadius();
        //Rayon de la roue du robot en m
        float WheelRadius = gladiator->robot->getWheelRadius();


        delay(500);

        
    }
}