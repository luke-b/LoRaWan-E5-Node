#include <Arduino.h>
#include <STM32LowPower.h>
#include <STM32RTC.h>
#include <RadioLib.h>

// --- PINS (Wio-E5 based) ---
#define DOOR_CONTACT_PIN PB1
#define PULSE_INPUT_PIN PB0
#define WLD_VCC_PIN PA5
#define WLD_ADC_PIN PA4

// --- LORAWAN KEYS ---
uint64_t joinEUI = 0x0000000000000000;
uint64_t devEUI  = 0x0000000000000000;
uint8_t nwkKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// RadioLib setup for STM32WL
STM32WLx radio = new STM32WLx_Module();
LoRaWANNode node(&radio, &EU868);

// --- MSG TYPES ---
#define MSG_TYPE_HEARTBEAT      0x01
#define MSG_TYPE_ALARM_DOOR     0x02
#define MSG_TYPE_ALARM_WATER    0x03

// --- STATE ---
volatile bool flag_door_alarm = false;
volatile bool flag_rtc_wakeup = false;
uint32_t totalWaterPulses = 0;
uint16_t lastLptimValue = 0;

STM32RTC& rtc = STM32RTC::getInstance();

void initPulseCounter() {
    __HAL_RCC_LPTIM1_CLK_ENABLE();

    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

    // pinMode and pin_function with right enums
    pinMode(PULSE_INPUT_PIN, INPUT_PULLUP);
    pin_function(digitalPinToPinName(PULSE_INPUT_PIN), STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_LPTIM1));

    LPTIM1->CR = 0;
    LPTIM1->CFGR = LPTIM_CFGR_COUNTMODE;
    LPTIM1->CR = LPTIM_CR_ENABLE;
    LPTIM1->ARR = 0xFFFF;
    LPTIM1->CR |= LPTIM_CR_CNTSTRT;
}

uint16_t getPulseCount() {
    return LPTIM1->CNT;
}

void updatePulseCounter() {
    uint16_t currentCounter = getPulseCount();
    if (currentCounter >= lastLptimValue) {
        totalWaterPulses += (uint32_t)(currentCounter - lastLptimValue);
    } else {
        totalWaterPulses += (uint32_t)((0xFFFFU - lastLptimValue) + currentCounter + 1U);
    }
    lastLptimValue = currentCounter;
}

bool checkWaterLeak() {
    digitalWrite(WLD_VCC_PIN, HIGH);
    delay(10);
    int val = analogRead(WLD_ADC_PIN);
    digitalWrite(WLD_VCC_PIN, LOW);
    return (val > 2000);
}

uint8_t getBatteryLevel() {
    return 254;
}

void doorInterrupt() {
    flag_door_alarm = true;
}

void rtcInterrupt(void *data) {
    flag_rtc_wakeup = true;
}

void sendLoraMessage(uint8_t msgType) {
    updatePulseCounter();

    bool doorOpen = (digitalRead(DOOR_CONTACT_PIN) == HIGH);
    bool waterDetected = checkWaterLeak();
    uint8_t batteryLevel = getBatteryLevel();

    uint8_t payload[7];
    payload[0] = msgType;
    payload[1] = (uint8_t)((totalWaterPulses >> 24) & 0xFF);
    payload[2] = (uint8_t)((totalWaterPulses >> 16) & 0xFF);
    payload[3] = (uint8_t)((totalWaterPulses >> 8) & 0xFF);
    payload[4] = (uint8_t)(totalWaterPulses & 0xFF);
    payload[5] = batteryLevel;
    payload[6] = 0;

    if (doorOpen) {
        payload[6] |= 1;
    }
    if (waterDetected) {
        payload[6] |= 2;
    }

    Serial.print("Sending LoRaWAN payload: ");
    for(int i=0; i<7; i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    int16_t state;
    if (msgType == MSG_TYPE_HEARTBEAT) {
        state = node.sendReceive(payload, 7, 1, false);
    } else {
        state = node.sendReceive(payload, 7, 1, true);
    }

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Send success.");
    } else {
        Serial.print("Send failed, code: ");
        Serial.println(state);
    }
}


void setup() {
    Serial.begin(115200);
    Serial.println("Telemetry App Booting...");

    pinMode(WLD_VCC_PIN, OUTPUT);
    digitalWrite(WLD_VCC_PIN, LOW);
    pinMode(DOOR_CONTACT_PIN, INPUT_PULLDOWN);

    initPulseCounter();

    int16_t state = radio.begin();
    if(state != RADIOLIB_ERR_NONE) {
        Serial.println("Radio init failed");
        while(true);
    }

    // Begin node and then activate OTAA (RadioLib v7+)
    node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
    state = node.activateOTAA();
    if(state != RADIOLIB_ERR_NONE) {
        Serial.println("Join failed");
        while(true);
    }

    LowPower.begin();
    rtc.begin();
    rtc.attachInterrupt(rtcInterrupt);
    LowPower.attachInterruptWakeup(DOOR_CONTACT_PIN, doorInterrupt, RISING);

    // Set initial 24h alarm
    uint32_t nowEpoch = rtc.getEpoch();
    rtc.setAlarmEpoch(nowEpoch + 86400);

    sendLoraMessage(MSG_TYPE_HEARTBEAT);
}

void loop() {
    if (flag_door_alarm) {
        flag_door_alarm = false;
        delay(50);
        if (digitalRead(DOOR_CONTACT_PIN) == HIGH) {
            Serial.println("DOOR ALARM!");
            sendLoraMessage(MSG_TYPE_ALARM_DOOR);
        }
    }

    if (flag_rtc_wakeup) {
        flag_rtc_wakeup = false;
        if (checkWaterLeak()) {
            Serial.println("WATER ALARM!");
            sendLoraMessage(MSG_TYPE_ALARM_WATER);
        } else {
            Serial.println("HEARTBEAT");
            sendLoraMessage(MSG_TYPE_HEARTBEAT);
        }

        uint32_t nowEpoch = rtc.getEpoch();
        rtc.setAlarmEpoch(nowEpoch + 86400); // 24 hours
    }

    Serial.println("Going to sleep...");
    delay(10);
    LowPower.deepSleep();

    Serial.println("Woke up!");
}
