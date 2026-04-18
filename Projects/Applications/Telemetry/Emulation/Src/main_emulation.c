#include <stdbool.h>
#include <stdint.h>

#include "telemetry_app.h"
#include "telemetry_emulation_log.h"

static void WritePayload(const TelemetryPayload_t *payload)
{
  uint8_t index;

  EmulationLog_WriteString("PAYLOAD ");
  for (index = 0; index < payload->size; ++index)
  {
    EmulationLog_WriteHexByte(payload->bytes[index]);
    if ((uint8_t)(index + 1U) < payload->size)
    {
      EmulationLog_WriteString(" ");
    }
  }
  EmulationLog_WriteString(" confirmed=");
  EmulationLog_WriteBool(payload->confirmed);
  EmulationLog_WriteString("\r\n");
}

static bool AssertEqualU32(const char *label, uint32_t actual, uint32_t expected)
{
  if (actual == expected)
  {
    EmulationLog_WriteString("ASSERT PASS ");
    EmulationLog_WriteLine(label);
    return true;
  }

  EmulationLog_WriteString("ASSERT FAIL ");
  EmulationLog_WriteString(label);
  EmulationLog_WriteString(" actual=");
  EmulationLog_WriteDecimal(actual);
  EmulationLog_WriteString(" expected=");
  EmulationLog_WriteDecimal(expected);
  EmulationLog_WriteString("\r\n");
  return false;
}

static bool AssertEqualByte(const char *label, uint8_t actual, uint8_t expected)
{
  return AssertEqualU32(label, actual, expected);
}

static bool RunScenario(void)
{
  bool ok = true;
  TelemetryState_t state;
  TelemetryPayload_t payload;

  Telemetry_InitState(&state, 0U);
  EmulationLog_WriteLine("SCENARIO heartbeat");
  Telemetry_BuildPayload(&state, 0U, TELEMETRY_MSG_TYPE_HEARTBEAT, 200U, false, false, &payload);
  WritePayload(&payload);
  ok &= AssertEqualByte("heartbeat.type", payload.bytes[0], TELEMETRY_MSG_TYPE_HEARTBEAT);
  ok &= AssertEqualU32("heartbeat.pulses", state.totalWaterPulses, 0U);
  ok &= AssertEqualByte("heartbeat.battery", payload.bytes[5], 200U);
  ok &= AssertEqualByte("heartbeat.status", payload.bytes[6], 0U);

  EmulationLog_WriteLine("SCENARIO door_alarm");
  Telemetry_BuildPayload(&state, 5U, TELEMETRY_MSG_TYPE_ALARM_DOOR, 199U, true, false, &payload);
  WritePayload(&payload);
  ok &= AssertEqualU32("door.pulses", state.totalWaterPulses, 5U);
  ok &= AssertEqualByte("door.status", payload.bytes[6], 0x01U);
  ok &= AssertEqualByte("door.confirmed", payload.confirmed ? 1U : 0U, 1U);

  EmulationLog_WriteLine("SCENARIO water_alarm");
  Telemetry_BuildPayload(&state, 9U, TELEMETRY_MSG_TYPE_ALARM_WATER, 198U, false, true, &payload);
  WritePayload(&payload);
  ok &= AssertEqualU32("water.pulses", state.totalWaterPulses, 9U);
  ok &= AssertEqualByte("water.status", payload.bytes[6], 0x02U);

  EmulationLog_WriteLine("SCENARIO counter_overflow");
  Telemetry_InitState(&state, 65534U);
  Telemetry_BuildPayload(&state, 3U, TELEMETRY_MSG_TYPE_HEARTBEAT, 197U, false, false, &payload);
  WritePayload(&payload);
  ok &= AssertEqualU32("overflow.pulses", state.totalWaterPulses, 5U);
  ok &= AssertEqualByte("overflow.type", payload.bytes[0], TELEMETRY_MSG_TYPE_HEARTBEAT);

  return ok;
}

void Error_Handler(void)
{
  EmulationLog_WriteLine("ERROR_HANDLER");
  while (1)
  {
  }
}

int main(void)
{
  bool success;

  EmulationLog_WriteLine("TELEMETRY EMULATION BOOT");
  EmulationLog_WriteLine("EMULATED JOIN OK");
  success = RunScenario();

  if (success)
  {
    EmulationLog_WriteLine("PROJECT EXECUTION SUCCESSFUL");
  }
  else
  {
    EmulationLog_WriteLine("PROJECT EXECUTION FAILED");
  }

  while (1)
  {
  }
}