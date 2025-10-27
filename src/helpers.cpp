#include "helpers.h"

float fFocal = 300; // Focal lenght for 3D things
float fXRot = 0, fYRot = 0, fZRot = 0;
float fXRotSpeed = 0, fYRotSpeed = 0, fZRotSpeed = 0;

float rMult = 1.0f;
float gMult = 1.0f;
float bMult = 1.0f;

sensors_event_t global_accelerometer;

uint16_t rgbto565Gamma( float r, float g, float b, float gamma = globalGamma )
{
  float rf = pow( r, gamma ) * rMult;
  float gf = pow( g, gamma ) * gMult;
  float bf = pow( b, gamma ) * bMult;

  // 5 bits (0-31) of red, 6 bits (0-63) of green, 5 bits (0-31) of blue
  return (uint16_t)(((int)(rf*31.0f)<<11) | ((int)(gf*63.0f)<<5) | (int)(bf*31.0f));
}

uint16_t rgbto565Gamma( byte r, byte g, byte b, float gamma = globalGamma )
{
  float rf = r/255.0f;
  float gf = g/255.0f;
  float bf = b/255.0f;

  return rgbto565Gamma( rf, gf, bf, gamma );
}


float v3_dot_product( vector3 u, vector3 v )
{
  return (u.x*v.x)+(u.y*v.y)+(u.z*v.z);
}

vector3 normalize( vector3 &v )
{
  float mag = sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
  v.x /= mag;
  v.y /= mag;
  v.z /= mag;
  
  return v;
}

#ifdef COLOR_ORDER_BGR
  void rgbFrom565( uint16_t c, int &r, int &g, int &b )
  {
    b = (c & 0b1111100000000000) >> 8; // 5 bits of blue shift into the top 8 bits
    g = (c & 0b0000011111100000) >> 3; // 6 bits of green shift into the top 8 bits
    r = (c & 0b0000000000011111) << 3; // 5 bits of red, shift into the top 8 bits
  }
#elif defined(COLOR_ORDER_RBG)
  void rgbFrom565( uint16_t c, int &r, int &g, int &b )
  {
    r = (c & 0b1111100000000000) >> 8; // 5 bits of red shift into the top 8 bits
    b = (c & 0b0000011111100000) >> 3; // 6 bits of green shift into the top 8 bits
    g = (c & 0b0000000000011111) << 3; // 5 bits of blue, shift into the top 8 bits
  }
#else
  void rgbFrom565( uint16_t c, int &r, int &g, int &b )
  {
    r = (c & 0b1111100000000000) >> 8; // 5 bits of red shift into the top 8 bits
    g = (c & 0b0000011111100000) >> 3; // 6 bits of green shift into the top 8 bits
    b = (c & 0b0000000000011111) << 3; // 5 bits of blue, shift into the top 8 bits
  }
#endif

/////////////////////////////////////////////
//
// make_counter_clockwise takes three x,y 
// point references and re-orders them to
// be in counter clockwise winding order if
// needed
//
/////////////////////////////////////////////
void make_counter_clockwise(int& x1, int& y1, int& x2, int& y2, int& x3, int& y3)
{
    int xm = (x1 + x2 + x3) / 3; // center point x coordinate
    int ym = (y1 + y2 + y3) / 3; // center point y coordiante

    float a1 = atan2(xm - x1, ym - y1); // angle between center point and first point
    float a2 = atan2(xm - x2, ym - y2); // angle between center point and second point
    float a3 = atan2(xm - x3, ym - y3); // angle between center point and third point

    // Points are already in order if...
    // 1 < 2 < 3 or
    // 2 < 3 < 1 or 
    // 3 < 1 < 2

    if ((a1 <= a2 && a2 <= a3) || (a2 <= a3 && a3 <= a1) || (a3 <= a1 && a1 <= a2))
    {
        // points are already in order - great!
    }
    else
    {
        // swap two of the points
        x1 = x1 ^ x3;
        x3 = x3 ^ x1;
        x1 = x1 ^ x3;

        y1 = y1 ^ y3;
        y3 = y3 ^ y1;
        y1 = y1 ^ y3;
    }
}

/////////////////////////////////////////////
//
// halfspace_triangle renders a filled 
// triangle using the fast halfspace method
//
/////////////////////////////////////////////
void halfspace_triangle(int x1, int y1, int x2, int y2, int x3, int y3, byte r, byte g, byte b)
{
  int minx = imin(x1,x2,x3);
  int miny = imin(y1,y2,y3);
  int maxx = imax(x1,x2,x3);
  int maxy = imax(y1,y2,y3);

  // constrain to viewport
  if( minx < 0 ) minx = 0;
  if( miny < 0 ) miny = 0;
  if( maxx > 63 ) maxx = 63;
  if( maxy > 63 ) maxy = 63;

  for( int y = miny; y < maxy; y++ )
  {
    for( int x = minx; x < maxx; x++ )
    {
      if( (x1 - x2) * (y - y1) - (y1 - y2) * (x - x1) >= 0 &&
          (x2 - x3) * (y - y2) - (y2 - y3) * (x - x2) >= 0 &&
          (x3 - x1) * (y - y3) - (y3 - y1) * (x - x3) >= 0 )
      {
        matrix->RGBPixel_R(x,y,r,g,b);
      }
    }
  }
}

// As above, but using vector3 values
void halfspace_triangle_v(vector3 v1, vector3 v2, vector3 v3, byte r, byte g, byte b)
{
  halfspace_triangle(v1.x,v1.y,v2.x,v2.y,v3.x,v3.y,r,g,b);
}

#define RED(v) v.x
#define GREEN(v) v.y
#define BLUE(v) v.z

/////////////////////////////////////////////
//
// halfspace_triangle_lerp renders a filled 
// triangle using the fast halfspace method
// and interpolates colors between the three
// points
//
/////////////////////////////////////////////
void halfspace_triangle_lerp( float *zbuffer, vector3 v1, vector3 v2, vector3 v3, vector3 c1, vector3 c2, vector3 c3)
{
  int minx = imin(v1.x,v2.x,v3.x);
  int miny = imin(v1.y,v2.y,v3.y);
  int maxx = imax(v1.x,v2.x,v3.x);
  int maxy = imax(v1.y,v2.y,v3.y);

  // constrain to viewport
  if( minx < 0 ) minx = 0;
  if( miny < 0 ) miny = 0;
  if( maxx > 63 ) maxx = 63;
  if( maxy > 63 ) maxy = 63;

  for( int y = miny; y <= maxy; y++ )
  {
    for( int x = minx; x <= maxx; x++ )
    {
      // barycentric coords for x,y
      float alpha,beta,gamma;
      alpha = (v1.x - v2.x) * (y - v1.y) - (v1.y - v2.y) * (x - v1.x);
      beta = (v2.x - v3.x) * (y - v2.y) - (v2.y - v3.y) * (x - v2.x);
      gamma = (v3.x - v1.x) * (y - v3.y) - (v3.y - v1.y) * (x - v3.x);

      // normalize
      float mag = alpha+beta+gamma;
      alpha /= mag;
      beta /= mag;
      gamma /= mag;

      if( alpha >= 0 && beta >= 0 && gamma >= 0 )
      {
        byte r,g,b;
        // lerp the vertex colors
        r = (c3.x*alpha + c1.x*beta + c2.x*gamma)*255;
        g = (c3.y*alpha + c1.y*beta + c2.y*gamma)*255;
        b = (c3.z*alpha + c1.z*beta + c2.z*gamma)*255;

        // bonus get lerped z position the same way
        float zl = (v3.z*alpha + v1.z*beta + v2.z*gamma); // use reciprocal instead of raw value

        if( zl < zbuffer[y*64+x] )
        {
          matrix->RGBPixel_R(x,y,r,g,b);
          zbuffer[y*64+x] = zl;
        }
      }
    }
  }
}

byte HSVColors[360*3]; // Buffer for HSV color values r,g,b bytes
uint16_t HSV565[360]; // Buffer for HSV color values RGB565 16 bit format
int hueOffset = 0;
bool cycleHue = true;
byte hueCycleDelay = 10;
byte hueCycleCount = 0;

// colorspace conversion
// Convert an rgb stucture to hsv structure
hsv rgb2hsv(rgb in)
{
    hsv         out;
    float      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = 0;// -1.0 / 0;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}

// colorspace conversion
// convert an hsv structure to an rgb structure
rgb hsv2rgb(hsv in)
{
    float      hh, p, q, t, ff;
    int        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (int)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

// fill the HSVTables so we don't need to calculate them on the fly
void generateHSVTable()
{
    // generate HSVColors
    for( int i = 0; i < 360; i++ )
    {
        hsv in;
        in.h = i;//(360/256)*i;
        in.s = 1.0f;
        in.v = 1.0f;
        rgb col = hsv2rgb(in);
        // col values are all 0-1
        HSVColors[i*3+0] = col.r * 255;
        HSVColors[i*3+1] = col.g * 255;
        HSVColors[i*3+2] = col.b * 255;
        HSV565[i] = rgbto565Gamma(col.r,col.g,col.b);
    }
}

void cycleTheHue( float delta )
{
  hueCycleCount++;
  if( hueCycleCount >= hueCycleDelay )
  {
    hueCycleCount = 0;
    hueOffset++;
    hueOffset %= 360;
  }
}

// Bresenham's algorithm for fast line drawing
void BresenhamLine( int x0, int y0, int x1, int y1, byte r, byte g, byte b )
{   
  int dx =  abs(x1-x0); // delta x
  int sx = x0 < x1 ? 1 : -1; // s direction
  
  int dy = -abs(y1-y0); // delta y
  int sy = y0 < y1 ? 1 : -1; // y direction
  
  int err = dx+dy;  // error value e_xy
  
  while (true)   // loop
  {
    // bound check
    if( x0 >= 0 && x0 < 64 && y0 >=0 && y0 <64 )
    {
      matrix->RGBPixel_R(x0,y0,r,g,b);
    }
    if (x0 == x1 && y0 == y1) break; // we reached the destination - stop now
    
    int e2 = 2*err;
    if (e2 >= dy) // e_xy + e_x > 0
    {
      err += dy;
      x0 += sx; // move along the x direction
    }
    if (e2 <= dx) // e_xy + e_y < 0
    {
      err += dx;
      y0 += sy; // move along the y direction
    }
  }     
}

// Bresenham's algorithm for fast circle drawing
void BresenhamsCircle(int x0, int y0, int radius, byte r, byte g, byte b )
{
  int x = 0;
  int y = radius;
  int d = 3 - 2 * radius;

  while (x <= y) 
  {
    matrix->RGBPixel_R( x + x0,  y + y0, r,g,b);
    matrix->RGBPixel_R( y + x0,  x + y0, r,g,b);
    matrix->RGBPixel_R(-x + x0,  y + y0, r,g,b);
    matrix->RGBPixel_R(-y + x0,  x + y0, r,g,b);

    matrix->RGBPixel_R( x + x0, -y + y0, r,g,b);
    matrix->RGBPixel_R( y + x0, -x + y0, r,g,b);
    matrix->RGBPixel_R(-x + x0, -y + y0, r,g,b);
    matrix->RGBPixel_R(-y + x0, -x + y0, r,g,b);
  
    if( d < 0 ) 
    {
      d += 4 * x + 6;
    }
    else 
    {
      d += 4 * (x - y) + 10;
      y--;
    }
    x++;
  }
}