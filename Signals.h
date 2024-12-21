#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define SIGHOLD SIGUSR1

void Signals_Setup();

void SegmentationFault(int sig);
void FuncStop(int sig);
void Interrupt(int sig);