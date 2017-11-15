/*  
A basic transmitter using the nRF24L01 module.
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const uint64_t pipeOut = 0xE8E8F0F0E1LL;

RF24 radio(9, 10);

// The sizeof this struct should not exceed 32 bytes
struct MyData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte dial1;
  byte dial2;
  byte switches; // bitflag
};

MyData data;

void resetData() 
{
  data.throttle = 0;
  data.yaw = 127;
  data.pitch = 127;
  data.roll = 127;
  data.dial1 = 0;
  data.dial2 = 0;
  data.switches = 0;
}

void setup()
{
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(pipeOut);

  resetData();
}

/**************************************************/

// Returns a corrected value for a joystick position that takes into account
// the values of the outer extents and the middle of the joystick range.
int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);
  if ( val < middle )
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}

void loop()
{
  boolean mode1 = false;//!digitalRead(7);
  
  // The calibration numbers used here should be measured 
  // for your joysticks using the TestJoysticks sketch.
  data.throttle = mapJoystickValues( analogRead(A0), 69, 515, 1001, false );
  data.yaw      = mapJoystickValues( analogRead(A1), 10, 428, 885, true );
  data.pitch    = mapJoystickValues( analogRead(A2), 111, 529, 914, true );
  data.roll     = mapJoystickValues( analogRead(A3), 79, 524, 979, false );

  data.dial1    = constrain( map( analogRead(A4), 70, 1000, 0, 255 ), 0, 255);
  data.dial2    = constrain( map( analogRead(A5), 70, 1000, 0, 255 ), 0, 255);
  
  data.switches = 0;
  if ( ! digitalRead(7) ) data.switches |= 0x1;
  if ( ! digitalRead(8) ) data.switches |= 0x2;
  
  radio.write(&data, sizeof(MyData));
}

