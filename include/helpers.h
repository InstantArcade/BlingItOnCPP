#pragma once

#include <arduino.h>
#include <Adafruit_Protomatter.h> // For RGB matrix
#include <Adafruit_LIS3DH.h>      // For accelerometer

#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64 // Number of pixels tall of each INDIVIDUAL panel module.

// Rotation modifier to auto-convert accelerometer and drawing commands
// to treat 0,0 as top left with the board sticking out of the top (wearable style)
// Be sure to use RGBPixel_R() to draw
#define BOARD_UP 1 // Set this to 0 if you want the normal panel orentation with the board sticking out the right hand side

#define drawPixelRGB888(x,y,r,g,b) drawPixel(x,y,matrix->color565(r,g,b))

#ifdef COLOR_ORDER_BGR
  #if BOARD_UP
    #define RGBPixel_R(x,y,r,g,b) drawPixelRGB888(63-(y),(x),b,g,r)
  #else
    #define RGBPixel_R(x,y,r,g,b) drawPixelRGB888(x,y,b,g,r)
  #endif
  #define ColorInt(r,g,b) color565(b,g,r)
#elif defined(COLOR_ORDER_RBG)
  #if BOARD_UP
    #define RGBPixel_R(x,y,r,g,b) drawPixelRGB888(63-(y),(x),r,b,g)
  #else
    #define RGBPixel_R(x,y,r,g,b) drawPixelRGB888(x,y,r,b,g)
  #endif
  #define ColorInt(r,g,b) color565(r,b,g)
#else
  #if BOARD_UP
    #define RGBPixel_R(x,y,r,g,b) drawPixelRGB888(63-(y),(x),r,g,b)
  #else
    #define RGBPixel_R(x,y,r,g,b) drawPixelRGB888(x,y,r,g,b)
  #endif
  #define ColorInt(r,g,b) color565(r,g,b)
#endif

#if BOARD_UP
  #define Pixel_R(x,y,col) drawPixel(63-y,x,col)
#else
  #define Pixel_R(x,y,col) drawPixel(x,y,col)
#endif

extern Adafruit_Protomatter *matrix;

#define Z_INFINITY (99999.99f)
extern bool cycleHue;

#define PANEL_GAMMA (1.1f) // Also try 1.2f
extern float globalGamma;

extern float rMult;
extern float gMult;
extern float bMult;

int inline imin(int a, int b, int c)
{
  if( a <= b && a <= c ) return a;
  if( b <= a && b <= c ) return b;
  return c;
}

int inline imax(int a, int b, int c)
{
  if( a >= b && a >= c ) return a;
  if( b >= a && b >= c ) return b;
  return c;
}

class vector3 
{
public:
    float x,y,z;

  vector3() { x = 0; y = 0; z = 0; }
  vector3(float x_, float y_, float z_){ x=x_; y=y_; z=z_; }
  vector3 operator-(const vector3 &rhs)
  {
    vector3 result;
    result.x = x - rhs.x;
    result.y = y - rhs.y;
    result.z = z - rhs.z;
    return vector3(result.x,result.y,result.z);
  }

  vector3 operator+(const vector3 &rhs)
  {
    vector3 result;
    result.x = x + rhs.x;
    result.y = y + rhs.y;
    result.z = z + rhs.z;
    return vector3(result.x,result.y,result.z);
  }

  void operator+=(const vector3 &rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
  }

  void operator-=(const vector3 &rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
  }

  vector3 operator*(const vector3 &rhs)
  {
    vector3 result;
    result.x = x * rhs.x;
    result.y = y * rhs.y;
    result.z = z * rhs.z;
    return vector3(result.x,result.y,result.z);
  }

  vector3 operator*(const float f)
  {
    vector3 result;
    result.x = x * f;
    result.y = y * f;
    result.z = z * f;
    return vector3(result.x,result.y,result.z);
  }
};

const float PI2 = PI*2.0f;
inline void constrainPI2( float &rfVal ){ while(rfVal>PI2){rfVal-=PI2;} while(rfVal<-PI2){rfVal+=PI2;} } 
inline void constrainPI2( vector3 &rVec ){ while(rVec.x>PI2){rVec.x-=PI2;} while(rVec.x<-PI2){rVec.x+=PI2;} while(rVec.y>PI2){rVec.y-=PI2;} while(rVec.y<-PI2){rVec.y+=PI2;} while(rVec.z>PI2){rVec.z-=PI2;} while(rVec.z<-PI2){rVec.z+=PI2;}} 

inline float sqrt3( float x)  // fast sqrt approximation (the Quake hack)
{
  union
  {
    int i;
    float x;
  } u;
  
  u.x = x;
  u.i = (1<<29) + (u.i >> 1) - (1<<22);
  return u.x;
}

typedef struct {
    float r;       // a fraction between 0 and 1
    float g;       // a fraction between 0 and 1
    float b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    float h;       // angle in degrees
    float s;       // a fraction between 0 and 1
    float v;       // a fraction between 0 and 1
} hsv;

struct colorbits565 {
    uint16_t r : 5;
    uint16_t g : 6;
    uint16_t b : 5;
};

union colorunion{
    uint16_t rawcolor;
    struct colorbits565 components;
};

hsv rgb2hsv(rgb in);
rgb hsv2rgb(hsv in);

extern byte HSVColors[360*3];
extern uint16_t HSV565[360];

extern int hueOffset;
extern float fFocal;
extern float fXRot, fYRot, fZRot;
extern float fXRotSpeed, fYRotSpeed, fZRotSpeed;
extern sensors_event_t global_accelerometer;

float v3_dot_product( vector3 u, vector3 v );
vector3 normalize( vector3 &v );

void handleColorBars( float );
void generateHSVTable();
void cycleTheHue( float );

void initNoise(void);
double noise(double x, double y, double z);

void clearZBuffer();
void make_counter_clockwise(int& x1, int& y1, int& x2, int& y2, int& x3, int& y3);
void halfspace_triangle(int x1, int y1, int x2, int y2, int x3, int y3, byte r, byte g, byte b);
void halfspace_triangle_v(vector3 v1, vector3 v2, vector3 v3, byte r, byte g, byte b);
void halfspace_triangle_lerp(vector3 v1, vector3 v2, vector3 v3, vector3 c1, vector3 c2, vector3 c3);
void BresenhamLine( int x0, int y0, int x1, int y1, byte r, byte g, byte b );
void BresenhamsCircle( int x0, int y0, int radius, byte r, byte g, byte b );

uint16_t rgbto565Gamma( float r, float g, float b, float gamma);
uint16_t rgbto565Gamma( byte r, byte g, byte b, float gamma);