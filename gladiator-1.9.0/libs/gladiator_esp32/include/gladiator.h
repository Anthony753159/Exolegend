#ifndef GLADIATOR_H
#define GLADIATOR_H

#include <Arduino.h>
#include "user_utils.h"

/*static constexpr float SQUARE_WIDTH = 0.214; //m
static constexpr float SQUARE_LENGTH = 14; 
static constexpr float ROBOT_RADIUS = 0.06; //m
static constexpr float WHEEL_RADIUS = 0.032; //M
static constexpr float MAZE_SIZE = 3; //m*/

class Maze {
    public:
        MazeSquare getNearestSquare();
        MazeSquare getSquare(uint8_t i, uint8_t j);
        const float getSize();
        const float getSquareSize();
};
class Robot {
    public:
        RobotData getData();
        const float getRobotRadius();
        const float getWheelRadius();
};

class Control {
    public:
        void setWheelSpeed(WheelAxis wheelAxis, float speed);
        void setWheelSpeed(WheelAxis wheelAxis, float speed, bool reset);
        double getWheelSpeed(WheelAxis wheelAxis);
        void setWheelPidCoefs(WheelAxis wheelAxis, float Kp, float Ki, float Kd);

};

class Weapon {
    public:
        void initWeapon(WeaponPin weaponPin, WeaponMode weaponMode);
        void setTarget(WeaponPin weaponPin, uint8_t value);

};
class Game {
    public:

        uint8_t sendOtherRobotMessage(GladiatorMsg message);
        void onReset(void(*resetFunction)());
        void onOtherRobotMessageReceive(void(*receiveFunction)(GladiatorMsg msg));
        bool isStarted();
        void enableFreeMode(RemoteMode enableRemote);
        void enableFreeMode(RemoteMode enableRemote, Position initPosition);
        RobotData getOtherRobotData(uint8_t id);
        RobotList getPlayingRobotsId();
        void setMinotorPin(int newPin);
};
class Gladiator {
    public:
        Gladiator();
        void log(const char* format, ... )  __attribute__ ((format (printf, 2, 3)));
        void saveUserWaypoint(float x, float y);
        Maze* maze;
        Robot* robot;
        Control* control;
        Weapon* weapon;
        Game* game;
};


#endif
