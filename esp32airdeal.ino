//All this software is lifted from somewhere but works well together!
#include <HardwareSerial.h>
// Update these with values suitable for your network.
#define BLYNK_PRINT Serial
#define VBATPIN A13 
#define DONEPIN 33
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
const int ledPin = BUILTIN_LED;
float measuredvbat = 0.0;
char auth[] = "token for Blynk App";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Your Wifi network";
char pass[] = "your password";

long lastMsg = 0;
char msg[50];
bool HPMAstatus = false;
 
int PM25;
int PM10;
HardwareSerial HPMA115S0(1);
#define RXD2 16
#define TXD2 17
void setup()
{
  Serial.begin(9600, SERIAL_8N1);
  while (!Serial);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);
  Blynk.begin(auth, ssid, pass);
  pinMode(DONEPIN, OUTPUT);
  digitalWrite(DONEPIN, LOW);
measuredvbat = analogRead(VBATPIN);
measuredvbat = (measuredvbat/4095)*2*3.3*1.1;
HPMA115S0.begin(9600, SERIAL_8N1, RXD2, TXD2);
  while (!HPMA115S0);
  start_autosend();
}


void loop()
{
  
  long now = millis();
  Blynk.run();

  // Publish data every 10 seconds
  if (now - lastMsg > 10000) {
    lastMsg = now;
    
    // Receive the particle data
    HPMAstatus = receive_measurement();
    if (!HPMAstatus) {
      digitalWrite(ledPin, 0);
      Serial.println("Cannot receive data from HPMA115S0!");
      return;
    }
    snprintf (msg, 16, "%D", PM25);
    
    snprintf (msg, 16, "%D", PM10);
    if(PM10 != 0){
    digitalWrite(ledPin, 1);
    Serial.println("PM 2.5:\t" + String(PM25) + " ug/m3");
    Serial.println("PM 10:\t" + String(PM10) + " ug/m3");
    Serial.println("MQTT published HPMA115S0.");
    Blynk.virtualWrite(V4, measuredvbat);
    Blynk.virtualWrite(V5, PM25);
    Blynk.virtualWrite(V6, PM10);
    delay(500);
  while (1) {
    digitalWrite(DONEPIN, HIGH);
    delay(100);
    digitalWrite(DONEPIN, LOW);
    delay(100);
  }
    }  
  }
}
 
bool receive_measurement (void)
{
  while(HPMA115S0.available() < 32);
  byte HEAD0 = HPMA115S0.read();
  byte HEAD1 = HPMA115S0.read();
  while (HEAD0 != 0x42) {
    if (HEAD1 == 0x42) {
      HEAD0 = HEAD1;
      HEAD1 = HPMA115S0.read();
    } else {
      HEAD0 = HPMA115S0.read();
      HEAD1 = HPMA115S0.read();
    }
  }
  if (HEAD0 == 0x42 && HEAD1 == 0x4D) {
    byte LENH = HPMA115S0.read();
    byte LENL = HPMA115S0.read();
    byte Data0H = HPMA115S0.read();
    byte Data0L = HPMA115S0.read();
    byte Data1H = HPMA115S0.read();
    byte Data1L = HPMA115S0.read();
    byte Data2H = HPMA115S0.read();
    byte Data2L = HPMA115S0.read();
    byte Data3H = HPMA115S0.read();
    byte Data3L = HPMA115S0.read();
    byte Data4H = HPMA115S0.read();
    byte Data4L = HPMA115S0.read();
    byte Data5H = HPMA115S0.read();
    byte Data5L = HPMA115S0.read();
    byte Data6H = HPMA115S0.read();
    byte Data6L = HPMA115S0.read();
    byte Data7H = HPMA115S0.read();
    byte Data7L = HPMA115S0.read();
    byte Data8H = HPMA115S0.read();
    byte Data8L = HPMA115S0.read();
    byte Data9H = HPMA115S0.read();
    byte Data9L = HPMA115S0.read();
    byte Data10H = HPMA115S0.read();
    byte Data10L = HPMA115S0.read();
    byte Data11H = HPMA115S0.read();
    byte Data11L = HPMA115S0.read();
    byte Data12H = HPMA115S0.read();
    byte Data12L = HPMA115S0.read();
    byte CheckSumH = HPMA115S0.read();
    byte CheckSumL = HPMA115S0.read();
    if (((HEAD0 + HEAD1 + LENH + LENL + Data0H + Data0L + Data1H + Data1L + Data2H + Data2L + Data3H + Data3L + Data4H + Data4L + Data5H + Data5L + Data6H + Data6L + Data7H + Data7L + Data8H + Data8L + Data9H + Data9L + Data10H + Data10L + Data11H + Data11L + Data12H + Data12L) % 256) != CheckSumL){
      Serial.println("Checksum fail");
      return 0;
    }
    PM25 = (Data1H * 256) + Data1L;
    PM10 = (Data2H * 256) + Data2L;
    return 1;
  }
}
 
bool start_autosend(void)
{
 // Start auto send
  byte start_autosend[] = {0x68, 0x01, 0x40, 0x57 };
  HPMA115S0.write(start_autosend, sizeof(start_autosend));
  HPMA115S0.flush();
  delay(500);
  //Then we wait for the response
  while(HPMA115S0.available() < 2);
  byte read1 = HPMA115S0.read();
  byte read2 = HPMA115S0.read();
  // Test the response
  if ((read1 == 0xA5) && (read2 == 0xA5)){
    // ACK
    return 1;
  }
  else if ((read1 == 0x96) && (read2 == 0x96))
  {
    // NACK
    return 0;
  }
  else return 0;
}
