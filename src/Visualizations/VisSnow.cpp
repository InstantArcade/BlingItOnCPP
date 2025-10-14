#include "VisSnow.h"
#include "helpers.h"

byte collision_bufffer[64][64];

class SnowParticle
{
  float x,y;
  float vx,vy;
  bool active;

  SnowParticle()
  {
    x = y = vx = vy = 0;
    active = false;
  }

  void move(float delta)
  {
    if( !active ) return;

    // Calc new pos
    float nx = x + vx*delta;
    float ny = y + vy*delta;

    // check for going off screen
    int inx = (int)nx;
    int iny = (int)ny;
    if( inx <0 || inx > 63 || iny < 0 || iny > 63 )
    {
      active = false;
      return;
    }

    // check collision
    if( collision_bufffer[inx][iny] > 0 )
    {
      // stop it
      nx = x;
      ny = y;
    }

    // set new x & y
    x = nx;
    y = ny;
  }

};

void resetSnow()
{
  // clear collision buffer
  for( int y = 0; y < 64; y++ )
    for( int x = 0; x < 64; x++ )
      collision_bufffer[x][y] = 0;

  // reset all particles

}

void VisSnow::init()
{
    resetSnow();
}

void VisSnow::update( float delta )
{
  // for( int iC = 0; iC < ActualNumCircles; iC++ )
  // {
  //   allCircles[iC].move(delta);
  //   allCircles[iC].draw();
  // }
}