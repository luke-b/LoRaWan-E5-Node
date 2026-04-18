#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "telemetry_logic.h"

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT_EQ(actual, expected, msg)                                        \
  do                                                                             \
  {                                                                              \
    g_tests_run++;                                                               \
    if ((actual) == (expected))                                                  \
    {                                                                            \
      g_tests_passed++;                                                          \
      printf("  PASS  %s\n", (msg));                                           \
    }                                                                            \
    else                                                                         \
    {                                                                            \
      g_tests_failed++;                                                          \
      printf("  FAIL  %s  (got %lld, expected %lld)\n",                        \
             (msg), (long long)(actual), (long long)(expected));                \
    }                                                                            \
  } while (0)

#define ASSERT_TRUE(cond, msg) ASSERT_EQ(!!(cond), 1, msg)

#define RUN_TEST(fn)                                                             \
  do                                                                             \
  {                                                                              \
    printf("\n--- %s ---\n", #fn);                                             \
    fn();                                                                        \
  } while (0)

static void test_ConsumeWakeFlags_readsAndClearsBothFlags(void)
{
  volatile uint8_t doorFlag = 1U;
  volatile uint8_t rtcFlag = 1U;
  bool doorPending = false;
  bool rtcPending = false;

  Telemetry_ConsumeWakeFlags(&doorFlag, &rtcFlag, &doorPending, &rtcPending);

  ASSERT_TRUE(doorPending, "door flag consumed");
  ASSERT_TRUE(rtcPending, "rtc flag consumed");
  ASSERT_EQ(doorFlag, 0U, "door flag cleared");
  ASSERT_EQ(rtcFlag, 0U, "rtc flag cleared");
}

static void test_ConsumeWakeFlags_handlesMixedFlags(void)
{
  volatile uint8_t doorFlag = 0U;
  volatile uint8_t rtcFlag = 1U;
  bool doorPending = true;
  bool rtcPending = false;

  Telemetry_ConsumeWakeFlags(&doorFlag, &rtcFlag, &doorPending, &rtcPending);

  ASSERT_EQ(doorPending, false, "door pending false when flag is 0");
  ASSERT_EQ(rtcPending, true, "rtc pending true when flag is set");
}

static void test_ConsumeWakeFlags_nullGuards(void)
{
  volatile uint8_t doorFlag = 1U;
  volatile uint8_t rtcFlag = 1U;
  bool doorPending = false;
  bool rtcPending = false;

  Telemetry_ConsumeWakeFlags(NULL, &rtcFlag, &doorPending, &rtcPending);
  Telemetry_ConsumeWakeFlags(&doorFlag, NULL, &doorPending, &rtcPending);
  Telemetry_ConsumeWakeFlags(&doorFlag, &rtcFlag, NULL, &rtcPending);
  Telemetry_ConsumeWakeFlags(&doorFlag, &rtcFlag, &doorPending, NULL);

  ASSERT_EQ(doorFlag, 1U, "null guard keeps door flag unchanged");
  ASSERT_EQ(rtcFlag, 1U, "null guard keeps rtc flag unchanged");
}

static void test_EvaluateDoorEvent_openDoorSendsAlarm(void)
{
  ASSERT_EQ(Telemetry_EvaluateDoorEvent(true), TELEMETRY_EVENT_ACTION_SEND_ALARM_DOOR,
            "open door returns door alarm action");
}

static void test_EvaluateDoorEvent_closedDoorNoAction(void)
{
  ASSERT_EQ(Telemetry_EvaluateDoorEvent(false), TELEMETRY_EVENT_ACTION_NONE,
            "closed door returns no action");
}

static void test_EvaluatePeriodicEvent_waterDetectedSendsWaterAlarm(void)
{
  ASSERT_EQ(Telemetry_EvaluatePeriodicEvent(true), TELEMETRY_EVENT_ACTION_SEND_ALARM_WATER,
            "water detected returns water alarm action");
}

static void test_EvaluatePeriodicEvent_drySendsHeartbeat(void)
{
  ASSERT_EQ(Telemetry_EvaluatePeriodicEvent(false), TELEMETRY_EVENT_ACTION_SEND_HEARTBEAT,
            "dry state returns heartbeat action");
}

static void test_HandleDoorWake_notPendingDoesNothing(void)
{
  ASSERT_EQ(Telemetry_HandleDoorWake(false, true), TELEMETRY_EVENT_ACTION_NONE,
            "door wake handler returns no action when event is not pending");
}

static void test_HandleDoorWake_pendingClosedDoesNothing(void)
{
  ASSERT_EQ(Telemetry_HandleDoorWake(true, false), TELEMETRY_EVENT_ACTION_NONE,
            "door wake handler returns no action when debounce reads closed");
}

static void test_HandleDoorWake_pendingOpenSendsAlarm(void)
{
  ASSERT_EQ(Telemetry_HandleDoorWake(true, true), TELEMETRY_EVENT_ACTION_SEND_ALARM_DOOR,
            "door wake handler returns alarm when debounce reads open");
}

static void test_HandlePeriodicWake_notPendingDoesNothing(void)
{
  ASSERT_EQ(Telemetry_HandlePeriodicWake(false, true), TELEMETRY_EVENT_ACTION_NONE,
            "periodic wake handler returns no action when rtc is not pending");
}

static void test_HandlePeriodicWake_pendingDrySendsHeartbeat(void)
{
  ASSERT_EQ(Telemetry_HandlePeriodicWake(true, false), TELEMETRY_EVENT_ACTION_SEND_HEARTBEAT,
            "periodic wake handler returns heartbeat for dry state");
}

static void test_HandlePeriodicWake_pendingWetSendsWaterAlarm(void)
{
  ASSERT_EQ(Telemetry_HandlePeriodicWake(true, true), TELEMETRY_EVENT_ACTION_SEND_ALARM_WATER,
            "periodic wake handler returns water alarm for wet state");
}

int main(void)
{
  printf("=== Telemetry Logic Unit Tests ===\n");

  RUN_TEST(test_ConsumeWakeFlags_readsAndClearsBothFlags);
  RUN_TEST(test_ConsumeWakeFlags_handlesMixedFlags);
  RUN_TEST(test_ConsumeWakeFlags_nullGuards);
  RUN_TEST(test_EvaluateDoorEvent_openDoorSendsAlarm);
  RUN_TEST(test_EvaluateDoorEvent_closedDoorNoAction);
  RUN_TEST(test_EvaluatePeriodicEvent_waterDetectedSendsWaterAlarm);
  RUN_TEST(test_EvaluatePeriodicEvent_drySendsHeartbeat);
  RUN_TEST(test_HandleDoorWake_notPendingDoesNothing);
  RUN_TEST(test_HandleDoorWake_pendingClosedDoesNothing);
  RUN_TEST(test_HandleDoorWake_pendingOpenSendsAlarm);
  RUN_TEST(test_HandlePeriodicWake_notPendingDoesNothing);
  RUN_TEST(test_HandlePeriodicWake_pendingDrySendsHeartbeat);
  RUN_TEST(test_HandlePeriodicWake_pendingWetSendsWaterAlarm);

  printf("\n============================\n");
  printf("Ran %d tests: %d passed, %d failed\n", g_tests_run, g_tests_passed, g_tests_failed);

  return (g_tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
