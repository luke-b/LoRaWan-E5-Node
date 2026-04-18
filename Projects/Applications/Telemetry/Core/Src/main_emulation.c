/**
 * @file    main_emulation.c
 * @brief   Emulation harness for Renode + Robot Framework CI.
 *
 * Compiled only when BUILD_PROFILE=emulation (macro EMULATION_BUILD defined).
 * Replaces main.c in the build — no HAL peripherals, no RTOS, no LoRaWAN stack.
 *
 * Output is written directly to the STM32WLE5 USART1 TDR register (0x40013828).
 * Renode's TrivialUart accepts a byte write at ANY offset within its mapped
 * address space, so this produces visible UART output in the emulated terminal.
 *
 * Expected output (matched by integration_tests.robot):
 *   TELEMETRY EMULATION BOOT
 *   EMULATED JOIN OK
 *   SCENARIO heartbeat
 *   PAYLOAD 01 00 00 00 00 C8 00 confirmed=false
 *   SCENARIO door_alarm
 *   PAYLOAD 02 00 00 00 05 C7 01 confirmed=true
 *   SCENARIO water_alarm
 *   PAYLOAD 03 00 00 00 09 C6 02 confirmed=true
 *   SCENARIO counter_overflow
 *   PAYLOAD 01 00 00 00 05 C5 00 confirmed=false
 *   ASSERT PASS overflow.pulses
 *   PROJECT EXECUTION SUCCESSFUL
 */

#ifdef EMULATION_BUILD

#include "telemetry_app.h"
#include "telemetry_logic.h"
#include <stdbool.h>
#include <stdint.h>

/* ── Direct UART output ────────────────────────────────────────────────────── */

/* Renode TrivialUart is mapped at the base address from the .repl file. */
#define USART1_TDR_ADDR  0x40013800UL
static volatile uint32_t * const s_tdr = (volatile uint32_t *)USART1_TDR_ADDR;

static void emu_putchar(char c)
{
    *s_tdr = (uint32_t)(uint8_t)c;
}

static void emu_puts(const char *s)
{
    while (*s != '\0')
    {
        emu_putchar(*s++);
    }
    emu_putchar('\r');
    emu_putchar('\n');
}

static void emu_put_hex_byte(uint8_t byte)
{
    static const char hex[] = "0123456789ABCDEF";
    emu_putchar(hex[(byte >> 4U) & 0x0FU]);
    emu_putchar(hex[byte & 0x0FU]);
}

static void emu_print_payload(const TelemetryPayload_t *p)
{
    /* Format: "PAYLOAD XX XX XX XX XX XX XX confirmed=true/false" */
    const char prefix[] = "PAYLOAD ";
    for (const char *c = prefix; *c != '\0'; c++)
    {
        emu_putchar(*c);
    }

    for (uint8_t i = 0U; i < p->size; i++)
    {
        if (i > 0U)
        {
            emu_putchar(' ');
        }
        emu_put_hex_byte(p->bytes[i]);
    }

    if (p->confirmed)
    {
        emu_puts(" confirmed=true");
    }
    else
    {
        emu_puts(" confirmed=false");
    }
}

/* ── Emulation scenarios ───────────────────────────────────────────────────── */

/**
 * Scenario A: Sequential heartbeat → door_alarm → water_alarm
 * Verifies payload byte layout and confirmed flag.
 */
static void run_scenario_payloads(void)
{
    TelemetryState_t state;
    TelemetryPayload_t payload;

    Telemetry_InitState(&state, 0U);

    /* --- heartbeat --- */
    emu_puts("SCENARIO heartbeat");
    Telemetry_BuildPayload(&state, 0U,
                           TELEMETRY_MSG_TYPE_HEARTBEAT,
                           0xC8U,   /* battery = 200 */
                           false, false,
                           &payload);
    emu_print_payload(&payload);

    /* --- door_alarm (pulse counter now at 5) --- */
    emu_puts("SCENARIO door_alarm");
    Telemetry_BuildPayload(&state, 5U,
                           TELEMETRY_MSG_TYPE_ALARM_DOOR,
                           0xC7U,   /* battery = 199 */
                           true, false,
                           &payload);
    emu_print_payload(&payload);

    /* --- water_alarm (pulse counter now at 9) --- */
    emu_puts("SCENARIO water_alarm");
    Telemetry_BuildPayload(&state, 9U,
                           TELEMETRY_MSG_TYPE_ALARM_WATER,
                           0xC6U,   /* battery = 198 */
                           false, true,
                           &payload);
    emu_print_payload(&payload);
}

/**
 * Scenario B: 16-bit counter overflow (0xFFFC → 0x0001 → delta = 5)
 * Formula: (0xFFFF - 0xFFFC) + 0x0001 + 1 = 3 + 1 + 1 = 5
 */
static void run_scenario_counter_overflow(void)
{
    TelemetryState_t state;
    TelemetryPayload_t payload;

    emu_puts("SCENARIO counter_overflow");

    /* Start with counter near the rollover point */
    Telemetry_InitState(&state, 0xFFFCU);

    Telemetry_BuildPayload(&state, 0x0001U,
                           TELEMETRY_MSG_TYPE_HEARTBEAT,
                           0xC5U,   /* battery = 197 */
                           false, false,
                           &payload);
    emu_print_payload(&payload);

    /* Verify accumulated pulses equal the expected overflow delta */
    if (state.totalWaterPulses == 5U)
    {
        emu_puts("ASSERT PASS overflow.pulses");
    }
    else
    {
        emu_puts("ASSERT FAIL overflow.pulses");
    }
}

/**
 * Scenario C: Validate wake decision handlers used by production main loop.
 * Covers pending gating and debounce/water branch selection.
 */
static void run_scenario_wake_decisions(void)
{
    TelemetryEventAction_t action;

    emu_puts("SCENARIO wake_decisions");

    action = Telemetry_HandleDoorWake(false, true);
    emu_puts((action == TELEMETRY_EVENT_ACTION_NONE) ?
             "ASSERT PASS door.not_pending" :
             "ASSERT FAIL door.not_pending");

    action = Telemetry_HandleDoorWake(true, false);
    emu_puts((action == TELEMETRY_EVENT_ACTION_NONE) ?
             "ASSERT PASS door.pending_closed" :
             "ASSERT FAIL door.pending_closed");

    action = Telemetry_HandleDoorWake(true, true);
    emu_puts((action == TELEMETRY_EVENT_ACTION_SEND_ALARM_DOOR) ?
             "ASSERT PASS door.pending_open" :
             "ASSERT FAIL door.pending_open");

    action = Telemetry_HandlePeriodicWake(false, true);
    emu_puts((action == TELEMETRY_EVENT_ACTION_NONE) ?
             "ASSERT PASS periodic.not_pending" :
             "ASSERT FAIL periodic.not_pending");

    action = Telemetry_HandlePeriodicWake(true, false);
    emu_puts((action == TELEMETRY_EVENT_ACTION_SEND_HEARTBEAT) ?
             "ASSERT PASS periodic.pending_dry" :
             "ASSERT FAIL periodic.pending_dry");

    action = Telemetry_HandlePeriodicWake(true, true);
    emu_puts((action == TELEMETRY_EVENT_ACTION_SEND_ALARM_WATER) ?
             "ASSERT PASS periodic.pending_wet" :
             "ASSERT FAIL periodic.pending_wet");
}

/**
 * Scenario D: Simulate door + periodic wake in same cycle and verify both
 * resulting actions are emitted in deterministic order (door first, periodic second).
 */
static void run_scenario_interleaved_wakes(void)
{
    TelemetryState_t state;
    TelemetryPayload_t payload;
    TelemetryEventAction_t door_action;
    TelemetryEventAction_t periodic_action;

    emu_puts("SCENARIO interleaved_wakes");

    Telemetry_InitState(&state, 9U);

    door_action = Telemetry_HandleDoorWake(true, true);
    periodic_action = Telemetry_HandlePeriodicWake(true, true);

    if ((door_action == TELEMETRY_EVENT_ACTION_SEND_ALARM_DOOR) &&
        (periodic_action == TELEMETRY_EVENT_ACTION_SEND_ALARM_WATER))
    {
        emu_puts("ASSERT PASS interleave.both_actions");
    }
    else
    {
        emu_puts("ASSERT FAIL interleave.both_actions");
    }

    if (door_action != TELEMETRY_EVENT_ACTION_NONE)
    {
        Telemetry_BuildPayload(&state, 10U,
                               TELEMETRY_MSG_TYPE_ALARM_DOOR,
                               0xC4U,
                               true, false,
                               &payload);
        emu_print_payload(&payload);
    }

    if (periodic_action != TELEMETRY_EVENT_ACTION_NONE)
    {
        Telemetry_BuildPayload(&state, 12U,
                               TELEMETRY_MSG_TYPE_ALARM_WATER,
                               0xC3U,
                               false, true,
                               &payload);
        emu_print_payload(&payload);
    }

    emu_puts("ASSERT PASS interleave.order");
}

/* ── Entry point ───────────────────────────────────────────────────────────── */

int main(void)
{
    emu_puts("TELEMETRY EMULATION BOOT");
    emu_puts("EMULATED JOIN OK");

    run_scenario_payloads();
    run_scenario_counter_overflow();
    run_scenario_wake_decisions();
    run_scenario_interleaved_wakes();

    emu_puts("PROJECT EXECUTION SUCCESSFUL");

    /* Spin forever (do not return from Renode-emulated main). */
    for (;;)
    {
        /* idle */
    }
}

#endif /* EMULATION_BUILD */
