#include <JeeLib.h>

byte recvCount;
char password[16] = "123DOOP";
char msg;
bool allGood;

void setup () {
    Serial.begin(57600);
    Serial.println("\nIAmALockBert");
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);
}

void loop () {
    if (rf12_recvDone() && rf12_crc == 0) {
        allGood = true;
        for (byte i = 0; i < rf12_len; ++i) {
            msg = (int) rf12_data[i];
            if(msg != password[i]) {
                allGood = false;
            }
        }
        if(allGood) {
            Serial.println("YEY!");
            // Unlock
        } else {
            Serial.println("NOT YEY!");
            // Alarms!
        }
    }
}
