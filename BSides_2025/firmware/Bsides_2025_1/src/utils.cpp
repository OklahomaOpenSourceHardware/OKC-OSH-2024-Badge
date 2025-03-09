#include "utils.h"

#include <stdlib.h>

// Pseudo atan2.
// Returns number from 0 to 11 that reflects the angle.
int patan2(int y, int x)
{
  int m = (abs(y) + abs(x)) / 4;
  y /= m;
  x /= m;
  if (x > 0)
  {
    if (y < 0)
    {
      return abs(x) > abs(y) ? 11 : 10;
    }
    else if (y > 0)
    {
      return abs(x) > abs(y) ? 1 : 2;
    }
    else
    {
      return 0;
    }
  }
  else if (x < 0)
  {
    if (y < 0)
    {
      return abs(x) > abs(y) ? 7 : 8;
    }
    else if (y > 0)
    {
      return abs(x) > abs(y) ? 5 : 4;
    }
    else
    {
      return 6;
    }
  }
  else
  {
    return y > 0 ? 3 : 9;
  }
}
