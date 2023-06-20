// Filename: ADClib882.h
// Version: 1.0
// Date: <Insert current date>
// Author: <Insert your name>
// Purpose: Provides functions to easily use PIC16F882 ADC
#define POT 9 // Define channel for POT on tutorial board (AN9)
#define EXT 11 // Define channel for EXT ADC on tutorial board (AN11)
//Function Prototypes
void ADC_initialise(void);
unsigned int ADC_read(void);
void ADC_channel_select(unsigned char channel);