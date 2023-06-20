// Author      : Callum Haigh
// Version:    : 2.4
// Finish Date : 28/04/23
// Filename    : main.c
// Purpose     : Functionality of an operational Drinks Vending Machine for the PIC16F882 Microcontroller

// PIC16F882 Configuration Bit Settings
#pragma config FOSC = INTRC_NOCLKOUT // Oscillator Selection bits
#pragma config WDTE = OFF            // Watchdog Timer Enable bit
#pragma config PWRTE = OFF           // Power-up Timer Enable bit
#pragma config MCLRE = ON            // RE3/MCLR pin function select bit
#pragma config CP = OFF              // Code Protection bit
#pragma config CPD = OFF             // Data Code Protection bit
#pragma config BOREN = OFF           // Brown Out Reset Selection bits
#pragma config IESO = OFF            // Internal External Switchover bit
#pragma config FCMEN = OFF           // Fail-Safe Clock Monitor Enabled bit
#pragma config LVP = OFF             // Low Voltage Programming Enable bit
#pragma config BOR4V = BOR40V        // Brown-out Reset Selection bit
#pragma config WRT = OFF             // Flash Program Memory Self Write Enable bits

#define _XTAL_FREQ 4000000 // MCU clock speed - required for delay macros

// Pins for the drink and coin buttons and the alarm
#define DISPENSE_DRINK RA0
#define DISPENSE_CHANGE RA1
#define ALARM RA2

// Pins for the pushbuttons and the tilt sensor
#define NEXT_DRINK RB0
#define SELECT_DRINK RB1
#define INSERT_10P RB0
#define INSERT_20P RB1
#define INSERT_50P RB2
#define TILT_SENSOR 9u // AN9 corresponds to ADC channel 9

#include <xc.h>
#include "LCDdrive882.h"

// Function Prototypes
void coin_insertion_mode();
void dispense_drink_mode();
void dispense_change_mode();
void drink_ready_mode();
void reset_vending_machine();
void alarm_mode();

// Global variables
int selected_drink = 0;
int required_balance = 0;
int inserted_balance = 0;
int alarm_trigger = 0;

int cursorVisible = 0; // We need to keep track of cursors visibility

void __interrupt(high_priority) ISR() {
    // Timer2 interrupt
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0; // Clear the interrupt flag

        int tilt_voltage = ADC_read(TILT_SENSOR);
        if (tilt_voltage > 200) // If the tilt sensor voltage exceeds 2 volts
        {
            LCD_clear();
            LCD_write("ALARM - MACHINE TILTED", 1);
            ALARM = 1; // Turn on the alarm
            // Flash the cursor on and off
            if (cursorVisible) {
                LCD_cursor_off();
                cursorVisible = 0;
            } else {
                LCD_cursor_on();
                cursorVisible = 1;
            }
        } else {
            ALARM = 0; // Turn the alarm off
            LCD_cursor_off(); // Turn off the cursor when the machine is not tilted
            cursorVisible = 0;
        }
    }

    // PORTB interrupt-on-change
    if (INTCONbits.RBIF) {
        INTCONbits.RBIF = 0; // Clear the interrupt flag

        if (INSERT_10P) // 10p coin inserted
        {
            inserted_balance += 10;
        } else if (INSERT_20P) // 20p coin inserted
        {
            inserted_balance += 20;
        } else if (INSERT_50P) // 50p coin inserted
        {
            inserted_balance += 50;
        }

        // Display remaining coins to be inserted
        LCD_clear();
        LCD_write("Remaining coins to be inserted: ", 1);
        LCD_display_value(required_balance - inserted_balance);

        // Clear button presses
        INSERT_10P = INSERT_20P = INSERT_50P = 0;
    }
}

void main(void) {
    // Initialize LCD module
    LCD_initialise();

    ANSEL = 0x00; // Used to configure the analog input pins of the microcontroller.
    ANSELH = 0x00; 
    TRISA = 0x00; // Used to configure whether each pin is an input or output and in our case they are all being set to outputs
    PORTA = 0x00; // Clears all output pins on Port A.
    PR2 = 124; // Sets the Timer2 period to 125 (124 + 1) clock cycles.
    TMR2 = 0; // Resets the Timer2 count to 0.
    T2CON = 0x4E; 

    while (1) {
        LCD_clear();
        // Display drink options
        switch (selected_drink) {
            case 0:
                LCD_write("Select Drink:", 1); // The "1" is passed to the function to tell it to display the string as data instead of a command
                LCD_write("Cola 80p", 1);
                required_balance = 80;
                break;
            case 1:
                LCD_write("Select Drink:", 1);
                LCD_write("Lemonade: 80p", 1);
                required_balance = 80;
                break;
            case 2:
                LCD_write("Select Drink:", 1);
                LCD_write("Orange Juice: 60p", 1);
                required_balance = 60;
                break;
            case 3:
                LCD_write("Select Drink:", 1);
                LCD_write("Water: 50p", 1);
                required_balance = 50;
                break;
        }

        while (1) {
            // Allow user to cycle through drink options
            if (NEXT_DRINK) {
                selected_drink = (selected_drink + 1) % 4;
                break;
            }

            // Execute vending machine functions when "Select Button" is pressed in this order
            if (SELECT_DRINK) {
                coin_insertion_mode();
                dispense_drink_mode();
                dispense_change_mode();
                drink_ready_mode();
                break;
            }

            __delay_ms(100);
        }
    }
}

void coin_insertion_mode() {
    LCD_clear();
    LCD_write("Insert coins:", 1);
    LCD_display_value(required_balance);

    // Enable the PORTB interrupt-on-change
    INTCONbits.RBIE = 1;

    while (inserted_balance < required_balance) {
        // Wait for the interrupt to be triggered by the user
        __delay_ms(100);
    }

    // Disable the PORTB interrupt-on-change
    INTCONbits.RBIE = 0;
}

void update_progress_bar(int progress) {
    LCD_cursor(0, 1); // Move cursor to second row
    LCD_write("[", 1); // Start of progress bar
    for (int i = 0; i < 10; i++) {
        if (i < progress) {
            LCD_write("-", 1); // Filled part of progress bar
        } else {
            LCD_write(" ", 1); // Empty part of progress bar
        }
    }
    LCD_write("]", 1); // End of progress bar
}

void dispense_drink_mode() {
    LCD_clear();
    LCD_write("Dispensing drink", 1);
    DISPENSE_DRINK = 1;

    // Set up Timer2 to trigger an interrupt every 100ms
    T2CONbits.T2CKPS = 0b01; // Prescaler 1:4. The timer will increment every 4 clock cycles of the microcontroller.
    T2CONbits.TMR2ON = 1; // Timer2 on
    PR2 = 249; // Set Timer2 period to 100ms
    PIE1bits.TMR2IE = 1; // Enable Timer2 overflow interrupt

    int progress = 0;
    while (DISPENSE_DRINK) {
        // Wait for the interrupt to be triggered by Timer2
        __delay_ms(100);

        // Update progress bar
        update_progress_bar(progress);

        // Update progress counter
        progress++;
        if (progress > 10) {
            progress = 10;
        }
    }

    T2CONbits.TMR2ON = 0; // Turn off Timer2
    PIE1bits.TMR2IE = 0; // Disable Timer2 overflow interrupt
    DISPENSE_DRINK = 0;
}

void dispense_change_mode() {
    int change = inserted_balance - required_balance;
    if (change <= 0)
        return;

    LCD_clear();
    LCD_write("Dispensing change", 1);
    DISPENSE_CHANGE = 1;
    int remaining_change = (int) change;

    for (int i = 7; i >= 3; i--) {
        int coin_value = 1 << i;
        int coins_to_dispense = remaining_change / coin_value;
        int remaining_change_inner = -(coins_to_dispense * coin_value); // Rename variable to avoid shadowing

        if (coins_to_dispense > 0) {
            LCD_cursor(1, 0);
            LCD_write("Dispensing: ", 1);
            LCD_putch((unsigned char) ((unsigned int) coins_to_dispense + '0'));
            switch (coin_value) {
                case 10:
                    LCD_write("x 10p", 1);
                    break;
                case 20:
                    LCD_write("x 20p", 1);
                    break;
                case 50:
                    LCD_write("x 50p", 1);
                    break;
            }

            // Dispense coins using interrupts
            for (int j = 0; j < coins_to_dispense; j++) {
                T2CONbits.T2CKPS = 0b10; // Prescaler 1:16
                T2CONbits.TMR2ON = 1; // Timer2 on
                PR2 = 250; // Set Timer2 period to 10ms
                PIE1bits.TMR2IE = 1; // Enable Timer2 overflow interrupt

                while (DISPENSE_CHANGE)
                    __delay_ms(100);

                T2CONbits.TMR2ON = 0; // Turn off Timer2
                PIE1bits.TMR2IE = 0; // Disable Timer2 overflow interrupt
                DISPENSE_CHANGE = 0;
            }

            remaining_change = remaining_change_inner; // Assign the new value to the outer variable
        }
    }

    // Dispense any remaining 1p coins
    for (int i = 0; i < remaining_change; i++) {
        LCD_cursor(1, 0);
        LCD_write("Dispensing: ", 1);
        LCD_write("1 x 1p", 1);

        // Dispense coins using interrupts
        T2CONbits.T2CKPS = 0b10; // Prescaler 1:16
        T2CONbits.TMR2ON = 1; // Timer2 on
        PR2 = 250; // Set Timer2 period to 10ms
        PIE1bits.TMR2IE = 1; // Enable Timer2 overflow interrupt

        while (DISPENSE_CHANGE)
            __delay_ms(100);

        T2CONbits.TMR2ON = 0; // Turn off Timer2
        PIE1bits.TMR2IE = 0; // Disable Timer2 overflow interrupt
        DISPENSE_CHANGE = 1;
    }

    DISPENSE_CHANGE = 0;
}

void drink_ready_mode() {
    LCD_clear();
    LCD_write("Drink ready! Thank you for shopping with us", 1);

    // Set up Timer1 to trigger an interrupt every 1 second
    T1CONbits.T1CKPS = 0b11; // 1:8 prescaler
    T1CONbits.TMR1ON = 1; // Timer1 on
    TMR1H = 0xFC; // Set Timer1 period to 1 second
    TMR1L = 0x18;
    PIE1bits.TMR1IE = 1; // Enable Timer1 overflow interrupt

    while (1) {
        // Wait for the interrupt to be triggered by Timer1
        __delay_ms(1000);
    }
    reset_vending_machine();
    return;
}

void alarm_mode() {
    LCD_clear();
    LCD_write("Alarm triggered", 1);
    ALARM = 1;

    while (1) {
        // Read the voltage from the tilt sensor
        int tilt_voltage = ADC_read(TILT_SENSOR);

        // If the voltage exceeds 2V, keep the alarm on and display a warning message
        if (tilt_voltage > 200) {
            LCD_clear();
            LCD_write("ALARM - THE MACHINE HAS BEEN TILTED!", 1);
        } else // If the voltage drops below 2V, turn off the alarm and exit the function
        {
            ALARM = 0;
            return;
        }

        // Poll the tilt sensor voltage at a rate of 2 Hz
        __delay_ms(500);
    }
}

void reset_vending_machine() {
    // Reset all global variables to their initial values
    selected_drink = 0;
    required_balance = 0;
    inserted_balance = 0;
    alarm_trigger = 0;

    // Clear the LCD display and turn off all outputs
    LCD_clear();
    DISPENSE_DRINK = 0;
    DISPENSE_CHANGE = 0;
    ALARM = 0;

    // Disable all interrupts
    INTCONbits.PEIE = 0;
    INTCONbits.GIE = 0;
    return;
}