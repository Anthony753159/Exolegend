#include "trajectory.hpp"

#include <algorithm>

#define WHEEL_TURN_SPEED 0.05
#define WHEEL_FORWARD_SPEED 0.4

#define GOTO_DISTANCE_THRESHOLD 0.03
#define GOTO_ANGLE_THRESHOLD 0.1

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
  case TrajectoryMsg::ORDER_GOTO:
    m_goto_x = msg.goto_x;
    m_goto_y = msg.goto_y;
    m_state = TrajectoryMsg::State::GOTO;
    break;
  case TrajectoryMsg::ORDER_SET_STATE:
    m_state = msg.state;
    break;
  case TrajectoryMsg::ORDER_ROTATE:
    m_target_angle = msg.angle;
    m_state = TrajectoryMsg::State::ROTATE;
    break;
  case TrajectoryMsg::ORDER_MOVE_DISTANCE:
    m_distance_remaining = msg.distance;
    m_distance_forward = msg.forward;
    m_start_x = -10;
    m_start_y = -10;
    m_state = TrajectoryMsg::State::MOVE_DISTANCE;
    break;
  default:
    break;
  }
}

float AngleDiffRad(float from, float to)
{
  return atan2(sin(to - from), cos(to - from));
}

float Abs(float a)
{
  return a < 0 ? -a : a;
}

bool Trajectory::Goto(const RobotData &data, const Vec2f &target, float speed)
{
  // calcul des composantes x et y du vecteur de déplacement
  float dx = target.x - data.position.x;
  float dy = target.y - data.position.y;
  float distance = sqrt(dx * dx + dy * dy); // distance entre la position actuelle et la cible
  float angle_rad = atan2f(dy, dx);         // angle en radians entre la position actuelle et la cible

  // Vérifier si la distance est suffisamment faible et régler la vitesse des roues en conséquence
  if (distance < GOTO_DISTANCE_THRESHOLD)
  {
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
    return true;
  }

  // Vérifier si l'angle actuel est proche de l'angle désiré
  float angle_diff = AngleDiffRad(angle_rad, data.position.a);
  bool reverse = false;
  if (Abs(angle_diff) > M_PI / 2)
  {
    angle_diff = AngleDiffRad(angle_rad + M_PI, data.position.a);
    reverse = true;
  }

  if (Abs(angle_diff) > GOTO_ANGLE_THRESHOLD)
  {
    // Si l'angle est trop différent, recalculer l'angle
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_TURN_SPEED * (angle_diff > 0 ? 1 : -1));
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_TURN_SPEED * (angle_diff > 0 ? 1 : -1));
  }
  else
  {
    // Si l'angle est proche, effectuer le déplacement
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_FORWARD_SPEED * (reverse ? -1 : 1));
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_FORWARD_SPEED * (reverse ? -1 : 1));
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

  case TrajectoryMsg::State::GOTO:
  {
    Vec2f target{m_goto_x, m_goto_y};
    if (Goto(data, target, WHEEL_FORWARD_SPEED))
    {
      m_state = TrajectoryMsg::State::IDLE;
    }
    break;
  }

  case TrajectoryMsg::State::ROTATE:
  {
    float angle_diff = AngleDiffRad(data.position.a, m_target_angle);
    if (Abs(angle_diff) < 0.1)
    {
      m_state = TrajectoryMsg::State::IDLE;
      m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
      m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
    }
    else
    {
      if (angle_diff > 0)
      {
        m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_TURN_SPEED);
        m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -WHEEL_TURN_SPEED);
      }
      else
      {
        m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_TURN_SPEED);
        m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_TURN_SPEED);
      }
    }
  }
  break;

  case TrajectoryMsg::State::MOVE_DISTANCE:
  {
    if (m_start_x < 0)
    {
      m_start_x = data.position.x;
      m_start_y = data.position.y;
    }

    float travelled_dist = std::sqrt(std::pow(data.position.x - m_start_x, 2) + std::pow(data.position.y - m_start_y, 2));

    if (travelled_dist >= m_distance_remaining)
    {
      m_state = TrajectoryMsg::State::IDLE;
      m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
      m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
    }
    else
    {
      m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_FORWARD_SPEED * (m_distance_forward ? 1 : -1));
      m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_FORWARD_SPEED * (m_distance_forward ? 1 : -1));
    }
  }
  break;
  }
}

TrajectoryMsg::State Trajectory::GetState() const
{
  return m_state;
}
