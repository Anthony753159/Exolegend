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

        //obtenir la liste des robots jouant actuellement sur sur le terrain
        RobotList robotList = gladiator->game->getPlayingRobotsId();
        //IL y'a 4 robots sur le terrain, on réccupère l'id du troisième robot (par exemple)
        unsigned char id = robotList.ids[2];

        //Obtenir les données du robot
        RobotData data = gladiator->game->getOtherRobotData(id);

        // position du robot (filtré)
        // ----------------------->
        //Position position = data.position; // cette fonction ne renvoie pas de position filtrée 
        // La position n'est pas flitrée pour les autre robot, elle est donc en retard et moins précise.


        // position du robot (brut)
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

        delay(500);
    }


    
}