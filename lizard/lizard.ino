#include <JeeLib.h>

/*
       RF12

    []      []
D4  []      []  A3
    []      []
    []      []
    []      []

    []      []
    []      []
    []      []
    []      []
    []      []

       FTDI
*/

#define LED_PIN 4
#define INDUCTOR_PIN 3

typedef struct {
  byte node;
  long reading;
} Payload;

Payload payload;

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

void send() {
    Serial.println("Sending message:");
    payload.reading = 1023 - analogRead(INDUCTOR_PIN);
    Serial.println(payload.reading);
    rf12_sendStart(0, &payload, sizeof payload);
}
void loop() {

    rf12_recvDone();

    if(rf12_canSend() && button == true) {
        send();
        blink();
    }

    delay(5000);

}

