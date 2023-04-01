#include "trajectory.hpp"

#include <algorithm>

#define MAX_ANGLE_DIFF (M_PI / 2)
#define MIN_ANGLE_DIFF (0.01f)

#define WHEEL_TURN_SPEED 0.15f
#define WHEEL_FORWARD_SPEED 0.12f

#define GOTO_BASE_DISTANCE_THRESHOLD 0.06f
#define GOTO_TARGET_DISTANCE_THRESHOLD 0.03f
#define GOTO_DELTA 0.06f

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
    m_goto_angle = msg.goto_angle;
    m_goto_base_reached = false;
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

bool Trajectory::Goto(const RobotData &data)
{
  float dx_base = m_goto_x - data.position.x;
  float dy_base = m_goto_y - data.position.y;
  float dx_target = m_goto_x + GOTO_DELTA * cos(m_goto_angle) - data.position.x;
  float dy_target = m_goto_y + GOTO_DELTA * sin(m_goto_angle) - data.position.y;

  float dist_to_base = sqrt(dx_base * dx_base + dy_base * dy_base);

  float dist_to_target = sqrt(dx_target * dx_target + dy_target * dy_target);
  float angle_to_target = atan2f(dy_target, dx_target);

  /* We reached the base target (center of the cell), or we passed it */
  if (dist_to_base < GOTO_BASE_DISTANCE_THRESHOLD || dist_to_base >= dist_to_target)
  {
    m_goto_base_reached = true;
  }

  /* We reached the target point (after the center of the cell), we go back to idle */
  if (dist_to_target < GOTO_TARGET_DISTANCE_THRESHOLD)
  {
    m_goto_base_reached = true;
    return true;
  }

  /* Checking if we should go backwards instead of forward */
  float angle_diff = AngleDiffRad(angle_to_target, data.position.a);
  bool reverse = false;
  if (Abs(angle_diff) > M_PI / 2)
  {
    angle_diff = AngleDiffRad(angle_to_target + M_PI, data.position.a);
    reverse = true;
  }

  float forward_speed = 0.0f;
  float turn_speed = 0.0f;

  if (Abs(angle_diff) > MAX_ANGLE_DIFF)
  {
    forward_speed = 0.0f;
    turn_speed = WHEEL_TURN_SPEED * (angle_diff > 0 ? 1.0f : -1.0f);
  }
  else if (Abs(angle_diff) < MIN_ANGLE_DIFF)
  {
    forward_speed = WHEEL_FORWARD_SPEED;
    turn_speed = 0.0f;
  }
  else
  {
    forward_speed = WHEEL_FORWARD_SPEED * (1.0f - (Abs(angle_diff) - MIN_ANGLE_DIFF) / (MAX_ANGLE_DIFF - MIN_ANGLE_DIFF));
    turn_speed = WHEEL_TURN_SPEED * (angle_diff > 0 ? 1.0f : -1.0f) * (Abs(angle_diff) - MIN_ANGLE_DIFF) / (MAX_ANGLE_DIFF - MIN_ANGLE_DIFF);
  }

  if (reverse)
  {
    forward_speed = -forward_speed;
  }

  float left_speed = forward_speed + turn_speed;
  float right_speed = forward_speed - turn_speed;
  m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, left_speed);
  m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, right_speed);
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
    if (Goto(data))
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

bool Trajectory::GotoBaseReached() const
{
  return m_goto_base_reached;
}

TrajectoryMsg::State Trajectory::GetState() const
{
  return m_state;
}
