# Telemetry pro Wio-E5

Jazyk: Cesky | [English](README.md)

Embedded aplikace ve stylu produkčního nasazení pro monitoring šachty s nízkou spotřebou, událostmi a LoRaWAN uplinky.

Můžete si ji představit takto: pořád poslouchá, většinou spí, občas promluví.

## Stručně
- Platforma: Wio-E5 (STM32WLE5xx)
- Doména: telemetrie šachty (počítání pulzů, alarm dveří, detekce vody)
- Síť: LoRaWAN
- Model spotřeby: preferuje hluboký spánek (STOP2)
- Build systém: Make + arm-none-eabi-gcc
- Artefakty: telemetry.elf, telemetry.hex, telemetry.bin

## Funkční specifikace

### Hlavní funkce
1. Asynchronní počítání pulzů pomocí LPTIM1
2. Alarm otevření dveří přes EXTI trigger
3. Kontrola úniku vody přes ADC se spínanou excitací
4. Periodický heartbeat uplink
5. Nízkoenergetický provoz přes STOP2 wake/sleep cyklus

### Model chování
- Event-driven hlavní smyčka s přechody stavů řízenými příznaky
- Přerušení jsou lehká a práci odkládají do logiky ve smyčce
- LoRaWAN zprávy se odesílají podle priority:
  - nejdřív alarmy
  - heartbeat periodicky

### Payload kontrakt (aktuální aplikace)
- Byte 0: typ zprávy
- Byte 1-4: kumulativní čítač pulzů (big-endian)
- Byte 5: úroveň baterie
- Byte 6: stavový bitfield

Typy zpráv:
- 0x01 heartbeat
- 0x02 alarm dveří
- 0x03 alarm vody

## Orientace v architektuře pro nováčky

### Velký obrázek
Telemetry skládá pět vrstev:
1. Aplikační logika v Core/Src/main.c
2. Podpůrné periferní jednotky (timer_if, adc_if, sys_app, usart_if, subghz atd.)
3. LoRaWAN aplikační glue v LoRaWAN/App
4. LoRaWAN middleware + SubGHz radio stack v Middlewares
5. HAL/CMSIS/BSP základ v Drivers

### Kde začít číst
1. Core/Src/main.c pro chování na nejvyšší úrovni
2. LoRaWAN/App/lora_app.c pro join/send logiku
3. Makefile pro pochopení skládání závislostí
4. docs/PROGRAMMERS_GUIDE.md pro hlubší interní detaily

## Informace o buildu

### Požadavky
- toolchain arm-none-eabi-gcc
- make

### Build příkaz
```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/Telemetry
make
```

### Build výstupy
- telemetry.elf
- telemetry.hex
- telemetry.bin
- telemetry.map

## Flash Wio-E5 z Windows přes USB-C

Níže uvedený postup je detailní a určený pro nováčky.

### Varianta A (doporučeno pro USB-C): UART bootloader přes STM32CubeProgrammer

#### 1. Nainstalujte software ve Windows
- STM32CubeProgrammer
- USB-UART ovladač pro bridge čip desky (CP210x/CH340 dle potřeby)

#### 2. Přepněte desku do STM32 ROM bootloader režimu
Typická sekvence:
1. Podržte BOOT tlačítko (nebo nastavte BOOT0)
2. Krátce stiskněte RESET
3. Pusťte BOOT

#### 3. Najděte sériový port
V Device Manageru zjistěte COMx přiřazený Wio-E5.

#### 4. Nahrajte firmware v STM32CubeProgrammer
1. Spusťte STM32CubeProgrammer
2. Vyberte UART
3. Port = COMx
4. Baud = 115200 (nebo podporovaná hodnota)
5. Connect
6. Vyberte telemetry.hex
7. Download/Program
8. Vraťte desku do normálního boot režimu a resetujte

#### 5. Rychlé ověření
- Zařízení se připojí do LoRaWAN sítě
- Vyvolejte událost dveří a sledujte alarm uplink
- Ověřte periodické heartbeat chování

### Varianta B: ST-LINK (pokud ho váš vývojový kit obsahuje)
1. Připojte USB-C
2. Otevřete STM32CubeProgrammer
3. Vyberte ST-LINK
4. Připojte se a nahrajte telemetry.hex
5. Resetujte a sledujte běh aplikace

## Checklist bring-upu pro nováčky
- Build proběhne úspěšně
- Flash proběhne úspěšně
- Deska čistě naběhne
- LoRaWAN join proběhne úspěšně
- Cesty alarm i heartbeat jsou ověřené
- Sleep/wake cyklus se chová očekávaně

## Troubleshooting

### Build proběhne, ale nejsou uplinky
- Špatné LoRaWAN credentials/region
- Gateway není dostupná
- Problém s anténou nebo RF cestou

### Příliš mnoho probuzení, baterie se rychle vybíjí
- Šum na EXTI vstupu
- Neshoda debounce/pull konfigurace
- Logická cesta brání dlouhému pobytu ve STOP2

### CubeProgrammer UART se nepřipojí
- Deska není v bootloader režimu
- Špatný COM nebo baud
- Chybí USB-UART ovladač

## Poznámka k bezpečnosti a nasazení
Před nasazením v terénu proveďte environmentální testy vlhkosti, stability napájení a chování při poruchách senzorů.

## Související dokumentace
- docs/USER_GUIDE.md
- docs/PROGRAMMERS_GUIDE.md
- docs/COOKBOOK.md
