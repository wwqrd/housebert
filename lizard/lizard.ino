#include <JeeLib.h>

// Hardware (Jeenode ports)
// Digital P1
#define LED_PIN 4
// Analog  P4
#define CT_PIN 3

// RF
byte node;

typedef struct {
  byte node;
  long reading;
} Payload;

// Not sure if there is a good reason for this to be global, I suspect not.
Payload payload;

// Power
#define SAMPLES 1000
#define VRMS 230
#define SUPPLY_V 3.3
#define CT_TURNS_RATIO 1350
#define BURDEN_R 18

void blink() {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
}

void setupPins() {
    pinMode(LED_PIN, OUTPUT);
}

void setupRf() {
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);
    payload.node = rf12_config();
}

void setup() {

    Serial.begin(57600);
    Serial.println("`Lizard` is setting up...");

    Serial.println("Initialising pins...");
    setupPins();
    blink();

    Serial.println("Initialising rf...");
    setupRf();
    blink();

    Serial.println("Setup complete.");
}

double sampleIRMS() {
    /*
     * Theory
     * ======
     * Some stuff on working on calI, etc:
     * http://openenergymonitor.org/emon/buildingblocks/ct-and-ac-power-adaptor-installation-and-calibration-theory
     *
     * Code based (nearly exactly) on:
     * https://github.com/openenergymonitor/EmonLib/blob/master/EmonLib.cpp
     *
     * No idea what is actually going on here, renamed the variables to attempt to
     * make it clearer, believe the `0.996` constant is something to do with
     * accuracy/phaseli, and the `1000.0` constant is something to do with changing
     * units.
     */

    double sampleI0;
    double sampleI1 = 0;
    double filteredI0;
    double filteredI1 = 0;
    double dSample;
    double sqI;
    double sumI = 0;
    double iRMS;

    for(int n = 0; n < SAMPLES; n++) {
      sampleI0 = sampleI1;
      sampleI1 = analogRead(CT_PIN);
      filteredI0 = filteredI1;
      dSample = sampleI1 - sampleI0;
      filteredI1 = 0.996 * (filteredI0 + dSample);

      // Root-mean-square method current
      // 1) square current values
      sqI = filteredI1 * filteredI1;
      // 2) sum
      sumI += sqI;
    }

    // Could these two be tidied up a little?
    double calI = CT_TURNS_RATIO / BURDEN_R;
    double ratioI = calI * (SUPPLY_V / 1023.0 * 1000.0);

    iRMS = ratioI * sqrt(sumI / SAMPLES);

    return iRMS;
}

void send() {
    Serial.println("Sending message:");
    Serial.println(payload.reading);
    rf12_sendStart(0, &payload, sizeof payload);
}

void loop() {
    payload.reading = sampleIRMS();

    rf12_recvDone();

    if(rf12_canSend()) {
        send();
        blink();
    }

    delay(5000);

}

