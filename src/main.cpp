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
#define BIT_DEPTH 5

#include "font.h"
#include "helpers.h"
#include "Visualization.h"
#include "VisConcentricCircles.h"
#include "VisCirclePacker.h"
#include "VisSnow.h"

Adafruit_Protomatter thematrix(
  WIDTH, BIT_DEPTH, 1, rgbPins, NUM_ADDR_PINS, addrPins,
  clockPin, latchPin, oePin, true);

Adafruit_Protomatter *matrix = &thematrix;
float wave = 0;

Visualization *pVis = nullptr;

float globalGamma = PANEL_GAMMA;

void setup() 
{
  Serial.begin(115200);

  generateHSVTable(); // create the HSV colors or they will all be black

  ProtomatterStatus status = thematrix.begin();

  thematrix.fillScreen(0);
  thematrix.print("Hello");
  thematrix.show();

  delay(100); // give the board a little time to get the accelerometer ready

  if (!accel.begin(0x19)) {
    Serial.println("Couldn't find accelerometer");
    thematrix.setCursor(0,10);
    thematrix.print("Accel err!");
    thematrix.show();
    while(1);
  }
  accel.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  // Comment back in to use WiFi
  // WiFi.begin(ssid, pass);
  // while (WiFi.status() != WL_CONNECTED) {
  //     delay(500);
  //     Serial.print(".");
  // }

  // Serial.println("");
  // Serial.println("Connected to WiFi");
  // printWifiStatus();


  // Set the visualization
  pVis = new VisConcentricCircles();
  //pVis = new VisCirclePacker();
  //pVis = new VisSnow();
  pVis->init();
}

// FPS Tracking
int lastTicks = 0;
int fpsavg[5]={0};
float fPTimer = 0;
int fpIndex = 0;

float red_wave = 0;

void loop() 
{
  int iNow = millis();
  float delta = (iNow-lastTicks)/1000.0f;
  lastTicks = iNow;

  // Track and display Frames per Second
  int fps = 1.0f/delta;
  fpsavg[fpIndex++] = fps;
  fpIndex %= 5;

  fPTimer += delta;
  if( fPTimer > 1.0f )
  {
    fPTimer -= 1.0f;
    float ff = (fpsavg[0]+fpsavg[1]+fpsavg[2]+fpsavg[3]+fpsavg[4])/5.0f;
    printf("%0.2f FPS\n",ff);
  }

  wave += 0.03f;

  if( wave > PI*2.0f)
    wave -= PI*2.0f;

  // Read accelerometer...
  sensors_event_t event;
  accel.getEvent( &event );
  #if BOARD_UP
    float nx = event.acceleration.y; // handle rotated board
    float ny = event.acceleration.x * -1.0f;
    event.acceleration.x = nx;
    event.acceleration.y = ny;
  #endif
  // copy it to global accel value
  global_accelerometer = event;

  // Serial.printf("(%0.1f, %0.1f, %0.1f)\n", event.acceleration.x, event.acceleration.y, event.acceleration.z);

  float diameter = 30.0f;
  float step = 0.62f;
  
  thematrix.fillScreen(0);
  pVis->update(delta);

  red_wave += 0.1f;
  if( red_wave > PI2 ) red_wave -= PI2;
  byte red = (byte)(128.0f+128.0f*sinf(red_wave));
  draw_string(1,28,red,0,0,"SUPERCON");
  thematrix.show();
}
