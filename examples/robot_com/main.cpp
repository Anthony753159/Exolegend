#include "gladiator.h"
Gladiator* gladiator;
void reset();
void receiveMessage(GladiatorMsg msg);
void setup() {
    //instanciation de l'objet gladiator
    gladiator = new Gladiator();
    //enregistrement de la fonction de reset qui s'éxecute à chaque fois avant qu'une partie commence
    gladiator->game->onReset(&reset);

    //ON enregistre la fonction de callback qui s'éxecute dès que le robot reçoit un message d'un autre robot
    gladiator->game->onOtherRobotMessageReceive(&receiveMessage);
}
void receiveMessage(GladiatorMsg msg) {
    //Réception d'un nouveau message d'un robot sur le terrain
    gladiator->log("Message reçu du robot avec l'id " + String(msg.id) + " : ");
    gladiator->log(msg.message);
}
void reset() {
    //Fonction de reset:
    //Initialisation de toutes vos variables avant le début d'un match
    gladiator->log("Appel de la fonction de reset");

    //Obtenir la liste des robots jouant actuellement sur sur le terrain
    RobotList robotList = gladiator->game->getPlayingRobotsId();
    //Il y'a 4 robots sur le terrain, on réccupère l'id du premier robot (par exemple)
    unsigned char robotId = robotList.ids[0];

    //Création d'un nouveau messsage
    GladiatorMsg message;
    strcpy(message.message, "coucou");
    message.id = robotId; //on envoie le message "coucou" au robot avec l'id robotId

    //On envoie le message
    //Attention le temps minimal entre deux message envoyé doit être de 5s
    unsigned char status = gladiator->game->sendOtherRobotMessage(message);

    switch(status) {
        case 0:
            gladiator->log("Message envoyé avec succès");
            break;
        case 1:
            gladiator->log("Erreur! Impossible d'envoyer le message à soit même");
            break;
        case 2:
            gladiator->log("Erreur! Le robot tente d'envoyer un message à un robot qui ne joue pas actuellement sur le terrain.");
            break;
        case 3:
            gladiator->log("Erreur! Temps minimal d'envoie de 5s non respecté");
            break;
    }
    delay(500);

}

void loop() {
    if(gladiator->game->isStarted()) { //tester si un match à déjà commencer
        //code de votre stratégie
        gladiator->log("Le jeu a commencé");
    }
}