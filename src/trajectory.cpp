#include "trajectory.hpp"

#include <algorithm>

#define WHEEL_TURN_SPEED 0.05
#define WHEEL_FORWARD_SPEED 0.2

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

float Abs(float a)
{
  return a < 0 ? -a : a;
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
