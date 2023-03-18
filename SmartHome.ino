#include <LiquidCrystal.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <Wire.h>
#include <dht.h>
dht DHT;
#include <Servo.h>
Servo doorServo;
Servo windowServo;

/* Pins define */
#define trigPin 7
#define echoPin 6
#define trigPin2 9
#define echoPin2 10
#define DHT11_PIN 5
#define ledPin 13
#define soundPin A3
#define buzzerPin 3
#define motorPin 2
#define smokePin A2
#define coridor 4

/* Variables */
const int soundThreshold = 90;
const int tempThreshold = 24;
const int sensorThreshold = 250;
//int sound = 250;

// flags
// int locked = 1;
int doorOpen = 0;
int screen = 1;
int locked = 1;
int correctPassword = 0;
int fanFlag = 0;
int windowFlag = 0;
int ledFlag = 0;


/* Variables For millis delay */
const unsigned long tempdelay = 2000;
const unsigned long smokedelay = 1000;
const unsigned long opendelay = 1000;
const unsigned long doordelay = 4000;
const unsigned long securitydelay = 1000;

unsigned long prevtempmillis = 0;
unsigned long prevsmokemillis = 0;
unsigned long prevopenmillis = 0;
unsigned long prevdoormillis = 0;
unsigned long prevsecuritymillis = 0;

void setup() {
  Serial.begin (9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(soundPin, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(buzzerPin , OUTPUT);
  pinMode(coridor,OUTPUT);
  pinMode(smokePin, INPUT);
  pinMode(motorPin, OUTPUT);

  doorServo.attach(8);
  doorServo.write(0);

  windowServo.attach(12);
  windowServo.write(0);

  lcd.init();
  lcd.backlight();
}

void temp()
{
  int chk = DHT.read11(DHT11_PIN);
  //Serial.print("Temperature = ");
  //Serial.println(DHT.temperature);
  //Serial.print("Humidity = ");
  //Serial.println(DHT.humidity);
  if (screen == 0) {
    lcd.setCursor(0, 0);
    lcd.println("Temp = ");
    lcd.setCursor(7, 0);
    lcd.print(DHT.temperature);
    lcd.print((char)223);
    lcd.print("C");
    if (DHT.temperature > tempThreshold) {
      analogWrite(motorPin, 200);
      fanFlag = 1;
    }
    else
    {
      analogWrite(motorPin, 0);
    }
    lcd.setCursor(0, 1);
    lcd.println("Humidity = ");
    lcd.setCursor(11, 1);
    lcd.println(DHT.humidity);
  }
}

void opendoor()
{
  //  if (correctPassword == 1) {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;
  Serial.println(distance);
  if (distance < 30 ){
    
    digitalWrite(coridor,HIGH);
  }else {
    digitalWrite(coridor,LOW);
  }
  if (distance < 15) {
    Serial.println("the distance is less than 10");
    doorServo.write(90);
    lcd.clear();
    lcd.println("  Welcome Home  ");
    //Serial.println("door opened");
    screen = 1;
    doorOpen = 1;
    locked = 0;
//    digitalWrite(coridor,HIGH);
    digitalWrite(buzzerPin, LOW);
//     if (distance > 30 ){
//    digitalWrite(coridor,LOW);
//  }

  }
  //  }
  if (locked == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("  Home  Locked   ");
  }
}

void security() {

  long duration2, distance2;
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = (duration2 / 2) / 29.1;
  if (distance2 < 20 ) {
    digitalWrite(buzzerPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println(" Intruder Alert   ");
  }
  
}

void soundsensor()
{
  //Serial.println("Hi");
  int soundsens = analogRead(soundPin);
  //Serial.println(soundsens);
  if (soundsens >=  soundThreshold) {
    digitalWrite(ledPin, HIGH); //turns led on
    ledFlag = 1;
    delay(100);
  }
  else {
    digitalWrite(ledPin, LOW);
    ledFlag = 0;
  }

}

void smoke() {
  int analogSensor = analogRead(smokePin);

    Serial.print("Smoke sensor: ");
    Serial.println(analogSensor);
  if (analogSensor > sensorThreshold)
  {
    tone(buzzerPin, 1000, 200);
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.println("Fire Alert");
    lcd.println("   FIRE ALERT   ");
    windowServo.write(90);
    windowFlag = 1;
  }
  else
  {
    noTone(buzzerPin);
    //windowServo.write(0);
  }
}


void loop() {
  unsigned long currentmillis = millis();
  if (doorOpen == 0) {
    if (currentmillis - prevopenmillis >= opendelay) {
      opendoor();
      prevdoormillis = prevopenmillis;
      prevopenmillis = currentmillis;
    }

  } else if (doorOpen == 1) {
    if (currentmillis - prevdoormillis >= doordelay) {
      doorServo.write(0);
      digitalWrite(coridor,LOW);
      //Serial.println("door closed");
      
      doorOpen = 0;
      screen = 0;
      prevdoormillis = currentmillis;
    }
  }
  if (locked == 0 ) {
    if (currentmillis - prevtempmillis >= tempdelay) {
      temp();
      soundsensor();
      prevtempmillis = currentmillis;
    }
  }
  if (currentmillis - prevsmokemillis >= smokedelay) {
    smoke();
    prevsmokemillis = currentmillis;
  }
  if ( locked == 1 ) {
    if (currentmillis - prevsecuritymillis >= securitydelay) {
      security();
      prevsecuritymillis = currentmillis;
    }
  }

  if (Serial.available() > 0)
  {
    //Serial.println("IN BLUETOOTH");
    char data = Serial.read(); // reading the data received from the bluetooth module
    switch (data)
    {
      case 'd': // toggles door
        //Serial.println("toggle door");
        if (doorOpen == 0) {
          doorServo.write(90);
          doorOpen = 1;
        }
        else {
          doorServo.write(0);
          doorOpen = 0;
        }

        break;

      case 'f': //toggle fan when 'f' sent
        //Serial.println("toggle fan");
        if (fanFlag == 0)
          analogWrite(motorPin, 200);
        else
          analogWrite(motorPin, 0);
        break;

      case 'w': //toggle window when 'w' sent
        //Serial.println("toggle window");
        if (windowFlag == 0)
        { windowServo.write(90);
          windowFlag = 1;
        }
        else {
          windowServo.write(0);
          windowFlag = 0;
        }
        break;

      case 'l': // toggle LED when 'l' sent
        //Serial.println("toggle led");
        if (ledFlag == 0)
          digitalWrite(ledPin, HIGH);
        else
          digitalWrite(ledPin, LOW);
        break;

      case 'L': // lock house when 'L' sent
        //Serial.println("locking house bluetooth");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.println("    Lockdown    ");
        doorOpen = 0;
        screen = 1;
        locked = 1;
        digitalWrite(13, LOW);
        doorServo.write(0);
        windowServo.write(0);

        break;

      case 'O':
        locked=0;
        break;
    }



  }
}
