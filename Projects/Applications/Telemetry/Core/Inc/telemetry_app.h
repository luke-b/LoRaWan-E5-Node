#ifndef TELEMETRY_APP_H
#define TELEMETRY_APP_H

#include <stdbool.h>
#include <stdint.h>

#define TELEMETRY_PAYLOAD_SIZE 7U

typedef enum
{
  TELEMETRY_MSG_TYPE_HEARTBEAT = 0x01,
  TELEMETRY_MSG_TYPE_ALARM_DOOR = 0x02,
  TELEMETRY_MSG_TYPE_ALARM_WATER = 0x03,
} TelemetryMessageType_t;

typedef struct
{
  uint32_t totalWaterPulses;
  uint16_t lastLptimValue;
} TelemetryState_t;

typedef struct
{
  uint8_t bytes[TELEMETRY_PAYLOAD_SIZE];
  uint8_t size;
  bool confirmed;
} TelemetryPayload_t;

void Telemetry_InitState(TelemetryState_t *state, uint16_t initialCounter);
void Telemetry_UpdatePulseCounter(TelemetryState_t *state, uint16_t currentCounter);
void Telemetry_BuildPayload(TelemetryState_t *state, uint16_t currentCounter, TelemetryMessageType_t msgType,
                            uint8_t batteryLevel, bool doorOpen, bool waterDetected, TelemetryPayload_t *payload);

#endif