#include <Servo.h>
#define TI_SERVO_PIN 11 // Pin for servo motors
#define MRP_SERVO_PIN 10 // Pin for servo motors
#define MOSFET_PIN_TI 12 // 1st MOFSET pin; thumb, index finger
#define MOSFET_PIN_MRP 13 // 2nd MOFSET pin; middle finger, ring finger, pinky
#define MYO_PIN A2 //Pin for the sensor
#define MODE_POT_PIN A5 // Potentiometer for mode control
//Pins for the 4 LED components
#define LED_PIN_R 2 // Red
#define LED_PIN_G 3 // Green
#define LED_PIN_B 4 // Blue
#define LED_IN A1   // Battery voltage signal

#define OPEN 0 // 0 degree rotation for open
#define CLOSE 180 // 180 degree rotation for close
#define PULSEWIDTH 50 //in ms
#define TRAVELDELAY 650 //in ms
#define SYSTEMDELAY 750 // delay for servo motors; want to try 1200
#define DEFAULT_THRESH 300 // must be above 300 to change position
#define DEFAULT_RELAXTHRESH 275 // must be below 275 to be able to read another flex
#define POT_MAX 504 // the maximum value for the potentiometer

// Parameters to fine tune
// Number of reads to take in order to smooth out noise; it is recommended to take a power of 2 for computational efficiency
#define SMOOTH_READS 8
// Multiplier for threshold so that the threshold is attainable. e.g. threshold = min(flex_max * 0.65,  moving_max)
#define THRESH_MULTIPLIER 0.65
// What the threshold for being allowed to flex is again as a proportion of the regular threshold
#define RELAX_THRESH_MULTIPLIER 0.9

//The thresholds for the various LED colors - must be above each to show each respective color
#define RED_THRESH 675
#define YELLOW_THRESH 757
#define GREEN_THRESH 859 //unnecessary but here for uniformity

// Don't worry about this. It just makes it more readable when turning on/off calibration mode
enum states {
  CALIBRATION,
  NORMAL_USE
};

// Number of samples to take for determining the maximum signal for moving one's arm around and for flexing
#define POLL_TIME 3000 // 3 seconds
// Delay between the blinking light, in milliseconds
#define DELAY_BLINK 500
// Simulated delay for the servo motors to activate
#define DELAY_SERVO 250

// Set whether we're using calibration mode or normal use, where the threshold is predefined
static states state = CALIBRATION;

enum Mode {
  full,
  ti,
  mrp,
  grab,
  any
};

/* Modes
 * Full: Go between open hand and fist or between the secondary positions; change both motor positions
 * TI: Pinching (update thumb and index finger)
 * MRP: Change middle finger, ring finger, and pinky position
 * Grab: Grab cycle - open hand -> pointing -> fist -> open hand -> ...
 * Any: Go between open hand and fist, exclusively
*/

// #define the location of the new stuff, and probably change the old stuff

Mode mode;
Servo ti_servo; // controls thumb and index.
Servo mrp_servo; // controls middle, ring, pinky fingers.
int ti_pos = OPEN;// position of ti_servo.
int mrp_pos = OPEN;//position of mrp_servo.
int volt_reg = 0; //input to display battery level
// The threshold to be set; above it, the motor will activate 
//unsigned threshold;

// I just extracted this to write the colors of the LED
inline void writeColors(uint8_t red, uint8_t green, uint8_t blue) {
  digitalWrite(LED_PIN_R, red);
  digitalWrite(LED_PIN_G, green);
  digitalWrite(LED_PIN_B, blue);
}

// Blink LED 3 times, leaving it on
void startupBlink(uint8_t red, uint8_t green, uint8_t blue) {
  for (uint8_t i = 0; i < 3; ++i) {
    //Serial.print("Blink");
    //Serial.println(i+1);
    delay(DELAY_BLINK);
    // Flash blue
    writeColors(red, green, blue);
    delay(DELAY_BLINK);
    // Flash off
    writeColors(LOW, LOW, LOW);
  }
}

unsigned smoothRead() {
  unsigned sum = 0;
  for (int i = 0; i < SMOOTH_READS; i++) {
    sum += analogRead(MYO_PIN);
  }
  return (sum / SMOOTH_READS);
}

unsigned relax_threshold = 0;
unsigned ambient_threshold = 0;
unsigned flex_threshold = 0;
//unsigned threshold = 0;
// Calibrate optimal threshold
void calibrateSimple() {
  
  // Blink 3 times to indicate they should relax
  startupBlink(HIGH, LOW, HIGH); //purple
  
  // stay purple for relaxed reading
  writeColors(HIGH, LOW, HIGH); //purple

  // Record signals for POLL_TIME in ms
  unsigned long end_time = millis() + POLL_TIME;

  // Record Relax Threshold
  while (millis() < end_time) {
      unsigned relax_signal = analogRead(MYO_PIN);
      if (relax_signal > relax_threshold) {
        relax_threshold = relax_signal;
      }
  } //end of relaxed calibration
  //Serial.println("relax threshold: ");
  //Serial.println(relax_threshold);

  // Blink yellow 3 times to indicate that the user must begin flexing after the blinks
  startupBlink(HIGH, HIGH, LOW);
  
  // stay yellow for ambient reading
  writeColors(HIGH, HIGH, LOW);
  
  //record Ambient Threshold first time
  end_time = millis() + POLL_TIME;
  while (millis() < end_time) {
      unsigned ambient_signal = analogRead(MYO_PIN);
      Serial.println("current ambient threshold: ");
      Serial.println(ambient_threshold);
      if (ambient_signal > ambient_threshold) {
        ambient_threshold = ambient_signal;
      }
  }

  //Record Ambient Threshold if original is incorrect
  while(ambient_threshold < 0.8*relax_threshold){
    writeColors(HIGH, LOW, LOW); //flash red to signal that calibration didn't work
    delay(5); //wait for a moment before restarting
    startupBlink(HIGH, HIGH, LOW);
    writeColors(HIGH, HIGH, LOW); //light up blue again
    end_time = millis() + POLL_TIME; //update end_time
    while (millis() < end_time) {
      //Serial.println("current ambient threshold: ");
      //Serial.println(ambient_threshold);
      unsigned ambient_signal = analogRead(MYO_PIN);
      if (ambient_signal > ambient_threshold) {
        ambient_threshold = ambient_signal;
      }
    }
  } //end of ambient calibration
  ambient_threshold *= 0.7;

  //Serial.println("final ambient threshold: ");
  //Serial.println(ambient_threshold);

  //start flex reading
  // Blink blue 3 times to indicate that the user must begin flexing after the blinks
  startupBlink(LOW, LOW, HIGH);
  
  // stay blue for flex reading
  writeColors(LOW, LOW, HIGH);
  end_time = millis() + POLL_TIME;
  while (millis() < end_time) {
      unsigned flex_signal = analogRead(MYO_PIN);
      if (flex_signal > flex_threshold) {
        flex_threshold = flex_signal;
      }
  }

  //Record Flex Threshold
  while(flex_threshold < 0.8*ambient_threshold){
    writeColors(HIGH, LOW, LOW); //flash red to signal that calibration didn't work
    delay(5); //wait for a moment before restarting
    startupBlink(LOW, LOW, HIGH);
    writeColors(LOW, LOW, HIGH); //light up blue again
    end_time = millis() + POLL_TIME; //update end_time
    while (millis() < end_time) {
      unsigned flex_signal = analogRead(MYO_PIN);
      if (flex_signal > flex_threshold) {
        flex_threshold = flex_signal;
      }
    }
  } //end of calibration

  //Serial.println("final flex threshold: ");
  //Serial.println(flex_threshold);
  
  //flash green to signal successful calibration
  writeColors(LOW, HIGH, LOW);

  // Set threshold to a fraction of its maximum reading
  flex_threshold *= THRESH_MULTIPLIER;
}

void setup() {
  //Set up servo
  ti_servo.attach(TI_SERVO_PIN);
  mrp_servo.attach(MRP_SERVO_PIN);
  
  // Set MOSFET and LED as outputs of the Arduino
  pinMode(MOSFET_PIN_TI, OUTPUT);
  pinMode(MOSFET_PIN_MRP, OUTPUT);
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
  pinMode(LED_IN, INPUT);
  pinMode(MODE_POT_PIN, INPUT);
  
  //Set Initial State of MOSFETS
  digitalWrite(MOSFET_PIN_TI,HIGH);
  digitalWrite(MOSFET_PIN_MRP,HIGH);
  
  //Open up servos
  ti_servo.write(OPEN);
  mrp_servo.write(OPEN);
  delay(TRAVELDELAY);
  
  //Set up MOSFET
  digitalWrite(MOSFET_PIN_TI,LOW);
  digitalWrite(MOSFET_PIN_MRP, LOW);
  Serial.begin(9600);
  
  //Default the LED to off
  digitalWrite(LED_PIN_R, LOW);
  digitalWrite(LED_PIN_G, LOW);
  digitalWrite(LED_PIN_B, LOW);

  // If state is set here to calibration, it will undergo normal calibration; if not, it will have a static threshold
  if (state == CALIBRATION) {
    calibrateSimple();
  } else {
    flex_threshold = DEFAULT_THRESH;
  }
  state = NORMAL_USE;
}

// Invert current servo position; in future iterations, may opt for a function that instead simply dictates desired positions
void invertServoPos(int &pos, Servo &servo, int mos){
  // NOTE: It would be sleeker to use pos = (pos + 180) % 360, but it takes more cycles
  // If pos is open, close it and write the servos to do such
  if (pos == OPEN){
    digitalWrite(mos,HIGH);    
    pos = CLOSE;
    servo.write(CLOSE);
    delay(TRAVELDELAY);
    //DEBUG
    //Serial.println("Closed motor connected to mosfet pin at: ");
    //Serial.println(mos);
  }
  // If pos is closed, open it and write the servos to do such
  else {
    pos = OPEN;
    servo.write(OPEN);
    delay(TRAVELDELAY);
    //DEBUG
    //Serial.println("Opened motor connected to mosfet pin at: ");
    //Serial.println(mos);
    digitalWrite(mos,LOW);
  }
}

// Change position of motors based on which mode is currently selected
void updateMotors () {
  // Mode will be updated via voice control
  switch(mode) {
    case ti:
      invertServoPos(ti_pos, ti_servo, MOSFET_PIN_TI);
      break;
    case mrp:
      invertServoPos(mrp_pos, mrp_servo, MOSFET_PIN_MRP);
      break;
    case full:
      invertServoPos(ti_pos, ti_servo, MOSFET_PIN_TI);
      invertServoPos(mrp_pos, mrp_servo, MOSFET_PIN_MRP);
      break;
    case grab:
      if (ti_pos == OPEN && mrp_pos == OPEN){
        invertServoPos(mrp_pos, mrp_servo, MOSFET_PIN_MRP);
      }
      else if (ti_pos != mrp_pos){
        invertServoPos(ti_pos, ti_servo, MOSFET_PIN_TI);
      }
      else if (ti_pos == CLOSE && mrp_pos == CLOSE){
        invertServoPos(ti_pos, ti_servo, MOSFET_PIN_TI);
        invertServoPos(mrp_pos, mrp_servo, MOSFET_PIN_MRP);
      }
      break;
    case any:
      invertServoPos(ti_pos, ti_servo, MOSFET_PIN_TI);
      if (ti_pos == mrp_pos) {
        invertServoPos(mrp_pos, mrp_servo, MOSFET_PIN_MRP);
      }
      break;
  }
  
  delay(SYSTEMDELAY-PULSEWIDTH-TRAVELDELAY);
}

// Potentiometer-based control of mode selection
void updateMode () {
  int pot_val = analogRead(MODE_POT_PIN);
  mode = (Mode) map(pot_val % POT_MAX, 0, POT_MAX, 0, 4);
}

void loop() {
  // Battery signal
  volt_reg = analogRead(LED_IN);
  //Serial.println(flex_threshold);
  
  // LED CONTROL SECION
  // If below RED_THRESH, the battery is extremely low; if below YELLOW_THRESH, should change soon
  // Low/Empty battery takes precendence over flex/relaxed indicators - no light for full battery
  if (volt_reg < RED_THRESH) {
    writeColors(HIGH, LOW, LOW);
    if(smoothRead() >= flex_threshold){
      updateMode();
      updateMotors();
    }
  }
  else if (volt_reg < YELLOW_THRESH) {
    writeColors(HIGH, HIGH, LOW);
    if(smoothRead() >= flex_threshold){
      updateMode();
      updateMotors();
    }
  }
  else if (smoothRead() >= flex_threshold){ //if flexing, glow green
    writeColors(LOW, HIGH, LOW);
    if(smoothRead() >= flex_threshold){
      updateMode();
      updateMotors();
    }
  }
  else {
    writeColors(HIGH, LOW, HIGH); //relaxed - be purple
  }
  
  // Wait until below relax threshold if not already
  while (smoothRead() > flex_threshold * RELAX_THRESH_MULTIPLIER) {} 
}
