/*combined Udp NTP Client example with ENC28J60 and 2x16 i2c LCD
https://github.com/ntruchsess/arduino_uip
https://github.com/mrkale/LiquidCrystal_I2C
*/

#include <SPI.h>
#include <Wire.h>
#include <UIPEthernet.h>
#include <LiquidCrystal_I2C.h>

#define tZone 2



unsigned int localPort = 8888;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
int h, m, s;
char pbuf[48];
unsigned long epoch, syncMillis, ss;

EthernetUDP Udp;
LiquidCrystal_I2C lcd(0x27, 20, 4);
byte mac[] = {
  0x88, 0x88, 0x88, 0x88, 0x88, 0x88
};

void setup()
{
  lcd.init();
  Serial.begin(9600);
  lcd.backlight();
  lcd.print("Starting");
  delay(500);

  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("err DHCP");
    lcd.print("err DHCP");
    for (;;)
      ;
  }
  Udp.begin(localPort);
  
  if(getTime()){
    Serial.println("err time");
    lcd.print("err time");
    for (;;)
      ;
  }

}



void loop() {
  if (millis() > syncMillis + ss) {
    ss += 1000;
    upTime();
    delay(100);
  }

}

void upTime() {
  lcd.clear();
  if (h / 10 == 0) {
    sprintf(pbuf, "0");
    sprintf(pbuf + 1, "%d", h);
  } else {
    sprintf(pbuf, "%d", h);
  }
  sprintf(pbuf + 2, ":");
  if (m / 10 == 0) {
    sprintf(pbuf + 3, "0");
    sprintf(pbuf + 4, "%d", m);
  } else {
    sprintf(pbuf + 3, "%d", m);
  }
  sprintf(pbuf + 5, ":");
  if (s / 10 == 0) {
    sprintf(pbuf + 6, "0");
    sprintf(pbuf + 7, "%d", s);
  } else {
    sprintf(pbuf + 6, "%d", s);
  }
  lcd.print(pbuf);
  s++;
  if (s == 60) {
    s = 0;
    m++;
    if (m == 60) {
      m = 0;
      h++;
      if (h == 24) {
        h = 0;
      }
    } else if (m == 30) {
      getTime();
    }
  }
}

int getTime() {

  sendNTPpacket();
  delay(1000);
  if ( Udp.parsePacket() ) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    epoch = secsSince1900 - seventyYears;
    h = (epoch  % 86400L) / 3600 + tZone;
    if (h / 24 != 0) {
      h = h % 24;
    }
    m = (epoch  % 3600) / 60;
    s = (epoch  % 60) + 1;
    syncMillis = millis();
    ss = 0;
    return 0;
  }
  return 1;

}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket()
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(IPAddress(66, 219, 116, 140), 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();

}
