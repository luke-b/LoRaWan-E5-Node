#include "telemetry_app.h"

void Telemetry_InitState(TelemetryState_t *state, uint16_t initialCounter)
{
  if (state == 0)
  {
    return;
  }

  state->totalWaterPulses = 0;
  state->lastLptimValue = initialCounter;
}

void Telemetry_UpdatePulseCounter(TelemetryState_t *state, uint16_t currentCounter)
{
  if (state == 0)
  {
    return;
  }

  if (currentCounter >= state->lastLptimValue)
  {
    state->totalWaterPulses += (uint32_t)(currentCounter - state->lastLptimValue);
  }
  else
  {
    state->totalWaterPulses += (uint32_t)((0xFFFFU - state->lastLptimValue) + currentCounter + 1U);
  }

  state->lastLptimValue = currentCounter;
}

void Telemetry_BuildPayload(TelemetryState_t *state, uint16_t currentCounter, TelemetryMessageType_t msgType,
                            uint8_t batteryLevel, bool doorOpen, bool waterDetected, TelemetryPayload_t *payload)
{
  if ((state == 0) || (payload == 0))
  {
    return;
  }

  Telemetry_UpdatePulseCounter(state, currentCounter);

  payload->bytes[0] = (uint8_t)msgType;
  payload->bytes[1] = (uint8_t)((state->totalWaterPulses >> 24) & 0xFFU);
  payload->bytes[2] = (uint8_t)((state->totalWaterPulses >> 16) & 0xFFU);
  payload->bytes[3] = (uint8_t)((state->totalWaterPulses >> 8) & 0xFFU);
  payload->bytes[4] = (uint8_t)(state->totalWaterPulses & 0xFFU);
  payload->bytes[5] = batteryLevel;
  payload->bytes[6] = 0U;

  if (doorOpen)
  {
    payload->bytes[6] |= 1U << 0;
  }

  if (waterDetected)
  {
    payload->bytes[6] |= 1U << 1;
  }

  payload->size = TELEMETRY_PAYLOAD_SIZE;
  payload->confirmed = (msgType != TELEMETRY_MSG_TYPE_HEARTBEAT);
}

void Telemetry_BuildTestMessage(uint32_t deviceId, TelemetryPayload_t *payload)
{
  if (payload == 0)
  {
    return;
  }

  /* Test payload: 5 bytes → [message_type(1) | device_id(4)] */
  payload->bytes[0] = (uint8_t)TELEMETRY_MSG_TYPE_TEST;
  payload->bytes[1] = (uint8_t)((deviceId >> 24) & 0xFFU);
  payload->bytes[2] = (uint8_t)((deviceId >> 16) & 0xFFU);
  payload->bytes[3] = (uint8_t)((deviceId >> 8) & 0xFFU);
  payload->bytes[4] = (uint8_t)(deviceId & 0xFFU);

  payload->size = 5U;
  payload->confirmed = false;  /* Test messages sent unconfirmed to reduce retries */
}