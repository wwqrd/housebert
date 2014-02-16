#include <JeeLib.h>
#include <EmonLib.h>

// Hardware (Jeenode ports)
// Digital P1
#define LED_PIN 4
// Analog  P3 (2) P2 (1)
#define CT_PIN 1

// RF
byte node;

typedef struct {
  byte node;
  long value;
} Payload;

// Power
#define SAMPLES 1350
#define VRMS 230
#define CT_TURNS_RATIO 1350
#define BURDEN_R 18

EnergyMonitor energyMonitor;

void blink() {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
}

void initializeEnergyMonitor() {
    double ical = CT_TURNS_RATIO / BURDEN_R;
    energyMonitor.current(CT_PIN, ical);
}

void setupPins() {
    pinMode(LED_PIN, OUTPUT);
}

void setupRf() {
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);
    node = rf12_config();
}

void setup() {

    Serial.begin(9600);
    Serial.println("HOUSEBERT[Sensor node] is setting up...");

    Serial.println("Initialising pins...");
    setupPins();
    blink();

    Serial.println("Initialising RF12...");
    setupRf();
    blink();

    Serial.println("Initializing EnergyMonitor...");
    initializeEnergyMonitor();
    blink();

    Serial.println("Setup complete.");
}

void loop() {

    Payload payload;
    double irms = energyMonitor.calcIrms(SAMPLES);
    double apparentPower = irms * VRMS;

    payload.node = node;
    payload.value = apparentPower;

    rf12_recvDone();

    if(rf12_canSend()) {
      Serial.println("Apparent power:");
      Serial.println(payload.value);
      rf12_sendStart(0, &payload, sizeof payload);
      blink();
    }

    delay(2000);

}

