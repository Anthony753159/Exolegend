#include "trajectory.hpp"

#define WHEEL_TURN_SPEED 0.2
#define WHEEL_FORWARD_SPEED 0.6

#include <math.h>

Trajectory::Trajectory(Gladiator *gladiator) : m_gladiator(gladiator)
{
  m_state = TrajectoryMsg::State::IDLE;
}

Trajectory::~Trajectory()
{
}

void Trajectory::HandleMessage(const TrajectoryMsg &msg)
{
  switch (msg.order)
  {
  case TrajectoryMsg::GOTO:

    break;
  case TrajectoryMsg::SET_STATE:
    m_state = msg.state;
    break;
  }
}

float AngleDiffRad(float from, float to)
{
  float diff = std::fmod(to - from, 2 * M_PI);
  if (diff > M_PI)
  {
    diff -= 2 * M_PI;
  }
  else if (diff < -M_PI)
  {
    diff += 2 * M_PI;
  }
  return diff;
}

bool Trajectory::GOTO(const RobotData &data, const Vector2f &target, float speed)
{
  Vector2f pos = data.position; // position actuelle du robot
  
  // calcul des composantes x et y du vecteur de déplacement
  float dx = target.x - pos.x;
  float dy = target.y - pos.y;
  float distance = sqrt(dx * dx + dy * dy); // distance entre la position actuelle et la cible
  float angle_rad = atan2f(dy, dx); // angle en radians entre la position actuelle et la cible

  // Vérifier si la distance est suffisamment faible et régler la vitesse des roues en conséquence
  if (distance < DISTANCE_THRESHOLD) {
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
    return true;
  }

  // Vérifier si l'angle actuel est proche de l'angle désiré
  float angle_diff = abs(AngleDiffRad(angle_rad, data.position.a));
  if (angle_diff > ANGLE_THRESHOLD) {
    // Si l'angle est trop différent, recalculer l'angle
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, speed);
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -speed);
  }
  else {
    // Si l'angle est proche, effectuer le déplacement
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, speed);
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, speed);
  }

  return false;
}

void Trajectory::Update(const RobotData &data)
{
  switch (m_state)
  {
  case TrajectoryMsg::State::IDLE:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
    break;

  case TrajectoryMsg::State::FORWARD:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_FORWARD_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_FORWARD_SPEED);
    break;

  case TrajectoryMsg::State::BACKWARD:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_FORWARD_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -WHEEL_FORWARD_SPEED);
    break;

  case TrajectoryMsg::State::LEFT:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_TURN_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -WHEEL_TURN_SPEED);
    break;

  case TrajectoryMsg::State::RIGHT:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_TURN_SPEED);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_TURN_SPEED);
    break;
  }
}
