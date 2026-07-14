#include "../int_lib.h"

COMPILER_RT_ABI si_int __mulsi3(si_int a, si_int b) {
  su_int x = a;
  su_int y = b;
  su_int r = 0;
  while (x) {
    if (x & 1)
      r += y;
    x >>= 1;
    y <<= 1;
  }
  return r;
}
