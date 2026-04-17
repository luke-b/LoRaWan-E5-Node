# HelloWorld pro Wio-E5

Jazyk: Cesky | [English](README.md)

Malý, přívětivý firmware bez kouzel, který ověří, že deska, toolchain i sériová linka fungují.

Když tato aplikace běží, základ projektu je zdravý.

## Stručně
- MCU: STM32WLE5xx (Wio-E5)
- Chování: každou sekundu odešle řádek přes UART
- Výstupní text: Hello World z Wio-E5!
- Build systém: Make + arm-none-eabi-gcc
- Artefakty: hello_world.elf, hello_world.hex, hello_world.bin

## Funkční specifikace

### Cíle
- Ověřit start systému, hodiny a inicializaci HAL
- Ověřit konfiguraci pinů USART1 a elektrické zapojení
- Poskytnout předvídatelný sériový heartbeat pro bring-up

### Chování za běhu
1. Start a inicializace HAL/systémových hodin
2. Inicializace USART1
3. Vstup do nekonečné smyčky
4. Odeslání jednoho řádku přes UART
5. Zpoždění ~1000 ms
6. Opakovat donekonečna

### Rozhraní
- UART TX: PB6
- UART RX: PB7
- Výchozí baudrate: 9600 8N1

## Struktura projektu
- Core/Inc: hlavičkové soubory
- Core/Src: zdrojové kódy firmware
- Makefile: vstupní bod buildu
- STM32WLE5JCIX_FLASH.ld: linker script
- startup_stm32wle5jcix.s: startup assembler
- docs/: podrobnější dokumentace pro uživatele i vývojáře

## Informace o buildu

### Požadavky
- toolchain arm-none-eabi-gcc
- make

### Build příkaz
```bash
cd /workspaces/LoRaWan-E5-Node/Projects/Applications/HelloWorld
make
```

### Build výstupy
- hello_world.elf (obraz pro debug)
- hello_world.hex (běžný obraz pro flash)
- hello_world.bin (raw binární obraz)
- hello_world.map (linker mapa)

## Flash Wio-E5 z Windows přes USB-C

Tato část je psaná pro nováčky a předpokládá pouze USB-C kabel.

### Varianta A (doporučeno pro USB-C): STM32 ROM bootloader přes UART
Použijte, pokud vaše Wio-E5 deska zpřístupňuje USB-UART přes USB-C konektor.

#### 1. Nainstalujte nástroje ve Windows
- Nainstalujte STM32CubeProgrammer (GUI + CLI)
- Pokud je potřeba, nainstalujte USB-UART ovladač (CP210x nebo CH340 dle desky)

#### 2. Přepněte desku do bootloader režimu
Přesné názvy tlačítek se liší podle revize desky, obvykle ale platí:
1. Podržte tlačítko BOOT (nebo nastavte BOOT0 na high)
2. Stiskněte a pusťte RESET
3. Pusťte BOOT

Pokud má deska samostatná tlačítka BOOT a RST, je to rychlé.

#### 3. Najděte COM port
V Device Manageru zjistěte COMx přiřazený desce.

#### 4. Nahrajte firmware v STM32CubeProgrammer GUI
1. Otevřete STM32CubeProgrammer
2. Vyberte UART connection
3. Nastavte Port = COMx
4. Baud = 115200 (nebo hodnota podporovaná deskou)
5. Connect
6. Otevřete soubor hello_world.hex
7. Download/Program
8. Resetujte desku do normálního boot režimu

#### 5. Ověřte výsledek
Otevřete sériový terminál na 9600 8N1 a potvrďte opakující se výpis.

### Varianta B: ST-LINK (pokud dev kit obsahuje onboard ST-LINK)
1. Připojte USB-C
2. Otevřete STM32CubeProgrammer
3. Vyberte ST-LINK
4. Connect
5. Nahrajte hello_world.hex
6. Reset a test UART

## Rychlý checklist ověření
- Build doběhne bez chyb
- Firmware se úspěšně nahraje
- UART terminál zobrazuje periodické hello hlášení
- Interval zpráv je přibližně 1 sekunda

## Časté problémy a řešení

### Není žádný sériový výstup
- Špatný COM port
- Špatná baudrate
- Prohozené TX/RX
- Deska je stále v bootloader režimu

### Nelze se připojit v CubeProgrammer (UART)
- Bootloader režim nebyl aktivován správně
- Špatný COM nebo baud
- Chybí USB-UART ovladač

### Build hlásí chybějící compiler
Nainstalujte toolchain arm-none-eabi a ověřte, že je na PATH.

## Co dál
- Pro interní detaily čtěte docs/PROGRAMMERS_GUIDE.md
- Pro praktické postupy čtěte docs/COOKBOOK.md
- Pro low-power + LoRaWAN scénář přejděte na projekt Telemetry
