#include "trajectory.hpp"

#include <algorithm>
#include <math.h>

Trajectory::Trajectory(Gladiator *gladiator) : m_gladiator(gladiator)
{
  m_state = TrajectoryState::IDLE;
  m_rotate_time_remaining = ROTATE_TIME;
  m_rotate_direction = true;
  m_last_update_ms = millis();
}

void Trajectory::HandleMessage(const StrategyToTrajectory &msg)
{
  m_state = msg.ordered_state;
  m_goto_params = msg.goto_parameters;
  m_goto_base_reached = false;
}

void Trajectory::Update(const GameData &data)
{
  unsigned long now_ms = millis();
  float dt = 0.001f * (now_ms - m_last_update_ms);
  m_last_update_ms = now_ms;

  switch (m_state)
  {
  case TrajectoryState::IDLE:
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0);
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, 0);
    break;

  case TrajectoryState::GOTO:
    if (Goto(data.robot_data))
    {
      m_state = TrajectoryState::IDLE;
    }
    break;

  case TrajectoryState::ROTATE:
    m_rotate_time_remaining -= dt;

    if (m_rotate_time_remaining < 0.0f)
    {
      m_rotate_time_remaining = ROTATE_TIME;
      m_rotate_direction = !m_rotate_direction;
    }

    /* Moving right wheel a bit slower to ensure that we drift while rotating, to avoid staying locked in place */
    m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, 0.8f * (m_rotate_direction ? -1.0f : 1.0f));
    m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, m_rotate_direction ? 1.0f : -1.0f);
    break;
  }
}

bool Trajectory::ShouldSearchStrategy() const
{
  return m_goto_base_reached || m_state == TrajectoryState::IDLE || m_state == TrajectoryState::ROTATE;
}

bool Trajectory::ShouldApplyStrategy() const
{
  return m_state == TrajectoryState::IDLE || m_state == TrajectoryState::ROTATE;
}

bool Trajectory::Goto(const RobotData &data)
{
  /* Quick note on angles and signs:
   * We don't have a f****ng clue about the signs of the angles, so we just tried to make it work
   * by iterating fast. In the end, we have one chance out of two :P */

  /* Shifting the center of the robot for better performances.
   * We suppose the center of detection was not exactly aligned with the center of rotation */
  Vec2f pos = Vec2f{data.position.x, data.position.y};
  pos.x -= cosf(data.position.a) * POSITION_SHIFT;
  pos.y -= sinf(data.position.a) * POSITION_SHIFT;

  /* A GOTO order is defined by a target point (center of a cell) plus a given direction */
  Vec2f p1 = Vec2f{m_goto_params.x, m_goto_params.y};
  Vec2f p2 = Vec2f{m_goto_params.x + cosf(m_goto_params.angle), m_goto_params.y + sinf(m_goto_params.angle)};
  /* We compute the orthogonal distance between the robot and the target line */
  float dist_to_line = abs((p2.x - p1.x) * (p1.y - pos.y) - (p1.x - pos.x) * (p2.y - p1.y)) / sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));

  /* Checking on which side of the target line we are */
  float angle_line_to_robot = atan2f(pos.y - p1.y, pos.x - p1.x);
  float diff_angle_line_to_robot = AngleDiffRad(m_goto_params.angle, angle_line_to_robot);
  bool is_on_right_side = diff_angle_line_to_robot > 0.0f;

  /* To compute the target angle at which the robot must point, we use a VECTOR FIELD approach.
   * - When we are ON THE LINE (dist == 0), the vector field is the direction of the target line,
   * - When we are REALLY FAR FROM THE LINE, the vector field is orthogonal to the target line and points to it,
   * - When we are CLOSE TO THE LINE, the vector field is a combination of the two previous ones.
   *
   * For the smoothing between these cases, we use the 'atan' function, which gives us -PI/2 or PI/2 at the
   * limit, and 0 in the middle. We use an arbitrary scaling factor of 7.0f to adapt the transition to our needs.
   *
   * (run the 'visualize/vector_field_visualize.py' script and play with the sliders to see the vector field in action)
   */
  float target_angle = m_goto_params.angle + atan(dist_to_line * 7.0f) * (is_on_right_side ? -1.0f : 1.0f);

  /* Computing the x and y deltas to the cell center ('base') and
   * the actual 'target' shifted by a small distance in the given GOTO direction,
   * as this guarantees we go through the cell center before starting a new turn. */
  float dx_base = m_goto_params.x - pos.x;
  float dy_base = m_goto_params.y - pos.y;
  float dx_target = m_goto_params.x + GOTO_DELTA * cos(m_goto_params.angle) - pos.x;
  float dy_target = m_goto_params.y + GOTO_DELTA * sin(m_goto_params.angle) - pos.y;

  float dist_to_base = sqrt(dx_base * dx_base + dy_base * dy_base);

  /* Instead of considering the GOTO over when we reach a certain distance to the target,
   * we check if we passed the 'finish line' defined as orthogonal to the GOTO direction.
   * This way, even if we fail to get close enough to the target point (because of another
   * robot, for instance), we don't start going in circles to reach the target and just
   * keep going instead. */
  float angle_to_target = atan2f(dy_target, dx_target);
  float diff_angle_direction = AngleDiffRad(angle_to_target, m_goto_params.angle);
  /* We passed the finish line if the vector from the target to the robot is in the opposite
   * direction from the movement direction (abs(diff_angle) > PI / 2) */
  bool goto_over = abs(diff_angle_direction) > M_PI / 2;

  /* When we reach the cell center (distance based, this time), we tell the Strategy that
   * we can start computing the next step, even if we still move a little bit meanwhile */
  if (dist_to_base < GOTO_BASE_DISTANCE_THRESHOLD)
  {
    m_goto_base_reached = true;
  }

  /* Checking if we should go backwards instead of forward */
  float angle_diff = AngleDiffRad(target_angle, data.position.a);
  if (RECOMPUTE_BACKWARD)
  {
    /* We select the 'backward' parameter based on what's simplest in our current orientation:
     * - We go forward if the target point is in front of us,
     * - We go backward if the target point is behind us.
     *
     * (not used in the competition code, as we found out going backwards is better for turns) */
    m_goto_params.backward = abs(angle_diff) > M_PI / 2;
  }

  if (m_goto_params.backward)
  {
    /* If going backward, we target the opposite angle */
    angle_diff = AngleDiffRad(target_angle + M_PI, data.position.a);
  }

  /* Computing the two speed components */
  float forward_speed = 0.0f;
  float turn_speed = 0.0f;
  float speed_limit = (m_goto_params.backward ? WHEEL_BACKWARD_SPEED : WHEEL_FORWARD_SPEED);
  if (m_goto_params.high_speed)
  {
    speed_limit *= WHEEL_HIGHSPEED_FACTOR;
  }

  /* When we face the correct direction, we go at full speed,
   * when we face the wrong direction, we go at zero speed,
   * and we smoothly transition between the two when the angle is in between. */
  forward_speed = speed_limit * (1.0f - abs(angle_diff) / (M_PI / 2));
  if (m_goto_base_reached)
  {
    /* When we reach the cell center, we slow down to avoid overshooting
     * the target too much while the strategy is working */
    forward_speed *= 0.5f;
  }
  if (m_goto_params.backward)
  {
    forward_speed = -forward_speed;
  }

  /* Turn speed is proportional to the angle difference (just the opposite of the forward speed!) */
  turn_speed = abs(angle_diff) / (M_PI / 2);
  turn_speed *= (m_goto_params.backward ? WHEEL_TURN_SPEED_BACKWARD : WHEEL_TURN_SPEED_FORWARD);
  turn_speed *= angle_diff > 0 ? 1.0f : -1.0f; // Turn left or right

  /* If the robot speed limit is the limiting factor instead of our parameters (because of a slow
   * down), we reduce everything proportionally so that we keep the same turn/forward ratio. */
  float reduction_factor = std::fmin(data.speedLimit, speed_limit) / speed_limit;
  float left_speed = reduction_factor * (forward_speed + turn_speed);
  float right_speed = reduction_factor * (forward_speed - turn_speed);

  /* Note: we reset the PIDs if the GOTO is over, but we don't know if that's really needed */
  m_gladiator->control->setWheelSpeed(WheelAxis::LEFT, left_speed, goto_over);
  m_gladiator->control->setWheelSpeed(WheelAxis::RIGHT, right_speed, goto_over);

  return goto_over;
}