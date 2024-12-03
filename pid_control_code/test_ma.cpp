#define MOVING_AVERAGE_BUFFER 15
#define MAX_READING 1023 //implies maximum pressure applied to sensor
#define MIN_READING 0 //implies minimum pressure applied to sensor

struct MovingAverage{
  double starting_value = (MAX_READING + MIN_READING) / 2;
  double arr[MOVING_AVERAGE_BUFFER];
  int counter = 0;
  double sum = starting_value * MOVING_AVERAGE_BUFFER;

  void initializeArray() {
    for (int i = 0; i < MOVING_AVERAGE_BUFFER; i++) {
      arr[i] = starting_value;
    }
  }

  //Returns one data point and updates average
  unsigned updateAvg (int new_value){
      //Take out oldest value from sum
    sum -= arr[counter];

    // Read in newest value and overwrite oldest
    // arr[counter] = analogRead(FSR_PIN);
    arr[counter] = new_value;

    // Update sum to reflect new value
    sum += arr[counter];

    //Increment counter and do modulo to make sure it wraps around
    counter = ((counter + 1) % MOVING_AVERAGE_BUFFER);

    //return average
    // Serial.println(sum / MOVING_AVERAGE_BUFFER);
    return (sum / (double(MOVING_AVERAGE_BUFFER)));

  }// updateAvg
};

int main() {
    
}