#include <Arduino.h>
#include <SoftwareSerial.h>

const byte rxPin = 8;
const byte txPin = 7;
SoftwareSerial bridge(rxPin, txPin);

// const uint8_t lightPin = 3;

const uint8_t redPin = 9;
const uint8_t greenPin = 10;
const uint8_t bluePin = 11;

const uint8_t motionSensorInternalPin = 12;
const uint8_t motionSensorExternalPin = 4;

const uint8_t switchPin = 2;
const uint8_t naturalLightPin = A0;
const bool DEBUG_MODE = true;

// const int naturalLightThreshold = 80;

volatile int lightState;
volatile bool switchOn;
volatile bool notifiedOn;
volatile bool notifiedOff;
volatile bool autoMode;

int naturalLightIntensity;
int naturalLightThreshold = 80;
unsigned long timestamp;
unsigned long timestampExternal;
unsigned long timestampInternal;
const unsigned long samplingTime = 6000;
const unsigned long checkTime = 3000;
const unsigned long debounceDelay = 200;
volatile unsigned long lastDebounceTime = 0;
volatile int lastButtonState = LOW;

enum PersonState
{
  IN,
  OUT,
  HALFWAY_IN,
  HALFWAY_OUT,
};

volatile PersonState currentPersonState;
volatile PersonState futurePersonState;
int peopleInTheRoom;
int valExt;
int valInt;
// int previousLightIntensity;
int red;
int green;
int blue;

/* -------------- DATA TO SEND -------------------- */

const byte AUTO_ENABLED = 238;
const byte AUTO_DISABLED = 239;
const byte PEOPLE_IN_THE_ROOM = 240; // random byte to be distinguished from other event signals

// LIGHT STATES AND CAUSES

const byte AUTO_OFF = 0;
const byte SWITCH_OFF = 2;
const byte SWITCH_ON = 3;
const byte MOBILE_APP_OFF = 4;
const byte VOICECOMMAND_OFF = 6;

// Signals

// The format is COLOR_MODE_INTENSITY (HIGH = 255, MEDIUM = 64, LOW = 8)

const byte RED_VOICE_HIGH = 8;
const byte RED_VOICE_MEDIUM = 9;
const byte RED_VOICE_LOW = 10;

const byte RED_MOBILE_APP_HIGH = 11;
const byte RED_MOBILE_APP_MEDIUM = 12;
const byte RED_MOBILE_APP_LOW = 13;

const byte RED_AUTO_HIGH = 14;
const byte RED_AUTO_MEDIUM = 15;
const byte RED_AUTO_LOW = 16;

const byte RED_SWITCH_HIGH = 17;
const byte RED_SWITCH_MEDIUM = 18;
const byte RED_SWITCH_LOW = 19;

const byte GREEN_VOICE_HIGH = 20;
const byte GREEN_VOICE_MEDIUM = 21;
const byte GREEN_VOICE_LOW = 22;

const byte GREEN_MOBILE_APP_HIGH = 23;
const byte GREEN_MOBILE_APP_MEDIUM = 24;
const byte GREEN_MOBILE_APP_LOW = 25;

const byte GREEN_AUTO_HIGH = 26;
const byte GREEN_AUTO_MEDIUM = 27;
const byte GREEN_AUTO_LOW = 28;

const byte GREEN_SWITCH_HIGH = 29;
const byte GREEN_SWITCH_MEDIUM = 30;
const byte GREEN_SWITCH_LOW = 31;

const byte BLUE_VOICE_HIGH = 32;
const byte BLUE_VOICE_MEDIUM = 33;
const byte BLUE_VOICE_LOW = 34;

const byte BLUE_MOBILE_APP_HIGH = 35;
const byte BLUE_MOBILE_APP_MEDIUM = 36;
const byte BLUE_MOBILE_APP_LOW = 37;

const byte BLUE_AUTO_HIGH = 38;
const byte BLUE_AUTO_MEDIUM = 39;
const byte BLUE_AUTO_LOW = 40;

const byte BLUE_SWITCH_HIGH = 41;
const byte BLUE_SWITCH_MEDIUM = 42;
const byte BLUE_SWITCH_LOW = 43;

const byte YELLOW_VOICE_HIGH = 44;
const byte YELLOW_VOICE_MEDIUM = 45;
const byte YELLOW_VOICE_LOW = 46;

const byte YELLOW_MOBILE_APP_HIGH = 47;
const byte YELLOW_MOBILE_APP_MEDIUM = 48;
const byte YELLOW_MOBILE_APP_LOW = 49;

const byte YELLOW_AUTO_HIGH = 50;
const byte YELLOW_AUTO_MEDIUM = 51;
const byte YELLOW_AUTO_LOW = 52;

const byte YELLOW_SWITCH_HIGH = 53;
const byte YELLOW_SWITCH_MEDIUM = 54;
const byte YELLOW_SWITCH_LOW = 55;

const byte PURPLE_VOICE_HIGH = 56;
const byte PURPLE_VOICE_MEDIUM = 57;
const byte PURPLE_VOICE_LOW = 58;

const byte PURPLE_MOBILE_APP_HIGH = 59;
const byte PURPLE_MOBILE_APP_MEDIUM = 60;
const byte PURPLE_MOBILE_APP_LOW = 61;

const byte PURPLE_AUTO_HIGH = 62;
const byte PURPLE_AUTO_MEDIUM = 63;
const byte PURPLE_AUTO_LOW = 64;

const byte PURPLE_SWITCH_HIGH = 65;
const byte PURPLE_SWITCH_MEDIUM = 66;
const byte PURPLE_SWITCH_LOW = 67;

const byte PINK_VOICE_HIGH = 68;
const byte PINK_VOICE_MEDIUM = 69;
const byte PINK_VOICE_LOW = 70;

const byte PINK_MOBILE_APP_HIGH = 71;
const byte PINK_MOBILE_APP_MEDIUM = 72;
const byte PINK_MOBILE_APP_LOW = 73;

const byte PINK_AUTO_HIGH = 74;
const byte PINK_AUTO_MEDIUM = 75;
const byte PINK_AUTO_LOW = 76;

const byte PINK_SWITCH_HIGH = 77;
const byte PINK_SWITCH_MEDIUM = 78;
const byte PINK_SWITCH_LOW = 79;

const byte ORANGE_VOICE_HIGH = 80;
const byte ORANGE_VOICE_MEDIUM = 81;
const byte ORANGE_VOICE_LOW = 82;

const byte ORANGE_MOBILE_APP_HIGH = 83;
const byte ORANGE_MOBILE_APP_MEDIUM = 84;
const byte ORANGE_MOBILE_APP_LOW = 85;

const byte ORANGE_AUTO_HIGH = 86;
const byte ORANGE_AUTO_MEDIUM = 87;
const byte ORANGE_AUTO_LOW = 88;

const byte ORANGE_SWITCH_HIGH = 89;
const byte ORANGE_SWITCH_MEDIUM = 90;
const byte ORANGE_SWITCH_LOW = 91;

const byte WHITE_VOICE_HIGH = 92;
const byte WHITE_VOICE_MEDIUM = 93;
const byte WHITE_VOICE_LOW = 94;

const byte WHITE_MOBILE_APP_HIGH = 95;
const byte WHITE_MOBILE_APP_MEDIUM = 96;
const byte WHITE_MOBILE_APP_LOW = 97;

const byte WHITE_AUTO_HIGH = 98;
const byte WHITE_AUTO_MEDIUM = 99;
const byte WHITE_AUTO_LOW = 100;

const byte WHITE_SWITCH_HIGH = 101;
const byte WHITE_SWITCH_MEDIUM = 102;
const byte WHITE_SWITCH_LOW = 103;

volatile byte lastColorIntensity;

// Variables to make the code more readable

const byte RED = 200;
const byte GREEN = 201;
const byte BLUE = 202;
const byte YELLOW = 203;
const byte PURPLE = 205;
const byte PINK = 206;
const byte ORANGE = 207;
const byte WHITE = 208;

// Light Intensities

const byte highIntensity = 255;
const byte mediumIntensity = 64;
const byte lowIntensity = 8;
const byte offIntensity = 0;

/* --------------------------------------------------- */

void onSwitchPressed(void);
void onSerialInput(void);
void onNaturalLight(void);
void debug(char mode);
void whatIsCurrentState(void);
void howManyPeople(void);
void lightsOff(void);
byte getColor(void);
byte mapValuesAndGetSignal(byte color, byte intensity);
byte mapIntensity(int intensity);
byte getIntensity(void);

void setup()
{
  peopleInTheRoom = 0;
  currentPersonState = OUT;
  lightState = LOW;
  switchOn = false;
  notifiedOn = false;
  notifiedOff = false;
  autoMode = true;
  // previousLightIntensity = 0;
  red = 255;
  green = 255;
  blue = 255;
  lastColorIntensity = 254;
  timestamp = millis();
  timestampExternal = millis() - samplingTime;
  timestampInternal = millis() - samplingTime;
  pinMode(switchPin, INPUT);
  // pinMode(lightPin, OUTPUT);
  pinMode(naturalLightPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(motionSensorInternalPin, INPUT);
  pinMode(motionSensorExternalPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(switchPin), onSwitchPressed, RISING);
  bridge.begin(9600);
  Serial.begin(9600);
}

void loop()
{

  onSerialInput();

  // TODO test the new FSM

  valExt = digitalRead(motionSensorExternalPin);

  if (valExt == HIGH)
  {
    if (millis() - timestampExternal > samplingTime)
    {
      Serial.println("The ext sensor read something");
      if (currentPersonState == OUT)
        futurePersonState = HALFWAY_IN;

      /* OLD VERSION
      if (currentPersonState == HALFWAY_IN)
      {
        if (peopleInTheRoom == 0)
        {
          futurePersonState = OUT;
          if (switchOn)
            switchOn = false;
          lightsOff();
          autoMode = true;
        }
        else
        {
          futurePersonState = IN;
        }
      }
      */

      // NEW VERSION
      if (currentPersonState == HALFWAY_IN)
        futurePersonState = HALFWAY_IN;

      if (currentPersonState == IN)
        futurePersonState = HALFWAY_IN;

      if (currentPersonState == HALFWAY_OUT)
      {
        if (peopleInTheRoom == 0)
        {
          futurePersonState = OUT;
          if (switchOn)
            switchOn = false;
          lightsOff();
          autoMode = true;
          Serial.println("Auto mode enabled");
          bridge.write(AUTO_ENABLED);
        }
        else
        {
          futurePersonState = IN;
          if (!switchOn)
            onNaturalLight();
        }
      }
      currentPersonState = futurePersonState;
      // whatIsCurrentState();
      timestampExternal = millis();
    }
  }

  valInt = digitalRead(motionSensorInternalPin);
  if (valInt == HIGH)
  {
    if (millis() - timestampInternal > samplingTime)
    {
      Serial.println("The int sensor read something");
      if (currentPersonState == HALFWAY_IN)
      {
        futurePersonState = IN;
        peopleInTheRoom = peopleInTheRoom + 1;
        bridge.write(PEOPLE_IN_THE_ROOM);
        bridge.write(peopleInTheRoom);
        if (!switchOn)
          onNaturalLight();
      }

      if (currentPersonState == IN)
      {
        futurePersonState = HALFWAY_OUT;
        peopleInTheRoom = peopleInTheRoom - 1;
        bridge.write(PEOPLE_IN_THE_ROOM);
        bridge.write(peopleInTheRoom);
      }
      /* OLD VERSION
      if (currentPersonState == HALFWAY_OUT)
      {
        futurePersonState = IN;
        peopleInTheRoom = peopleInTheRoom + 1;
        bridge.write(PEOPLE_IN_THE_ROOM);
        bridge.write(peopleInTheRoom);
        if (!switchOn)
          onNaturalLight();
      }
      */
      // NEW VERSION
      if (currentPersonState == HALFWAY_OUT)
        futurePersonState = HALFWAY_OUT;

      currentPersonState = futurePersonState;
      // whatIsCurrentState();
      timestampInternal = millis();
    }
  }
  // NEW VERSION
  if (valExt == LOW && valInt == LOW)
  {
    if (currentPersonState == HALFWAY_IN && peopleInTheRoom == 0)
    {
      currentPersonState = OUT;
      if (switchOn)
        switchOn = false;
      lightsOff();
      autoMode = true;
      // Serial.println("Auto mode enabled");
      bridge.write(AUTO_ENABLED);
    }
    if (currentPersonState == HALFWAY_OUT)
    {
      currentPersonState = IN;
      peopleInTheRoom = peopleInTheRoom + 1;
      bridge.write(PEOPLE_IN_THE_ROOM);
      bridge.write(peopleInTheRoom);
      if (!switchOn)
        onNaturalLight();
    }
  }

  if (millis() - timestamp > checkTime)
  {
    if (currentPersonState == IN && !switchOn)
    {
      onNaturalLight();
      timestamp = millis();
    }
  }
}

void onSwitchPressed()
{
  noInterrupts();
  if (millis() - lastDebounceTime > debounceDelay)
  {
    lightState = lightState > 0 ? LOW : HIGH;
    if (lightState > 0)
    {
      byte color = getColor();
      byte intensity = getIntensity();
      byte colorIntensity = mapValuesAndGetSignal(color, intensity);
      analogWrite(redPin, red);
      analogWrite(greenPin, green);
      analogWrite(bluePin, blue);
      // Serial.println("sending SWITCH signal");
      bridge.write(colorIntensity);
      lastColorIntensity = colorIntensity;
    }
    else
    {
      analogWrite(redPin, lightState);
      analogWrite(greenPin, lightState);
      analogWrite(bluePin, lightState);
      // Serial.println("sending SWITCH OFF signal");
      bridge.write(SWITCH_OFF);
    }
    switchOn = !switchOn;
    lastDebounceTime = millis();
  }

  interrupts();
}

byte getIntensity()
{
  if ((lastColorIntensity - 8) % 3 == 0)
    return highIntensity;
  if (lastColorIntensity % 3 == 0)
    return mediumIntensity;
  if ((lastColorIntensity - 10) % 3 == 0)
    return lowIntensity;
}

void onSerialInput()
{
  if (bridge.available() > 0)
  {
    // Serial.println("Reading from bridge");
    //  look for the next valid integer in the incoming serial stream:
    int value = bridge.parseInt(SKIP_ALL, '\n');
    Serial.print("value: ");
    Serial.println(value);

    if (value >= 0)
    {

      red = value;
      // do it again:
      green = bridge.parseInt();
      // do it again:
      blue = bridge.parseInt();

      int mode = bridge.parseInt(SKIP_ALL, '\n');

      Serial.print("red: ");
      Serial.print(red);
      Serial.print(" green: ");
      Serial.print(green);
      Serial.print(" blue: ");
      Serial.print(blue);
      Serial.print(" mode: ");
      Serial.println(mode); // 1 = mobile app, 0 voice

      if (red == green && green == blue && red > 0) // shade of white
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (red == 8)
          {
            // Serial.println("sending WHITE MOBILE APP LOW signal");
            bridge.write(WHITE_MOBILE_APP_LOW);
            lastColorIntensity = WHITE_MOBILE_APP_LOW;
          }
          if (red == 64)
          {
            // Serial.println("sending WHITE MOBILE APP MEDIUM signal");
            bridge.write(WHITE_MOBILE_APP_MEDIUM);
            lastColorIntensity = WHITE_MOBILE_APP_MEDIUM;
          }
          if (red == 255)
          {
            // Serial.println("sending WHITE MOBILE APP HIGH signal");
            bridge.write(WHITE_MOBILE_APP_HIGH);
            lastColorIntensity = WHITE_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (red == 8)
          {
            // Serial.println("sending WHITE VOICE LOW signal");
            bridge.write(WHITE_VOICE_LOW);
            lastColorIntensity = WHITE_VOICE_LOW;
          }
          if (red == 64)
          {
            // Serial.println("sending WHITE VOICE MEDIUM signal");
            bridge.write(WHITE_VOICE_MEDIUM);
            lastColorIntensity = WHITE_VOICE_MEDIUM;
          }
          if (red == 255)
          {
            // Serial.println("sending WHITE VOICE HIGH signal");
            bridge.write(WHITE_VOICE_HIGH);
            lastColorIntensity = WHITE_VOICE_HIGH;
          }
        }
        lightState = HIGH;
      }
      if (red == 0 && green == 0 && blue == 0) // off
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          // Serial.println("sending MOBILE APP OFF signal");
          bridge.write(MOBILE_APP_OFF);
        }
        if (mode == 0)
        {
          // Serial.println("sending VOICECOMMAND OFF signal");
          bridge.write(VOICECOMMAND_OFF);
        }
        lightState = LOW;
      }
      if (red > 0 && green == 0 && blue == 0) // shade of red
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (red == 8)
          {
            // Serial.println("sending RED MOBILE APP LOW signal");
            bridge.write(RED_MOBILE_APP_LOW);
            lastColorIntensity = RED_MOBILE_APP_LOW;
          }
          if (red == 64)
          {
            // Serial.println("sending RED MOBILE APP MEDIUM signal");
            bridge.write(RED_MOBILE_APP_MEDIUM);
            lastColorIntensity = RED_MOBILE_APP_MEDIUM;
          }
          if (red == 255)
          {
            // Serial.println("sending RED MOBILE APP HIGH signal");
            bridge.write(RED_MOBILE_APP_HIGH);
            lastColorIntensity = RED_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (red == 8)
          {
            // Serial.println("sending RED VOICE LOW signal");
            bridge.write(RED_VOICE_LOW);
            lastColorIntensity = RED_VOICE_LOW;
          }
          if (red == 64)
          {
            // Serial.println("sending RED VOICE MEDIUM signal");
            bridge.write(RED_VOICE_MEDIUM);
            lastColorIntensity = RED_VOICE_MEDIUM;
          }
          if (red == 255)
          {
            // Serial.println("sending RED VOICE HIGH signal");
            bridge.write(RED_VOICE_HIGH);
            lastColorIntensity = RED_VOICE_HIGH;
          }
        }
        lightState = HIGH;
      }
      if (red == 0 && green > 0 && blue == 0) // shade of green
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (green == 8)
          {
            // Serial.println("sending GREEN MOBILE APP LOW signal");
            bridge.write(GREEN_MOBILE_APP_LOW);
            lastColorIntensity = GREEN_MOBILE_APP_LOW;
          }
          if (green == 64)
          {
            // Serial.println("sending GREEN MOBILE APP MEDIUM signal");
            bridge.write(GREEN_MOBILE_APP_MEDIUM);
            lastColorIntensity = GREEN_MOBILE_APP_MEDIUM;
          }
          if (green == 255)
          {
            // Serial.println("sending GREEN MOBILE APP HIGH signal");
            bridge.write(GREEN_MOBILE_APP_HIGH);
            lastColorIntensity = GREEN_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (green == 8)
          {
            // Serial.println("sending GREEN VOICE LOW signal");
            bridge.write(GREEN_VOICE_LOW);
            lastColorIntensity = GREEN_VOICE_LOW;
          }
          if (green == 64)
          {
            // Serial.println("sending GREEN VOICE MEDIUM signal");
            bridge.write(GREEN_VOICE_MEDIUM);
            lastColorIntensity = GREEN_VOICE_MEDIUM;
          }
          if (green == 255)
          {
            // Serial.println("sending GREEN VOICE HIGH signal");
            bridge.write(GREEN_VOICE_HIGH);
            lastColorIntensity = GREEN_VOICE_HIGH;
          }
        }
        lightState = HIGH;
      }
      if (red == 0 && green == 0 && blue > 0) // shade of blue
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (blue == 8)
          {
            // Serial.println("sending BLUE MOBILE APP LOW signal");
            bridge.write(BLUE_MOBILE_APP_LOW);
            lastColorIntensity = BLUE_MOBILE_APP_LOW;
          }
          if (blue == 64)
          {
            // Serial.println("sending BLUE MOBILE APP MEDIUM signal");
            bridge.write(BLUE_MOBILE_APP_MEDIUM);
            lastColorIntensity = BLUE_MOBILE_APP_MEDIUM;
          }
          if (blue == 255)
          {
            // Serial.println("sending BLUE MOBILE APP HIGH signal");
            bridge.write(BLUE_MOBILE_APP_HIGH);
            lastColorIntensity = BLUE_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (blue == 8)
          {
            // Serial.println("sending BLUE VOICE LOW signal");
            bridge.write(BLUE_VOICE_LOW);
            lastColorIntensity = BLUE_VOICE_LOW;
          }
          if (blue == 64)
          {
            // Serial.println("sending BLUE VOICE MEDIUM signal");
            bridge.write(BLUE_VOICE_MEDIUM);
            lastColorIntensity = BLUE_VOICE_MEDIUM;
          }
          if (blue == 255)
          {
            // Serial.println("sending BLUE VOICE HIGH signal");
            bridge.write(BLUE_VOICE_HIGH);
            lastColorIntensity = BLUE_VOICE_HIGH;
          }
        }
        lightState = HIGH;
      }
      if (red == green && blue == 0) // shade of yellow
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (red == 8 && green == 8)
          {
            // Serial.println("sending YELLOW MOBILE APP LOW signal");
            bridge.write(YELLOW_MOBILE_APP_LOW);
            lastColorIntensity = YELLOW_MOBILE_APP_LOW;
          }
          if (red == 64 && green == 64)
          {
            // Serial.println("sending YELLOW MOBILE APP MEDIUM signal");
            bridge.write(YELLOW_MOBILE_APP_MEDIUM);
            lastColorIntensity = YELLOW_MOBILE_APP_MEDIUM;
          }
          if (red == 255 && green == 255)
          {
            // Serial.println("sending YELLOW MOBILE APP HIGH signal");
            bridge.write(YELLOW_MOBILE_APP_HIGH);
            lastColorIntensity = YELLOW_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (red == 8 && green == 8)
          {
            // Serial.println("sending YELLOW VOICE LOW signal");
            bridge.write(YELLOW_VOICE_LOW);
            lastColorIntensity = YELLOW_VOICE_LOW;
          }
          if (red == 64 && green == 64)
          {
            // Serial.println("sending YELLOW VOICE MEDIUM signal");
            bridge.write(YELLOW_VOICE_MEDIUM);
            lastColorIntensity = YELLOW_VOICE_MEDIUM;
          }
          if (red == 255 && green == 255)
          {
            // Serial.println("sending YELLOW VOICE HIGH signal");
            bridge.write(YELLOW_VOICE_HIGH);
            lastColorIntensity = YELLOW_VOICE_HIGH;
          }
        }
        lightState = HIGH;
      }
      if (red > 0 && (green >= red / 2 && green < red) && blue == 0) // shade of orange
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (red == 8 && green == 4)
          {
            // Serial.println("sending ORANGE MOBILE APP LOW signal");
            bridge.write(ORANGE_MOBILE_APP_LOW);
            lastColorIntensity = ORANGE_MOBILE_APP_LOW;
          }
          if (red == 64 && green == 32)
          {
            // Serial.println("sending ORANGE MOBILE APP MEDIUM signal");
            bridge.write(ORANGE_MOBILE_APP_MEDIUM);
            lastColorIntensity = ORANGE_MOBILE_APP_MEDIUM;
          }
          if (red == 255 && green == 128)
          {
            // Serial.println("sending ORANGE MOBILE APP HIGH signal");
            bridge.write(ORANGE_MOBILE_APP_HIGH);
            lastColorIntensity = ORANGE_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (red == 8 && green == 4)
          {
            // Serial.println("sending ORANGE VOICE LOW signal");
            bridge.write(ORANGE_VOICE_LOW);
            lastColorIntensity = ORANGE_VOICE_LOW;
          }
          if (red == 64 && green == 32)
          {
            // Serial.println("sending ORANGE VOICE MEDIUM signal");
            bridge.write(ORANGE_VOICE_MEDIUM);
            lastColorIntensity = ORANGE_VOICE_MEDIUM;
          }
          if (red == 255 && green == 128)
          {
            // Serial.println("sending ORANGE VOICE HIGH signal");
            bridge.write(ORANGE_VOICE_HIGH);
            lastColorIntensity = ORANGE_VOICE_HIGH;
          }
        }
        lightState = HIGH;
      }
      if ((red <= blue / 2 && red > 0) && green == 0 && blue > 0) // shade of purple
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (red == 4 && blue == 8)
          {
            // Serial.println("sending PURPLE MOBILE APP LOW signal");
            bridge.write(PURPLE_MOBILE_APP_LOW);
            lastColorIntensity = PURPLE_MOBILE_APP_LOW;
          }
          if (red == 32 && blue == 64)
          {
            // Serial.println("sending PURPLE MOBILE APP MEDIUM signal");
            bridge.write(PURPLE_MOBILE_APP_MEDIUM);
            lastColorIntensity = PURPLE_MOBILE_APP_MEDIUM;
          }
          if (red == 127 && blue == 255)
          {
            // Serial.println("sending PURPLE MOBILE APP HIGH signal");
            bridge.write(PURPLE_MOBILE_APP_HIGH);
            lastColorIntensity = PURPLE_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (red == 4 && blue == 8)
          {
            // Serial.println("sending PURPLE VOICE LOW signal");
            bridge.write(PURPLE_VOICE_LOW);
            lastColorIntensity = PURPLE_VOICE_LOW;
          }
          if (red == 32 && blue == 64)
          {
            // Serial.println("sending PURPLE VOICE MEDIUM signal");
            bridge.write(PURPLE_VOICE_MEDIUM);
            lastColorIntensity = PURPLE_VOICE_MEDIUM;
          }
          if (red == 127 && blue == 255)
          {
            // Serial.println("sending PURPLE VOICE HIGH signal");
            bridge.write(PURPLE_VOICE_HIGH);
            lastColorIntensity = PURPLE_VOICE_HIGH;
          }
        }
        lightState = HIGH;
      }
      if (red > 0 && green == 0 && (blue <= red / 2 && blue > 0)) // shade of pink
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1) // mobile app
        {
          if (red == 8 && blue == 4)
          {
            // Serial.println("sending PINK MOBILE APP LOW signal");
            bridge.write(PINK_MOBILE_APP_LOW);
            lastColorIntensity = PINK_MOBILE_APP_LOW;
          }
          if (red == 64 && blue == 32)
          {
            // Serial.println("sending PINK MOBILE APP MEDIUM signal");
            bridge.write(PINK_MOBILE_APP_MEDIUM);
            lastColorIntensity = PINK_MOBILE_APP_MEDIUM;
          }
          if (red == 255 && blue == 127)
          {
            // Serial.println("sending PINK MOBILE APP HIGH signal");
            bridge.write(PINK_MOBILE_APP_HIGH);
            lastColorIntensity = PINK_MOBILE_APP_HIGH;
          }
        }
        if (mode == 0) // voice
        {
          if (red == 8 && blue == 4)
          {
            // Serial.println("sending PINK VOICE LOW signal");
            bridge.write(PINK_VOICE_LOW);
            lastColorIntensity = PINK_VOICE_LOW;
          }
          if (red == 64 && blue == 32)
          {
            // Serial.println("sending PINK VOICE MEDIUM signal");
            bridge.write(PINK_VOICE_MEDIUM);
            lastColorIntensity = PINK_VOICE_MEDIUM;
          }
          if (red == 255 && blue == 127)
          {
            // Serial.println("sending PINK VOICE HIGH signal");
            bridge.write(PINK_VOICE_HIGH);
            lastColorIntensity = PINK_VOICE_HIGH;
            ;
          }
        }
        lightState = HIGH;
      }
      autoMode = false;
      bridge.write(AUTO_DISABLED);
    }
    else
    {
      autoMode = value == -1 ? true : false;
      if (autoMode)
      {
        // Serial.println("Auto mode enabled");
        bridge.write(AUTO_ENABLED);
      }
      else
      {
        // Serial.println("Auto mode disabled");
        bridge.write(AUTO_DISABLED);
      }
    }
  }
}

void lightsOff()
{
  // Serial.println("I'm in the lightsOff() function");
  // howManyPeople();
  lightState = LOW;
  // analogWrite(lightPin, lightState);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
  // Serial.println("sending AUTO OFF signal");
  bridge.write(AUTO_OFF);
}

void onNaturalLight()
{
  if (autoMode)
  {
    naturalLightIntensity = analogRead(naturalLightPin);
    // debug('n');
    if (naturalLightIntensity < naturalLightThreshold)
    {
      // OLD VERSION
      // int previousLightState = lightState;
      // lightState = map(naturalLightIntensity, 0, 1023, 255, 0);
      // analogWrite(lightPin, lightState);

      // NEW VERSION
      int intensity = map(naturalLightIntensity, 0, 1023, 255, 0);
      byte mappedIntensity = mapIntensity(intensity); // TODO implement

      byte color = getColor();

      byte signal = mapValuesAndGetSignal(color, mappedIntensity);

      analogWrite(redPin, red);
      analogWrite(greenPin, green);
      analogWrite(bluePin, blue);

      // Serial.println("sending signal");
      bridge.write(signal);

      if (signal != AUTO_OFF)
        lastColorIntensity = signal;

      /* OLD VERSION
      if (previousLightState == 0)
      {
        Serial.println("sending AUTO ON signal");
        bridge.write(AUTO_ON);
      }
      if (lightState > mediumIntensityThreshold && previousLightIntensity != highIntensityThreshold)
      {
        Serial.println("sending HIGH INTENSITY signal");
        bridge.write(highIntensityThreshold);
        previousLightIntensity = highIntensityThreshold;
      }
      if (lightState > lowIntensityThreshold && lightState <= mediumIntensityThreshold && previousLightIntensity != mediumIntensityThreshold)
      {
        Serial.println("sending MEDIUM INTENSITY signal");
        bridge.write(mediumIntensityThreshold);
        previousLightIntensity = mediumIntensityThreshold;
      }
      if (lightState > 0 && lightState <= lowIntensityThreshold && previousLightIntensity != lowIntensityThreshold)
      {
        Serial.println("sending LOW INTENSITY signal");
        bridge.write(lowIntensityThreshold);
        previousLightIntensity = lowIntensityThreshold;
      }
      */
    }
    else
    {
      lightsOff();
    }
    // debug('l');
  }
}

byte mapIntensity(int intensity)
{
  // TODO
}

byte getColor()
{
  if (lastColorIntensity >= 8 && lastColorIntensity <= 19)
    return RED;
  if (lastColorIntensity >= 20 && lastColorIntensity <= 31)
    return GREEN;
  if (lastColorIntensity >= 32 && lastColorIntensity <= 43)
    return BLUE;
  if (lastColorIntensity >= 44 && lastColorIntensity <= 55)
    return YELLOW;
  if (lastColorIntensity >= 56 && lastColorIntensity <= 67)
    return PURPLE;
  if (lastColorIntensity >= 68 && lastColorIntensity <= 79)
    return PINK;
  if (lastColorIntensity >= 80 && lastColorIntensity <= 91)
    return ORANGE;
  if (lastColorIntensity >= 92 && lastColorIntensity <= 103)
    return WHITE;
}

byte mapValuesAndGetSignal(byte color, byte intensity)
{

  if (intensity == offIntensity)
  {
    red = 0;
    green = 0;
    blue = 0;
    return AUTO_OFF;
  }
  if (intensity == lowIntensity)
  {
    if (color == RED)
    {
      red = 8;
      green = 0;
      blue = 0;
      return RED_AUTO_LOW;
    }
    if (color == WHITE)
    {
      red = 8;
      green = 8;
      blue = 8;
      return WHITE_AUTO_LOW;
    }
    if (color == GREEN)
    {
      red = 0;
      green = 8;
      blue = 0;
      return GREEN_AUTO_LOW;
    }
    if (color == BLUE)
    {
      red = 0;
      green = 0;
      blue = 8;
      return BLUE_AUTO_LOW;
    }
    if (color == YELLOW)
    {
      red = 8;
      green = 8;
      blue = 0;
      return YELLOW_AUTO_LOW;
    }
    if (color == ORANGE)
    {
      red = 8;
      green = 4;
      blue = 0;
      return ORANGE_AUTO_LOW;
    }
    if (color == PURPLE)
    {
      red = 4;
      green = 0;
      blue = 8;
      return PURPLE_AUTO_LOW;
    }
    if (color == PINK)
    {
      red = 8;
      green = 0;
      blue = 4;
      return PINK_AUTO_LOW;
    }
  }

  if (intensity == mediumIntensity)
  {
    if (color == WHITE)
    {
      red = 64;
      green = 64;
      blue = 64;
      return WHITE_AUTO_MEDIUM;
    }
    if (color == RED)
    {
      red = 64;
      green = 0;
      blue = 0;
      return RED_AUTO_MEDIUM;
    }
    if (color == GREEN)
    {
      red = 0;
      green = 64;
      blue = 0;
      return GREEN_AUTO_MEDIUM;
    }
    if (color == BLUE)
    {
      red = 0;
      green = 0;
      blue = 64;
      return BLUE_AUTO_MEDIUM;
    }
    if (color == YELLOW)
    {
      red = 64;
      green = 64;
      blue = 0;
      return YELLOW_AUTO_MEDIUM;
    }
    if (color == ORANGE)
    {
      red = 64;
      green = 32;
      blue = 0;
      return ORANGE_AUTO_MEDIUM;
    }
    if (color == PURPLE)
    {
      red = 32;
      green = 0;
      blue = 64;
      return PURPLE_AUTO_MEDIUM;
    }
    if (color == PINK)
    {
      red = 64;
      green = 0;
      blue = 32;
      return PINK_AUTO_MEDIUM;
    }
  }

  if (intensity == highIntensity)
  {
    if (color == WHITE)
    {
      red = 255;
      green = 255;
      blue = 255;
      return WHITE_AUTO_HIGH;
    }
    if (color == RED)
    {
      red = 255;
      green = 0;
      blue = 0;
      return RED_AUTO_HIGH;
    }
    if (color == GREEN)
    {
      red = 0;
      green = 255;
      blue = 0;
      return GREEN_AUTO_HIGH;
    }
    if (color == BLUE)
    {
      red = 0;
      green = 0;
      blue = 255;
      return BLUE_AUTO_HIGH;
    }
    if (color == YELLOW)
    {
      red = 255;
      green = 255;
      blue = 0;
      return YELLOW_AUTO_HIGH;
    }
    if (color == ORANGE)
    {
      red = 255;
      green = 128;
      blue = 0;
      return ORANGE_AUTO_HIGH;
    }
    if (color == PURPLE)
    {
      red = 127;
      green = 0;
      blue = 255;
      return PURPLE_AUTO_HIGH;
    }
    if (color == PINK)
    {
      red = 255;
      green = 0;
      blue = 127;
      return PINK_AUTO_HIGH;
    }
  }
}

void debug(char mode)
{
  if (DEBUG_MODE)
  {
    if (mode == 'l')
    {
      Serial.print("lightState: ");
      Serial.println(lightState);
    }
    if (mode == 'n')
    {
      Serial.print("naturalLightIntensity: ");
      Serial.println(naturalLightIntensity);
    }
    if (mode == 's')
    {
      Serial.print("switchOn: ");
      Serial.println(switchOn);
    }
  }
}

void whatIsCurrentState()
{
  Serial.print("current state is ");
  if (currentPersonState == IN)
    Serial.print("IN");
  if (currentPersonState == OUT)
    Serial.print("OUT");
  if (currentPersonState == HALFWAY_IN)
    Serial.print("HALFWAY_IN");
  if (currentPersonState == HALFWAY_OUT)
    Serial.print("HALFWAY_OUT");
  if (switchOn)
    Serial.println(" and the switch is ON");
  else
    Serial.println(" and the switch is OFF");
}

void howManyPeople()
{
  Serial.print(peopleInTheRoom);
  Serial.println(" people in the room");
}
