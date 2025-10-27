#include "VisCirclePacker.h"
#include "helpers.h"


class Circle
{
public:
  float x;
  float y;
  float r;
  bool active;
  int hue = 0;
  
  Circle(float x_, float y_, float r_, bool a_)
  {
    x=x_;
    y=y_;
    r=r_;
    active=a_;
  }

  Circle()
  {
    x=0;
    y=0;
    r=0;
    active=false;
  }
};

const int MAX_CIRCLES = 100;
Circle circles[MAX_CIRCLES];
int lastNumActiveCircles = 0;
int numActiveCircles = 0;
int hueOff = 0;
float clearPause = 0;

float CircleStepFactor = 360.0f;

bool stopTrying = false;
void resetCircles()
{
  clearPause = 0.0f;
  for( int i = 0; i < MAX_CIRCLES; i++ )
  {
    circles[i].active = false;
    circles[i].hue = random(360);
  }
  stopTrying = false;
}

void VisCirclePacker::focus()
{
  resetCircles();
}

void VisCirclePacker::reset()
{
    resetCircles();
}

void VisCirclePacker::init()
{
    resetCircles();
}

extern float gfCommon;
extern float gxCommon;
extern float gyCommon;

float frad = 0.1;
float fdelay = 0.0f;
void VisCirclePacker::update( float delta )
{
  // reset circles if required
  if( numActiveCircles == lastNumActiveCircles )
  {
    clearPause += delta;
    if( clearPause > 5.0f )
    {
      clearPause = 0.0f;
      resetCircles();
    }
  }
  else
  {
    lastNumActiveCircles = numActiveCircles;
    clearPause = 0;
  }
  
  numActiveCircles = 0;
  int addThisFrame = 1; // was 2
  hueOff+=3;
  hueOff %=360;
  
  for( int i = 0; i < MAX_CIRCLES; i++ )
  {
    Circle &c = circles[i];
    if( c.active )
    {
      numActiveCircles++;
      // Now check the circle isn't touching any other circle
      boolean canGrow = true;
      for( int j = 0; j < MAX_CIRCLES; j++ )
      {
        if( i==j ) continue;
        
        Circle &c2 = circles[j];
        if( c2.active )
        {
          if( fabs(c.x - c2.x) < c.r+1+c2.r && fabs(c.y - c2.y) < c.r+1+c2.r ) 
          {
            // can't grow
            canGrow = false;
            break;
          }
        }
      }

      if( canGrow )
      {
        // Not touching anything, can still grow.
        c.r += 1;//0.25f;
      }

      // redraw
      BresenhamsCircle(c.x,c.y,int(c.r), HSVColors[((c.hue+hueOff)%360)*3+0], HSVColors[((c.hue+hueOff)%360)*3+1], HSVColors[((c.hue+hueOff)%360)*3+2] );
    }
    else if( !stopTrying && addThisFrame > 0 && numActiveCircles < MAX_CIRCLES )
    {
      boolean clear = false;
      // shift method
      int numTries = 200;
      while( clear == false && numTries > 0 )
      {
        numTries--;
        clear = true;
  
        // truly random method
        c.x = random(64);
        c.y = random(64);
        c.r = 1.0f;
        
        // Now check the new circle isn't inside any other circle
        for( int j = 0; j < MAX_CIRCLES; j++ )
        {
          if( i==j ) continue;
          
          Circle &c2 = circles[j];
          if( c2.active )
          {
            if( fabs(c.x - c2.x) < (c2.r + c.r + 1) && fabs(c.y - c2.y) < (c2.r+c.r+1) ) { clear = false; break; }
          }
        }
      }
      if( numTries > 0 && clear )
      {
        c.r = 1.0f;
        c.active = true;
        addThisFrame--;
      }
      if( numTries == 0 )
      {
        // exhaused all our tries
        stopTrying = true;
      }
    }
  }
}
