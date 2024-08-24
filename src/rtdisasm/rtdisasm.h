#ifndef __RTDISASM_H
#define __RTDISASM_H

#include <stdint.h>
#include "rtdisasm_table.h"

// "code" should point to place with machine instructions, and "limit"
// limits the area of analyze, so no segfaults would be triggered on
// page boundaries. if "found" is non-zero, on instruction hit it
// would be set to found instruction table entry
// returns 0 when no instruction was found, -1 when limit reached
// and non-negative-non-zero number of actual instruction size
int rtdisasm_analyze_single(const uint8_t* code, unsigned limit, const instruction_t** found);

// analyze all instructions at "code" until "limit" is reached or
// instruction of "rt_target" equal was found. returns -1 when size limit hit,
// 0 if rtdisasm encountered unknown instruction
// and non-zero integer is offset from "code"
int rtdisasm_find_target(const uint8_t* code, unsigned limit, unsigned rt_target);

// will analyze all instructions until "wanted" size of bytes is reached
// within instruction boundaries, therefore making that address suitable
// for patching and not break instruction fetching. on success returns
// non-zero value - patch size, within instructions aligned,
// returns 0 if unknown instruction met and -1 if "limit" is reached.
int rtdisasm_estimate_patch(const uint8_t* code, unsigned limit, int wanted);

#endif