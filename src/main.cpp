#include <Arduino.h>
#include <SoftwareSerial.h>

const byte rxPin = 8;
const byte txPin = 7;
SoftwareSerial bridge(rxPin, txPin);

const uint8_t lightPin = 3;

const uint8_t redPin = 9;
const uint8_t greenPin = 10;
const uint8_t bluePin = 11;

const uint8_t motionSensorInternalPin = 12;
const uint8_t motionSensorExternalPin = 4;

const uint8_t switchPin = 2;
const uint8_t naturalLightPin = A0;
const bool DEBUG_MODE = true;

const int naturalLightThreshold = 80;
const int ON = 255;

volatile int lightState;
volatile bool switchOn;
volatile bool notifiedOn;
volatile bool notifiedOff;
volatile bool autoMode;

bool lastSwitchOn;
int naturalLightIntensity;
unsigned long timestamp;
unsigned long timestampExternal;
unsigned long timestampInternal;
const unsigned long samplingTime = 6000;
const unsigned long checkTime = 3000;

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
int val;
int previousLightIntensity;

// char buffer[3];

/* -------------- DATA TO SEND -------------------- */

const byte PEOPLE_IN_THE_ROOM = 73;

// LIGHT STATES AND CAUSES

const byte AUTO_OFF = 0;
const byte AUTO_ON = 1;
const byte SWITCH_OFF = 2;
const byte SWITCH_ON = 3;
const byte USER_OFF = 4;
const byte USER_ON = 5;
const byte VOICECOMMAND_OFF = 6;
const byte VOICECOMMAND_ON = 7;

// Colors

const byte RED_VOICE = 8;
const byte RED_USER = 9;
const byte GREEN_VOICE = 10;
const byte GREEN_USER = 11;
const byte BLUE_VOICE = 12;
const byte BLUE_USER = 13;
const byte YELLOW_VOICE = 14;
const byte YELLOW_USER = 15;
const byte PURPLE_VOICE = 16;
const byte PURPLE_USER = 17;
const byte PINK_VOICE = 18;
const byte PINK_USER = 19;
const byte ORANGE_VOICE = 20;
const byte ORANGE_USER = 21;

// Natural Intensities

// Light Intensities

const byte highIntensityThreshold = 255;
const byte mediumIntensityThreshold = 175;
const byte lowIntensityThreshold = 100;

/* --------------------------------------------------- */

void onSwitchPressed(void);
void onSerialInput(void);
void onNaturalLight(void);
void debug(char mode);
void whatIsCurrentState(void);
void howManyPeople(void);
void lightsOff(void);

void setup()
{
  peopleInTheRoom = 0;
  currentPersonState = OUT;
  lightState = LOW;
  lastSwitchOn = false;
  switchOn = false;
  notifiedOn = false;
  notifiedOff = false;
  autoMode = true;
  previousLightIntensity = 0;
  timestamp = millis();
  timestampExternal = millis() - samplingTime;
  timestampInternal = millis() - samplingTime;
  pinMode(switchPin, INPUT);
  pinMode(lightPin, OUTPUT);
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

  val = digitalRead(motionSensorExternalPin);

  if (val == HIGH)
  {
    if (millis() - timestampExternal > samplingTime)
    {
      // Serial.println("The ext sensor read something");
      if (currentPersonState == OUT)
        futurePersonState = HALFWAY_IN;

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

  val = digitalRead(motionSensorInternalPin);
  if (val == HIGH)
  {
    if (millis() - timestampInternal > samplingTime)
    {
      // Serial.println("The int sensor read something");
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

      if (currentPersonState == HALFWAY_OUT)
      {
        futurePersonState = IN;
        peopleInTheRoom = peopleInTheRoom + 1;
        bridge.write(PEOPLE_IN_THE_ROOM);
        bridge.write(peopleInTheRoom);
        if (!switchOn)
          onNaturalLight();
      }
      currentPersonState = futurePersonState;
      // whatIsCurrentState();
      timestampInternal = millis();
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
  lightState = lightState > 0 ? LOW : ON;
  analogWrite(lightPin, lightState);
  if (lightState > 0)
  {
    Serial.println("sending SWITCH ON signal");
    bridge.write(SWITCH_ON);
  }
  else
  {
    Serial.println("sending SWITCH OFF signal");
    bridge.write(SWITCH_OFF);
  }
  switchOn = !switchOn;
  interrupts();
}

void onSerialInput()
{
  if (bridge.available() > 0)
  {
    Serial.println("Reading from bridge");
    // look for the next valid integer in the incoming serial stream:
    int value = bridge.parseInt(SKIP_ALL, '\n');
    Serial.print("value: ");
    Serial.println(value);

    if (value >= 0)
    {

      int red = value;
      // do it again:
      int green = bridge.parseInt();
      // do it again:
      int blue = bridge.parseInt();

      int mode = bridge.parseInt(SKIP_ALL, '\n');

      Serial.print("red: ");
      Serial.print(red);
      Serial.print(" green: ");
      Serial.print(green);
      Serial.print(" blue: ");
      Serial.print(blue);
      Serial.print("mode: ");
      Serial.println(mode); // 1 = mobile app, 0 voice

      if (red == 255 && green == 255 && blue == 255)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending USER ON signal");
          bridge.write(USER_ON);
        }
        if (mode == 0)
        {
          Serial.println("sending VOICECOMMAND ON signal");
          bridge.write(VOICECOMMAND_ON);
        }
        lightState = HIGH;
      }
      if (red == 0 && green == 0 && blue == 0)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending USER OFF signal");
          bridge.write(USER_OFF);
        }
        if (mode == 0)
        {
          Serial.println("sending VOICECOMMAND OFF signal");
          bridge.write(VOICECOMMAND_OFF);
        }
        lightState = LOW;
      }
      if (red == 255 && green == 0 && blue == 0)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending RED USER signal");
          bridge.write(RED_USER);
        }
        if (mode == 0)
        {
          Serial.println("sending RED VOICE signal");
          bridge.write(RED_VOICE);
        }
        lightState = HIGH;
      }
      if (red == 0 && green == 255 && blue == 0)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending GREEN USER signal");
          bridge.write(GREEN_USER);
        }
        if (mode == 0)
        {
          Serial.println("sending GREEN VOICE signal");
          bridge.write(GREEN_VOICE);
        }
        lightState = HIGH;
      }
      if (red == 0 && green == 0 && blue == 255)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending BLUE USER signal");
          bridge.write(BLUE_USER);
        }
        if (mode == 0)
        {
          Serial.println("sending BLUE VOICE signal");
          bridge.write(BLUE_VOICE);
        }
        lightState = HIGH;
      }
      if (red == 255 && green == 255 && blue == 0)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending YELLOW USER signal");
          bridge.write(YELLOW_USER);
        }
        if (mode == 0)
        {
          Serial.println("sending YELLOW VOICE signal");
          bridge.write(YELLOW_VOICE);
        }
        lightState = HIGH;
      }
      if (red == 255 && green == 128 && blue == 0)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending ORANGE USER signal");
          bridge.write(ORANGE_USER);
        }
        if (mode == 0)
        {
          Serial.println("sending ORANGE VOICE signal");
          bridge.write(ORANGE_VOICE);
        }
        lightState = HIGH;
      }
      if (red == 127 && green == 0 && blue == 255)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending PURPLE USER signal");
          bridge.write(PURPLE_USER);
        }
        if (mode == 0)
        {
          Serial.println("sending PURPLE VOICE signal");
          bridge.write(PURPLE_VOICE);
        }
        lightState = HIGH;
      }
      if (red == 255 && green == 0 && blue == 127)
      {
        analogWrite(redPin, red);
        analogWrite(greenPin, green);
        analogWrite(bluePin, blue);
        if (mode == 1)
        {
          Serial.println("sending PINK USER signal");
          bridge.write(PINK_USER);
        }
        if (mode == 0)
        {
          Serial.println("sending PINK VOICE signal");
          bridge.write(PINK_VOICE);
        }
        lightState = HIGH;
      }
      previousLightIntensity = highIntensityThreshold;
      autoMode = false;
    } else {
      Serial.println("updating the auto mode");
      autoMode = value == -1? true : false;
    }
  }
}

void lightsOff()
{
  // Serial.println("I'm in the lightsOff() function");
  // howManyPeople();
  lightState = LOW;
  analogWrite(lightPin, lightState);
  //analogWrite(redPin, 0);
  //analogWrite(greenPin, 0);
  //analogWrite(bluePin, 0);
  Serial.println("sending AUTO OFF signal");
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
      int previousLightState = lightState;
      lightState = map(naturalLightIntensity, 0, 1023, 255, 0);
      analogWrite(lightPin, lightState);
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
    }
    else
    {
      lightsOff();
    }
    // debug('l');
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
