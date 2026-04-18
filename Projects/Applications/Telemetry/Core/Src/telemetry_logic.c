#include "telemetry_logic.h"

void Telemetry_ConsumeWakeFlags(volatile uint8_t *doorFlag, volatile uint8_t *rtcFlag,
                                bool *doorPending, bool *rtcPending)
{
  if ((doorFlag == 0) || (rtcFlag == 0) || (doorPending == 0) || (rtcPending == 0))
  {
    return;
  }

  *doorPending = (*doorFlag != 0U);
  *rtcPending = (*rtcFlag != 0U);

  *doorFlag = 0U;
  *rtcFlag = 0U;
}

TelemetryEventAction_t Telemetry_EvaluateDoorEvent(bool doorStillOpen)
{
  if (!doorStillOpen)
  {
    return TELEMETRY_EVENT_ACTION_NONE;
  }

  return TELEMETRY_EVENT_ACTION_SEND_ALARM_DOOR;
}

TelemetryEventAction_t Telemetry_EvaluatePeriodicEvent(bool waterDetected)
{
  if (waterDetected)
  {
    return TELEMETRY_EVENT_ACTION_SEND_ALARM_WATER;
  }

  return TELEMETRY_EVENT_ACTION_SEND_HEARTBEAT;
}

TelemetryEventAction_t Telemetry_HandleDoorWake(bool doorPending, bool doorStillOpenAfterDebounce)
{
  if (!doorPending)
  {
    return TELEMETRY_EVENT_ACTION_NONE;
  }

  return Telemetry_EvaluateDoorEvent(doorStillOpenAfterDebounce);
}

TelemetryEventAction_t Telemetry_HandlePeriodicWake(bool rtcPending, bool waterDetected)
{
  if (!rtcPending)
  {
    return TELEMETRY_EVENT_ACTION_NONE;
  }

  return Telemetry_EvaluatePeriodicEvent(waterDetected);
}
