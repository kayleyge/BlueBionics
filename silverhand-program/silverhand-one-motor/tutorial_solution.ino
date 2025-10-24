// This code will be used for stringing the servos of a one motor system
#include <Servo.h>
#define SERVO_PIN1 9//11
//#define SERVO_PIN2 10
#define MOSFET_PIN1 8
//#define MOSFET_PIN2 13

Servo myservo1;
Servo myservo2;
Servo myservo3;
Servo myservo4;
Servo myservo5;
//Servo myservo2;
int pos = 0;
int deg = 0; // Change this value to change the servos' position; input 0, 179, 180

int val = 0;

void setup() {
  //Setup IO
  myservo1.attach(SERVO_PIN1);
  //myservo2.attach(SERVO_PIN2);
  pinMode(MOSFET_PIN1, OUTPUT);
  //pinMode(MOSFET_PIN2, OUTPUT);
  //Set Initial State
  digitalWrite(MOSFET_PIN1,HIGH);
  //digitalWrite(MOSFET_PIN2,HIGH);
  Serial.begin(9600);
  delay(50);

  // myservo1.write(0);
  // delay(500);
  // myservo1.write(100);
  if (deg == 0) {
    //For 0 degrees:

    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo1.write(pos);
                    // tell servo to go to position in variable ‘pos’
      //myservo2.write(pos);
      // Serial.println(pos);
      delay(15);                    // waits 15ms for the servo to reach the position
    }
    deg = 180;
  }
  else {
    //For 180 Degrees
    for (pos = 0; pos <= 180; pos++) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo1.write(pos);              // tell servo to go to position in variable ‘pos’
      //myservo2.write(pos);
      // Serial.println(pos);
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    deg = 0;
  }

  
}

void loop() {
  // myservo1.write(deg);
  // //myservo2.write(deg);
  // delay(500);
  // deg = (deg + 90) % 180;
  // Serial.println("in loop");
  // myservo1.write(0);
  // delay(500);
  // myservo1.write(100);
  // delay(500);
  // if (deg == 0) {
  //   //For 0 degrees:

  //   for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
  //     myservo1.write(pos);
  //                   // tell servo to go to position in variable ‘pos’
  //     //myservo2.write(pos);
  //     // Serial.println(pos);
  //     delay(15);                    // waits 15ms for the servo to reach the position
  //   }
  //   deg = 180;
  // }
  // else {
  //   //For 180 Degrees
  //   for (pos = 0; pos <= 180; pos++) { // goes from 0 degrees to 180 degrees
  //     // in steps of 1 degree
  //     myservo1.write(pos);              // tell servo to go to position in variable ‘pos’
  //     //myservo2.write(pos);
  //     // Serial.println(pos);
  //     delay(15);                       // waits 15ms for the servo to reach the position
  //   }
  //   deg = 0;
  // }
  // myservo1.write(90);
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  } 
}
