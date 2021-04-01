#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>

#define RADIO_CE 7 // CE PIN ON TRANCIEVER CONNECTED TO PIN 7
#define RADIO_CSN 8 // CSN PIN ON TRANCIEVER CONNECTED TO PIN 8
//PIN 13 SCK
//PIN 11 MOSI
//12 MISO
#define ARM_BUTTON 9 //button for arming and disarming system
#define BUZZER A0 //attach pin D9 Arduino to buzzer +VE INPUT

//lcd display digital pins
const int rs = 10, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
int ARMED_STATUS=0; //armed status value 0 = unarmed,1 =armed
int a = 2;  //button status value, a=2 -> unarmed, previous button state unarmed
            //a=1 -> armed previous button state pressed to arm        
const byte addresses[][6] = {"00001", "00002"}; //address of wireless connection
int val;  //current value of input button

RF24 radio(RADIO_CE, RADIO_CSN); // CE, CSN
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  //initialize Display

void setup() {
  lcd.begin(16, 2);
  lcd_unarmed();
  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  pinMode(BUZZER,OUTPUT);
  pinMode(ARM_BUTTON,INPUT);  //set arm button as digital input
  Serial.begin(9600); // Serial Communication is starting with 9600
  radio.stopListening();
}
void loop() {

  val = digitalRead(ARM_BUTTON);
  delay(50);
  if(val==HIGH && a==2){
    a=1;
    delay(100);
  }
  
  if(a ==1 && ARMED_STATUS ==0){
    
    while(!radio.write(&a, sizeof(a))){}
    lcd_armed();
    ARMED_STATUS=1;
    delay(100);
  }
  else if(ARMED_STATUS ==1){  
     delay(200);
     wait_in_arm();
  }
}

void wait_in_arm() {
   radio.startListening();
   int msg;
   int button_val=0;
   int num_alerts =0; //number of alerts that have occured in current state is reset to 0
   
   while(true){ //stay is loop until told to disarm
      if(radio.available()) {
          radio.read(&msg, sizeof(msg));
          //recieving a value of 3 indicates alarm has been tripped
          lcd.setCursor(7, 1);
          if(msg==3){
            lcd.setCursor(7, 1);
            num_alerts++;
            lcd.print(num_alerts);
            tone(BUZZER, 1000,5000);
            delay(5000);
          } 
       }
      button_val = digitalRead(ARM_BUTTON); 
      if(button_val==HIGH){
          delay(200);
          break;
      }
   }
   radio.stopListening();
   lcd_unarmed();
   //set all status values back to unarmed state
   ARMED_STATUS=0;
   a=2;
   while(!radio.write(&a, sizeof(a))){}
   delay(300);
}
void lcd_unarmed(){ //prints unarmed STATUS on LCD
   lcd.setCursor(0, 0);
   lcd.print("UNARMED                  ");
   lcd.setCursor(0, 1);
   lcd.print("                         ");
}
void lcd_armed(){ //prints armed STATUS on LCD
    lcd.setCursor(0, 0);
    lcd.print("ARMED                       ");
    lcd.setCursor(0, 1);
    lcd.print("ALERTS                      ");
    
}
  
