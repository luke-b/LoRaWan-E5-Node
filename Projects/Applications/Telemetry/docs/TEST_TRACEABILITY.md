# Telemetry Business Logic Traceability

This matrix tracks business requirements to automated tests and CI evidence.

| Requirement | Description | Unit Test | Integration/Emulation | CI Workflow |
|---|---|---|---|---|
| REQ-001 | 24h heartbeat message policy | Emulation/tests/unit/test_telemetry_app.c | Emulation/tests/integration_tests.robot | .github/workflows/telemetry-emulation.yml |
| REQ-002 | Door alarm is confirmed message | Emulation/tests/unit/test_telemetry_app.c | Emulation/tests/integration_tests.robot | .github/workflows/telemetry-emulation.yml |
| REQ-003 | Water alarm is confirmed message | Emulation/tests/unit/test_telemetry_app.c | Emulation/tests/integration_tests.robot | .github/workflows/telemetry-emulation.yml |
| REQ-004 | 16-bit LPTIM rollover accumulation | Emulation/tests/unit/test_telemetry_app.c | Emulation/tests/integration_tests.robot | .github/workflows/telemetry-emulation.yml |
| REQ-005 | Event flag consumption/clearing | Emulation/tests/unit/test_telemetry_logic.c | N/A (unit-covered rule) | .github/workflows/telemetry-emulation.yml |
| REQ-006 | Door event decision logic | Emulation/tests/unit/test_telemetry_logic.c | N/A (unit-covered rule) | .github/workflows/telemetry-emulation.yml |
| REQ-007 | Periodic wake decision logic (water vs heartbeat) | Emulation/tests/unit/test_telemetry_logic.c | N/A (unit-covered rule) | .github/workflows/telemetry-emulation.yml |
| REQ-008 | Door debounce gating (pending + sampled state) | Emulation/tests/unit/test_telemetry_logic.c | Emulation/tests/integration_tests.robot (wake_decisions) | .github/workflows/telemetry-emulation.yml |
| REQ-009 | RTC periodic gating (pending + water decision) | Emulation/tests/unit/test_telemetry_logic.c | Emulation/tests/integration_tests.robot (wake_decisions) | .github/workflows/telemetry-emulation.yml |
| REQ-010 | Concurrent wake handling without action loss (door + periodic) | Emulation/tests/unit/test_telemetry_logic.c | Emulation/tests/integration_tests.robot (interleaved_wakes) | .github/workflows/telemetry-emulation.yml |

## Coverage Policy

- Every new business rule must add/update a `REQ-*` row.
- Every `REQ-*` row must map to at least one automated test.
- CI summary should report pass/fail for all linked test suites.
