#include "gladiator.h"
Gladiator* gladiator;
void reset();
void setup() {
    //instanciation de l'objet gladiator
    gladiator = new Gladiator();
    //enregistrement de la fonction de reset qui s'éxecute à chaque fois avant qu'une partie commence
    gladiator->game->onReset(&reset);
    //positionner le robot aux coordonnées de la première base (0.1; 1.6)
    Position initialPosition = {0.1, 1.6, 0};
    gladiator->game->enableFreeMode(RemoteMode::OFF,initialPosition); 
    //lorsque le robot commencera en free mode sa position de départ sera la valeur de initalPosition
    //Seuls les encodeurs seront utilisés pour calculer la position du robot (elle sera moins précise que celle lorsque le robot est connecté à l'arène)
    //IL est toujours possible de connecter l'outil de debugage minotor au robot lorque celui ci est en free mode
    //Le free mode n'est pas utilisable en simulation
}
 
void reset() {
    //fonction de reset:
    //initialisation de toutes vos variables avant le début d'un match
}

void loop() {
    if(gladiator->game->isStarted()) { // Cette fonction est inutile en free mode
        //code de votre stratégie
        //Voici les fonctions utilisables dans ce mode :
        // - Toutes les fonctions de control (contrôler le robot, voir GFA chapitre 4.2)
        // - Toutes les fonctions de robot (réccupérer les données du robot, voir GFA chapitre 4.6)
        // - Les fonction de debugs (chapitre 4.5 du GFA)
        // - Toutes les fonctions de Weapon (Contrôler l'arme du robot, voir GFA chapitre 4.8)
    }
}