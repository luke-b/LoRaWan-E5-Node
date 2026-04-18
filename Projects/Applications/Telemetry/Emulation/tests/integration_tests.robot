*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Resource        ${RENODEKEYWORDS}

*** Variables ***
${REPL}         ${CURDIR}/../renode/wio_e5_telemetry_emulation.repl
${ELF}          ${CURDIR}/../../telemetry_emulation.elf
${UART}         sysbus.uart1

*** Keywords ***
Create Machine
    Execute Command          mach create
    Execute Command          machine LoadPlatformDescription @${REPL}
    Execute Command          sysbus LoadELF @${ELF}
    Create Terminal Tester   ${UART}

*** Test Cases ***
Should Boot Emulated Telemetry Harness
    Create Machine
    Start Emulation
    Wait For Line On Uart    TELEMETRY EMULATION BOOT
    Wait For Line On Uart    EMULATED JOIN OK

Should Validate Heartbeat And Alarm Payloads
    Create Machine
    Start Emulation
    Wait For Line On Uart    SCENARIO heartbeat
    Wait For Line On Uart    PAYLOAD 01 00 00 00 00 C8 00 confirmed=false
    Wait For Line On Uart    SCENARIO door_alarm
    Wait For Line On Uart    PAYLOAD 02 00 00 00 05 C7 01 confirmed=true
    Wait For Line On Uart    SCENARIO water_alarm
    Wait For Line On Uart    PAYLOAD 03 00 00 00 09 C6 02 confirmed=true

Should Validate Counter Overflow Handling
    Create Machine
    Start Emulation
    Wait For Line On Uart    SCENARIO counter_overflow
    Wait For Line On Uart    PAYLOAD 01 00 00 00 05 C5 00 confirmed=false
    Wait For Line On Uart    ASSERT PASS overflow.pulses

Should Validate Wake Decision Gating
    Create Machine
    Start Emulation
    Wait For Line On Uart    SCENARIO wake_decisions
    Wait For Line On Uart    ASSERT PASS door.not_pending
    Wait For Line On Uart    ASSERT PASS door.pending_closed
    Wait For Line On Uart    ASSERT PASS door.pending_open
    Wait For Line On Uart    ASSERT PASS periodic.not_pending
    Wait For Line On Uart    ASSERT PASS periodic.pending_dry
    Wait For Line On Uart    ASSERT PASS periodic.pending_wet

Should Validate Interleaved Door And Periodic Wakes
    Create Machine
    Start Emulation
    Wait For Line On Uart    SCENARIO interleaved_wakes
    Wait For Line On Uart    ASSERT PASS interleave.both_actions
    Wait For Line On Uart    PAYLOAD 02 00 00 00 01 C4 01 confirmed=true
    Wait For Line On Uart    PAYLOAD 03 00 00 00 03 C3 02 confirmed=true
    Wait For Line On Uart    ASSERT PASS interleave.order
    Wait For Line On Uart    PROJECT EXECUTION SUCCESSFUL