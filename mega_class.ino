//#include <avr/sleep.h>
//#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
//#ifdef __AVR__
//#include <avr/power.h>
//#endif

#define TANK_PIN 6
#define LINE_PIN 8
#define IR_PIN_FRONT A0
#define IR_PIN_END A1
#define CAR_CLOSE 600
#define CAR_AWAY  450
#define LEDS_TANK_NUM 6
#define LEDS_SIDE_NUM 60
#define TANK_COLOR strip.Color(165,42,0)
#define TANK_ERROR_COLOR strip.Color(165,0,0)
#define TANK_OFF_COLOR strip.Color(0,0,0)
#define SIDE_COLOR strip2.Color(0,165,0)
#define SIDE_ERROR_COLOR strip2.Color(165,0,0)
#define SIDE_OFF_COLOR strip2.Color(0,0,0)
#define SIDE_OFF_SPEED 5
#define SIDE_ON_SPEED 85
#define TANK_FLASH_SPEED 150
#define TANK_ERROR_FLASH_SPEED 500
#define TANK_ERROR_DURATION 30
#define BELT_FLASH_SPEED 500
#define CAR_START_DELAY 2000
#define REFILL_SPEED 1000

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS_TANK_NUM, TANK_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(LEDS_SIDE_NUM, LINE_PIN, NEO_GRB + NEO_KHZ800);

//======================================================================================================
void turnoffWipe_side(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void restart_mega(void);
void beltError(uint8_t ledState);
void sendSerial(String data);
void checkString(String toSend);

class IRSensor
{
  char* pin;
  uint16_t carClose;
  uint16_t carAway;
  uint32_t updateInterval;
  uint32_t lastUpdate;
  
  public: 
  IRSensor(char* irPin, uint16_t distanceClose, uint16_t distanceAway, uint32_t updateInterval)
  {
    pin = irPin;
    pinMode(pin, INPUT);
    carClose = distanceClose;
    carAway = distanceAway;
    lastUpdate = 0;
  }
  uint8_t Update()
  {
    uint16_t irValue = 0;
    if(millis() - lastUpdate > updateInterval)
    {
      irValue = analogRead(pin);
      //Serial.println(irValue);
      lastUpdate = millis();
      if(irValue > carClose)
      {
        return 1;
      }
      if(irValue < carAway)
      {
        return 0;
      }
    }
  }
};

class SerialInfo
{
  uint8_t index;
  uint32_t lastUpdate;
  uint32_t updateInterval;
  
  public:
  SerialInfo(uint32_t updateInt)
  {
    index = 0;
    lastUpdate = 0;
    updateInterval = updateInt;
  }
  void Update(char* message)
  { 
    if(millis() - lastUpdate > updateInterval)
    {
      index = 0;
      char incomingByte;
      lastUpdate = millis();
      if(Serial.available())
      {
        while(Serial.available())
        {
          incomingByte = Serial.read();
          message[index] = incomingByte;
          index++;
        }
      }
      else
      {
        message[0] = '\0';
      }
      if(message[0] != '\0')
      {
        message[index] = '\0';
        Serial.flush();
      }
    }
  }
};

IRSensor irFront(IR_PIN_FRONT,  CAR_CLOSE,  CAR_AWAY, 500);
IRSensor irEnd(IR_PIN_END,   CAR_CLOSE,  CAR_AWAY, 500);
SerialInfo serialData(1000);

uint8_t startWipe;
uint8_t offWipe;
uint8_t turnOnLeds;
uint8_t lapCount;
uint8_t ledNumOn;
uint8_t ledNumOff;
uint8_t ledsAreOn;
uint8_t tankError;
uint8_t startTankError;
uint8_t tankErrorDuration;
uint8_t beltErrorReset;
char* inputCh;
uint32_t millisLedOn;
uint32_t millisLedOff;
uint32_t millisTankFlash;
uint32_t millisTankErrorFlash;
uint32_t millisBeltErrorFlash;
uint32_t millisDelay;
uint32_t millisRefill;
uint8_t gwError;
uint8_t gwPowerError;
uint8_t plcError;
uint8_t refillTank;
uint8_t refillCounter;
uint8_t startDelayOn;
uint8_t startDelayOff;
uint8_t toggle;
void setup() 
{
  strip.begin();
  strip2.begin();
  colorWipe(TANK_COLOR, 20);
  strip.show();
  turnoffWipe_side(SIDE_OFF_COLOR, 10);
  strip2.show();
  startWipe = 0;
  offWipe = 0;
  turnOnLeds = 0;
  lapCount = 0;
  ledNumOn = 0;
  ledNumOff = LEDS_SIDE_NUM;
  ledsAreOn = 0;
  tankError = 0;
  tankErrorDuration = TANK_ERROR_DURATION;
  startTankError = 0;
  beltErrorReset = 0;
  inputCh = calloc(30,sizeof(char));
  Serial.begin(115200);  
  millisLedOn = 0;
  millisLedOff = 0;
  millisTankFlash = 0;
  millisTankErrorFlash = 0;
  millisBeltErrorFlash = 0;
  millisDelay = 0;
  millisRefill = 0;
  gwError = 0;
  gwPowerError = 0;
  plcError = 0;
  refillTank = 0;
  refillCounter = 5;
  startDelayOn = 0;
  startDelayOff = 0;
  toggle = 0;
  Serial.flush();
  delay(2000);
}

void loop() 
{
//=================================SERIAL================================
  serialData.Update(inputCh);
  checkString(inputCh);
  if(inputCh[0] !=  '\0')
  {   
    inputCh[0] = '\0';
  }
  
  //comm_error & comm_power_error !
  if(!gwError && !gwPowerError && !plcError && !startTankError)
  {
    if(startWipe == 0)
    {
      startWipe = irFront.Update();
      if(startWipe)
      {
        //turnOnLeds = 1;
        offWipe = 0;
        startDelayOn = 1;
        millisDelay = millis();
        //outputString = "positionManagement.0.";
        sendSerial("positionManagement.0.");
        
      }
    }
    if(offWipe == 0)
    {
      offWipe = irEnd.Update();
      if(offWipe)
      {
        startWipe = 0;
        startDelayOff = 1;
        millisDelay = millis();
        //outputString = "positionManagement.1.";
        sendSerial("positionManagement.1.");
      }
    }
  }
//  if(startWipe == 1 && offWipe == 1)
//  {
//    startWipe = 0;
//  }
  //delay turn on 2mp
  if(startDelayOn)
  {
    if(millis() - millisDelay > CAR_START_DELAY)
    {
      turnOnLeds = 1;
      startDelayOn = 0;
    }
  }
  if(ledNumOn < LEDS_SIDE_NUM && turnOnLeds && lapCount < 6)
  {
    if((millis() - millisLedOn > SIDE_ON_SPEED) && (plcError == 0) && (startTankError == 0))
    {
      millisLedOn = millis();
      strip2.setPixelColor(ledNumOn,SIDE_COLOR);
      strip2.show();
      if(tankError == 1 && ledNumOn == 30)
      {
        startTankError = 1;
        //tankError = 0;
      }
      if((millis() - millisTankFlash > TANK_FLASH_SPEED) && (refillTank == 0))
      {
        millisTankFlash = millis();
        toggle = toggle? 0 : 1;
        strip.setPixelColor(lapCount, toggle? TANK_COLOR : TANK_OFF_COLOR);
        strip.show();
      }
      //ledAllOn
      if(ledNumOn == (LEDS_SIDE_NUM - 1))
      {
        turnOnLeds = 0;
        ledNumOff = LEDS_SIDE_NUM;
        ledsAreOn = 1;
        strip.setPixelColor(lapCount, TANK_COLOR);
        strip.show();
      }
      ledNumOn++;
    }
    //beltPLCERROR + ledTurnOn
    if((millis()- millisBeltErrorFlash > BELT_FLASH_SPEED) && (startTankError == 0))
    {
      millisBeltErrorFlash = millis();
      beltError(1);
    }
    //start tank error
    if(startTankError == 1)
    {
      if(millis() - millisTankErrorFlash > TANK_ERROR_FLASH_SPEED)
      {
        millisTankErrorFlash = millis();
        toggle = toggle? 0 : 1;
        strip.setPixelColor(5, toggle? TANK_ERROR_COLOR : TANK_OFF_COLOR);
        strip.show();
        if(tankErrorDuration == TANK_ERROR_DURATION)
        {
          sendSerial("no_liquid_error.0.");
        }
        tankErrorDuration--;
        if(tankErrorDuration == 0)
        {
          //lapCount++;
          strip.setPixelColor(5, TANK_OFF_COLOR);
          strip.show();
          startTankError = 0;
          tankError = 0;
          refillTank = 1;
          sendSerial("no_liquid_error_reset.0.");
        }
      }
    }
  }
  //refill tank after no_liquid_error
  if(refillTank == 1)
  {
    if(millis() - millisRefill > REFILL_SPEED)
    {
      millisRefill = millis();
      strip.setPixelColor(refillCounter, TANK_COLOR);
      strip.show();
      if(refillCounter == 0)
      {
        refillTank = 0;
      }
      else
      {
        refillCounter--;
      }
    }
  }
  //beltPLCERROR + ledsAreOn //beltPLCERROR + ledsAreOff
  if((millis() - millisBeltErrorFlash > BELT_FLASH_SPEED) && (ledNumOn == 0 || ledNumOn == LEDS_SIDE_NUM) && (startTankError == 0) && (lapCount != 6))
  {
    millisBeltErrorFlash = millis();
    beltError(0);
  }

  //delay turnoff
  if(startDelayOff)
  {
    if(millis() - millisDelay > CAR_START_DELAY)
    {
      startDelayOff = 0;
    }
  }
  
  if(ledNumOn == 60 && !turnOnLeds && offWipe && lapCount < 6 && !startDelayOff)
  {
    if(millis()- millisLedOff > SIDE_OFF_SPEED)
    {
      millisLedOff = millis();
      strip2.setPixelColor(ledNumOff,SIDE_OFF_COLOR);
      strip2.show();
      
      if(ledNumOff ==  0)
      {
        if(lapCount != 5)
        {
          strip.setPixelColor(lapCount, TANK_OFF_COLOR);
          strip.show();
        }
        //startWipe = 0;
        ledNumOn = 0;
        ledsAreOn = 0;
        lapCount++;
        //NEW TANK ERROR
        if(lapCount == 5)
        {
          tankError = 1;
        }
      }
      ledNumOff--;
    }
  }  
}
//=========================================================================  
void checkString(String toSend)
{
    if(toSend == "gateway_error")
    {
        gwError = 1;
    }
    if(toSend == "gateway_error_reset")
    {
        gwError = 0;
    }
    if(toSend == "gateway_power_error")
    {
        gwPowerError = 1;
    }
    if(toSend == "gateway_power_error_reset")
    {
        gwPowerError = 0;
    }
    if(toSend == "belt_plc_error")
    {
        plcError = 1;
    }
    if(toSend == "belt_plc_error_reset")
    {
        plcError = 0;
        beltErrorReset = 1;
    }
    if(toSend == "restart_system")
    {
        restart_mega();
    }
    return;
}
void sendSerial(String data)
{
  if(data.length() > 0)
  {
    Serial.print(data);
    delay(300);
  }
  data = "";
}
void restart_mega(void)
{
  free(inputCh);
  asm volatile ("  jmp 0");
}
void beltError(uint8_t ledState)
{
  uint8_t leds;
  if(ledState == 1)
  {
    leds = ledNumOn;
  } 
  else
  {
    leds = LEDS_SIDE_NUM;
  }
  if(plcError)
    {
      
      toggle = toggle? 0 : 1;
      for(uint8_t k = 0;k <= leds; k++)
      {
        strip2.setPixelColor(k, toggle ? SIDE_ERROR_COLOR : SIDE_OFF_COLOR);
      }
      for(uint8_t k = leds+1;k <= LEDS_SIDE_NUM; k++)
      {
        strip2.setPixelColor(k, SIDE_OFF_COLOR);
      }
      strip2.show();
    }
  if(beltErrorReset)
    {
      
      for(uint8_t k = 0;k <= leds; k++)
      {
        
        if(ledState == 1)
        {
          strip2.setPixelColor(k, SIDE_COLOR);
        }
        else
        {
          if(ledsAreOn)
          {
            strip2.setPixelColor(k, SIDE_COLOR);
          }
          else
          {
            strip2.setPixelColor(k, SIDE_OFF_COLOR);
          }
        }
      }
      for(uint8_t k = leds+1;k <= LEDS_SIDE_NUM; k++)
      {
        strip2.setPixelColor(k, SIDE_OFF_COLOR);
      }
      strip2.show();
      beltErrorReset = 0;
    }
  
}
void turnoffWipe_side(uint32_t c, uint8_t wait)
{
  for(uint8_t i = LEDS_SIDE_NUM; i > 0; i--)
  {
    strip2.setPixelColor(i,c);
    strip2.show();
    delay(wait);
  }
  strip2.setPixelColor(0,c);
  strip2.show();
}
void colorWipe(uint32_t c, uint8_t wait) 
{
  for (uint8_t i = 0; i < LEDS_TANK_NUM; i++)
  {
    strip.setPixelColor(i, c);
    delay(wait);
  }
     strip.show();
}
