/* Engineer: Izuka Ikedionwu
 * Date Created: 10/1/23
 * 
 * 
 * 
 * 
 * 
 */

 //Arduino analog pin number
int analogPin = 0;
//Max Pressure of the pressure transducer
int Pmax = 300;
//Resistance in ohms (in series)
float R = 244;
//Minimum current output in Amps of the pressure transducer
float I0 = 0.004;
//Maximum current output in Amps of the pressure transducer
float Imax = 0.02;
//initialize sum for pressure smoothing
float pres_sum = 0;
//Define the offset from the calibration code
float offset = 16.03;
//how many samples you want to take for each pressure output
int pres_samples = 100;


int valve1 = 2;
int valve2 = 4;
char input;
//double pt1_data = A5;
//double pt2_data = A2;
double pt1_data = A0;
int data =0;
int perc = 0;
int cap = 1023;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(pt1_data,INPUT);
  //pinMode(pt2_data,INPUT);
  //pinMode(pt3_data,INPUT);
  Serial.begin(115200);
  pinMode(valve1,OUTPUT);
  pinMode(valve2,OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
   if(Serial.available() > 0){
      delay(2);
      input = Serial.read();
      if(input == '1'){
        digitalWrite(valve1,HIGH);
        delay(250);
        digitalWrite(valve1,LOW);
        delay(2);
      }else if (input == '2'){
        digitalWrite(valve2,HIGH);
        delay(250);
        digitalWrite(valve2,LOW);
        delay(2);
      }
   }
   // put your main code here, to run repeatedly:
    for (int i = 0; i < pres_samples; i++) {
    //read the raw data coming in on analog pin 0
    float RawVoltage = analogRead(analogPin);
    // Convert the raw data value (0 - 1023) to voltage (0.0V - 5.0V)
    float Vread = RawVoltage * (5.0 / 1024.0);
    //Convert voltage read to gauge pressure in psi and store in an array
    float pressure = (Pmax*(Vread-(I0*R)))/(R*(Imax-I0));
    //Create the sum of sample pressures 
    pres_sum = pres_sum + pressure;
  }
  //Calculate the average pressure and calibrate
  float avg_pressure = pres_sum / pres_samples;
  pres_sum = 0;
  //Calibrate pressure and change to absolute
  float abs_pressure_cal = avg_pressure + offset;
  // write the pressure value to the serial monitor:
  Serial.write((int)abs_pressure_cal);
  delay(2);
   /*
  // reading 3 pressure transducers
   Serial.write(analogRead(pt1_data));
   delay(2);
   Serial.write(analogRead(pt2_data));
   delay(2);
   Serial.write(analogRead(pt3_data));
   delay(2);
   // wait for a second
   */
}
