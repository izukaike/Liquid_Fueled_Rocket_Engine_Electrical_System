/* Comments and headers still need to be added but this file 
writes sensor data to pythona nd reds button data from python
*/
#include <Servo.h>
//servo motor used for testing
Servo servo1;
Servo servo2;
Servo servo3;
//initializzing i/o pins
//digital
int valve1 = 2;
int valve2 = 4;
int valve3 = 7;

char input;
//analog pins
double pt1_data = A5;
double pt2_data = A2;
double pt3_data = A0;

int data =0;
int perc = 0;
int cap = 1023;
int  read_delay =   2000;
int start = 0;
int calibration_read = 0;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(pt1_data,INPUT);
  pinMode(pt2_data,INPUT);
  pinMode(pt3_data,INPUT);
  Serial.begin(57600);
  servo1.attach(valve1);
  servo2.attach(valve2);
  servo3.attach(valve3);
}

// the loop function runs over and over again forever
void loop() {
   //Checks if input buffer has stuff so that it doestn read randomly
   if(Serial.available() > 0){
      delay(2);
      input = Serial.read();
      //1 is sent by Python
      if(input == '1'){
        servo1.write(180);  
      }else if(input == '4'){
        servo1.write(-180); 
       //2 is sent by Python
      }else if (input == '2'){
        servo2.write(180);
      }else if (input == '5'){
        servo2.write(-180); 
      //3 is sent by Python
      }else if(input == '3'){
        servo3.write(180);
      }else if(input == '6'){
        servo3.write(-180); 
      }
   }

  if(Serial.available() > 0){
    if(Serial.read() == '8'){
      calibration_read = 1;
    }else if(Serial.read() == '7'){
      calibration_read = 0;
    }
  }
  if(calibration_read == 1){
      // reading 3 pressure transducers
      Serial.write(analogRead(pt1_data));
      //delay is needed but we should see if it can be synced
      delayMicroseconds(read_delay);
      Serial.write(analogRead(pt2_data));
      delayMicroseconds(read_delay);
      Serial.write(analogRead(pt3_data));
      delayMicroseconds(read_delay);
  }

  if(Serial.available() > 0){
    if(Serial.read() == '9'){
      start = 1;
    }
  }
  if(start == 1){
      // reading 3 pressure transducers
      Serial.write(analogRead(pt1_data));
      //delay is needed but we should see if it can be synced
      delayMicroseconds(read_delay);
      Serial.write(analogRead(pt2_data));
      delayMicroseconds(read_delay);
      Serial.write(analogRead(pt3_data));
      delayMicroseconds(read_delay);
  }
}
