#ifndef TELEMETRY_LOGIC_H
#define TELEMETRY_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  TELEMETRY_EVENT_ACTION_NONE = 0,
  TELEMETRY_EVENT_ACTION_SEND_HEARTBEAT = 1,
  TELEMETRY_EVENT_ACTION_SEND_ALARM_DOOR = 2,
  TELEMETRY_EVENT_ACTION_SEND_ALARM_WATER = 3,
  TELEMETRY_EVENT_ACTION_SEND_TEST = 4,
} TelemetryEventAction_t;

void Telemetry_ConsumeWakeFlags(volatile uint8_t *doorFlag, volatile uint8_t *rtcFlag,
                                bool *doorPending, bool *rtcPending);

TelemetryEventAction_t Telemetry_EvaluateDoorEvent(bool doorStillOpen);

TelemetryEventAction_t Telemetry_EvaluatePeriodicEvent(bool waterDetected);

TelemetryEventAction_t Telemetry_HandleDoorWake(bool doorPending, bool doorStillOpenAfterDebounce);

TelemetryEventAction_t Telemetry_HandlePeriodicWake(bool rtcPending, bool waterDetected);

#endif
