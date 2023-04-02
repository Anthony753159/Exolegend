#include "trajectory.hpp"

#include <algorithm>

#define FORCE_REVERSE true
#define POSITION_SHIFT 0.02f
#define MAX_ANGLE_DIFF (M_PI / 2)

#define WHEEL_TURN_SPEED_FORWARD 0.5f
#define WHEEL_TURN_SPEED_BACKWARD 0.5f
#define WHEEL_FORWARD_SPEED 0.5f
#define WHEEL_BACKWARD_SPEED 0.35f

#define GOTO_BASE_DISTANCE_THRESHOLD 0.06f
#define GOTO_DELTA -0.03f

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
    m_goto_reverse = msg.goto_reverse;
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
  /* Orthogonal distance to target line */

  Vec2f pos = Vec2f{data.position.x, data.position.y};
  pos.x -= cosf(data.position.a) * POSITION_SHIFT;
  pos.y -= sinf(data.position.a) * POSITION_SHIFT;

  Vec2f p1 = Vec2f{m_goto_x, m_goto_y};
  Vec2f p2 = Vec2f{m_goto_x + cosf(m_goto_angle), m_goto_y + sinf(m_goto_angle)};
  float dist_to_line = Abs((p2.x - p1.x) * (p1.y - pos.y) - (p1.x - pos.x) * (p2.y - p1.y)) / sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));

  float angle_line_to_robot = atan2f(pos.y - p1.y, pos.x - p1.x);
  float diff_angle_line_to_robot = AngleDiffRad(m_goto_angle, angle_line_to_robot);

  float target_angle = m_goto_angle + atan(dist_to_line * 7.0f) * (diff_angle_line_to_robot > 0 ? -1.0f : 1.0f);

  float dx_base = m_goto_x - pos.x;
  float dy_base = m_goto_y - pos.y;
  float dx_target = m_goto_x + GOTO_DELTA * cos(m_goto_angle) - pos.x;
  float dy_target = m_goto_y + GOTO_DELTA * sin(m_goto_angle) - pos.y;

  float dist_to_base = sqrt(dx_base * dx_base + dy_base * dy_base);

  float angle_to_target = atan2f(dy_target, dx_target);
  float diff_angle_direction = AngleDiffRad(angle_to_target, m_goto_angle);

  bool goto_over = Abs(diff_angle_direction) > M_PI / 2;

  /* We reached the base target (center of the cell), or we passed it */
  if (dist_to_base < GOTO_BASE_DISTANCE_THRESHOLD)
  {
    m_goto_base_reached = true;
  }

  /* Checking if we should go backwards instead of forward */
  float angle_diff = AngleDiffRad(target_angle, data.position.a);

  if (FORCE_REVERSE)
  {
    if (m_goto_reverse)
    {
      angle_diff = AngleDiffRad(target_angle + M_PI, data.position.a);
    }
  }
  else
  {
    m_goto_reverse = false;
    if (Abs(angle_diff) > M_PI / 2)
    {
      m_goto_reverse = true;
      angle_diff = AngleDiffRad(target_angle + M_PI, data.position.a);
    }
  }

  float forward_speed = 0.0f;
  float turn_speed = 0.0f;
  float speed_limit = (m_goto_reverse ? WHEEL_BACKWARD_SPEED : WHEEL_FORWARD_SPEED);

  forward_speed = speed_limit * (1.0f - Abs(angle_diff) / MAX_ANGLE_DIFF);
  if (m_goto_base_reached)
  {
    forward_speed *= 0.5f;
  }

  turn_speed = (m_goto_reverse ? WHEEL_TURN_SPEED_BACKWARD : WHEEL_TURN_SPEED_FORWARD) * (angle_diff > 0 ? 1.0f : -1.0f) * Abs(angle_diff) / MAX_ANGLE_DIFF;

  float reduction_factor = std::fmin(data.speedLimit, speed_limit) / speed_limit;

  if (m_goto_reverse)
  {
    forward_speed = -forward_speed;
  }

  float left_speed = reduction_factor * (forward_speed + turn_speed);
  float right_speed = reduction_factor * (forward_speed - turn_speed);
  m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, left_speed, goto_over);
  m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, right_speed, goto_over);

  return goto_over;
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
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_TURN_SPEED_FORWARD);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -WHEEL_TURN_SPEED_FORWARD);
    break;

  case TrajectoryMsg::State::RIGHT:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_TURN_SPEED_FORWARD);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_TURN_SPEED_FORWARD);
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
        m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, WHEEL_TURN_SPEED_FORWARD);
        m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, -WHEEL_TURN_SPEED_FORWARD);
      }
      else
      {
        m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, -WHEEL_TURN_SPEED_FORWARD);
        m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, WHEEL_TURN_SPEED_FORWARD);
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
