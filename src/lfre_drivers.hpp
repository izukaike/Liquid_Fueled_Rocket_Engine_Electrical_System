/*
 * Contributors: 
 * - Izuka Ikedionwu
 * 
 * Date Created: 12/1/23
 * 
 * Description:
 *  .hpp file but C++ class framework is used but the functions and variables are used as C
 *  this file file has memoey mapped gpio, 10 bit analog read, and reading and writing
 *  to anf from eeprom (electrically erasable programmable read only memory) it is non volatile
 *  so it is good for saving critical system data, hardcoding tests(max is 1023 bytes so testing is limited), and serial read
 *  and write drivers are writte here. They are not implemented into the main code yet.
 *  
 *  
 *  TODO:
 *  *fix eeprom default constrcutor
 *  
 *  Dependencies:
 *   
 */

#ifndef LFRE_DRIVERS_HPP
#define LFRE_DRIVERS_HPP

#include <stdint.h>

//Base Address of EEPROM memory
#define EEPROM_BASE_ADDR 0x00

//Memory addresses for analog read *variable are volatile because they interact with hardware
#define ADMUX   (*(volatile uint8_t*)0x7C)
#define ADCSRA  (*(volatile uint8_t*)0x7A)
//adc low and high bits
#define ADCL    (*(volatile uint8_t*)0x78)
#define ADCH    (*(volatile uint8_t*)0x79)

#define ADPSO  0
#define ADPS1  1
#define ADPS2  2
#define ADEN   7
#define REFS0  6 
#define ADSC   6

//GPIO  addresses
#define DDRB  ((volatile uint8_t*) 0x24) // Data Direction Register for Port B
#define PORTB ((volatile uint8_t*) 0x25) // Port Output Register for Port B
#define DDRD  ((volatile uint8_t*) 0x2A) // Data Direction Register for Port B
#define PORTD ((volatile uint8_t*) 0x2B) // Port Output Register for Port B

// Base addresses for UART registers
#define UART_BASE           0xC0
#define UBRRH_OFFSET        0x20
#define UBRRL_OFFSET        0x29
#define UCSRA_OFFSET        0x2B
#define UCSRB_OFFSET        0x2A
#define UDR_OFFSET          0x2C

// Bit positions in UCSRA and UCSRB registers
#define RXC_BIT             7
#define TXC_BIT             6
#define UDRE_BIT            5
#define RXEN_BIT            4
#define TXEN_BIT            3
#define UCSZ1_BIT           1
#define UCSZ0_BIT           0


/* 
 *  class that interfaces with the eeprom on the arduino uno 
*/
struct eeprom
{
  public:
    /* default constructor that sets test data in memory
     *  
     * passes write test from main because the function is not working right now
     * 
     * returns nothing
     */
    eeprom(bool write_test)
    {
      //if activated in main file to write to mem
      if(write_test == true)
      {
          int samples = 15;
          //for 1000 psi
          int psi[4] = {600,600,600,600};
          int count = 1;
          int mem_val = 0; 

         //loops through numbers of data points set by user ^^
         for(int i = 0; i < samples;++i)
         {
            //passes pressure and cycles through array of values
            mem_val = this->psi_to_mem(500, (i+1) % 4);
            this->write_(mem_val);
         }
      }
    }
    /*
     * writes to eeprom. to use 10 bit numbers we truncate 16-bit int (default arduino size) into 8 bits
     * i need 0-1000 range but I want to only use 1 byte each reading on the DAQ side so i write a byte  
     * and shift by 2 (4x) on the other. Pros: Good for memory Cons: increments of 4 for testing, 
     * limited write to mem
     * 
     * takes in value
     * 
     * returns nothing * may need to chekc that this function check
     */
    void write_(int val)
    {
      // check if not passed memory val is small enugh and hasnt written too many times (overkill)
      if( (this->head_write + sizeof(uint8_t) <= max_addy) && (uint8_t)val <= 255 && this->num <= 124)
      {
        this->update_p(this->head_write, (uint8_t)val);
        this->num += 1;
      }

      //automatically updates write value and tail
      this->head_write += 0x08;
      this->tail += 0x08;
    }
    
    /*
     * reads byte from memory 
     * 
     * passes nothing
     * 
     * return 10 bit number
     * 
     */
    int read_()
    {
        uint8_t data;
        //checks if in memory not overwrriten and not passed the write value of the current instance
        if(head_read + sizeof(uint8_t) <= max_addy && this->num <= 124 && head_read + sizeof(uint8_t) <= head_write){
          // this can be written i c driver
  
          //just replace this fucntion for the c code
          data = this->read_p(head_read);
        }else{
          //returns 0 and I know that is a problem because min pressure is 13/14/15
          data = 0;
        }
        head_read += 0x08;
        // to use full 0-1023 range we bit shift 8-bit to 10-bit with 4 psi increments
        return (int)(data << 2);
    }
    /*
     * returns number of data_points
     * 
     * passes nothing
     * 
     * returns number of bytes
     * 
     */
    int size_()
    {
      return num;
    }
    /*turn bool if end of read 
     * 
     *passes nothign  
     *
     *return boolean if read is max write address
     */
    bool end_()
    {
      return (this->head_read + 0x08) > this->head_write;
    }

    //TODO:
    //add a function to automate calibrating too !! to get 14.7 stuff
    
    private:
      //base address of eeprom 
      int base = 0x00;
      //max address set by mean actual max is 1024 (make multiple of 4)
      int max_addy = 1020;
      //default address at beginning
      int head_write= 0x00;
      int head_read = 0x00;
      int tail  = 0x00;
      int num = 0;

    /*given psi returned byte to put into mem for psi output in analogread()
     * 
     * passes deisred pressure and numebr of ducer 
     * 
     * return 8-bit value 
    */
    int psi_to_mem(int p,int ducer)
    {
      //determines 1000 or 300 psi ducer and uses formula to convert
      //formula in high level file
      //floor function uses because read/write slows down a lot when value is too big (5us -> 100ms
      //TODO:
      //make this functions scalable (what happens if i add 1,2,3 more ducers?)
      if(ducer == 1 || ducer == 4)
      {
          return floor((p+250.0)/5.0832);
      }
      else if ( ducer == 2 || ducer == 3)
      {
          return floor((p+75.0)/1.525);
      }
    }

    /* Function to read a byte from EEPROM using memory-mapped register
     *  
     *  passes addess
     *  
     *  return byte
     */
    uint8_t read_p(uint16_t address)
    {
        // Cast the address to a pointer and dereference to get the value
        return *((uint8_t*)(EEPROM_BASE_ADDR + address));
    }
    
    /*Function to write a byte to EEPROM using memory-mapped register
     * 
     * passes address and byte
     * 
     * return nothig
     */
    void write_p(uint16_t address, uint8_t value)
    {
        // Cast the address to a pointer and assign the value
        *((uint8_t*)(EEPROM_BASE_ADDR + address)) = value;
    }
    
    /*Function to update EEPROM only if the new value is different
     * 
     * passes address and value
     * 
     * return nothing
     */
    void update_p(uint16_t address, uint8_t newValue)
    {
        uint8_t currentValue = this->read_p(address);
        //compares current value to old value in memory to save write cycles to eeprom
        if (newValue != currentValue) {
            this->write_p(address, newValue);
        }
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * gpio class
 * 
 * 
 */
class GPIODriver {
    public:
        void pinMode_custom(uint8_t pin, uint8_t mode);
        void digitalWrite_custom(uint8_t pin, uint8_t value);
        int  analog_read_ten_bits(int pin);

    private:
        volatile uint8_t* portMode(uint8_t pin);
        volatile uint8_t* portOutput(uint8_t pin);
};
/*
 * sets pin and mode outputs/input
 * low ports are port b 2-13 and port c is upper gpio pins
 * 
 *passes pin number and mode as a character
 *
 *return nothing
 */
void GPIODriver::pinMode_custom(uint8_t pin, uint8_t mode) {
    if (pin >= 2 && pin <= 13) {
        volatile uint8_t* port = portMode(pin);
        if(port){
            if (mode == 1) {
                *port |= (1 << (pin % 8));
            } else {
                *port &= ~(1 << (pin % 8));
            }
        }
    }
}
/*
 * sets port register value to high -> ON and vice versa
 * 
 * passes pin and HIGH or LOW char
 * 
 * returns nothing
 * 
 */
void GPIODriver::digitalWrite_custom(uint8_t pin, uint8_t value) {
    if (pin >= 2 && pin <= 13) {
        volatile uint8_t* port = portOutput(pin);
        if (value == 1) {
            *port |= (1 << (pin % 8));
        } else {
            *port &= ~(1 << (pin % 8));
        }
    }
}
/* sets port to be output or input
 *  
 *  passes pin 
 *  
 *  return  correct register bases in input or output
 */
volatile uint8_t* GPIODriver::portMode(uint8_t pin) {
    if (pin >= 2 && pin <= 7) {
        return DDRD;
    } else if (pin >= 8 && pin <= 13) {
        return DDRB;
    } else {
        return nullptr;
    }
}
/*
 * computes portb or portc for gpio pins we need
 * 
 * passes pin number
 * 
 * return pointer gpio pin  -> is this suppose to be 1 byte or 2 bytes? else nullptr i should check for this in functiin 
 */
volatile uint8_t* GPIODriver::portOutput(uint8_t pin) {
    if (pin >= 2 && pin <= 7) {
        return PORTD;
    } else if (pin >= 8 && pin <= 13) {
        return PORTB;
    } else {
        return nullptr;
    }
}
/* read 10-bit from analog sensor data to int
 *  
 *  passes analog pin
 *  
 *  returns in between 0-1023
 */
int  GPIODriver::analog_read_ten_bits(int pin)
{
    // Set ADC reference to AVcc, right adjust result
    ADMUX |= (1 << REFS0);
    // Enable ADC, set prescaler to 128 (16MHz/128 = 125kHz)
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Set ADC channel
    ADMUX = (ADMUX & 0xF0) | (pin & 0x0F);

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));

    // Read the result from ADCL and ADCH
    uint16_t result = ADCL;
    result |= (ADCH << 8);

    return result;
}
/* serial comm
 *  
 */
class serial_comm{
  public:
      /*default constructor that sets serial comm setup registers
       * 
       * passes baudrat
       * 
       * returns nothing
       */
      serial_comm(int baudrate)
      {
         //sets baudrate into 2 bytes
         UBRRH_REG = (uint8_t)(baudrate >> 8);
         UBRRL_REG =  (uint8_t)(baudrate);
    
         // Set frame format: 8 data bits, 1 stop bit, no parity
         //THIS MAY NEED TO BE CHANGED TO UCSRA OR B
         UCSRA_REG = (1 << UCSZ1_BIT) | (1 << UCSZ0_BIT);
    
         // Enable receiver and transmitter
         UCSRB_REG |= (1 << RXEN_BIT) | (1 << TXEN_BIT);
       }

        /*
         * sends bytes over tx bus 
         * 
         * passes byte to be sent
         * 
         * return nothing
         */
        void sendbyte(uint8_t data)
        {
            // Wait until the transmit buffer is empty
            while (!(UCSRA_REG & (1 << UDRE_BIT)));
  
            // Put the data into the buffer
            UDR_REG = data;
        }
        /*
         * reads byte from rx bus 
         * 
         * passes nothing 
         * 
         * return 1 byte from rx bus
         */
        uint8_t receivebyte()
        {
            // Wait until data is received
            while (!(UCSRA_REG & (1 << RXC_BIT)));
      
            // Return received data
            return UDR_REG;
        }
        /*
         * checks rx buff for data
         * 
         * passes nothign
         * 
         * returns bool
         */
        bool is_rx_buff_empty()
        {
            return (UCSRA_REG & (1 << RXC_BIT)) != 0;
        }
        /*checks tx for data
         * 
         * passes nothing
         * 
         * returns bool
         */
        bool is_tx_buff_empty()
        {
            return (UCSRA_REG & (1 << UDRE_BIT)) != 0;
        }
       
  private:
      //buadrate addrss avariables
      volatile uint8_t   UBRRH_REG = (*(volatile uint8_t *)(UART_BASE + UBRRH_OFFSET));
      volatile uint8_t   UBRRL_REG = (*(volatile uint8_t *)(UART_BASE + UBRRL_OFFSET));
      
      volatile uint8_t   UCSRA_REG = (*(volatile uint8_t *)(UART_BASE + UCSRA_OFFSET));
      volatile uint8_t   UCSRB_REG = (*(volatile uint8_t *)(UART_BASE + UCSRB_OFFSET));
      volatile uint8_t   UDR_REG =   (*(volatile uint8_t *)(UART_BASE + UDR_OFFSET));

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
