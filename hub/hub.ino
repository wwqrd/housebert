#include <JeeLib.h>
#include <EtherCard.h>

#define LED_PIN 6

// mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte myip[] = { 192,168,0,100 };
// gateway ip address
static byte gwip[] = { 192,168,0,1 };

typedef struct {
  byte node;
  long reading;
} Payload;

long d0 = 0;
long d1 = 0;
long reading;

byte Ethernet::buffer[500];
BufferFiller bfill;

void blink () {
    // These are "inverted" to work with a Nanode's built in LED
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
}

const char json_OK[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Pragma: no-cache\r\n\r\n";

static word api() {
    d1 = millis();
    long dT = d1 - d0;

    bfill = ether.tcpOffset();
    bfill.emit_p(PSTR(
        "$F"
        "{"
        "    \"reading\": $D,"
        "    \"dT\": $D"
        "}"),
        json_OK,
        (word) reading,
        (word) dT/1000);

    return bfill.position();
}

static word ping() {
    bfill = ether.tcpOffset();
    bfill.emit_p(PSTR(
        "$F"
        "{\"hub\": true}"),
        json_OK);
    return bfill.position();
}

void setupPins () {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
}

void setupRf () {
    rf12_initialize(1, RF12_868MHZ);
    rf12_encrypt(RF12_EEPROM_EKEY);
}

void setupNetwork() {
    if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
        Serial.println( "Failed to access Ethernet controller");
    }

    // Use static IP
    if (!ether.staticSetup(myip, gwip)) {
        Serial.println("Static setup failed");
    }

    // Get IP via DHCP
    // if (!ether.dhcpSetup()) {
    //     Serial.println("DHCP failed");
    // }

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
}

void setup () {

    Serial.begin(57600);
    Serial.println("`Hub` is setting up...");

    Serial.println("Initialising pins...");
    setupPins();
    blink();

    Serial.println("Initialising rf...");
    setupRf();
    blink();

    Serial.println("Initialising network...");
    setupNetwork();
    blink();
}

void loop () {
    // if (ether.dhcpExpired()) {
    //     Serial.println("Acquiring DHCP lease again");
    //     ether.dhcpSetup();
    // }

    word len = ether.packetReceive();
    word pos = ether.packetLoop(len);

    // check if valid tcp data is received
    if (pos) {
        bfill = ether.tcpOffset();
        char *data = (char *) Ethernet::buffer + pos;

        if (strncmp("GET /", data, 5) == 0) {
            data += 5;
            if (strncmp("reading ", data, 8) == 0) {
                Serial.println( "Requested a reading" );
                ether.httpServerReply(api());
            } else if (strncmp("who ", data, 4) == 0) {
                Serial.println( "Received a ping." );
                ether.httpServerReply(ping());
            } else {
                // ether.httpServerReply(api());
            }
            blink();
        }
    }

    if (rf12_recvDone() && rf12_crc == 0 && rf12_len == sizeof (Payload)) {
        const Payload* payload = (const Payload*) rf12_data;
        Serial.print((word) payload->node);
        Serial.print(": ");
        Serial.println((word) payload->reading);
        reading = payload->reading;
        d0 = millis();
    }

}
