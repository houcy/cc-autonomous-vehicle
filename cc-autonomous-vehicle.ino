/*
Jordan Shaw
Added Secondary Motor
Timer to test and control wheel movements with forward, stop, left and right
*/

//Setting up the QTR Sensor
#include <QTRSensors.h>
#include <Ultrasonic.h>

//#define NUM_SENSORS             6  // number of sensors used
#define NUM_SENSORS             5  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

// sensors 0 through 5 are connected to analog inputs 0 through 5, respectively
//QTRSensorsAnalog qtra((unsigned char[]) {0, 1, 2, 3, 4, 5}, 
//  NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
QTRSensorsAnalog qtra((unsigned char[]) {0, 1, 2, 3, 4, 5}, 
  NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];

#define KP .2
#define KD 5
#define M1_DEFAULT_SPEED 40
#define M2_DEFAULT_SPEED 40
#define M1_MAX_SPEED 300
#define M2_MAX_SPEED 300
#define DEBUG 0 // set to 1 if serial debug output needed


Ultrasonic ultrasonic(7, 4);
int distance = 0;
boolean hasStarted = false;


//For the start button
#define START_BUTTON_PIN 8  // emitter is controlled by digital pin 2
int motorState = LOW;
int motorStateButton;
int motorPrevious = LOW;
long time = 0;
long debounce = 200;

int lastError = 0;
int last_proportional = 0;
int integral = 0;


// Motor One
int enablePin = 11;
int in1Pin = 10;
int in2Pin = 9;

// Motor Two
int enablePin2 = 6;
int in3Pin = 2;
int in4Pin = 3;

//int switchPin = 7;
int potPin = 0;

const int onTime1 = 0; // in ms
const int offTime1 = 1000; // in ms

const int onTime2 = 1001; // in ms
const int offTime2 = 2000; // in ms

const int onTime3 = 2001; // in ms
const int offTime3 = 3000; // in ms

const int onTime4 = 3001; // in ms
const int offTime4 = 4000; // in ms

const int onTime5 = 4001; // in ms
const int offTime5 = 5000; // in ms

boolean currentlyOn = false;
boolean forwardOn = false;
boolean backwardOn = false;
boolean leftOn = false;
boolean rightOn = false;

// will store last time LED was updated
unsigned long previousMillis = 0;
unsigned long startTime;

//  Print the line position every...
const long lineInterval = 2000;
unsigned long linePreviousMillis = 0;
unsigned long lineAveragePreviousMillis = 0;

void setup()
{

  Serial.begin(9600);

  startTime = millis();
  
  //  Motor One
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  //Motor Two
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  pinMode(enablePin2, OUTPUT);
  
//  pinMode(switchPin, INPUT_PULLUP);

  // For start Button
  pinMode(START_BUTTON_PIN, INPUT);

  // Collaboration of the QTR Sensor
  
  delay(500);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);    // turn on Arduino's LED to indicate we are in calibration mode
  for (int i = 0; i < 400; i++)  // make the calibration take about 10 seconds
  {
    qtra.calibrate();       // reads all sensors 10 times at 2.5 ms per six sensors (i.e. ~25 ms per call)
  }
  digitalWrite(13, LOW);     // turn off Arduino's LED to indicate we are through with calibration

  if(DEBUG){
  
    // print the calibration minimum values measured when emitters were on
    for (int i = 0; i < NUM_SENSORS; i++)
    {
      Serial.print(qtra.calibratedMinimumOn[i]);
      Serial.print(' ');
    }
    Serial.println();
    
    // print the calibration maximum values measured when emitters were on
    for (int i = 0; i < NUM_SENSORS; i++)
    {
      Serial.print(qtra.calibratedMaximumOn[i]);
      Serial.print(' ');
    }
    Serial.println();
    Serial.println();
    delay(1000);
  }
  
}

void loop() {

  unsigned long currentMillis = millis();

  motorStateButton = digitalRead(START_BUTTON_PIN);
  if(motorStateButton == LOW && motorPrevious == HIGH && currentMillis - time > debounce) {
    if(motorState == LOW){
      motorState = HIGH; 
    } else {
      Serial.print("DOES THIS END UP DOING SOMETHING ONCE START???");
       motorState = LOW; 
    }
    time = millis();
  }
  motorPrevious = motorStateButton;

  distance = ultrasonic.Ranging(CM);//get the current result;

  if(distance > 400){
    distance = 400;
  }
  
//  Serial.print("distance: ");
//  Serial.println(distance);
 
  //  Read speed earlier on to help with the rotation to find the opening
  int speed = analogRead(potPin) / 4;
  if (speed < 100){
    speed = 350; 
  }

  // Wait before the start button is pushed to start driving
  if (motorState == LOW){
    Serial.println("DO NOTHING!!!");
    analogWrite(enablePin, 0);
    analogWrite(enablePin2, 0);
    digitalWrite(in1Pin, LOW); 
    digitalWrite(in2Pin, LOW); 
    digitalWrite(in3Pin, LOW); 
    digitalWrite(in4Pin, LOW); 
    return;  
  }

//  Serial.print("hasStarted: ");
//  Serial.println(hasStarted);

  // Uncomment for testing without the sonar sensor enabled
  // distance = 50;

  if(distance <= 20 && hasStarted == 0){
    // spin in circles to find an exit
//    Serial.println("======================");
//    Serial.print("SPEED IN CASE: ");
    
    if (speed > M1_MAX_SPEED ) speed = M1_MAX_SPEED; // limit top speed
    if (speed < 0) speed = 0; // keep motor above 0
//    Serial.println(speed);

    // SPIN IN CIRCLE TO FIND EXIT
    analogWrite(enablePin, speed);
    //  digitalWrite(in1Pin, HIGH);
    //  digitalWrite(in2Pin, LOW);
    digitalWrite(in1Pin, LOW); 
    digitalWrite(in2Pin, HIGH); 

    analogWrite(enablePin2, speed);
    digitalWrite(in3Pin, HIGH); 
    digitalWrite(in4Pin, LOW);
    return;
  } else {
    // Go forward
    // Don't do anything else!
    // Set the has started flag to true so we know not to go 
    // into a frantic spin cycle when following the line
    hasStarted = 1;
  }

//  LOG SPEED
//  Serial.print("speed: ");
//  Serial.println(speed);
  
  //  boolean reverse = digitalRead(switchPin);
  //  setMotor(speed, reverse);
  
  // Get averaged line position
  unsigned int position = qtra.readLine(sensorValues);
  int error = position - 2000;

  int motorSpeed = KP * error + KD * (error - lastError);
  lastError = error;

  int leftMotorSpeed = M1_DEFAULT_SPEED + motorSpeed;
  int rightMotorSpeed = M2_DEFAULT_SPEED - motorSpeed;

  // If there's isnt any lines, go straight
  if (position >= 4000 || position <= 0) {
    // set motor speeds using the two motor speed variables above
    set_motors(300, 300, speed);
  } else {
    // set motor speeds using the two motor speed variables above
    set_motors(leftMotorSpeed, rightMotorSpeed, speed);
  }

  if(currentMillis - lineAveragePreviousMillis >= lineInterval) {
    lineAveragePreviousMillis = currentMillis;
    // comment this line out if you are using raw values
    Serial.print("Line Position: ");
    Serial.println(position);
  }
  
//  if (currentMillis >= startTime + onTime1 && currentMillis <= startTime + offTime1){ // Switch resistor off
//    drive_forward(speed, reverse);
//    currentlyOn=false;
//    Serial.println("1");
//  } else if(currentMillis >= startTime + onTime2 && currentMillis <= startTime + offTime2) {
//    drive_backward(speed, reverse);  
//    currentlyOn=true;
//    Serial.println("2");
//  } else if (currentMillis >= startTime + onTime3 && currentMillis <= startTime + offTime3) {
//    turn_left(speed, reverse);  
//    Serial.println("3");
//  } else if (currentMillis >= startTime + onTime4 && currentMillis <= startTime + offTime4) {
//    turn_right(speed, reverse);  
//    Serial.println("4");
//  } else if (currentMillis >= startTime + onTime5 && currentMillis <= startTime + offTime5) {
//    motor_stop(speed, reverse);
//    Serial.println("5");
//  } else {
//    // Reset timer
//    startTime=millis();  
//  }
}

void set_motors(int motor1speed, int motor2speed, int speed) {
  
  unsigned long currentMillisMotor = millis();
  
  if (motor1speed > M1_MAX_SPEED ) motor1speed = M1_MAX_SPEED; // limit top speed
  if (motor2speed > M2_MAX_SPEED ) motor2speed = M2_MAX_SPEED; // limit top speed
  if (motor1speed < 0) motor1speed = 0; // keep motor above 0
  if (motor2speed < 0) motor2speed = 0; // keep motor speed above 0

  // Print Motor speed debugging every "line Interval" seconds
  if(currentMillisMotor - linePreviousMillis >= lineInterval) {
    linePreviousMillis = currentMillisMotor;
    
    Serial.print("Motor 1 Speed: ");
    Serial.print(motor1speed);
    
    Serial.print("\t Motor 2 Speed: ");
    Serial.println(motor2speed);
  }

    //  analogWrite(enablePin, speed);
    analogWrite(enablePin, motor1speed * 2);
    digitalWrite(in1Pin, HIGH); 
    digitalWrite(in2Pin, LOW); 

    //  analogWrite(enablePin2, speed);
    analogWrite(enablePin2, motor2speed * 2);
    digitalWrite(in3Pin, HIGH); 
    digitalWrite(in4Pin, LOW);
}


// Function for testing direction control
void setMotor(int speed, boolean reverse)
{
  //  Motor One
  analogWrite(enablePin, speed);
  digitalWrite(in1Pin, ! reverse);
  digitalWrite(in2Pin, reverse);

  //  Motor Two
  analogWrite(enablePin2, speed);
  digitalWrite(in3Pin, ! reverse);
  digitalWrite(in4Pin, reverse);
}


// Stearing The Motor
void motor_stop(int speed, boolean reverse){
  analogWrite(enablePin, speed);
  digitalWrite(in1Pin, LOW); 
  digitalWrite(in2Pin, LOW); 

  analogWrite(enablePin2, speed);
  digitalWrite(in3Pin, LOW); 
  digitalWrite(in4Pin, LOW);
}

void drive_forward(int speed, boolean reverse){
  analogWrite(enablePin, speed);
  digitalWrite(in1Pin, HIGH); 
  digitalWrite(in2Pin, LOW); 

  analogWrite(enablePin2, speed);
  digitalWrite(in3Pin, HIGH); 
  digitalWrite(in4Pin, LOW); 
}

void drive_backward(int speed, boolean reverse){
  analogWrite(enablePin, speed);
  digitalWrite(in1Pin, LOW); 
  digitalWrite(in2Pin, HIGH); 

  analogWrite(enablePin2, speed);
  digitalWrite(in3Pin, LOW); 
  digitalWrite(in4Pin, HIGH); 
}

void turn_left(int speed, boolean reverse){
  analogWrite(enablePin, speed);
  digitalWrite(in1Pin, LOW); 
  digitalWrite(in2Pin, HIGH); 

  analogWrite(enablePin2, speed);
  digitalWrite(in3Pin, HIGH); 
  digitalWrite(in4Pin, LOW);
}

void turn_right(int speed, boolean reverse){
  analogWrite(enablePin, speed);
  digitalWrite(in1Pin, HIGH); 
  digitalWrite(in2Pin, LOW); 

  analogWrite(enablePin2, speed);
  digitalWrite(in3Pin, LOW); 
  digitalWrite(in4Pin, HIGH); 
}
