#include <JeeLib.h>

/*
       RF12

    []      []
D4  []      []  D7
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

#define INDUCTOR_PIN 7
#define LED_PIN 4

byte recvCount;
char password[16] = "123DOOP";
char msgChar;
bool allGood;
int button = false;
bool wasOpened = false;
long unlocked = 0;
long now = 0;
Stash stash;
byte Ethernet::buffer[500];
BufferFiller bfill;

void unlock () {
    wasOpened = true;
    unlocked = millis();
    rf12_sendStart(0, password, sizeof password);
    blink();
    button = false;
}

void unlock (int time) {
    digitalWrite(LED_PIN, 1);
    digitalWrite(RELAY_PIN, 1);
    delay(time);
    digitalWrite(RELAY_PIN, 0);
    digitalWrite(LED_PIN, 0);
}

void blink () {
    digitalWrite(LED_PIN, 1);
    delay(150);
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
    blink();
}

void loop () {
    if (rf12_recvDone() && rf12_crc == 0) {
        if(checkPassword()) {
            // Unlock
            Serial.println("YEY!");
            unlock(5000);
        } else {
            // Alarms!
            Serial.println("NOT YEY!");
        }
    }
}
