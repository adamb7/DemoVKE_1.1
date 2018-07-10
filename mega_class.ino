#include <avr/sleep.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define INT_PIN 2
#define PIN 6
#define LINE_PIN 8
#define IR_PIN_FRONT A0
#define IR_PIN_END A1
#define CAR_CLOSE 500
#define CAR_AWAY  300
#define LEDS_TANK_NUM 6
#define LEDS_SIDE_NUM 60
#define LAP_NUM 6
#define ERR_RESET 15
#define TANK_COLOR strip.Color(165,42,0)
#define SIDE_COLOR strip2.Color(0,165,0)
#define ERROR_COLOR strip2.Color(165,0,0)
#define OFF_COLOR strip2.Color(0,0,0)
#define SIDE_OFF_SPEED 20
#define SIDE_DELAY 85 //500
#define ERROR_DELAY 1000

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS_TANK_NUM, PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(LEDS_SIDE_NUM, LINE_PIN, NEO_GRB + NEO_KHZ800);

//======================================================================================================
void turnoffWipe_side(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);

class IRSensor
{
  char* pin;
  uint16_t carClose;
  uint16_t carAway;
  uint16_t updateInterval;
  uint16_t lastUpdate;
  
  public: 
  IRSensor(char* irPin, uint16_t distanceClose, uint16_t distanceAway, uint16_t updateInterval)
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
  String toSend;
  uint16_t lastUpdate;
  uint16_t updateInterval;
  
  public:
  SerialInfo(uint16_t updateInt)
  {
    toSend = "";
    lastUpdate = 0;
    updateInterval = updateInt;
  }
  String Update()
  { 
    toSend = "";
    if(millis() - lastUpdate > updateInterval)
    {
      char incomingByte;
      lastUpdate = millis();
      while(Serial.available())
      {
        incomingByte = Serial.read();
        toSend += incomingByte;
      }
      if(toSend.length() > 0)
      {
        return toSend;
      }
    }
  }
};

class SideLeds
{
  Adafruit_NeoPixel strip2;
  uint8_t toggle;
  uint16_t lastUpdate;
  uint16_t lastUpdateOff;
  uint16_t updateInterval;
  
  public:
  SideLeds(uint16_t ledSpeed)
  {
    strip2 = Adafruit_NeoPixel(LEDS_SIDE_NUM, LINE_PIN, NEO_GRB + NEO_KHZ800);
    updateInterval = ledSpeed;
    lastUpdate = 0;
    lastUpdateOff = 0;
    toggle = 0;
    strip2.begin();
    turnoffWipe_side(strip2.Color(0,0,0), 10);
    strip2.show();
  }
  void Update(String task, uint8_t i)
  {
    if(millis() - lastUpdate > updateInterval)
    {
      lastUpdate = millis();
      if(task == "turnOn")
      {
        turnOn(i);
      }
    }
    if(millis() - lastUpdate > ERROR_DELAY)
    {
      lastUpdate = millis(); 
      if(task == "errorFlash")
      {
        errorFlash(i);
      }
    }
    if(millis() - lastUpdateOff > SIDE_OFF_SPEED)
    {
      lastUpdateOff = millis();
      if(task == "turnOff")
      {
        turnOff(i);
      }
    }
  }
  void turnOn(uint8_t i)
  {
    strip2.setPixelColor(i,SIDE_COLOR);
    strip2.show();
  }
  void turnOff(uint8_t i)
  {
    strip2.setPixelColor(i,OFF_COLOR);
    strip2.show();
  }
  void errorFlash(uint8_t i)
  {
    toggle != toggle;
    for(uint8_t k = 0;k <= i;k++)
    {
      strip2.setPixelColor(i , toggle ? ERROR_COLOR : OFF_COLOR);
    }
    strip2.show();
  }
  void turnoffWipe_side(uint32_t c, uint8_t wait)
  {
    for(uint16_t i = LEDS_SIDE_NUM; i > 0; i--)
    {
      strip2.setPixelColor(i,c);
      strip2.show();
      delay(wait);
    }
    strip2.setPixelColor(0,c);
    strip2.show();
  }
};

class TankLeds 
{
  uint8_t timer;
  uint8_t toggle;
  uint16_t lastUpdate;
  uint16_t lastUpdateError;
  uint16_t updateInterval;
  Adafruit_NeoPixel strip;
  
  public:
  TankLeds(uint16_t ledSpeed)
  {
    strip = Adafruit_NeoPixel(LEDS_TANK_NUM, PIN, NEO_GRB + NEO_KHZ800);
    timer = LEDS_TANK_NUM - 1;
    toggle = 0;
    lastUpdate = 0;
    lastUpdateError = 0;
    updateInterval = ledSpeed;
    strip.begin();
    colorWipe(TANK_COLOR, 20);
    strip.show();
  }
  void turnOff()
  {
    strip.setPixelColor(timer,strip.Color(0,0,0));
    strip.show();
    timer--;
  }
  void errorFlash()
  {
    if(millis() - lastUpdate > ERROR_DELAY)
    {
      lastUpdateError = millis();
      toggle != toggle;
      strip.setPixelColor(0, toggle? strip.Color(160,0,0) : strip.Color(0,0,0));
      strip.show();
    }
  }
  void flash()
  {
    if(millis() - lastUpdate > updateInterval)
    {
      lastUpdate = millis();
      toggle != toggle;
      strip.setPixelColor(timer, toggle? TANK_COLOR : strip.Color(0,0,0));
      strip.show();
    }
  }
  void colorWipe(uint32_t c, uint8_t wait) 
  {
    for (uint16_t i = 0; i < LEDS_TANK_NUM; i++)
    {
      strip.setPixelColor(i, c);
      delay(wait);
    }
       strip.show();
  }
  
};

IRSensor irFront(IR_PIN_FRONT,  CAR_CLOSE,  CAR_AWAY, 500);
IRSensor irEnd(IR_PIN_END,   CAR_CLOSE,  CAR_AWAY, 500);
//SideLeds ledBelt(SIDE_DELAY);
//TankLeds ledTank(1000);
SerialInfo serialData(1000);

uint8_t startWipe;
uint8_t offWipe;

uint8_t errorReset;
String inputString;
String outputString;
uint8_t error;
uint8_t ledNum;
void setup() 
{
  startWipe = 0;
  offWipe = 0;
  errorReset = 0;
  inputString = "";
  outputString = "jo";
  error &= 0x000;
  ledNum = 0;
  Serial.begin(115200);
  Serial.println("START");
  delay(2000);
}

void loop() 
{
//=================================SERIAL================================
  //inputString = serialData.Update();
  //error = checkString(inputString);
  //Serial.println(inputString);
  
  if(!(error & 0x001 || error & 0x010))//comm_error & comm_power_error !
  {
    startWipe = irFront.Update();
    if(startWipe) outputString = "positionManagement_0 ";
    offWipe = irEnd.Update();
    if(offWipe) outputString = "positionManagement_1 ";
  }

  sendSerial(outputString);
}//meh
//==================================IR+LED================================  
//  //set side led & tank led!
//  //TURN ON
//  if(startWipe)
//  {
//    if(error & 0x100)
//    {
//      ledBelt.Update("errorFlash",ledNum);
//    }
//    else
//    {
//      if(errorReset)
//      {
//        for(uint8_t k = 0; k <= ledNum; k++)
//        {
//                  strip2.setPixelColor(k, strip2.Color(0,165,0));
//        }
//        errorReset = 0;
//      }
//      ledNum++;
//      ledBelt.Update("turnOn",ledNum);
//      ledTank.flash();
//      if(ledNum == LEDS_SIDE_NUM)
//      {
//        startWipe = 0;
//        ledTank.turnOff();
//      }
//    }
//  }
//  //TURN OFF
//  if(offWipe)
//  {
//    ledNum--;
//    ledBelt.Update("turnOff",ledNum);
//    if(ledNum == 0)
//    {
//      offWipe = 0;
//      //tankerror !!
//      //ledTank.errorFlash();
//    }
//  }
//  
//}
//=========================================================================  
uint8_t checkString(String toSend)
{
    uint8_t error_input = 0x000;
    if(toSend == "gateway_error"){
        error_input |= 0x001;
    }
    if(toSend == "gateway_error_reset"){
        error_input &= ~0x001;
    }
    if(toSend == "gateway_power_error"){
        error_input |= 0x010;
    }
    if(toSend == "gateway_power_error_reset"){
        error_input &= ~0x001;
    }
    if(toSend == "belt_plc_error"){
        error_input |= 0x100;
    }
    if(toSend == "belt_plc_error_reset"){
        error_input &= ~0x100;
        errorReset = 1;
    }
//    if(toSend == "go_to_sleep"){
//        delay(1000);
//        go_to_sleep();
//    }
    return error_input;
}
void sendSerial(String data)
{
  Serial.flush();
  if(data.length() > 0)
  {
    Serial.print(data);
    delay(300);
  }
  data = "";
}
//void turnoffWipe_side(uint32_t c, uint8_t wait)
//{
//  for(uint16_t i = LEDS_SIDE_NUM; i > 0; i--){
//    strip2.setPixelColor(i,c);
//    strip2.show();
//    delay(wait);
//  }
//  strip2.setPixelColor(0,c);
//  strip2.show();
//}
//void colorWipe(uint32_t c, uint8_t wait) 
//{
//  for (uint16_t i = 0; i < LEDS_TANK_NUM; i++) {
//    strip.setPixelColor(i, c);
//    delay(wait);
//  }
//     strip.show();
//}
