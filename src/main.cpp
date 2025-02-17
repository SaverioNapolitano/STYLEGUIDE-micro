#include <Arduino.h>
#include <SoftwareSerial.h>

const byte rxPin = 8;
const byte txPin = 7;
SoftwareSerial bridge(rxPin, txPin);

const uint8_t redPin = 9;
const uint8_t greenPin = 10;
const uint8_t bluePin = 11;

const uint8_t motionSensorInternalPin = 12;
const uint8_t motionSensorExternalPin = 4;

const uint8_t switchPin = 2;
const uint8_t naturalLightPin = A0;
const bool DEBUG_MODE = true;

volatile int lightState;
volatile bool autoMode;

int naturalLightIntensity;
unsigned long timestamp;
unsigned long timestampExternal;
unsigned long timestampInternal;
const unsigned long samplingTime = 6000;
const unsigned long checkTime = 3000;
const unsigned long debounceDelay = 200;
volatile unsigned long lastDebounceTime = 0;
volatile int buttonState = LOW;

enum PersonState
{
  IN,
  OUT,
  HALFWAY_IN,
  HALFWAY_OUT,
};

volatile PersonState currentPersonState;
volatile PersonState futurePersonState;
byte peopleInTheRoom;
byte peopleInTheRoomWithOffset;
int valExt;
int valInt;
int red;
int green;
int blue;

/* -------------- DATA TO SEND -------------------- */

const byte AUTO_ENABLED = 238;
const byte AUTO_DISABLED = 239;
const byte PEOPLE_IN_THE_ROOM = 240; // random byte to be distinguished from other event signals
const byte peopleInTheRoomOffset = 240;

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
void getCurrentState(void);
void getPeople(void);
void lightsOff(void);
byte getColor(void);
byte mapValuesAndGetSignal(byte color, byte intensity, bool switchPressed);
byte mapIntensity(int intensity);
byte getIntensity(void);

void setup()
{
  peopleInTheRoom = 0;
  currentPersonState = OUT;
  lightState = LOW;
  autoMode = true;
  red = 255;
  green = 255;
  blue = 255;
  lastColorIntensity = 254;
  timestamp = millis();
  timestampExternal = millis() - samplingTime;
  timestampInternal = millis() - samplingTime;
  pinMode(switchPin, INPUT);
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

  if (millis() - timestampExternal > samplingTime)
  {
    valExt = digitalRead(motionSensorExternalPin);
    if (valExt == HIGH)
    {
      Serial.println("The ext sensor read something");
      if (currentPersonState == OUT)
        futurePersonState = HALFWAY_IN;

      if (currentPersonState == HALFWAY_IN)
        futurePersonState = HALFWAY_IN;

      if (currentPersonState == IN)
        futurePersonState = HALFWAY_IN;

      if (currentPersonState == HALFWAY_OUT)
      {
        if (peopleInTheRoom == 0)
        {
          futurePersonState = OUT;
          lightsOff();
          autoMode = true;
          Serial.println("Auto mode enabled");
          bridge.write(AUTO_ENABLED);
        }
        else
        {
          futurePersonState = IN;
          if (autoMode)
            onNaturalLight();
        }
      }
      currentPersonState = futurePersonState;
      getCurrentState();
      timestampExternal = millis();
    }
  }

  if (millis() - timestampInternal > samplingTime)
  {
    valInt = digitalRead(motionSensorInternalPin);
    if (valInt == HIGH)
    {
      Serial.println("The int sensor read something");
      if (currentPersonState == HALFWAY_IN)
      {
        futurePersonState = IN;
        peopleInTheRoom = peopleInTheRoom + 1;
        peopleInTheRoomWithOffset = peopleInTheRoomOffset + peopleInTheRoom;
        bridge.write(peopleInTheRoomWithOffset);
        getPeople();
        if (autoMode)
          onNaturalLight();
      }

      if (currentPersonState == IN)
      {
        futurePersonState = HALFWAY_OUT;
        peopleInTheRoom = peopleInTheRoom - 1;
        getPeople();
        peopleInTheRoomWithOffset = peopleInTheRoomOffset + peopleInTheRoom;
        bridge.write(peopleInTheRoomWithOffset);
      }

      if (currentPersonState == HALFWAY_OUT)
        futurePersonState = HALFWAY_OUT;

      currentPersonState = futurePersonState;
      getCurrentState();
      timestampInternal = millis();
    }
  }

  if (valExt == LOW && valInt == LOW)
  {
    if (currentPersonState == HALFWAY_IN && peopleInTheRoom == 0)
    {
      currentPersonState = OUT;
      lightsOff();
      autoMode = true;
      Serial.println("Auto mode enabled");
      bridge.write(AUTO_ENABLED);
    }
    if (currentPersonState == HALFWAY_IN && peopleInTheRoom > 0)
    {
      currentPersonState = IN;
      lightsOff();
      autoMode = true;
      Serial.println("Auto mode enabled");
      bridge.write(AUTO_ENABLED);
    }
    if (currentPersonState == HALFWAY_OUT)
    {
      currentPersonState = IN;
      peopleInTheRoom = peopleInTheRoom + 1;
      getPeople();
      peopleInTheRoomWithOffset = peopleInTheRoomOffset + peopleInTheRoom;
      bridge.write(peopleInTheRoomWithOffset);
      if (autoMode)
        onNaturalLight();
    }
    getCurrentState();
  }

  if (millis() - timestamp > checkTime)
  {
    getCurrentState();
    if (currentPersonState == IN && autoMode)
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
    Serial.println("lightState: " + String(lightState));
    if (lightState > 0)
    {
      byte color = getColor();
      byte intensity = getIntensity();
      // Serial.println("color: " + String(color) + " intensity: " + String(intensity));
      byte colorIntensity = mapValuesAndGetSignal(color, intensity, true);
      analogWrite(redPin, red);
      analogWrite(greenPin, green);
      analogWrite(bluePin, blue);
      // Serial.println("sending SWITCH signal");
      // Serial.println(colorIntensity);
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
    lastDebounceTime = millis();
    autoMode = !autoMode;
    if (!autoMode)
      bridge.write(AUTO_DISABLED);
    else
      bridge.write(AUTO_ENABLED);
  }
  interrupts();
}

byte getIntensity()
{
  if (lastColorIntensity == 8 || lastColorIntensity == 11 || lastColorIntensity == 14 || lastColorIntensity == 17 || lastColorIntensity == 20 || lastColorIntensity == 23 || lastColorIntensity == 26 || lastColorIntensity == 29 || lastColorIntensity == 32 || lastColorIntensity == 35 || lastColorIntensity == 38 || lastColorIntensity == 41 || lastColorIntensity == 44 || lastColorIntensity == 47 || lastColorIntensity == 50 || lastColorIntensity == 53 || lastColorIntensity == 56 || lastColorIntensity == 59 || lastColorIntensity == 62 || lastColorIntensity == 65 || lastColorIntensity == 68 || lastColorIntensity == 71 || lastColorIntensity == 74 || lastColorIntensity == 77 || lastColorIntensity == 80 || lastColorIntensity == 83 || lastColorIntensity == 86 || lastColorIntensity == 89 || lastColorIntensity == 92 || lastColorIntensity == 95 || lastColorIntensity == 98 || lastColorIntensity == 101)
    return highIntensity;
  if (lastColorIntensity == 9 || lastColorIntensity == 12 || lastColorIntensity == 15 || lastColorIntensity == 18 || lastColorIntensity == 21 || lastColorIntensity == 24 || lastColorIntensity == 27 || lastColorIntensity == 30 || lastColorIntensity == 33 || lastColorIntensity == 36 || lastColorIntensity == 39 || lastColorIntensity == 42 || lastColorIntensity == 45 || lastColorIntensity == 48 || lastColorIntensity == 51 || lastColorIntensity == 54 || lastColorIntensity == 57 || lastColorIntensity == 60 || lastColorIntensity == 63 || lastColorIntensity == 66 || lastColorIntensity == 69 || lastColorIntensity == 72 || lastColorIntensity == 75 || lastColorIntensity == 78 || lastColorIntensity == 81 || lastColorIntensity == 84 || lastColorIntensity == 87 || lastColorIntensity == 90 || lastColorIntensity == 93 || lastColorIntensity == 96 || lastColorIntensity == 99 || lastColorIntensity == 102)
    return mediumIntensity;

  return lowIntensity;
}

void onSerialInput()
{
  if (bridge.available() > 0)
  {
    // Serial.println("Reading from bridge");
    //  look for the next valid integer in the incoming serial stream:
    int value = bridge.parseInt(SKIP_ALL, '\n');
    // Serial.print("value: ");
    // Serial.println(value);

    if (value >= 0)
    {

      red = value;
      // do it again:
      green = bridge.parseInt();
      // do it again:
      blue = bridge.parseInt();

      int mode = bridge.parseInt(SKIP_ALL, '\n');
      /*
            Serial.print("red: ");
            Serial.print(red);
            Serial.print(" green: ");
            Serial.print(green);
            Serial.print(" blue: ");
            Serial.print(blue);
            Serial.print(" mode: ");
            Serial.println(mode); // 1 = mobile app, 0 voice
      */
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
      if (value == -1)
      {
        autoMode = true;
        Serial.println("Auto mode enabled");
        bridge.write(AUTO_ENABLED);
      }
      if (value == -2)
      {
        autoMode = false;
        Serial.println("Auto mode disabled");
        bridge.write(AUTO_DISABLED);
      }
    }
  }
}

void lightsOff()
{
  // Serial.println("I'm in the lightsOff() function");
  // getPeople();
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
    debug('n');
    byte mappedIntensity = mapIntensity(naturalLightIntensity);
    if (mappedIntensity == offIntensity)
    {
      lightsOff();
    }
    else
    {
      byte color = getColor();

      byte signal = mapValuesAndGetSignal(color, mappedIntensity, false);

      analogWrite(redPin, red);
      analogWrite(greenPin, green);
      analogWrite(bluePin, blue);

      Serial.println("sending " + String(signal) + " byte");
      bridge.write(signal);

      if (signal != AUTO_OFF)
        lastColorIntensity = signal;

      lightState = HIGH;
    }
  }
  // debug('l');
}

byte mapIntensity(int intensity)
{
  if (intensity < 256)
  {
    return highIntensity;
  }
  if (intensity < 512)
  {
    return mediumIntensity;
  }
  if (intensity < 768)
  {
    return lowIntensity;
  }
  return offIntensity;
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
  return WHITE;
}

byte mapValuesAndGetSignal(byte color, byte intensity, bool switchPressed)
{

  if (intensity == offIntensity)
  {
    red = 0;
    green = 0;
    blue = 0;
    if (switchPressed)
      return SWITCH_OFF;
    return AUTO_OFF;
  }
  if (intensity == lowIntensity)
  {
    if (color == RED)
    {
      red = 8;
      green = 0;
      blue = 0;
      if (switchPressed)
        return RED_SWITCH_LOW;
      return RED_AUTO_LOW;
    }
    if (color == WHITE)
    {
      red = 8;
      green = 8;
      blue = 8;
      if (switchPressed)
        return WHITE_SWITCH_LOW;
      return WHITE_AUTO_LOW;
    }
    if (color == GREEN)
    {
      red = 0;
      green = 8;
      blue = 0;
      if (switchPressed)
        return GREEN_SWITCH_LOW;
      return GREEN_AUTO_LOW;
    }
    if (color == BLUE)
    {
      red = 0;
      green = 0;
      blue = 8;
      if (switchPressed)
        return BLUE_SWITCH_LOW;
      return BLUE_AUTO_LOW;
    }
    if (color == YELLOW)
    {
      red = 8;
      green = 8;
      blue = 0;
      if (switchPressed)
        return YELLOW_SWITCH_LOW;
      return YELLOW_AUTO_LOW;
    }
    if (color == ORANGE)
    {
      red = 8;
      green = 4;
      blue = 0;
      if (switchPressed)
        return ORANGE_SWITCH_LOW;
      return ORANGE_AUTO_LOW;
    }
    if (color == PURPLE)
    {
      red = 4;
      green = 0;
      blue = 8;
      if (switchPressed)
        return PURPLE_SWITCH_LOW;
      return PURPLE_AUTO_LOW;
    }
    if (color == PINK)
    {
      red = 8;
      green = 0;
      blue = 4;
      if (switchPressed)
        return PINK_SWITCH_LOW;
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
      if (switchPressed)
        return WHITE_SWITCH_MEDIUM;
      return WHITE_AUTO_MEDIUM;
    }
    if (color == RED)
    {
      red = 64;
      green = 0;
      blue = 0;
      if (switchPressed)
        return RED_SWITCH_MEDIUM;
      return RED_AUTO_MEDIUM;
    }
    if (color == GREEN)
    {
      red = 0;
      green = 64;
      blue = 0;
      if (switchPressed)
        return GREEN_SWITCH_MEDIUM;
      return GREEN_AUTO_MEDIUM;
    }
    if (color == BLUE)
    {
      red = 0;
      green = 0;
      blue = 64;
      if (switchPressed)
        return BLUE_SWITCH_MEDIUM;
      return BLUE_AUTO_MEDIUM;
    }
    if (color == YELLOW)
    {
      red = 64;
      green = 64;
      blue = 0;
      if (switchPressed)
        return YELLOW_SWITCH_MEDIUM;
      return YELLOW_AUTO_MEDIUM;
    }
    if (color == ORANGE)
    {
      red = 64;
      green = 32;
      blue = 0;
      if (switchPressed)
        return ORANGE_SWITCH_MEDIUM;
      return ORANGE_AUTO_MEDIUM;
    }
    if (color == PURPLE)
    {
      red = 32;
      green = 0;
      blue = 64;
      if (switchPressed)
        return PURPLE_SWITCH_MEDIUM;
      return PURPLE_AUTO_MEDIUM;
    }
    if (color == PINK)
    {
      red = 64;
      green = 0;
      blue = 32;
      if (switchPressed)
        return PINK_SWITCH_MEDIUM;
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
      if (switchPressed)
        return WHITE_SWITCH_HIGH;
      return WHITE_AUTO_HIGH;
    }
    if (color == RED)
    {
      red = 255;
      green = 0;
      blue = 0;
      if (switchPressed)
        return RED_SWITCH_HIGH;
      return RED_AUTO_HIGH;
    }
    if (color == GREEN)
    {
      red = 0;
      green = 255;
      blue = 0;
      if (switchPressed)
        return GREEN_SWITCH_HIGH;
      return GREEN_AUTO_HIGH;
    }
    if (color == BLUE)
    {
      red = 0;
      green = 0;
      blue = 255;
      if (switchPressed)
        return BLUE_SWITCH_HIGH;
      return BLUE_AUTO_HIGH;
    }
    if (color == YELLOW)
    {
      red = 255;
      green = 255;
      blue = 0;
      if (switchPressed)
        return YELLOW_SWITCH_HIGH;
      return YELLOW_AUTO_HIGH;
    }
    if (color == ORANGE)
    {
      red = 255;
      green = 128;
      blue = 0;
      if (switchPressed)
        return ORANGE_SWITCH_HIGH;
      return ORANGE_AUTO_HIGH;
    }
    if (color == PURPLE)
    {
      red = 127;
      green = 0;
      blue = 255;
      if (switchPressed)
        return PURPLE_SWITCH_HIGH;
      return PURPLE_AUTO_HIGH;
    }
    if (color == PINK)
    {
      red = 255;
      green = 0;
      blue = 127;
      if (switchPressed)
        return PINK_SWITCH_HIGH;
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
      Serial.print("autoMode: ");
      Serial.println(autoMode);
    }
  }
}

void getCurrentState()
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
  if (autoMode)
    Serial.println(" and the auto mode is ON");
  else
    Serial.println(" and the auto mode is OFF");
}

void getPeople()
{
  Serial.print(String(peopleInTheRoom) + " people in the room");
}
