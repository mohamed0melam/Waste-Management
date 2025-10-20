#pragma once
#include <stdint.h>
#include <stdio.h>

// Minimal Arduino mock definitions for native testing
#define HIGH 1
#define LOW  0
#define INPUT 0
#define OUTPUT 1
#define true 1
#define false 0
typedef uint8_t byte;

inline void delay(int) {}
inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalRead(int) { return 0; }
inline unsigned long millis() { return 0; }
