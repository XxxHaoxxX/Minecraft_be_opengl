#ifndef _RELOCATE_H
#define _RELOCATE_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <sys/mman.h>
// #include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
void relocateInstruction(uint32_t target_addr, void *orig_instructions, int length, void *trampoline_instructions, int *orig_boundaries, int *trampoline_boundaries, int *count);

#endif