#include <Arduino.h>
#include <Adafruit_LIS3DH.h>      // For accelerometer
#include <Adafruit_Protomatter.h> // For RGB matrix

#include <WiFi.h>
char ssid[] = "YOUR_SSID";             // your network SSID (name)
char pass[] = "YOUR_SSID_PASSWORD";    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
WiFiClient client;

Adafruit_LIS3DH accel = Adafruit_LIS3DH();

uint8_t rgbPins[]  = {42, 41, 40, 38, 39, 37};
uint8_t addrPins[] = {45, 36, 48, 35, 21};
uint8_t clockPin   = 2;
uint8_t latchPin   = 47;
uint8_t oePin      = 14;

#define WIDTH   64 // Matrix width (pixels)
#define HEIGHT   64 // Matrix height (pixels)
#define NUM_ADDR_PINS 5

Adafruit_Protomatter matrix(
  WIDTH, 4, 1, rgbPins, NUM_ADDR_PINS, addrPins,
  clockPin, latchPin, oePin, true);

float wave = 0;

void setup() 
{
  Serial.begin(115200);
  accel.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  ProtomatterStatus status = matrix.begin();

  matrix.fillScreen(0);
  matrix.print("Hello");
  matrix.show();
/*
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
  //printWifiStatus();
*/
}

void loop() 
{
  wave += 0.1f;

  if( wave > 6.242f)
    wave -=6.242f;

  // Read accelerometer...
  sensors_event_t event;
  accel.getEvent(&event);
  Serial.printf("(%0.1f, %0.1f, %0.1f)\n", event.acceleration.x, event.acceleration.y, event.acceleration.z);

  matrix.fillScreen(matrix.color565(64,32,32));
  matrix.drawPixel(32+sinf(wave)*15,32+cosf(wave*0.97)*15,matrix.color565(255,64,64));
  matrix.drawPixel(32+sinf(wave+0.1)*15,32+cosf((wave+0.1)*0.97)*15,matrix.color565(255,0,0));
  matrix.drawPixel(32+sinf(wave+0.2)*15,32+cosf((wave+0.2)*0.97)*15,matrix.color565(0,255,0));
  matrix.drawPixel(32+sinf(wave+0.3)*15,32+cosf((wave+0.3)*0.97)*15,matrix.color565(0,0,255));
  matrix.show();
}
