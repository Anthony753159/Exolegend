#include "trajectory.hpp"

#include "gladiator.h"
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

#define ROTATE_TIME 1.0f

#include <math.h>

Trajectory::Trajectory(Gladiator *gladiator) : m_gladiator(gladiator)
{
  m_state = TrajectoryMsg::State::IDLE;
  m_rotate_time_remaining = ROTATE_TIME;
  m_rotate_direction = true;
  m_last_time_updated = millis();
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
    m_goto_high_speed = msg.goto_high_speed;
    m_state = TrajectoryMsg::State::GOTO;
    break;
  case TrajectoryMsg::ORDER_ROTATE:
    m_state = TrajectoryMsg::State::ROTATE;
    break;
  default:
    break;
  }
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
  if (m_goto_high_speed)
  {
    speed_limit *= 2.0f;
  }

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
  unsigned long now = millis();
  float dt = 0.001f * (now - m_last_time_updated);
  m_last_time_updated = now;

  switch (m_state)
  {
  case TrajectoryMsg::State::IDLE:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
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
    m_rotate_time_remaining -= dt;
    m_gladiator->log("Rotate time remaining: %f - dt: %f", m_rotate_time_remaining, dt);

    if (m_rotate_time_remaining < 0.0f)
    {
      m_rotate_time_remaining = ROTATE_TIME;
      m_rotate_direction = !m_rotate_direction;
      m_gladiator->log("Change direction");
    }

    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0.95f * (m_rotate_direction ? -1.0f : 1.0f));
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, m_rotate_direction ? 1.0f : -1.0f);
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

bool Trajectory::ShouldSearchStrategy() const
{
  return GotoBaseReached() || m_state == TrajectoryMsg::State::IDLE || m_state == TrajectoryMsg::State::ROTATE;
}

bool Trajectory::ShouldApplyStrategy() const
{
  return m_state == TrajectoryMsg::State::IDLE || m_state == TrajectoryMsg::State::ROTATE;
}
