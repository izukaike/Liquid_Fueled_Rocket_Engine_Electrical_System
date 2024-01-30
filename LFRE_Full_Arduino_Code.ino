/*
 * Contributors: 
 * - Izuka Ikedionwu
 * - Dylan Murphy 
 * 
 * Date Created: *TO BE FILLED* 
 * 
 * Description:
 *  DAQ software for 10 engine parts and communicates with real-time graphical user interface
 *  It may get confusing about differentiated between interfacing between hardware or front end
 *  I will add comments but just generalize top of loop is frotn end and bottom of loop is hardware and 
 *  in between is operation specific ( abort handling and test specific code 
 *  serves as middle tech PYTHON->ARDUINO->ELECTRONICS 
 *  
 *  
 *  Work:
 *  * check analog read
 *  * chek eeprom read
 *  * check eeprom write
 *  * check serial write
 *  * check serial read
 *  * check rx buffer
 *  * check tx buffr
 *  
 *  * erase other variables
 *  
 *  Dependencies:
 *   
 */

#include "stdlib.h"
#include "lfre_drivers.hpp"
#include "stdint.h"

class coil{
  public:
    /* initializes coil object
     *  
     *  passes pin
     *  
     *  returns nothing
     */
    coil(int pin)
    {
      this->pin = pin;
      pinMode(pin,OUTPUT);
      digitalWrite(pin,LOW);
    }
    /* sparks coil
     *  
     *  passes nothing
     *  
     *  returns nothing
     */
    void spark()
    {
      digitalWrite(this->pin,HIGH);
      //dwell time
      delayMicroseconds(5100);
      //
      digitalWrite(this->pin,LOW);
    } 
    private:
    int pin;
};

class buttons{
  public:
    /*  checks for open valve 1 command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool open_valve1(int input)
    {
      return (char)input == '1';
    }
    /*  checks for close valve 1 command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool close_valve1(int input)
    {
      return (char)input == '4';
    }
    /*  checks for open valve 2 command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool open_valve2(int input)
    {
      return (char)input == '2';
    }
    /*  checks for close valve 2 command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool close_valve2(int input)
    {
      return (char)input == '5';
    }
    /*  checks for spark coil command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool spark_coil(int input)
    {
      return (char)input == '7';
    }
    /*  checks for start plot command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool start_sending_data(int input)
    {
      return (char)input == '9';
    }
    /*  checks for calibrate command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool calibrate_offset(int input)
    {
      return (char)input == '@';
    }
    /*  checks for start plot  command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool activate_comm(int input)
    {
      return (char)input == '(';
    }
    /*  checks for close command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool deactivate_comm(int input)
    {
      return (char)input == ')';
    }
    /*  checks for test command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool use_test_data(int input)
    {
      return (char)input == '^';
    }
    /*  checks for valid serial data 
     *  
     *  passes input character/byte
     *  
     *  returns bool
     */
    bool valid_serial_read(int input)
    {
      return (int)input != -1;
    }
    /*  checks for test procedure command from  serial
     *  
     *  passes input character/byte
     *  
     *  returns bool
    */
    bool apply_safety_test_measures(int input)
    {
      return (int)input == '!';
    }
  
};

class valves{
  public:
    /*  default constructor
     *  
     *  passes pin
     *  
     *  returns nothing
     */
      valves(int pin)
      {
        pinMode(pin,OUTPUT);
        digitalWrite(pin,LOW);
        this->pin = pin;
      }
    /*  opens valve
     *  
     *  passes nothing
     *  
     *  returns nothing
     */
      void open_valve()
      {
        digitalWrite(pin,HIGH);
      }
    /*  closes valve
     *  
     *  passes nothing
     *  
     *  returns nothing
     */
      void close_valve()
      {
        digitalWrite(pin,LOW);
      }
    private:
      int pin;
};

class pressure_transducers{
  public:
      /*
       * default constructor
       * 
       * passes pina nd max pressure
       * 
       * returns nothing
       */
      pressure_transducers(int pin, int Pmax_pt)
      {
          pinMode(pin,INPUT);
          this->pin = pin;
          this->Pmax_pt = Pmax_pt;
       
      }
     /*  converts analog integer to pressure
     *  
     *  passes integer
     *  
     *  returns float pressure
     */
      float analog_to_pressure(int num)
      {
          float RawVoltage = num;
      
          // Convert the raw data value (0 - 1023) to voltage (0.0V - 5.0V)
          float Vread = RawVoltage * (5.0 / 1023.0);
  
          //Voltage adj from valve circuit
  
        return floor((Pmax_pt*(Vread-(I0*R)))/(R*(Imax-I0)));
      }
     /*  return offet
     *  
     *  passes nothing
     *  
     *  returns float offset
     */
      float get_offset()
      {
        return this->offset;
      }
    /*  set offset
     *  
     *  passes offset
     *  
     *  returns nothing
     */
      void set_offset(float offset)
      {
        this->offset = offset;
      }

  private:
      int pin;
      //Max Pressure of the pressure transducer
      int Pmax_pt = 1000;
      //Minimum voltage output in Volts of the pressure transducer
      int R =  240;
      //Maximum voltage output in Volts of the pressure transducer
      float I0 = 0.004;
      //Maximum current output in Amps of the pressure transducer
      float Imax = 0.02;
      //Define the offset from the calibration code
      float offset = 0;
};

class abort_framework{
    public:
        //FILL IN THESE FUNCTIONS

        /*check abort conditions for ducer 1
         * 
         * passes pressure from ducer 1
         * 
         * returns bool
         */
        bool abort_a1(float v1)
        {
          //ISHAAN change the bounds for ducer that reads chamber pressure
          return v1 > 1000 || v1 < 0;
        }
        /*check abort conditions for ducer 2
         * 
         * passes pressure from ducer 2
         * 
         * returns bool
         */
        bool abort_a2(float v1)
        {
          // //ISHAAN change the bounds for ducer that reads 
          return v1 > 1000 || v1 < 0;
        }
        /*check abort conditions for ducer 3
         * 
         * passes pressure from ducer 3
         * 
         * returns bool
         */
        bool abort_a3(float v1)
        {
           //ISHAAN change the bounds for ducer that reads chamber side closest to handle
          return v1 > 1000 || v1 < 0;
        }
        /*check abort conditions for ducer 4
         * 
         * passes pressure from ducer 4
         * 
         * returns bool
         */
        bool abort_a4(float v1)
        {
          //ISHAAN change the bounds for ducer that reads opposite chamber side closest to handle
          return v1 > 1000 || v1 < 0;
        }
        //ISHAAN PATEL
        void ISHAANS_FUNCTION()
        {
          //TODO
        }
        
    private:
        //max and min for ducer 1;
        int pt1_upper_limit = 230;
        int pt1_lower_limit = 175;

        //max and min for ducer 2
        int pt2_upper_limit = 230;
        int pt2_lower_limit = 175;

        //max and min for ducer 3
        int pt3_upper_limit = 475;
        int pt3_lower_limit = 400;

        //max and min for ducer 4
        int pt4_upper_limit = 475;
        int pt4_lower_limit = 400;
};

class status_register{
    public:
      /*
       * sets valve 1 flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void set_v1_flag()
      { 
        status_buff = status_buff | (1 << 6);
      }
      /*
       * clears valve 1 flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void clear_v1_flag()
      {
        status_buff = status_buff & (~(1 << 6));
      }
      /*
       * sets valve 2 flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void set_v2_flag()
      { 
        status_buff = status_buff | (1 << 5);
      }
      /*
       * cleats valve 1 flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void clear_v2_flag()
      {
        status_buff = status_buff & (~(1 << 5));
      }
      /*
       * sets igniter 1 flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void set_i1_flag()
      { 
        status_buff = status_buff | (1 << 4);
      }
      /*
       * clear igniter 1 flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void clear_i1_flag()
      {
        status_buff = status_buff & (~(1 << 4));
      }
      /*
       * sets abort pt1  flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void set_a1_flag()
      { 
        status_buff = status_buff | (1 << 3);
      }
      /*
       * clears abort pt1 flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void clear_a1_flag()
      {
        status_buff = status_buff & (~(1 << 3));
      }
      /*
       * sets abort pt2  flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void set_a2_flag()
      { 
        status_buff = status_buff | (1 << 2);
      }
      /*
       * clear abort pt2  flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void clear_a2_flag()
      {
        status_buff = status_buff & (~(1 << 2));
      }
      /*
       * sets abort pt3  flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void set_a3_flag()
      { 
        status_buff = status_buff | (1 << 1);
      }
      /*
       * clears abort pt3  flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void clear_a3_flag()
      {
        status_buff = status_buff & (~(1 << 1));
      }
      /*
       * sets abort pt4  flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void set_a4_flag()
      { 
        status_buff = status_buff | (1 << 7);
      }
      /*
       * cleats abort p4  flag
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void clear_a4_flag()
      {
        status_buff = status_buff & (~(1 << 7));
      }
      /*
       * resets register to default
       * 
       * passes nothing
       * 
       * return nothing
       * 
       */
      void reset_register()
      {
          status_buff = 0b00000001;
      }
      /*
       * return buffer
       * 
       * passes nothing
       * 
       * return byte/character
       * 
       */
      char get_buffer()
      {
        return status_buff;
      }
  
  private:
      //status buffer for testing and communicating excel and data analysis
      // register  8= pt 4, 7=v1, 6=v2, 5=i1,4=a1, 3=a2, 2=a3, 1 = start
      char status_buff = 0b00000001; 
};


class telemetry{
    public:
      /* writes 10 bits to serial comm
       *  
       *  passes int pressure
       *  
       *  returns nothing
       */
      void write_x_bits(int val)
      {
         // preparing 10-bit to be sent over 8-bit serial comm
         // crunching 10-bit value into 2 bytes
         uint8_t temp1_l = (uint8_t)((int)val >> 0);
         uint8_t temp1_h = (uint8_t)((int)val >> 8);
    
         delayMicroseconds(this->ser_comm_delay);
         Serial.write(temp1_l);
         delayMicroseconds(this->ser_comm_delay);
         Serial.write(temp1_h);
      }
      /* writes error message to serial comm
       *  
       *  passes nothing
       *  
       *  returns nothing
       */
      void write_error_message()
      {
         delayMicroseconds(this->ser_comm_delay);
         Serial.write(0x00);
         delayMicroseconds(this->ser_comm_delay);
         Serial.write(0x00);
      }
      uint8_t start_byte()
      {
         return 0b00000111;
      }    
    private:
      int ser_comm_delay = 100;
  
};

//MAYBE UNUSED
int test = false; 

//WILL BE DELETED
//analog pins
double pt1_data = A2;
double pt2_data = A1;
double pt3_data = A0;
double pt4_data = A3;

//sensor variables
float  avg_pressure1 = 0;
float  avg_pressure2 = 0;
float  avg_pressure3 = 0;
float  avg_pressure4 = 0;

float abs_pressure_cal1;
float abs_pressure_cal2;
float abs_pressure_cal3;
float abs_pressure_cal4;

//led for error checkinfg on the hardware level
int buff_check_led = 10;

//button handling variable
int rx_byte[5] = {0,0,0,0,0};
//coil emi protection variables 
bool coil_on = false;
int data = 0;
//serial comm variable
int ser_comm = 0;
//delay between serial read/write in microseconds
int ser_comm_delay =  100;

//button handling variables for if statements
int start_sending_data = 0;
int start_test_measures = 1;
int calibration_read = 0;

// pressure variables
float pressure1;
float pressure2;
float pressure3;
float pressure4;

int sample_rate = 1;

float current_sum1 = 0;
float current_sum2 = 0;
float current_sum3 = 0;
float current_sum4 = 0;

float sum1 = 0;
float sum2 = 0;
float sum3 = 0;
float sum4 = 0;

int calibration_sample_rate = 5000;

// serial comm variables
int avail_bytes;
int update_count = 1; 
int use_test_data = 0;
int memory_sync = 0;

//Class instantiation and initialization
//eeprom tmem(false);
coil c1(9);
buttons button;
valves valve1(3);
valves valve2(12);
pressure_transducers press_trans1(A2,1000);
pressure_transducers press_trans2(A1,300);
pressure_transducers press_trans3(A0,300);
pressure_transducers press_trans4(A3,1000);
abort_framework abort_check;
status_register status_buff;
telemetry tele_sys;

int test_val = 52;
int count = 0;
int te =0;
void setup() {
  //Serial setp and 2 second time is needed for proper usage
  Serial.begin(115200);
  delay(2000); // seconds
  
  //HARDWARE INTERFACE SET UP
  pinMode(buff_check_led,OUTPUT);
  
  //default on so it is obvious if something went wrong
  digitalWrite(buff_check_led,HIGH);
  delayMicroseconds(ser_comm_delay);
}

// the loop function runs over and over again...forever
void loop() 
{
//uncomment for runtime analsyis
//int start_time = millis();

   //BUTTON HANDLING
   delayMicroseconds(ser_comm_delay);
   avail_bytes = Serial.available();
   //if byte is in rx buff?
   for(int i = 0; i < avail_bytes;++i)
   {
      //read command from pc
      rx_byte[i] = Serial.read();
      
      delayMicroseconds(ser_comm_delay);

      if( button.valid_serial_read(rx_byte[i])){
          delayMicroseconds(ser_comm_delay);
          
               //OPENS VALVE 1
          if(button.open_valve1(rx_byte[i]))
          {
              valve1.open_valve();
              
              status_buff.set_v1_flag();
              
              //CLOSES VALVE 1  
          }
          else if(button.close_valve1(rx_byte[i]))
          {
              valve1.close_valve();
              status_buff.clear_v1_flag();
              
              //OPENS VALVE 2
          }
          else if (button.open_valve2(rx_byte[i]))
          {
              valve2.open_valve();
              status_buff.set_v2_flag();
              //status_buff = status_buff | (1 << 5);
              
              //CLOSES VALVE 2
          }
          else if (button.close_valve2(rx_byte[i]))
          {
              valve2.close_valve();
              status_buff.clear_v2_flag();
              //status_buff = status_buff & (~(1 << 5));
              
              //SIGNAL TO START SENDING DATA
          }
          else if(button.start_sending_data(rx_byte[i]))
          {
              start_sending_data = 1;
        
              //ACTIVATES COIL
          }
          else if(button.spark_coil(rx_byte[i]))
          {
              status_buff.set_i1_flag();
              //status_buff = status_buff | (1 << 4);
              coil_on = true;
              c1.spark();
              
          }
          else if(button.apply_safety_test_measures(rx_byte[i]))
          {
              start_test_measures = 1;
              
              //CALIBRATES TRANSDUCER FOR OFFSET
          }
          else if(button.calibrate_offset(rx_byte[i]))
          {
              calibration_read = 1;

              //DEACTIVATES COMMM 
          }
          else if(button.deactivate_comm(rx_byte[i]))
          {
              ser_comm = 0;
              status_buff.reset_register();
              //status_buff = 0b00000001;
              //ACTIVATES COMMM 
          }
          else if(button.activate_comm(rx_byte[i]))
          {
              ser_comm = 1;
            
            //SEND TEST DATA
          }
          else if(button.use_test_data(rx_byte[i]))
          {
              use_test_data = 1;
              status_buff.reset_register();
              //status_buff = 0b00000001;
              memory_sync +=1;
          }
       }
       
   }
//////////////////////////////////////////////////////////////
//Calibration Code // repeat just with more samples and only offset taken in consideration
if(calibration_read == 1){
     //close valves
     /*
    delayMicroseconds(ser_comm_delay);
    valve1.open_valve();
    valve2.open_valve();
    delayMicroseconds(ser_comm_delay);
    */
    
  for (int i = 0; i < calibration_sample_rate; i++) {
 
        //read the raw data coming in on analog pins
         //pressure1 = press_trans1.analog_to_pressure(analogRead(pt1_data));
         pressure1 = press_trans1.analog_to_pressure(197);
         pressure2 = press_trans2.analog_to_pressure(197);
         pressure3 = press_trans3.analog_to_pressure(197);
         pressure4 = press_trans4.analog_to_pressure(197);
         
        //computes sum the sum of sample pressures 
        current_sum1 = current_sum1 + pressure1;
        current_sum2 = current_sum2 + pressure2;
        current_sum3 = current_sum3 + pressure3;
        current_sum4 = current_sum4 + pressure4;
    }
    
    //Calculate the average pressure and calibrate
    avg_pressure1 = current_sum1 / calibration_sample_rate;
    avg_pressure2 = current_sum2 / calibration_sample_rate;
    avg_pressure3 = current_sum3 / calibration_sample_rate;
    avg_pressure4 = current_sum4 / calibration_sample_rate;

    current_sum1 = 0;
    current_sum2 = 0;
    current_sum3 = 0;
    current_sum4 = 0;

    //Calibrate pressure and change to absolute
    press_trans1.set_offset(14.7 - avg_pressure1);
    press_trans2.set_offset(14.7 - avg_pressure2);
    press_trans3.set_offset(14.7 - avg_pressure3);
    press_trans4.set_offset(14.7 - avg_pressure4);
    
    //close valves
    valve1.close_valve();
    valve2.close_valve();
    
    calibration_read = 0;
}

/////////////////////////////////////////////////////////////////
  //DATA HANDLING
  if(start_sending_data == 1)
  {
    //main resolution bottle neck is pres_sampes1 value ( lower = faster) 
    for (int i = 0; i < sample_rate; i++)
    {
        //read the raw data coming in on analog pins
        if(use_test_data == 0){
            //this is a problem i am accessing pin A2 from 2 places
            pressure1 = press_trans1.analog_to_pressure(analogRead(pt1_data));
            pressure2 = press_trans2.analog_to_pressure(analogRead(pt2_data));
            pressure3 = press_trans3.analog_to_pressure(analogRead(pt3_data));
            pressure4 = press_trans4.analog_to_pressure(analogRead(pt4_data));
            
            //Serial.println(pressure1);
        //use data and memory sync to start sending test data right after memory sync -> details below
        }else if(use_test_data == 1 && memory_sync >= 2){
            //NEED TO FIX EEPROM CONSTRUCTOR
            /*
            //replace sensar with manufactured data from 
            press_trans1.analog_to_pressure(tmem.read_());
            press_trans2.analog_to_pressure(tmem.read_());
            press_trans3.analog_to_pressure(tmem.read_());
            press_trans4.analog_to_pressure(tmem.read_());
            */
 
        }
        
        //Compute the sum of sample pressures 
        current_sum1 += pressure1;
        current_sum2 += pressure2;
        current_sum3 += pressure3;
        current_sum4 += pressure4;
 
    }

    if(start_sending_data == 0){
      abs_pressure_cal1 = 2;
      abs_pressure_cal2 = 2;
      abs_pressure_cal3 = 2;
      abs_pressure_cal4 = 4; 
    }

    
    //Calculate the average pressure and calibrate
    avg_pressure1 = current_sum1 / sample_rate;
    avg_pressure2 = current_sum2 / sample_rate;
    avg_pressure3 = current_sum3 / sample_rate;
    avg_pressure4 = current_sum4 / sample_rate;
    
    current_sum1 = 0;
    current_sum2 = 0;
    current_sum3 = 0;
    current_sum4 = 0;
    
    //Calibrate pressure and change to absolute
    abs_pressure_cal1 = avg_pressure1 + press_trans1.get_offset();
    abs_pressure_cal2 = avg_pressure2 + press_trans2.get_offset();
    abs_pressure_cal3 = avg_pressure3 + press_trans3.get_offset();
    abs_pressure_cal4 = avg_pressure4 + press_trans4.get_offset();

    /////////////////////////////////////////////////////////////////////
    // Function used here that implements MPC and other filtering techniques
    // use python for data and filter visual comparison
    //////////////////////////////////////////////////////////////////////
    
    //Exception Code: INCOMPLETE
    //only applied when test procedure is on
    if(start_test_measures == 1)
    {
        if(abort_check.abort_a1((int)abs_pressure_cal1))
        {
            valve1.close_valve();
            valve2.close_valve();
            status_buff.set_a1_flag();
            //status_buff = status_buff | (1 << 3);
        }
        
        if(abort_check.abort_a2((int)abs_pressure_cal2))
        {
             valve1.close_valve();
             valve2.close_valve();
             status_buff.set_a2_flag();
             //status_buff = status_buff | (1 << 2);
        }
    
        if(abort_check.abort_a3((int)abs_pressure_cal3))
        {    
             valve1.close_valve();
             valve2.close_valve();
             status_buff.set_a3_flag();
             //status_buff = status_buff | (1 << 1);
        }
 
        if(abort_check.abort_a4((int)abs_pressure_cal4))
        {
             valve1.close_valve();
             valve2.close_valve();
             status_buff.set_a4_flag();
             //status_buff = status_buff | (1 << 7);
        }
    }
    /*
    abs_pressure_cal1 = 250;
    abs_pressure_cal2 = 250;
    abs_pressure_cal3 = 768;
    abs_pressure_cal4 = 768;
    */ 
     // start bit is 0x01 so default is 0x02 and 0x04 so start bit is in 
     // correct position for data sync on serial comm with python
     if(start_sending_data == 0 && ser_comm == 0 && update_count % 3 != 0){
      abs_pressure_cal1 = 2;
      abs_pressure_cal2 = 2;
      abs_pressure_cal3 = 2;
      abs_pressure_cal4 = 4; 
    }
    if(abs_pressure_cal1 == 7){abs_pressure_cal1++;}
    if(abs_pressure_cal2 == 7){abs_pressure_cal2++;}
    if(abs_pressure_cal3 == 7){abs_pressure_cal3++;}
    if(abs_pressure_cal4 == 7){abs_pressure_cal4++;}
    
     
    //MAIN CODE THAT SENDS TO PYTHON
    //data sync
    //HANDLE COIL ON THE PYTHON SIDE  ~~ CHECK FOR FLAG
    if(ser_comm == 1)
    {   
    
        if( update_count % 3 == 0 )
        {
          delayMicroseconds(ser_comm_delay);
          Serial.write(tele_sys.start_byte());
          delayMicroseconds(ser_comm_delay);
          Serial.write(status_buff.get_buffer());
          
          //resets the coil flag in status buffer
          status_buff.clear_i1_flag();
          //status_buff = status_buff & (~(1 << 4));
        } 
        if(ser_comm == 1)
        {
            //writes pressure 1
            tele_sys.write_x_bits((int)abs_pressure_cal1);

            //writes pressure 2
            tele_sys.write_x_bits((int)abs_pressure_cal2);

            //write pressure 3
            tele_sys.write_x_bits((int)abs_pressure_cal3);

            //write pressure 4
            tele_sys.write_x_bits((int)abs_pressure_cal4);
            
        }
        else
        {   //Zero means something bad because the lowest should be 13/14/15
            tele_sys.write_error_message();
        }
        
        // properly increments memory variable so test data is sent right after
        // flag register and serial comm is synchronized
        if(use_test_data == 1 && (update_count+1) % 3 == 0)
        {
            memory_sync += 1;
        }
    }
    //always updates to keep everything in sync
    update_count += 1;
    
    //always resets the coil because it lasts 1 cycle 
    //to avoid emi
    coil_on = false;
  }
}
