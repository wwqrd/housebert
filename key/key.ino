#include <JeeLib.h>
#include <EtherCard.h>

#define BUTTON_PIN 5
#define LED_PIN 6

char password[] = "123DOOP";
int button = false;
bool wasOpened = false;
long unlocked = 0;
long now = 0;
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,1,203 };
byte Ethernet::buffer[500];
BufferFiller bfill;

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

void setup () {
    // Welcome
    Serial.begin(57600);
    Serial.println("\nIAmAKeyBert");

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
    if (!ether.dhcpSetup()) {
        Serial.println("DHCP failed");
    }

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);  
    ether.printIp("DNS: ", ether.dnsip);

    blink();
}

void loop () {
    // if (ether.dhcpExpired()) {
    //     Serial.println("Acquiring DHCP lease again");
    //     ether.dhcpSetup();
    // }

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
        // Serial.println(data);

        Serial.println( "REQUEST" );

        if (strncmp("GET /", data, 5) == 0) {
            data += 5;
            if (strncmp("8ec49811 ", data, 9) == 0) {

                Serial.println( "YEY!" );
                unlock();

            }

            ether.httpServerReply(api());
        }
    }

    rf12_recvDone();

    if(rf12_canSend() && button == true) {
        unlock();
    }
}
