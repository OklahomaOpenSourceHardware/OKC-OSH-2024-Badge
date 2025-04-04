#include "utils.h"

#include <stdlib.h>

#include "PinConfigured.h"

#define LONESHA256_STATIC
#include "lonesha256.h"

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

void configPinInput(uint32_t pin)
{
  PinName p = digitalPinToPinName(pin);
  pin_function(p, CH_PIN_DATA(CH_MODE_INPUT, CH_CNF_INPUT_FLOAT, 0, 0));
}

void configPinOutput(uint32_t pin)
{
  PinName p = digitalPinToPinName(pin);
  pin_function(p, CH_PIN_DATA(CH_MODE_OUTPUT_50MHz, CH_CNF_OUTPUT_PP, 0, 0));
}

void sha256(uint8_t hash[32], const uint8_t *data, int len)
{
  lonesha256(hash, data, len);
}

const Storage storage{
    {2,
     {0b1010, 0b0101}},
};

void updateStorage(const Storage &newData)
{
  const uint32_t *src = reinterpret_cast<const uint32_t *>(&newData);
  uint32_t dest = uint32_t(&storage) | FLASH_BASE; // Alias flash region
  static_assert(sizeof(storage) == 64);

  FLASH_Unlock_Fast();
  FLASH_ErasePage_Fast(dest);
  FLASH_BufReset();
  for (int i = 0; i < 16; i++)
  {
    FLASH_BufLoad(dest + (i << 2), src[i]);
  }
  FLASH_ProgramPage_Fast(dest);
  FLASH_Lock_Fast();
}
