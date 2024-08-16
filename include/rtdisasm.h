#ifndef __RTDISASM_H
#define __RTDISASM_H

#include <stdint.h>

// code should point to place with machine instructions, and size
// limits the area of analyze, so no segfaults would be triggered on
// page boundaries.
// returns 0 when no instruction was found, -1 when size limit reached
// and non-negative-non-zero number of actual instruction size
int rtdisasm_analyze_single(const uint8_t* code, uint8_t size);

#endif