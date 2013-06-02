#include <JeeLib.h>

#define BUTTON_PIN 1
#define LED_PIN 2
#define SPEAKER_PIN 3

char password[] = "123DOOP";
int button = false;

void unlock () {
    rf12_sendStart(0, password, sizeof password);
    button = false;
}

void blink () {
    digitalWrite(LED_PIN, 1);
    delay(100);
    digitalWrite(LED_PIN, 0);
}

void alert () {
}

void setup () {
    Serial.begin(57600);
    Serial.println("\nIAmAKeyBert");
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(SPEAKER_PIN, OUTPUT);
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);
}

void loop () {
    if(digitalRead(BUTTON_PIN) == 1) {
        digitalWrite(LED_PIN, 1);
        button = true;
    }

    rf12_recvDone();

    if(rf12_canSend() && button == true) {
        unlock();
        delay(100);
        digitalWrite(LED_PIN, 0);
    }
}
