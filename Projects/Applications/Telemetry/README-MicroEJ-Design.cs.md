# Technický návrh: Migrace aplikace Telemetry na MicroEJ (STM32WLE5)

Tento dokument popisuje detailní technický návrh migrace stávající C aplikace "Telemetry" na architekturu využívající MicroEJ VEE (Virtual Execution Environment) pro modul Wio-E5 (MCU STM32WLE5JC). Cílem je implementovat plnou podporu MicroEJ Javy a přesunout do ní veškerou byznys logiku.

## 1. Architektura a rozdělení vrstev

Po migraci se aplikace rozdělí na tři hlavní vrstvy:

1.  **BSP (Board Support Package) & RTOS (C vrstva):**
    *   Založeno na aktuálním kódu s využitím STM32Cube HAL a LoRaWAN middleware.
    *   Bude začleněn FreeRTOS (nezbytný pro běh MicroEJ Core Engine).
    *   Obsahuje inicializaci hardwaru (ADC, LPTIM1, EXTI, RTC, SubGHz).
    *   Stará se o hluboký spánek (STOP2) a probouzení.

2.  **MicroEJ VEE Port (C/Java propojení):**
    *   MicroEJ Core Engine (LLJVM) běžící jako úloha ve FreeRTOS.
    *   **SNI (Simple Native Interface):** C API pro volání hardwarových funkcí z Javy (a naopak). Zde se napojí Java metody na C funkce pro čtení senzorů a odesílání LoRaWAN zpráv.

3.  **Byznys logika (Java vrstva):**
    *   Hlavní aplikační smyčka, zpracování událostí (Heartbeat, Door Alarm, Water Leak).
    *   Sestavování LoRaWAN payloadu (včetně bitových operací pro big-endian pulzy a status byte).
    *   Rozhodovací logika pro odeslání zpráv.

## 2. Krok 1: Klonování a příprava projektu

1.  **Naklonování adresáře:** Zkopírujte složku `Telemetry` jako `Telemetry-MicroEJ`.
2.  **Zavedení FreeRTOS:** Stávající "bare-metal" smyčka (s `HAL_SuspendTick` a `HAL_PWREx_EnterSTOP2Mode`) se musí přepsat tak, aby využívala FreeRTOS "tickless idle" režim. FreeRTOS převezme kontrolu nad přechodem do STOP2 módu, když nejsou připraveny žádné úlohy (včetně úlohy MicroEJ).
3.  **Příprava CMake / Makefile:** Úprava buildovacího systému (Makefile) pro integraci MicroEJ knihoven (`microejruntime.a` a `microejapp.o`), které vygeneruje MicroEJ Module Manager.

## 3. Krok 2: Vytvoření MicroEJ VEE Portu

Pomocí MicroEJ SDK (případně MicroEJ Module Manageru přes Gradle) vytvoříme VEE Port:

1.  **Architektura:** Výběr architektury `ARM Cortex-M4` (kompatibilní s Cortex-M4F v STM32WLE5) a příslušného C kompilátoru (GCC `arm-none-eabi-gcc`).
2.  **Packs:** Přidání základních balíčků (Core, SNI). Zvažte balíček "Event Queue", pokud by se hodil pro asynchronní zprávy, i když pro jednoduchou telemetrii postačí SNI volání a nativní přerušení odemykající Java vlákna.
3.  **BSP Connection:** Konfigurace cest v MicroEJ SDK tak, aby výstupní soubory (`microejapp.o`, hlavičkové soubory) byly generovány do složky `Telemetry-MicroEJ/MicroEJ/`.

## 4. Krok 3: Návrh SNI (Simple Native Interface)

Zde definujeme rozhraní mezi Javou a C.

**Java deklarace (např. `TelemetryNatives.java`):**
```java
package com.example.telemetry;

public class TelemetryNatives {
    // Čtení senzorů (volá C funkce)
    public static native int getWaterPulses();
    public static native boolean checkWaterLeak();
    public static native int getBatteryLevel();
    public static native boolean isDoorOpen();

    // Odeslání zprávy přes LoRaWAN
    public static native void sendLoraMessage(int type, byte[] payload);

    // Blokující čekání na další událost (probuzení)
    // Vrací bitmasku událostí: Bit 0 = Heartbeat, Bit 1 = Dveře
    public static native int waitForEvent();
}
```

**C implementace (např. `sni_telemetry.c`):**
```c
#include "sni.h"
#include "main.h" // Obsahuje aktuální logiku a globální proměnné

extern uint32_t total_water_pulses;
extern uint8_t CheckWaterLeak(void);
// ... další deklarace

jint Java_com_example_telemetry_TelemetryNatives_getWaterPulses(void) {
    UpdatePulseCounter(); // Zajistí nejaktuálnější hodnotu z HW LPTIM
    return (jint)total_water_pulses;
}

jboolean Java_com_example_telemetry_TelemetryNatives_checkWaterLeak(void) {
    return (jboolean)(CheckWaterLeak() != 0);
}

void Java_com_example_telemetry_TelemetryNatives_sendLoraMessage(jint type, jbyte* payload) {
    // Použije C LoRaWAN API (LmHandler) pro odeslání.
    // Pozor: Z důvodu asynchronnosti LmHandleru by toto volání mělo buď čekat na potvrzení (TX_DONE),
    // nebo zprávu jen zařadit do fronty C vrstvy.
    // Musí se ošetřit převod jbyte* (což je pole v Jave) na C buffer.
    jint length = SNI_getArrayLength(payload);
    // ... vytvoření LmHandlerAppData_t a volání LmHandlerSend ...
}
```

### Řešení asynchronních událostí a spánku (STOP2)

Jelikož je zařízení bateriově napájeno, CPU musí spát ve STOP2.
1.  **V C vrstvě:** Nativní přerušení (EXTI pro dveře, RTC pro Heartbeat) probudí MCU. Nastaví příznak (např. ve FreeRTOS Task Notification nebo EventGroup).
2.  **Volání `waitForEvent()` v Javě:** Tato metoda na straně C zavolá `SNI_suspendCurrentJavaThread()` a čeká na událost. Vlákno MicroEJ se uspí.
3.  **Probuzení Javy:** Když přerušení (nebo C úloha po přerušení) zachytí událost, zavolá `SNI_resumeJavaThread()`, čímž se Java probudí, vyhodnotí příznak a provede logiku.

## 5. Krok 4: Byznys logika v Javě

Hlavní logika se přesune z `main.c` do Javy.

```java
package com.example.telemetry;

public class TelemetryApp {

    private static final int MSG_TYPE_HEARTBEAT   = 0x01;
    private static final int MSG_TYPE_ALARM_DOOR  = 0x02;
    private static final int MSG_TYPE_ALARM_WATER = 0x03;

    private static final int EVENT_HEARTBEAT = 1 << 0;
    private static final int EVENT_DOOR      = 1 << 1;

    public static void main(String[] args) {
        while (true) {
            int eventMask = TelemetryNatives.waitForEvent(); // Uspí se do STOP2

            if ((eventMask & EVENT_DOOR) != 0) {
                handleDoorEvent();
            }
            if ((eventMask & EVENT_HEARTBEAT) != 0) {
                handleHeartbeatEvent();
            }
        }
    }

    private static void handleDoorEvent() {
        // Softwarový debounce se dá řešit i v Javě pomocí Thread.sleep(),
        // ale efektivnější na spotřebu je nechat jej v C (hardware timer).
        if (TelemetryNatives.isDoorOpen()) {
            sendPayload(MSG_TYPE_ALARM_DOOR);
        }
    }

    private static void handleHeartbeatEvent() {
        if (TelemetryNatives.checkWaterLeak()) {
            sendPayload(MSG_TYPE_ALARM_WATER);
        } else {
            sendPayload(MSG_TYPE_HEARTBEAT);
        }
    }

    private static void sendPayload(int msgType) {
        int pulses = TelemetryNatives.getWaterPulses();
        int battery = TelemetryNatives.getBatteryLevel();
        boolean doorOpen = TelemetryNatives.isDoorOpen();

        byte[] payload = new byte[7];
        payload[0] = (byte) msgType;

        // Big Endian kodovani pulzu
        payload[1] = (byte) (pulses >>> 24);
        payload[2] = (byte) (pulses >>> 16);
        payload[3] = (byte) (pulses >>> 8);
        payload[4] = (byte) (pulses);

        payload[5] = (byte) battery;

        payload[6] = 0x00;
        if (doorOpen) {
            payload[6] |= (1 << 0);
        }

        TelemetryNatives.sendLoraMessage(msgType, payload);
    }
}
```

## 6. Integrace a Spouštění (Start-up sekvence)

V `main.c` se kód upraví následovně:

```c
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_RTC_Init();
    MX_LPTIM1_Init();
    MX_ADC_Init();
    MX_LoRaWAN_Init();

    HAL_LPTIM_Counter_Start(&hlptim1, 0xFFFF);

    // Vytvoření MicroEJ úlohy (FreeRTOS)
    xTaskCreate(microej_task, "MicroEJ", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);

    // Spuštění FreeRTOS plánovače
    vTaskStartScheduler();

    while(1); // Sem by program neměl dojít
}

void microej_task(void *pvParameters) {
    void* vm = SNI_createVM();
    if (vm != NULL) {
        SNI_startVM(vm, 0, NULL); // Spustí Java main()
        SNI_destroyVM(vm);
    }
    vTaskDelete(NULL);
}
```

## 7. Kompilace a sestavení

Proces sestavení bude dvoufázový:
1.  Sestavení MicroEJ aplikace pomocí Gradle/Ant, což vygeneruje `microejapp.o`.
2.  Spuštění existujícího `make` v adresáři projektu C, který linkuje původní C soubory, HAL, LoRaWAN middleware, FreeRTOS, `microejapp.o` a `microejruntime.a` do výsledného `telemetry.elf/bin`.

## 8. Doporučení pro vývoj

*   **RAM/Flash optimalizace:** STM32WLE5JC má 256KB Flash a 64KB RAM. Oříznutí FreeRTOS a MicroEJ funkcí, které nejsou potřeba (GUI, složité sítě), je klíčové, aby se do paměti vešel LoRaWAN stack i Java engine.
*   **Spotřeba:** "Tickless idle" mód FreeRTOSu musí být správně nakonfigurován tak, aby automaticky přecházel do `STOP2` (funkce `PreSleepProcessing` a `PostSleepProcessing`). MicroEJ musí respektovat tento spánek a neprobouzet procesor vlastním vnitřním časovačem častěji, než je nutné.
