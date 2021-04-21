#include <ros.h>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int16.h>
#include <std_msgs/Bool.h>

#include <Adafruit_MLX90614.h>

// Temp sensor objects
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const int pressurePin = A0;
const int trigPin = 10;           //connects to the trigger pin on the distance sensor
const int echoPin = 12;           //connects to the echo pin on the distance sensor
const int buttonPin = 2; 

float distance = 0;               //stores the distance measured by the distance sensor
int pressure = 0;
int buttonState = 0; 

ros::NodeHandle node_handle;
std_msgs::Float32 distance_msg;
std_msgs::Int16 pressure_msg;
std_msgs::Bool reset_msg;
std_msgs::Float32MultiArray temp_msg;

ros::Publisher distance_publisher("distance", &distance_msg);
ros::Publisher pressure_publisher("pressure", &pressure_msg);
ros::Publisher reset_publisher("arm_control/reset", &reset_msg);
ros::Publisher pub_temp("biosensors/temp", &temp_msg);

void setup()
{
  // Start temp sensor
  mlx.begin();
  
  pinMode(trigPin, OUTPUT);   //the trigger pin will output pulses of electricity
  pinMode(echoPin, INPUT);    //the echo pin will measure the duration of pulses coming back from the distance sensor
  pinMode(buttonPin, INPUT);

  temp_msg.layout.dim[0].size = 1;
  temp_msg.layout.data_offset = 0;
  temp_msg.data = (float *)malloc(sizeof(float));
  temp_msg.data_length = 1;
  
  node_handle.initNode();
  node_handle.advertise(distance_publisher);
  node_handle.advertise(pressure_publisher);
  node_handle.advertise(reset_publisher);
  node_handle.advertise(pub_temp);
}

void loop() {
  pressure = analogRead(pressurePin);
  pressure_msg.data = pressure;
  pressure_publisher.publish(&pressure_msg);
  
  distance = getDistance();   //variable to store the distance measured by the sensor
  distance_msg.data = distance;
  distance_publisher.publish(&distance_msg);

  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    reset_msg.data = true;
    reset_publisher.publish(&reset_msg);
  }

  temp_msg.data[0] = mlx.readObjectTempF() + 2;
//  temp_msg.data[0] = rand() % 35 + 65;
  pub_temp.publish(&temp_msg);
  
  node_handle.spinOnce();
  delay(50);      //delay 50ms between each reading
}

//------------------FUNCTIONS-------------------------------

//RETURNS THE DISTANCE MEASURED BY THE HC-SR04 DISTANCE SENSOR
float getDistance()
{
  float echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  float calculatedDistance;         //variable to store the distance calculated from the echo time

  //send out an ultrasonic pulse that's 10ms long
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  echoTime = pulseIn(echoPin, HIGH);      //use the pulsein command to see how long it takes for the
                                          //pulse to bounce back to the sensor

  calculatedDistance = echoTime / 148.0;  //calculate the distance of the object that reflected the pulse (half the bounce time multiplied by the speed of sound)

  return calculatedDistance;              //send back the distance that was calculated
}
