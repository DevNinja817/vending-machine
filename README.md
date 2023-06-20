# Vending Machine Embedded System

## Introduction
This project is an embedded system for a vending machine, designed around the PIC16882F microcontroller. This software handles fundamental vending machine operations such as coin input/output, product dispensation, and drink selection.

<img width="340" alt="image" src="https://github.com/regX1/vending-machine/assets/98269264/d1ff4668-9abe-4062-b399-a61e8933ef83">

The inputs to the system are simulated by pushbutton switches (SW0-2) and the potentiometer (VR2) on the sensor board. Pushbuttons are used to simulate inputs required to drive the user interface for drink selection and also to simulate coin insertion. The potentiometer (VR2) will be used to simulate the voltage output of an analogue tilt sensor used for anti-theft detection. 

System outputs are simulated using the LEDs and the LCD display. LEDs are used to simulate control outputs to the drink and coin dispensing mechanisms. The LCD is used to provide instructions and information to the user such as selected drink type, price, current balance and any change due. 

## Requirements
- Hardware: PIC16882F microcontroller and vending machine components
- Software: MPLAB X IDE, XC8 Compiler

## Running the Firmware
To install the firmware, you need to compile the source code using the MPLAB X IDE and XC8 Compiler, and then flash the compiled hex file to the PIC16882F microcontroller. Please refer to the Microchip website for detailed instructions.

## Known Issues
Please note that this version of the software is not currently building successfully due to some unresolved bugs. We are actively working to rectify these issues. Your patience is greatly appreciated.

## Observations and Thoughts
Building an embedded system for a vending machine has been a valuable learning experience. Working with the PIC16882F microcontroller has given me insights into the design and implementation of embedded systems in a real-world application. While I have encountered some challenges, I'm continuously learning and improving the system.

## Time Spent on the Project
Work on this project is ongoing. We are committed to resolving the current build issues and enhancing the overall functionality of the vending machine system.
