
#ifndef USER_UTILS_H
#define USER_UTILS_H

#include <Arduino.h>

struct Position {
  float x{}, y{}, a{};
};

enum class WheelAxis {
  RIGHT, LEFT
};

enum class WeaponPin {
  M1, M2, M3
};
enum class WeaponMode {
  SERVO, PWM
};
enum class RemoteMode {
  ON , OFF
};

struct RobotData {
  Position position {}, cposition {};
  double speedLimit {}, vl {}, vr {};
  short score {};
  uint8_t lifes {}, id {}, teamId {};
  String macAddress{};
  bool remote = false;
  
};



struct Wall{
  bool active;
  Position p1, p2;
};

struct Coin{
  uint8_t value {};
  Position p {};
};

struct Module{
  Wall left , right, top, bottom;
  Coin coin;
  uint8_t value, i, j;
};

struct GladiatorMsg {
  char type {};
  char message[30] = "";
  uint8_t id {};
};

struct MazeSquare{
  uint8_t i {0}, j{0};
  MazeSquare *northSquare {nullptr}, *southSquare {nullptr}, *westSquare {nullptr}, *eastSquare {nullptr};
  Coin coin {};
};

struct RobotList {
  uint8_t ids[4];
};
#endif
