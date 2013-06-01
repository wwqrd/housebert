#include <JeeLib.h>

MilliTimer sendTimer;
char password[] = "123DOOP";

void setup () {
    Serial.begin(57600);
    Serial.println("\nIAmAKeyBert");
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);
}

void loop () {
    rf12_recvDone();
    if (rf12_canSend() && sendTimer.poll(3000)) {
        rf12_sendStart(0, password, sizeof password);
    }
}