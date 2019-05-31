#ifndef ANALOG_H 
#define ANALOG_H

    void ADC_Init(uint8_t channel);    
    int ADC_Read(uint8_t channel);
    int adc_get_last_value();
#endif
