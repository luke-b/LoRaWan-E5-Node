# HelloWorld Cookbook

Practical recipes for common tasks.

## Recipe 1: Clean rebuild from scratch

```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/HelloWorld
make clean
make
```

## Recipe 2: Show binary size quickly

```bash
arm-none-eabi-size hello_world.elf
```

## Recipe 3: Inspect symbols

```bash
arm-none-eabi-nm -n hello_world.elf | head -n 40
```

## Recipe 4: Verify UART message in firmware binary

```bash
strings hello_world.elf | grep -i "hello world"
```

## Recipe 5: Switch to 115200 baud
1. Edit UART baud in Core/Src/main.c
2. Rebuild and reflash
3. Update terminal baud to 115200

## Recipe 6: Make message text configurable
Create a macro in Core/Inc/main.h:

```c
#define HELLO_TEXT "Hello World z Wio-E5!\r\n"
```

Use it from main.c transmit code.

## Recipe 7: Add a heartbeat counter
Use a static counter in loop and format text with tiny snprintf or fixed-width conversion.

## Recipe 8: Fast sanity check after hardware changes
- Build
- Flash
- Confirm repeated UART output
- Confirm no hard fault resets

If this works, your board bring-up remains healthy.
