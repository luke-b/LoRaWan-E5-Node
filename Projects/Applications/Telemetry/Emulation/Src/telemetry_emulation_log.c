#include "telemetry_emulation_log.h"

#define EMULATION_UART_BASE ((volatile uint32_t *)0x40013800U)

static void EmulationLog_WriteChar(char ch)
{
  *EMULATION_UART_BASE = (uint32_t)(uint8_t)ch;
}

void EmulationLog_WriteString(const char *text)
{
  const char *current = text;

  if (current == 0)
  {
    return;
  }

  while (*current != '\0')
  {
    EmulationLog_WriteChar(*current++);
  }
}

void EmulationLog_WriteHexByte(uint8_t value)
{
  static const char digits[] = "0123456789ABCDEF";

  EmulationLog_WriteChar(digits[(value >> 4) & 0x0FU]);
  EmulationLog_WriteChar(digits[value & 0x0FU]);
}

void EmulationLog_WriteDecimal(uint32_t value)
{
  char buffer[10];
  uint32_t index = 0;

  if (value == 0U)
  {
    EmulationLog_WriteChar('0');
    return;
  }

  while ((value > 0U) && (index < sizeof(buffer)))
  {
    buffer[index++] = (char)('0' + (value % 10U));
    value /= 10U;
  }

  while (index > 0U)
  {
    EmulationLog_WriteChar(buffer[--index]);
  }
}

void EmulationLog_WriteLine(const char *text)
{
  EmulationLog_WriteString(text);
  EmulationLog_WriteString("\r\n");
}

void EmulationLog_WriteBool(bool value)
{
  EmulationLog_WriteString(value ? "true" : "false");
}