#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define RADIO_CE 7 // CE PIN ON TRANCIEVER CONNECTED TO PIN 7
#define RADIO_CSN 8 // CSN PIN ON TRANCIEVER CONNECTED TO PIN 8
//PIN 13 SCK
//PIN 11 MOSI
//12 MISO

#define NOISE_SENS A0 //attach pin A0 Arduino to noise sensor OUTPUT
#define BUZZER 9 //attach pin D9 Arduino to buzzer +VE INPUT
#define BUZZER_BUTTON 4 //attach pin D9 Arduino to buzzer +VE INPUT
#define ECHO_PIN 2 // attach pin D2 Arduino to Echo of ultrasonic
#define TRIG_PIN 3 //attach pin D3 Arduino to Trig ultrasonic


long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

long val_noise;
int measurement;
boolean arm_state = true;
int msg;
RF24 radio(RADIO_CE, RADIO_CSN); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

void setup() {
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002
  radio.setPALevel(RF24_PA_MIN);
  pinMode(BUZZER,OUTPUT);
  pinMode(BUZZER_BUTTON,INPUT);
  pinMode(NOISE_SENS,INPUT);
  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // Serial Communication is starting with 9600
  radio.startListening();
}
void loop(){
  // code starts here and will wait until tranceiver sends input
  int val = digitalRead(BUZZER_BUTTON);
  if(val==HIGH){
   tone(BUZZER, 1000,5000);
   delay(5000);
  }
  if (radio.available()) {
    radio.read(&msg, sizeof(msg));
  }
  if(msg == 1){
    arm_state=true;
    armed();
  }
}
void armed() {
 // loop continues until message recieved to unarm.
 
  while(arm_state)
  {
      measurement = read_ultrasensor(); // read ultrasonic value
      val_noise = analogRead(NOISE_SENS);  //read noise value
      noTone(BUZZER);
      radio.stopListening();
      int alert = 3;
      if(measurement<40 && measurement>15 && val_noise<650)
      {
          //if measurement val in range and noise val in range
          //sound the alarm and send alert to controlleR
          while(!radio.write(&alert, sizeof(alert))){} //sends an alert to controller 
          delay(100);
          tone(BUZZER, 1000,5000);
          delay(5000);
      }
   
      radio.startListening();
      radio.read(&msg, sizeof(msg));
      if(msg ==2){
          arm_state=false;
      }
  }
}

int read_ultrasensor() {
  //FORCE set trig to low to start off
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  //set trig to high then wt 10us
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
   //set trig to low again
  digitalWrite(TRIG_PIN, LOW);
  //read echo pulse
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 
 
  return distance;
}
