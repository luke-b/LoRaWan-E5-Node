/**
 * @file test_telemetry_app.c
 * @brief Host-native unit tests for telemetry_app.c (no hardware required).
 *        Compile with: gcc -std=c11 -I./Core/Inc -o test_telemetry_app
 *                           test_telemetry_app.c Core/Src/telemetry_app.c
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "telemetry_app.h"

/* ── Minimal test harness ──────────────────────────────────────────────────── */
static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT_EQ(actual, expected, msg)                                        \
    do {                                                                        \
        g_tests_run++;                                                          \
        if ((actual) == (expected)) {                                           \
            g_tests_passed++;                                                   \
            printf("  PASS  %s\n", (msg));                                      \
        } else {                                                                \
            g_tests_failed++;                                                   \
            printf("  FAIL  %s  (got %lld, expected %lld)\n",                  \
                   (msg), (long long)(actual), (long long)(expected));          \
        }                                                                       \
    } while (0)

#define ASSERT_TRUE(cond, msg) ASSERT_EQ(!!(cond), 1, msg)
#define ASSERT_FALSE(cond, msg) ASSERT_EQ(!!(cond), 0, msg)

#define RUN_TEST(fn)   \
    do {               \
        printf("\n--- %s ---\n", #fn); \
        fn();          \
    } while (0)

/* ── Tests: Telemetry_InitState ────────────────────────────────────────────── */

static void test_InitState_zeroesPulses(void)
{
    TelemetryState_t s = {0xDEADBEEFU, 0xABCDU};
    Telemetry_InitState(&s, 100U);
    ASSERT_EQ(s.totalWaterPulses, 0U, "totalWaterPulses reset to 0");
}

static void test_InitState_setsLastLptim(void)
{
    TelemetryState_t s;
    Telemetry_InitState(&s, 0x1234U);
    ASSERT_EQ(s.lastLptimValue, 0x1234U, "lastLptimValue set to initialCounter");
}

static void test_InitState_nullGuard(void)
{
    /* Must not crash on NULL pointer */
    Telemetry_InitState(NULL, 0U);
    ASSERT_TRUE(1, "NULL pointer does not crash");
}

/* ── Tests: Telemetry_UpdatePulseCounter ───────────────────────────────────── */

static void test_UpdatePulse_normalIncrement(void)
{
    TelemetryState_t s;
    Telemetry_InitState(&s, 100U);
    Telemetry_UpdatePulseCounter(&s, 150U);
    ASSERT_EQ(s.totalWaterPulses, 50U, "normal increment: 150 - 100 = 50");
    ASSERT_EQ(s.lastLptimValue, 150U, "lastLptimValue updated to 150");
}

static void test_UpdatePulse_zeroIncrement(void)
{
    TelemetryState_t s;
    Telemetry_InitState(&s, 200U);
    Telemetry_UpdatePulseCounter(&s, 200U);
    ASSERT_EQ(s.totalWaterPulses, 0U, "same value: 0 pulses added");
}

static void test_UpdatePulse_counterOverflow(void)
{
    /* Counter wraps: was 0xFFF0, now 0x000A → delta = (0xFFFF - 0xFFF0) + 0x000A + 1 = 15 + 10 + 1 = 26 */
    TelemetryState_t s;
    Telemetry_InitState(&s, 0xFFF0U);
    Telemetry_UpdatePulseCounter(&s, 0x000AU);
    ASSERT_EQ(s.totalWaterPulses, 26U, "counter overflow: delta = 26");
}

static void test_UpdatePulse_overflowAtMaxMinus1(void)
{
    /* last = 0xFFFF, current = 0x0000 → delta = 1 */
    TelemetryState_t s;
    Telemetry_InitState(&s, 0xFFFFU);
    Telemetry_UpdatePulseCounter(&s, 0x0000U);
    ASSERT_EQ(s.totalWaterPulses, 1U, "overflow at 0xFFFF->0x0000: delta = 1");
}

static void test_UpdatePulse_accumulation(void)
{
    TelemetryState_t s;
    Telemetry_InitState(&s, 0U);
    Telemetry_UpdatePulseCounter(&s, 10U);
    Telemetry_UpdatePulseCounter(&s, 20U);
    Telemetry_UpdatePulseCounter(&s, 30U);
    ASSERT_EQ(s.totalWaterPulses, 30U, "three steps: 0->10->20->30 = 30 total");
}

static void test_UpdatePulse_nullGuard(void)
{
    Telemetry_UpdatePulseCounter(NULL, 42U);
    ASSERT_TRUE(1, "NULL pointer does not crash");
}

/* ── Tests: Telemetry_BuildPayload ─────────────────────────────────────────── */

static void test_BuildPayload_heartbeat_bytes(void)
{
    TelemetryState_t s;
    TelemetryPayload_t p;
    Telemetry_InitState(&s, 0U);
    /* counter goes from 0 to 200 → 200 pulses = 0x000000C8 */
    Telemetry_BuildPayload(&s, 200U, TELEMETRY_MSG_TYPE_HEARTBEAT, 0xC8U, false, false, &p);

    ASSERT_EQ(p.bytes[0], 0x01U, "bytes[0] = HEARTBEAT msg type");
    ASSERT_EQ(p.bytes[1], 0x00U, "bytes[1] = pulses[31:24] = 0");
    ASSERT_EQ(p.bytes[2], 0x00U, "bytes[2] = pulses[23:16] = 0");
    ASSERT_EQ(p.bytes[3], 0x00U, "bytes[3] = pulses[15:8]  = 0");
    ASSERT_EQ(p.bytes[4], 0xC8U, "bytes[4] = pulses[7:0]   = 200");
    ASSERT_EQ(p.bytes[5], 0xC8U, "bytes[5] = batteryLevel  = 200");
    ASSERT_EQ(p.bytes[6], 0x00U, "bytes[6] = flags = 0 (no door, no water)");
    ASSERT_EQ(p.size, TELEMETRY_PAYLOAD_SIZE, "size = TELEMETRY_PAYLOAD_SIZE");
}

static void test_BuildPayload_heartbeat_notConfirmed(void)
{
    TelemetryState_t s;
    TelemetryPayload_t p;
    Telemetry_InitState(&s, 0U);
    Telemetry_BuildPayload(&s, 0U, TELEMETRY_MSG_TYPE_HEARTBEAT, 0xC8U, false, false, &p);
    ASSERT_FALSE(p.confirmed, "heartbeat is NOT confirmed");
}

static void test_BuildPayload_doorAlarm_confirmed(void)
{
    TelemetryState_t s;
    TelemetryPayload_t p;
    Telemetry_InitState(&s, 0U);
    Telemetry_BuildPayload(&s, 0U, TELEMETRY_MSG_TYPE_ALARM_DOOR, 0xC7U, true, false, &p);
    ASSERT_EQ(p.bytes[0], 0x02U, "bytes[0] = ALARM_DOOR msg type");
    ASSERT_TRUE(p.confirmed, "door alarm IS confirmed");
    ASSERT_EQ(p.bytes[6] & 0x01U, 0x01U, "flag bit0 set (doorOpen)");
    ASSERT_EQ(p.bytes[6] & 0x02U, 0x00U, "flag bit1 clear (no water)");
}

static void test_BuildPayload_waterAlarm_confirmed(void)
{
    TelemetryState_t s;
    TelemetryPayload_t p;
    Telemetry_InitState(&s, 0U);
    Telemetry_BuildPayload(&s, 0U, TELEMETRY_MSG_TYPE_ALARM_WATER, 0xC6U, false, true, &p);
    ASSERT_EQ(p.bytes[0], 0x03U, "bytes[0] = ALARM_WATER msg type");
    ASSERT_TRUE(p.confirmed, "water alarm IS confirmed");
    ASSERT_EQ(p.bytes[6] & 0x01U, 0x00U, "flag bit0 clear (no door)");
    ASSERT_EQ(p.bytes[6] & 0x02U, 0x02U, "flag bit1 set (waterDetected)");
}

static void test_BuildPayload_bothFlags(void)
{
    TelemetryState_t s;
    TelemetryPayload_t p;
    Telemetry_InitState(&s, 0U);
    Telemetry_BuildPayload(&s, 0U, TELEMETRY_MSG_TYPE_ALARM_WATER, 0xFFU, true, true, &p);
    ASSERT_EQ(p.bytes[6], 0x03U, "both flags set: bytes[6] = 0x03");
}

static void test_BuildPayload_nullState(void)
{
    TelemetryPayload_t p;
    Telemetry_BuildPayload(NULL, 0U, TELEMETRY_MSG_TYPE_HEARTBEAT, 0U, false, false, &p);
    ASSERT_TRUE(1, "NULL state does not crash");
}

static void test_BuildPayload_nullPayload(void)
{
    TelemetryState_t s;
    Telemetry_InitState(&s, 0U);
    Telemetry_BuildPayload(&s, 0U, TELEMETRY_MSG_TYPE_HEARTBEAT, 0U, false, false, NULL);
    ASSERT_TRUE(1, "NULL payload does not crash");
}

static void test_BuildPayload_pulseEncoding_largeValue(void)
{
    /* 0x01020304 = 16909060 pulses */
    TelemetryState_t s;
    TelemetryPayload_t p;
    Telemetry_InitState(&s, 0U);
    s.totalWaterPulses = 0x01020304U;
    Telemetry_BuildPayload(&s, 0U, TELEMETRY_MSG_TYPE_HEARTBEAT, 0U, false, false, &p);
    /* After UpdatePulseCounter with current=0, last=0 → delta=0, totalWaterPulses unchanged */
    ASSERT_EQ(p.bytes[1], 0x01U, "bytes[1] = pulses[31:24]");
    ASSERT_EQ(p.bytes[2], 0x02U, "bytes[2] = pulses[23:16]");
    ASSERT_EQ(p.bytes[3], 0x03U, "bytes[3] = pulses[15:8]");
    ASSERT_EQ(p.bytes[4], 0x04U, "bytes[4] = pulses[7:0]");
}

/* ── Main ──────────────────────────────────────────────────────────────────── */

int main(void)
{
    printf("=== Telemetry Unit Tests ===\n");

    RUN_TEST(test_InitState_zeroesPulses);
    RUN_TEST(test_InitState_setsLastLptim);
    RUN_TEST(test_InitState_nullGuard);

    RUN_TEST(test_UpdatePulse_normalIncrement);
    RUN_TEST(test_UpdatePulse_zeroIncrement);
    RUN_TEST(test_UpdatePulse_counterOverflow);
    RUN_TEST(test_UpdatePulse_overflowAtMaxMinus1);
    RUN_TEST(test_UpdatePulse_accumulation);
    RUN_TEST(test_UpdatePulse_nullGuard);

    RUN_TEST(test_BuildPayload_heartbeat_bytes);
    RUN_TEST(test_BuildPayload_heartbeat_notConfirmed);
    RUN_TEST(test_BuildPayload_doorAlarm_confirmed);
    RUN_TEST(test_BuildPayload_waterAlarm_confirmed);
    RUN_TEST(test_BuildPayload_bothFlags);
    RUN_TEST(test_BuildPayload_nullState);
    RUN_TEST(test_BuildPayload_nullPayload);
    RUN_TEST(test_BuildPayload_pulseEncoding_largeValue);

    printf("\n============================\n");
    printf("Ran %d tests: %d passed, %d failed\n",
           g_tests_run, g_tests_passed, g_tests_failed);

    return (g_tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
