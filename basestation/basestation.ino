#include <JeeLib.h>
#include <EtherCard.h>

#define BUTTON_PIN 5
#define LED_PIN 6

// mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// // ethernet interface ip address
static byte myip[] = { 192,168,1,203 };
// // gateway ip address
static byte gwip[] = { 192,168,1,1 };

int button = false;
bool wasOpened = false;
long unlocked = 0;
long now = 0;
Stash stash;
byte Ethernet::buffer[500];
BufferFiller bfill;
byte recvCount;
char password[16] = "123DOOP";
char msgChar;
bool allGood;

void unlock () {
    wasOpened = true;
    unlocked = millis();
    rf12_sendStart(0, password, sizeof password);
    blink();
    button = false;
}

void blink () {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
}

const char json_OK[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Pragma: no-cache\r\n\r\n";

static word api() {
    bool unlocking = false;
    long timeSinceOpened = now - unlocked;
    now = millis();

    if(timeSinceOpened < 5000 && wasOpened) {
        unlocking = true;
    }

    if(!wasOpened) {
        timeSinceOpened = 0;
    }

    bfill = ether.tcpOffset();
    bfill.emit_p(PSTR(
        "$F"
        "{"
        "    \"lock_status\": \"$F\","
        "    \"last_opened\": $D"
        "}"),
        json_OK,
        unlocking?PSTR("OPEN"):PSTR("CLOSED"),
        (word) timeSinceOpened/1000);
    return bfill.position();
}

static word ping() {
    bfill = ether.tcpOffset();
    bfill.emit_p(PSTR(
        "$F"
        "IAmKeybert"),
        json_OK);
    return bfill.position();
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
    // Welcome
    Serial.begin(57600);
    Serial.println("\nIAmAHouseBert");

    // Set up pins
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, HIGH);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    blink();

    // Set up RF12
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);

    blink();

    // Set up ethernet
    if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
        Serial.println( "Failed to access Ethernet controller");
    }

    // Static
    if (!ether.staticSetup(myip, gwip)) {
        Serial.println("Static setup failed");
    }

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);  
    ether.printIp("DNS: ", ether.dnsip);

    blink();

    sendToTwitter();

    blink();
}

void loop () {
    if(digitalRead(BUTTON_PIN) != HIGH) {
        button = true;
    }

    word len = ether.packetReceive();
    word pos = ether.packetLoop(len);

    // check if valid tcp data is received
    if (pos) {
        bfill = ether.tcpOffset();
        char *data = (char *) Ethernet::buffer + pos;

        // Serial.println(bfill);
        Serial.println(data);

        Serial.println( "REQUEST" );

        if (strncmp("GET /", data, 5) == 0) {
            data += 5;
            if (strncmp("8ec49811 ", data, 9) == 0) {
                Serial.println( "YEY!" );
                unlock();
                ether.httpServerReply(api());
            } else if (strncmp("areyoukeybert ", data, 14) == 0) {
                Serial.println( "IAmKeybert" );
                ether.httpServerReply(ping());
            } else {
                ether.httpServerReply(api());
            }
        }
    }

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

    if(rf12_canSend()) {
        unlock();
    }
}
