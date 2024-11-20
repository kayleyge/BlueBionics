const int FSR_PIN = A0; // Analog pin connected to FSR
#define MOVING_AVERAGE_BUFFER 8
#define MAX_READING 1023 //implies maximum pressure applied to sensor
#define MIN_READING 0 //implies minimum pressure applied to sensor

struct MovingAverage{
  double starting_value = (MAC_READING + MIN_READING) / 2
  uint8_t arr[MOVING_AVERAGE_BUFFER] = { starting_value };
  uint8_t counter = 0;
  double sum = starting_value * MOVING_AVERAGE_BUFFER;

  //Returns one data point and updates average
  unsigned updateAvg (int new_value){
      //Take out oldest value from sum
    sum -= arr[counter];

    // Read in newest value and overwrite oldest
    arr[counter] = analogRead(FSR_PIN);

    // Update sum to reflect new value
    sum += arr[counter];

    //Increment counter and do modulo to make sure it wraps around
    counter = (counter + 1) % MOVING_AVERAGE_BUFFER

    //return average
    Serial.println(sum / MOVING_AVERAGE_BUFFER);
    return sum / (double(MOVING_AVERAGE_BUFFER));

  }// updateAvg
}

MovingAverage avg;

void setup() {
  Serial.begin(9600);
  pinMode(FSR_PIN, INPUT);
}

void loop() {
  // Read analog value from FSR
  int fsrReading = analogRead(FSR_PIN);

  int balancedfsrReading = avg.updateAvg(fsrReading);

  // Print the FSR value to the Serial Monitor
  Serial.print("FSR Analog Reading: ");
  Serial.println(fsrReading);

  Serial.print("FSR Smooth Reaidng: ");
  Serial.println(balancedfsrReading);

  // Add a delay to make readings more readable
  delay(500); // 500 ms delay
}
