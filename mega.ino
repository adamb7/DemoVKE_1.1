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
#define SIDE_DELAY 85 //500

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS_TANK_NUM, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(LEDS_SIDE_NUM, LINE_PIN, NEO_GRB + NEO_KHZ800);
String toSend;
uint16_t timer;
uint16_t old_timer;
uint16_t ir_value0;
uint16_t ir_value1;
uint8_t sensor_flag0;
uint8_t sensor_flag1;
uint8_t reset_sent;
uint8_t error_sent;
uint8_t flash_led;
uint8_t led_state;
uint8_t plc_error;
uint8_t comm_error;
uint8_t comm_power_error;
uint8_t error_duration;
uint8_t all_led_on;
uint8_t side_led;

void setup() {
  strip.begin();
  strip.show();
  colorWipe(TANK_COLOR, 20);
  strip2.begin();
  turnoffWipe_side(strip2.Color(0,0,0), 10);
  strip2.show();
  Serial.begin(115200);
  Serial.flush();
  pinMode(IR_PIN_FRONT, INPUT);
  pinMode(IR_PIN_END, INPUT);
  timer = LAP_NUM;
  old_timer = timer;
  sensor_flag0 = 1;
  sensor_flag1 = 1;
  reset_sent = 1;
  error_sent = 1;
  flash_led = 0;
  led_state = 0;
  comm_error = 0;
  comm_power_error = 0;
  error_duration = ERR_RESET;
  plc_error = 0;
  all_led_on = 0;
  side_led = 0;
  ir_value0 = 0;
  ir_value1 = 0;
  pinMode(INT_PIN, INPUT);
  pinMode(INT_PIN, INPUT_PULLUP);
  delay(3000);
}
void loop() {
    toSend = check_serial();
    if(toSend == "gateway_error"){
        comm_error = 1;
    }
    if(toSend == "gateway_error_reset"){
        comm_error = 0;
    }
    if(toSend == "gateway_power_error"){
        comm_power_error = 1;
    }
    if(toSend == "gateway_power_error_reset"){
        comm_power_error = 0;
    }
    if(toSend == "belt_plc_error"){
        plc_error = 1;
    }
    if(toSend == "belt_plc_error_reset"){
        plc_error = 0;
    }
    if(toSend == "go_to_sleep"){
        delay(1000);
        go_to_sleep();
    }
    toSend="";
    delay(1000);  
    //======================================================================  
    if(comm_error == 0 && comm_power_error == 0){
      ir_value0 = analogRead(IR_PIN_FRONT);
      if(ir_value0 > CAR_CLOSE && sensor_flag0 == 1){
        Serial.flush();        
        //Serial.print("start_belt_begin");
        sensor_flag0 = 0;
        delay(1000);
        colorWipe_side(strip2.Color(0,165,0), SIDE_DELAY);
      }
      if(ir_value0 < CAR_AWAY && sensor_flag0 == 0){
        sensor_flag0 = 1;
        delay(1000);
      }
     }
     if(comm_error == 0 && comm_power_error == 0){ 
      ir_value1 = analogRead(IR_PIN_END);
      if(ir_value1 > CAR_CLOSE && sensor_flag1 == 1){
        delay(2000);
        Serial.flush();
        //Serial.print("start_belt_end");
        sensor_flag1 = 0;
        old_timer = timer;
        timer--;
        delay(1000);
        turnoffWipe_side(strip2.Color(0,0,0), 0);
      }
      if(ir_value1 < CAR_AWAY && sensor_flag1 == 0){
        sensor_flag1 = 1;
        delay(1000);
      }
    }
    if(timer == LAP_NUM && reset_sent == 1){
        colorWipe(TANK_COLOR, 50);
        Serial.flush();
        Serial.print("no_liquid_error_reset");
        reset_sent = 0;
        flash_led = 0;
      }
    else{
        if(timer == 0 && error_sent == 1){
            flash_led = 1;
            Serial.flush();
            Serial.print("no_liquid_error");
            error_sent = 0;
          }
        else{
            if(old_timer != timer){
                turnoffWipe(strip.Color(0,0,0), 50, 6-timer); //turn off 1 led timer -> 6-timer
                old_timer = timer;
              }
            else{
                //no off
                
              }
          }
      }
    while(flash_led == 1){
        strip.setPixelColor(5, strip.Color( (led_state)? 150 : 0 ,0,0)); //red
        strip.show();
        if(Serial.available()){
            toSend = check_serial();
            if(toSend == "belt_plc_error"){
                plc_error = 1;
              }
            if(toSend == "belt_plc_error_reset"){
                plc_error = 0;
              }
          }
        if(plc_error == 1){
            led_state? errorFlash_side(strip2.Color(165,0,0), 0) : errorFlash_side(strip2.Color(0,0,0), 0); 
          }
        if(plc_error == 0){
            errorFlash_side(strip2.Color(0,0,0), 0);
          }
        led_state = !led_state;
        delay(1000);
        error_duration--;
        if(error_duration == 0){
          flash_led = 0;
          error_duration = ERR_RESET;
          errorFlash_side(strip2.Color(0,0,0), 0);
          colorWipe(strip.Color(0,0,0), 50);
          Serial.flush();
          Serial.print("no_liquid_error_reset");
          delay(3000);
          Serial.flush();
          Serial.print("sixpack_off");
          delay(200);
        }
      }
    while(plc_error == 1){
        side_led? errorFlash_side(strip2.Color(165,0,0), 0) : errorFlash_side(strip2.Color(0,0,0), 0); 
        side_led = !side_led;
        if(Serial.available()){
            toSend = check_serial();
            if(toSend == "gateway_error"){
                comm_error = 1;
              }
            if(toSend == "gateway_error_reset"){
                comm_error = 0;
              }
            if(toSend == "gateway_power_error"){
                comm_power_error = 1;
              }
            if(toSend == "gateway_power_error_reset"){
                comm_power_error = 0;
              }
            if(toSend == "belt_plc_error_reset"){
              plc_error = 0;
              delay(2000);
              if(all_led_on == 1){
                  colorWipe_side(strip2.Color(0,165,0), 10);
                }
              else{
                  turnoffWipe_side(strip2.Color(0,0,0), 10);
                }
              }
            delay(200);
          }
        delay(1000);
      }
    if(timer == 0){
        while(check_serial() != "restart_system"){ 
            delay(500);
        }
        asm volatile ("  jmp 0");
      }
      
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < LEDS_TANK_NUM; i++) {
    strip.setPixelColor(i, c);
    delay(wait);
  }
     strip.show();
}
void turnoffWipe(uint32_t c, uint8_t wait, uint32_t leds){
  for(uint16_t i = 0; i < leds; i++){
    strip.setPixelColor(i,c);
    delay(wait);
  }
  strip.show();
}
void colorWipe_side(uint32_t c, uint8_t wait) {
  String serial_data;
  plc_error = 0;
  uint8_t led_state = 0;
  for (uint16_t i = 0; i < LEDS_SIDE_NUM; i++) {
    strip2.setPixelColor(i, c);
    if(i%3 == 0){
        strip.setPixelColor(6-timer, led_state? TANK_COLOR : strip.Color(0,0,0));
        strip.show();
      
    if(led_state == 0){
        led_state = 1;
      }
    else{
        led_state = 0;
      }
    }
    strip.show();
    strip2.show();
    if(Serial.available()){
        serial_data = check_serial();
        if(serial_data == "gateway_power_error") {
            comm_power_error = 1;
          } 
        if(serial_data == "gateway_power_error_reset") {
            comm_power_error = 0;
          } 
        if(serial_data == "gateway_error") {
            comm_error = 1;
          }
        if(serial_data == "gateway_error_reset") {
            comm_error = 0;
          }
        delay(200);
        if(serial_data == "belt_plc_error"){
            plc_error = 1;
            for(uint8_t k = 0;k<=i;k++){
                strip2.setPixelColor(k, strip2.Color(165,0,0));
              }
            strip2.show(); 
            delay(2000);
            while(plc_error !=0){
                for(uint8_t k = 0;k<=i;k++){
                    strip2.setPixelColor(k, strip2.Color(165,0,0));
                  }
                strip2.show();  
                delay(1000);
                for(uint8_t k = 0;k<=i;k++){
                    strip2.setPixelColor(k, strip2.Color(0,0,0));
                  }
                strip2.show();  
                delay(1000);
                if(Serial.available()){
                    if(check_serial() == "belt_plc_error_reset"){
                        plc_error = 0;
                      }
                  }
              }
             for(uint8_t k = 0;k<=i;k++){
                strip2.setPixelColor(k, strip2.Color(0,165,0));
              }
              strip2.show();
        }
    }
    delay(wait);
  }
  strip.setPixelColor(6-timer, strip.Color(0,0,0));
  strip.show();
  all_led_on = 1;
  sensor_flag0 = 1;
  sensor_flag1 = 1;
  delay(200);
}
void turnoffWipe_side(uint32_t c, uint8_t wait){
  for(uint16_t i = LEDS_SIDE_NUM; i > 0; i--){
    strip2.setPixelColor(i,c);
    strip2.show();
    delay(wait);
  }
  strip2.setPixelColor(0,c);
  strip2.show();
  all_led_on = 0;
}
void errorFlash_side(uint32_t c, uint8_t wait){
  for(uint16_t i = 0; i < LEDS_SIDE_NUM; i++){
    strip2.setPixelColor(i,c);  
    delay(wait);
  }
  strip2.show();
}
void wake_up_isr(void){
  Serial.println("ISR");  
  detachInterrupt(0);
}
void go_to_sleep(void){
  attachInterrupt(digitalPinToInterrupt(INT_PIN), wake_up_isr, LOW);
  delay(100);
  Serial.println("SLEEP");
  delay(1000);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  sleep_enable();
  sleep_mode();
  /* The program will continue from here. */
  sleep_disable(); 
  asm volatile ("  jmp 0");
}
String check_serial(){
  String toSend="";
  char incomingByte;
  while(Serial.available()){
        incomingByte = Serial.read();
        toSend += incomingByte;
      }
    if(toSend.length() > 0){
        //toSend.trim();
        return toSend;
      }
}
