#ifndef __RUN_H__
#define __RUN_H__

#include <stdint.h>
#include <stdbool.h>

#include "basic.h"

Issue_Queue *IQ;
Reorder_Buffer *ROB;

void        commit();
void        dispatch();
void        renaming();
void        decode();
void        fetch();
int         findPhysicalRegister();

void        issue();
void        regRead();
void        execute();
void        writeBack();

void        delete_EXE(int index);
void        delete_WB(int index);

void        print_ROB();

#endif