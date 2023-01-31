#ifndef __MODEL_H__
#define __MODEL_H__

#include <stdint.h>
#include <stdbool.h>

#include "basic.h"


void            run();
bool            advance_cycle();
void            print_result();
void            print_command();

void            print_scope();
#endif