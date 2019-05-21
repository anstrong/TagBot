#include <Wire.h>
#include <NewPing.h>
#include <Robot.h>
#include <Wheel.h>

#define uchar unsigned char
#define arraySize 16 // Size of line sensor array
uchar t;
uchar data[arraySize];

#define triggerPin 12
#define echoPin 13
#define maxDistance 100

#define leftPWM 5
#define left1 6
#define left2 7
#define rightPWM 10
#define right1 9
#define right2 8

int sensorValue;
int senseLeft; // First sensor on each side that detects a line
int senseRight;
int leftDifference; // Distance of sense(side) from desired position of line (in this case on the far right)
int rightDifference;
int difference; // Distance between senseLeft and senseRight (typically the width of the line)

int basePwr; // Base power for drive
float pwrScale; // Scale power up or down to accomodate changing battery voltage
float jerk; // Scale for line data in power equation
float acceleration; // Scale for ultrasonic data in power equation
int rightDrive; // Total power for each wheel
int leftDrive;

bool lineStatus; // Indicates whether a line is sensed (plus "null", meaning on the black mat")
int distance; // Indicates whether an object is within the ultrasonic sensing range and its distance from the robot

void getData() // Cycle through sensors and put data into array
{
  Wire.requestFrom(9, 16);
  while (Wire.available())
  {
    data[t] = Wire.read();
    if (t < arraySize - 1)
      t++;
    else
      t = 0;
  }
}

int detectObject(int range) // Read ultrasonic sensor and determine if obstable exists
{
  int reading = sonar.ping_cm();

  if (reading > 0 && reading <= range)
  {
    return reading;
  }
  else
  {
    return range + 1;
  }
}

bool findLine(int idealIndex) // Check each sensor from each side to determine if a line exists and, if so, where on the array
{
  senseLeft = 16;
  senseRight = 16;

  for (int p = arraySize; p >= 0; p = p - 2) // Cycle from right
  {
    sensorValue = data[p];

    if (sensorValue < 10)
    {
      senseLeft = p;
    }
  }

  for (int i = 0; i < arraySize; i = i + 2) // Cycle from left
  {
    sensorValue = data[i];

    if (sensorValue < 10)
    {
      senseRight = i;
    }
  }

  // Calculate where line is, width
  leftDifference = idealIndex - senseLeft; //senseLeft - (arraySize - idealIndex); //senseLeft
  rightDifference = senseRight - idealIndex; //idealIndex - senseRight; //16-senseRight
  difference = (senseRight - senseLeft);

   /*Serial.println(" ");
   Serial.print(senseLeft);
   Serial.print(", ");
   Serial.print(senseRight);*/
  /*Serial.println(" ");
   Serial.print(leftDifference);
   Serial.print(", ");
   Serial.print(rightDifference);*/
   
   /*Serial.println(" ");
   Serial.print(difference);*/

  if(senseLeft == 0 and senseRight == 2)
  {
    return false;
  }
  else
  {
    return true;
  }
}


Robot myRobot(leftPWM, left1, left2, rightPWM, right1, right2);

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  t = 0;

  basePwr = 150;
  pwrScale = .7;
  jerk = 5;
  acceleration = 5;
}

void loop()
{
  getData(); // Read line sensor array
  lineStatus = findLine(16); // Determine line existence and location
  distance = detectObject(30); // Check for object within 20cm

  /*Serial.println(" ");
    Serial.print(sonar.ping_cm());
    Serial.print(", ");
    Serial.print(object);*/
   /*Serial.println(" ");
    Serial.print(lineStatus);*/

  if (lineStatus == false) // If all black detected (on the mat and about to drive off table), stop
  {
    rightDrive = 0;
    leftDrive = 0;
  }
  else
  {
    // Set each wheel equal to base power plus scaled line data and ultrasonic data, scale all according to battery voltage
    //rightDrive = (basePwr + ((rightDifference - leftDifference) * jerk) + (distance * acceleration)) * pwrScale;
    //leftDrive = ((basePwr * .75) + ((leftDifference - rightDifference) * jerk) + (distance * acceleration)) * pwrScale;
      rightDrive = (basePwr - (rightDifference*jerk));//*distance/20;
      leftDrive = (basePwr - (leftDifference*jerk));//*distance/20;
  }

  /*Serial.println(" ");
  Serial.print(leftDrive);
  Serial.print(", ");
  Serial.print(rightDrive);*/

  myRobot.drive(leftDrive, rightDrive); // Set robot's wheels to computed power values
}
