#include <JeeLib.h>

#define RELAY_PIN 1
#define LED_PIN 2

byte recvCount;
char password[16] = "123DOOP";
char msgChar;
bool allGood;

void unlock (int time) {
    digitalWrite(RELAY_PIN, 1);
    delay(time);
    digitalWrite(RELAY_PIN, 0);
}

void blink () {
    digitalWrite(LED_PIN, 1);
    delay(100);
    digitalWrite(LED_PIN, 0);
}

bool checkPassword () {
    allGood = true;
    for (byte i = 0; i < rf12_len; ++i) {
        msgChar = (int) rf12_data[i];
        if(msgChar != password[i]) {
            allGood = false;
        }
    }
    return allGood;
}

void setup () {
    Serial.begin(57600);
    Serial.println("\nIAmALockBert");
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);
}

void loop () {
    if (rf12_recvDone() && rf12_crc == 0) {
        if(checkPassword()) {
            // Unlock
            Serial.println("YEY!");
            blink();
            unlock(5000);
        } else {
            // Alarms!
            Serial.println("NOT YEY!");
        }
    }
}
