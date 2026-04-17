# Telemetry Cookbook

Practical recipes for day-to-day development.

## Recipe 1: Clean and full rebuild

```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/Telemetry
make clean
make
```

## Recipe 2: Verify final artifacts

```bash
ls -lh telemetry.elf telemetry.hex telemetry.bin telemetry.map
```

## Recipe 3: Inspect top memory consumers

```bash
arm-none-eabi-nm -S --size-sort telemetry.elf | tail -n 30
```

## Recipe 4: Check for duplicate callback symbols

```bash
arm-none-eabi-nm telemetry.elf | grep HAL_GPIO_EXTI_Callback
```

Should show a single definition.

## Recipe 5: Switch from confirmed to unconfirmed alarm uplinks
In main send function, replace CONFIRMED_MSG with UNCONFIRMED_MSG for alarm branches.

## Recipe 6: Add a second status byte
1. Increase payload size
2. Set AppData.BufferSize accordingly
3. Update backend decoder

## Recipe 7: Validate LoRaWAN region wiring in build
Search Makefile for region include path and enabled region macro in target config.

## Recipe 8: Reduce binary size
- Remove unused region sources if deployment is fixed to one region
- Keep only required package sources
- Keep compiler optimization and gc-sections enabled

## Recipe 9: Smoke test sequence after flash
1. Confirm boot does not hard fault
2. Confirm join attempt
3. Trigger door event and observe uplink
4. Trigger water check path
5. Confirm periodic heartbeat

## Recipe 10: Diagnose link errors quickly
When undefined references appear:
1. Locate symbol owner with ripgrep or nm
2. Add the owning source file to SRC_FILES
3. Rebuild and repeat

This repo uses many split utility and middleware units, so undefined references are usually missing source list entries, not C bugs.
