#ifndef TELEMETRY_EMULATION_LOG_H
#define TELEMETRY_EMULATION_LOG_H

#include <stdbool.h>
#include <stdint.h>

void EmulationLog_WriteString(const char *text);
void EmulationLog_WriteHexByte(uint8_t value);
void EmulationLog_WriteDecimal(uint32_t value);
void EmulationLog_WriteLine(const char *text);
void EmulationLog_WriteBool(bool value);

#endif