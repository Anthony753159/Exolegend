#include "utils.hpp"

#include <math.h>

float AngleDiffRad(float from, float to)
{
  return atan2(sin(to - from), cos(to - from));
}
