/* ----------------------------------------------------------------------
------------------------------------------------------------------------- */

#include <Wire.h>                 // For I2C communication
#include <Adafruit_LIS3DH.h>      // For accelerometer
#include <Adafruit_Protomatter.h> // For RGB matrix

#define HEIGHT  64 // Matrix height (pixels) - SET TO 64 FOR 64x64 MATRIX!
#define WIDTH   64 // Matrix width (pixels)

uint8_t rgbPins[]  = {42, 41, 40, 38, 39, 37};
uint8_t addrPins[] = {45, 36, 48, 35, 21};
uint8_t clockPin   = 2;
uint8_t latchPin   = 47;
uint8_t oePin      = 14;

#define NUM_ADDR_PINS 5

Adafruit_Protomatter matrix(
  WIDTH, 4, 1, rgbPins, NUM_ADDR_PINS, addrPins,
  clockPin, latchPin, oePin, true);

Adafruit_LIS3DH accel = Adafruit_LIS3DH();

uint16_t colors[8];

void err(int x) {
  uint8_t i;
  pinMode(LED_BUILTIN, OUTPUT);       // Using onboard LED
  for(i=1;;i++) {                     // Loop forever...
    digitalWrite(LED_BUILTIN, i & 1); // LED on/off blink to alert user
    delay(x);
  }
}

void setup(void) 
{
  Serial.begin(115200);

  ProtomatterStatus status = matrix.begin();
  Serial.printf("Protomatter begin() status: %d\n", status);

  if (!accel.begin(0x19)) {
    Serial.println("Couldn't find accelerometer");
    err(250);  // Fast bink = I2C error
  }
  accel.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  colors[0] = matrix.color565(64, 64, 64);  // Dark Gray
  colors[1] = matrix.color565(120, 79, 23); // Brown
  colors[2] = matrix.color565(228,  3,  3); // Red
  colors[3] = matrix.color565(255,140,  0); // Orange
  colors[4] = matrix.color565(255,237,  0); // Yellow
  colors[5] = matrix.color565(  0,128, 38); // Green
  colors[6] = matrix.color565(  0, 77,255); // Blue
  colors[7] = matrix.color565(117,  7,135); // Purple
}

int lastTicks = 0;
int fpsavg[5]={0};
float fPTimer = 0;
int fpIndex = 0;

void loop() 
{

  int iNow = millis();
  float delta = (iNow-lastTicks)/1000.0f;
  lastTicks = iNow;

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

  // Read accelerometer...
  sensors_event_t event;
  accel.getEvent(&event);
  //Serial.printf("(%0.1f, %0.1f, %0.1f)\n", event.acceleration.x, event.acceleration.y, event.acceleration.z);

  // double xx, yy, zz;
  // xx = event.acceleration.x * 1000;
  // yy = event.acceleration.y * 1000;
  // zz = event.acceleration.z * 1000;

  int x, y;
  matrix.fillScreen(0x0);
  for(int i=0; i<100 ; i++) 
  {
    int n = random(8);
    x = random(64);
    y = random(64);
    uint16_t flakeColor = colors[n];
    matrix.drawPixel(x, y, flakeColor);
    //Serial.printf("(%d, %d)\n", x, y);
  }
  matrix.show(); // Copy data to matrix buffers
}
