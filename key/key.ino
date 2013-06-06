#include <JeeLib.h>
#include <EtherCard.h>

#define BUTTON_PIN 5
#define LED_PIN 6

// supertweet.net username:password in base64
#define KEY "aWFtbG9ja2JlcnQ6MmI1NCJjMSZjNA=="
#define API_URL "/1.1/statuses/update.json"

// mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// // ethernet interface ip address
// static byte myip[] = { 192,168,1,203 };
// // gateway ip address
// static byte gwip[] = { 192,168,1,1 };
// password
char password[] = "123DOOP";

char website[] PROGMEM = "api.supertweet.net";

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

static void sendToTwitter () {
    // generate two fake values as payload - by using a separate stash,
    // we can determine the size of the generated message ahead of time
    const byte *buf = ether.myip;

    byte sd = stash.create();
    stash.print("status=@domakesaythings i am lockbert my IP is: ");

    for (byte i = 0; i < 4; ++i) {
        Serial.print( buf[i], DEC );
        if (i < 3)
            stash.print('.');
    }
    stash.println();
    stash.save();

    // generate the header with payload - note that the stash size is used,
    // and that a "stash descriptor" is passed in as argument using "$H"
    Stash::prepare(PSTR(API_URL), website, PSTR(KEY), stash.size(), sd);

    // send the packet - this also releases all stash buffers once done
    ether.tcpSend();
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

    // Static
    // if (!ether.staticSetup(myip, gwip)) {
    //     Serial.println("Static setup failed");
    // }

    // DHCP
    if (!ether.dhcpSetup()) {
        Serial.println("DHCP failed");
    }

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);  
    ether.printIp("DNS: ", ether.dnsip);

    blink();

    sendToTwitter();

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

    rf12_recvDone();

    if(rf12_canSend() && button == true) {
        unlock();
    }
}
