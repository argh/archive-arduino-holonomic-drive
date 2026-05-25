#include <Wire.h>


/*

*/

#define RED_PIN 10
#define GREEN_PIN 11
#define BLUE_PIN 12

int MOTOR_TX = 18;

int MC_1 = 128;
int MC_2 = 129;

int MA = 0;
int MB = 1;

// 0-127, 0 is power save
#define ST_M1_FORWARD    0
#define ST_M1_BACKWARDS  1

// data = (desired volts - 6) * 5
// 0 = 6v, which is the minimum
#define ST_MIN_VOLTAGE   2

// default = 30V, data = (desired volts * 5.12)
#define ST_MAX_VOLTAGE   3

#define ST_M2_FORWARD    4
#define ST_M2_BACKWARDS  5

// 7-bit motion, 0 = full reverse, 127 = full forward, 64 = stop
#define ST_M1_DRIVE      6
#define ST_M2_DRIVE      7

// mixed mode, where one motor is d
#define ST_MIXED_FORWARD  8
#define ST_MIXED_BACKWARDS 9
#define ST_MIXED_RIGHT  10
#define ST_MIXED_LEFT  11
#define ST_MIXED_DRIVE  12
#define ST_MIXED_TURN  13


// timeout disabled = 0, value is 100ms per unit
#define ST_SERIAL_TIMEOUT  14

// 1 = 2400, 2 = 9600, 3 = 19200, 4 = 38400
#define ST_SERIAL_BAUD  15
#define ST_19200 3
#define ST_38400 4

// 1-10 = fast ramp, 256/(~1000 * command value)
// 11-20 = slow ramp, 21-80 = intermediate ramp
// 256 / (15.25 * command value - 10)
// 1 = 1/4 second, 2 = 1/8, 3 = 1/12, 
#define ST_RAMPING     16

// 127-command < motors off < 128 + command
// 0 = default, 124 < off < 131
// (wtf, max speed is 127?)
#define ST_DEADBAND  17

// My robot:
#define ST_FRONT  128
#define ST_REAR  129

#define ST_RIGHT  ST_M1_DRIVE
#define ST_LEFT  ST_M2_DRIVE

#define ST_STOP 64

#define ST_FORWARD_SLOW 72
#define ST_FORWARD 88
#define ST_FORWARD_FAST 104
#define ST_FORWARD_MAX 127

#define ST_BACKWARDS_SLOW 56
#define ST_BACKWARDS 40
#define ST_BACKWARDS_FAST 24
#define ST_BACKWARDS_MAX 0

#define JOY1_VERT  15
#define JOY1_HORZ  14

#define JOY2_VERT  13
#define JOY2_HORZ  12

// mm
//#define WHEEL_RADIUS_MEASURED_1 46.4
//#define WHEEL_RADIUS 51
#define WHEEL_RADIUS 46.4
#define WHEELBASE 228 
#define WIDTH 250
#define K (WHEELBASE/2.0 + WIDTH/2.0)

//239

void setup()
{
  Serial.begin(9600);
  log("hello!");

  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);

  Wire.begin();

  log("wire init");
  pinMode(JOY1_HORZ, INPUT);
  pinMode(JOY1_VERT, INPUT);
  pinMode(JOY2_HORZ, INPUT);
  pinMode(JOY2_VERT, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  calibrateJoystick();
  
  Serial1.begin(9600);
  sabertooth(ST_FRONT, ST_SERIAL_BAUD, ST_38400);
  sabertooth(ST_REAR, ST_SERIAL_BAUD, ST_38400);
  delay(500);
  Serial1.end();
  
  Serial1.begin(19200);
  sabertooth(ST_FRONT, ST_SERIAL_BAUD, ST_38400);
  sabertooth(ST_REAR, ST_SERIAL_BAUD, ST_38400);
  delay(500);
  Serial1.end();  
  
  Serial1.begin(38400);
  sabertooth(ST_FRONT, ST_SERIAL_BAUD, ST_38400);
  sabertooth(ST_REAR, ST_SERIAL_BAUD, ST_38400);
  
  sabertooth(ST_FRONT, ST_RAMPING, 30);
  sabertooth(ST_REAR, ST_RAMPING, 30);

  vex_ime_setup();
}

void setColor(byte red, byte green, byte blue)
{
  analogWrite(RED_PIN, 255-red);
  analogWrite(GREEN_PIN, 255-green);
  analogWrite(BLUE_PIN, 255-blue);
}

void loop()
{
  //  log_wheel_speeds();
  remote();
}

int joy1h;
int joy1hc;

int joy1v;
int joy1vc;

int joy2h;
int joy2hc;

int joy2v;
int joy2vc;

void calibrateJoystick()
{
  joy1hc = analogRead(JOY1_HORZ); // right = 0, left = 1024, middle = ~502
  joy1vc = analogRead(JOY1_VERT); // up = 1024, down = 0, middle = ~522
  
  joy2hc = analogRead(JOY2_HORZ); // middle = 508
  joy2vc = analogRead(JOY2_VERT); // middle = 507
  
}

#define DEADZONE 10

void readJoystick()
{
  int h1 = analogRead(JOY1_HORZ); // right = 0, left = 1024, middle = ~502
  int v1 = analogRead(JOY1_VERT); // up = 1024, down = 0, middle = ~522
  
  int h2 = analogRead(JOY2_HORZ); // middle = 508
  int v2 = analogRead(JOY2_VERT); // middle = 507

  if (h1 >= joy1hc + DEADZONE)
  {
    joy1h = map(h1, joy1hc + DEADZONE, 1024, 0, 100);
  }
  else if (h1 <= joy1hc - DEADZONE)
  {
    joy1h = map(h1, 0, joy1hc - DEADZONE, -100, 0);
  }
  else
  {
    joy1h = 0;
  }
  
  if (v1 >= joy1vc + DEADZONE)
  {
    joy1v = map(v1, joy1vc + DEADZONE, 1024, 0, 100);
  }
  else if (v1 <= joy1vc - DEADZONE)
  {
    joy1v = map(v1, 0, joy1vc - DEADZONE, -100, 0);
  }
  else
  {
    joy1v = 0;
  }




  if (h2 >= joy2hc + DEADZONE)
  {
    joy2h = map(h2, joy2hc + DEADZONE, 1024, 0, 100);
  }
  else if (h2 <= joy2hc - DEADZONE)
  {
    joy2h = map(h2, 0, joy2hc - DEADZONE, -100, 0);
  }
  else
  {
    joy2h = 0;
  }
  
  if (v2 >= joy2vc + DEADZONE)
  {
    joy2v = map(v2, joy2vc + DEADZONE, 1024, 0, 100);
  }
  else if (v2 <= joy2vc - DEADZONE)
  {
    joy2v = map(v2, 0, joy2vc - DEADZONE, -100, 0);
  }
  else
  {
    joy2v = 0;
  }



}


// mm/sec
#define MAX_COMMAND_SPEED 500.0

#define MAX_RPS 12.0

boolean prt = false;
float oldvx = 0;
float oldvy = 0;
float oldwv = 0;

float fmap(float val, float fromlo, float fromhi, float tolo, float tohi)
{
  return ((val - fromlo) * (tohi - tolo) / (fromhi - fromlo)) + tolo;
}

void remote()
{
  readJoystick();
  
  // translation velocity is left joystick
  // rotation velocity is right joystick
  
  // vx is aligned with robot body, i.e. forward speed = left joystick vertical axis
  
  float vx = fmap(joy1v, -100, +100, -MAX_COMMAND_SPEED, +MAX_COMMAND_SPEED);
  float vy = fmap(joy1h, -100, +100, -MAX_COMMAND_SPEED, +MAX_COMMAND_SPEED);
  float wv = fmap(joy2h, -100, +100, -MAX_COMMAND_SPEED/K, +MAX_COMMAND_SPEED/K);

  int r = map(abs(joy1v), 0, +100, 0, 255);
  int g = map(abs(joy1h), 0, +100, 0, 255);
  int b = map(abs(joy2h), 0, +100, 0, 255);

  setColor(byte(r), byte(g), byte(b));

  move(vx, vy, wv);  
  
  if (vx != oldvx || vy != oldvy || wv != oldwv)
  {
    prt = true;
    oldvx = vx;
    oldvy = vy;
    oldwv = wv;
  }
  else
  {
    prt = false;
  }
  
  
  if (prt)
  {
    Serial.print("vx = ");
    Serial.print(vx);
    Serial.print(", vy = ");
    Serial.print(vy);
    Serial.print(", wv = ");
    Serial.print(wv);
    Serial.println(".");
  }
}


#define R00 1.0
#define R01 (-1.0)
#define R02 (-K)
#define R10 1.0
#define R11 1.0
#define R12 (-K)
#define R20 1.0
#define R21 (-1.0)
#define R22 K
#define R30 1.0
#define R31 1.0
#define R32 K


// units are mm/sec
void move(float vx, float vy, float wv)
{
  
   float w1 = (R00 * vx + R01 * vy + R02 * wv) / WHEEL_RADIUS;
   float w2 = (R10 * vx + R11 * vy + R12 * wv) / WHEEL_RADIUS;
   float w3 = (R20 * vx + R21 * vy + R22 * wv) / WHEEL_RADIUS;
   float w4 = (R30 * vx + R31 * vy + R32 * wv) / WHEEL_RADIUS;

   // results are in 1/s after dividing out wheel radius

   scale(w1, w2, w3, w4);
 
   if (prt)
   {
     Serial.print("w1=");
     Serial.print(w1);
     Serial.print(", w2=");
     Serial.print(w2);
     Serial.print(", w3=");
     Serial.print(w3);
     Serial.print(", w4=");
     Serial.print(w4);
     Serial.println(".");
   }

   drive(w1, w2, w3, w4);
}

float scale(float &a, float &b, float &c, float &d)
{
  float aa = abs(a);
  float ab = abs(b);
  float ac = abs(c);
  float ad = abs(d);

  float m = aa;
  if (ab > m) m = ab;
  if (ac > m) m = ac;
  if (ad > m) m = ad;
  
  if (m > MAX_RPS)
    {
      float ratio = MAX_RPS / m;
      a *= ratio;
      b *= ratio;
      c *= ratio;
      d *= ratio;
    }
}


void allaheadfull()
{
  sabertooth(ST_FRONT, ST_LEFT, ST_FORWARD_MAX);
  sabertooth(ST_REAR, ST_LEFT, ST_FORWARD_MAX);
  sabertooth(ST_FRONT, ST_RIGHT, ST_FORWARD_MAX);
  sabertooth(ST_REAR, ST_RIGHT, ST_FORWARD_MAX);
}

// -100 to +100
void drive(float fl, float rl, float rr, float fr)
{
  sabertooth(ST_FRONT, ST_LEFT, saberscale(fl));
  sabertooth(ST_REAR, ST_LEFT, saberscale(rl));
  sabertooth(ST_REAR, ST_RIGHT, saberscale(rr));
  sabertooth(ST_FRONT, ST_RIGHT, saberscale(fr));
}

int saberscale(float input)
{
   int output = fmap(input, -MAX_RPS, +MAX_RPS, ST_BACKWARDS_FAST, ST_FORWARD_FAST);
   
  if (output < ST_BACKWARDS_MAX)
    output = ST_BACKWARDS_MAX;
  if (output > ST_FORWARD_MAX)
    output = ST_FORWARD_MAX;

  return output;
}


void stop()
{
  sabertooth(ST_FRONT, ST_RIGHT, ST_STOP);
  sabertooth(ST_FRONT, ST_LEFT,  ST_STOP);
  sabertooth(ST_REAR, ST_RIGHT, ST_STOP);
  sabertooth(ST_REAR, ST_LEFT, ST_STOP);
}


void sabertooth(byte address, byte command, byte data)
{
  Serial1.write(address);
  Serial1.write(command);
  Serial1.write(data);
  Serial1.write((byte)((address + command + data) & 127));
}

#define VEX_IME_DEFAULT_ADDRESS 0x30

#define VEX_IME_ADDRESS_1 0x10
#define VEX_IME_ADDRESS_2 0x11
#define VEX_IME_ADDRESS_3 0x12
#define VEX_IME_ADDRESS_4 0x13

#define IS_TERMINATED(status) ((status & 1) != 0)
#define IME_OVERFLOW(status) ((status & 2) != 0)
#define DIAGNOSTIC(status) ((status & 4) != 0)
#define IS_IME ((status & 0x00ff0000) == 0x00010000)
#define IS_SMALL_IME ((status & 0x0000ff00) == 0x00000200)
#define IS_LARGE_IME ((status & 0x0000ff00) == 0x00000300)


void vex_ime_setup()
{
  while(!vex_i2c_reset())
  {
    delay(100);
  }

  vex_init_ime(VEX_IME_ADDRESS_1, false);
  vex_init_ime(VEX_IME_ADDRESS_2, false);
  vex_init_ime(VEX_IME_ADDRESS_3, false);
  vex_init_ime(VEX_IME_ADDRESS_4, true);

}



boolean vex_i2c_reset()
{
  log("i2c reset");
  Wire.beginTransmission(0x00);
  Wire.write(byte(0x4E));
  Wire.write(byte(0xCA));
  Wire.write(byte(0x03));
  delay(500);
  int ret =  Wire.endTransmission();
  logbyte(ret);
  return ret == 0;
}

boolean vex_init_ime(byte addr, boolean terminate)
{
  Serial.print("init ime ");
  Serial.println(addr);

  boolean result = true;
  result = result && vex_i2c_assign_address(addr);
  result = result && vex_ime_clear(addr);
  result = result && vex_i2c_terminate(addr, terminate);
  int status = vex_ime_status(addr);
  Serial.print("status was ");
  Serial.println(status);
  return result && (terminate == IS_TERMINATED(status));
}

boolean vex_i2c_assign_address(byte addr)
{
  log("assign address");
  Wire.beginTransmission(VEX_IME_DEFAULT_ADDRESS);
  Wire.write(byte(0x4D));
  Wire.write(byte(addr << 1));
  int ret = Wire.endTransmission();

  logbyte(ret);

  return ret == 0;
}


int vex_ime_status(byte addr)
{
  log("status");
  Wire.beginTransmission(addr);
  Wire.write(byte(0x20));
  Wire.endTransmission();
  Wire.requestFrom(addr, byte(4));

  while (Wire.available() < 4)
    {
      // spin
    }

  byte b0 = Wire.read();
  byte b1 = Wire.read();
  byte b2 = Wire.read();
  byte b3 = Wire.read();

  return b0 << 24 | b1 << 16 | b2 << 8 | b3;
}


boolean vex_ime_clear(byte addr)
{
  log("clear");
  Wire.beginTransmission(addr);
  Wire.write(byte(0x4a));
  int ret =  Wire.endTransmission();
  logbyte(ret);

  return ret == 0;
}

boolean vex_i2c_print_device_info(byte addr)
{
  Wire.beginTransmission(addr);
  Wire.write(byte(0x00));
  int result = Wire.endTransmission();

  if (result != 0)
    {
      return false;
    }

  Wire.requestFrom(addr, byte(0x18));

  while (Wire.available())
    {
      char c = Wire.read();
      Serial.print(c);
    }

  Serial.println("");
  return true;
}



boolean vex_i2c_terminate(byte addr, boolean enable)
{

    Wire.beginTransmission(addr);
    if (enable)
    {
      log("terminate enable");
        Wire.write(byte(0x4C));
    }
    else
    {
      log("terminate disable");
        Wire.write(byte(0x4B));
    }
    int ret = Wire.endTransmission();
    logbyte(ret);
    return ret == 0;
}

void logbyte(byte message)
{
  Serial.println(message);
  Serial.flush();
}
void log(String message)
{
  Serial.println(message);
  Serial.flush();
}

float w1_rps;
int w1_ticks;
float w2_rps;
int w2_ticks;
float w3_rps;
int w3_ticks;
float w4_rps;
int w4_ticks;

void log_wheel_speeds()
{
  vex_ime_update(VEX_IME_ADDRESS_1, w1_rps, w1_ticks);
  vex_ime_update(VEX_IME_ADDRESS_2, w2_rps, w2_ticks);
  vex_ime_update(VEX_IME_ADDRESS_3, w3_rps, w3_ticks);
  vex_ime_update(VEX_IME_ADDRESS_4, w4_rps, w4_ticks);

  Serial.print("wheel 1: ");
  Serial.print(w1_rps);
  Serial.print(" ");
  Serial.println(w1_ticks);
  Serial.print("wheel 2: ");
  Serial.print(w2_rps);
  Serial.print(" ");
  Serial.println(w2_ticks);
  Serial.print("wheel 3: ");
  Serial.print(w3_rps);
  Serial.print(" ");
  Serial.println(w3_ticks);
  Serial.print("wheel 4: ");
  Serial.print(w4_rps);
  Serial.print(" ");
  Serial.println(w4_ticks);
}


boolean vex_ime_update(byte addr, float& speed, int& ticks)
{
  Wire.beginTransmission(addr);
  Wire.write(0x40);
  Wire.endTransmission();

  Wire.requestFrom(addr, byte(0x8));

  int count = 100000;
  while (Wire.available() < 8)
    {
      if (--count == 0)
	{
	  log("gave up");
	  logbyte(Wire.available());
	  return false;
	}
    }

  byte a = Wire.read();
  byte b = Wire.read();
  byte c = Wire.read();
  byte d = Wire.read();
  byte e = Wire.read();
  byte f = Wire.read();
  byte g = Wire.read();
  byte h = Wire.read();

  ticks = c << 24 | d << 16 | a << 8 | b;

  int vel = e << 8 | f;

  if (vel == 0xffff)
    {
      speed = 0.0;
    }
  else
    {
      speed = (1.0 / (vel * 0.000064 * 2)) * (1.0/39.2);
    }

  return true;
}

