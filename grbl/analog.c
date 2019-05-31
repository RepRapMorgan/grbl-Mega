#include "grbl.h"
#include "analog.h"

    // High when a value is ready to be read
    volatile int adc_read_flag;

    // Value to store analog result
    volatile int analog_value;
    ISR(ADC_vect){
      // Done reading
      adc_read_flag = 1;
     
      // Must read low first
      analog_value = ADCL | (ADCH << 8);
     
      // Not needed because free-running mode is enabled.
      // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
      // ADCSRA |= B01000000;
    }

    void ADC_Init(uint8_t channel) {


     ADCSRA = ((1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0)); //ADC conversion clock frequency divider to 64. ADC Clock 16MHz/64 = 250kHz
     ADMUX = (1 << REFS0); //Set Voltage reference to Avcc (5v)
     ADMUX |= channel;
     ADCSRB &= ~((0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0)); //Select free running. 13 ADC clock cycles per converson ADC sample rate 250kHz/13 = 19.23kS/s
     ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADATE); //Turn on ADC, Enable interrupts, enable automatic triggering
     ADCSRA |= (1 << ADSC); //start conversion
      // Kick off the first ADC
      adc_read_flag = 0;
      // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
      ADCSRA |=0xB01000000;
      

    }
    
    int adc_get_last_value() {
        return analog_value;
    }
    
    int ADC_Read(uint8_t channel) {
        int Ain,AinLow;
	
        ADMUX=ADMUX|(channel & 0x0f);	/* Set input channel to read */

        ADCSRA |= (1<<ADSC);		/* Start conversion */
        while((ADCSRA&(1<<ADIF))==0);	/* Monitor end of conversion interrupt */
        
        _delay_us(10);
        AinLow = (int)ADCL;		/* Read lower byte*/
        Ain = (int)ADCH*256;		/* Read higher 2 bits and 
                        Multiply with weight */
        Ain = Ain + AinLow;				
        return(Ain);			/* Return digital value*/
    }