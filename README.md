## EE 474 Class Project - Doctor at Your Fingertip

### Description
There are two main files for this project: SystemControl_Mega.ino is the file that is responsible for control processor and PeripheralSystem_Uno.ino is responsible for peripheral processor.

### Control Processor (Arduino MEGA2560)
Has stronger computing power than Arduino Uno. Has multiple functionalities like:
- Protocol with SSH Connection
- Implemented user interactive GUI to measure, and recognize alert
- Measure temperature, blood pressure (diastolic & systolic pressure), pulse rate, respiration rate, and EKG
- Compute EKG by importing Fast-Fourier Transform the find out the most dominant frequency

### Peripheral Processor (Arduino UNO)
Receptor of sensors. It will constantly read any possible inputs from the control processor and if there are, along with a task information, it will execute the given task and writeback the information. It is capable of:
- Convert unit (From a voltage to temperature using map)
- ISR receptor to detect interrupt to increment/decrement values accordingly from the peripheral (Blood pressure cuff)
- Alert user if the value is out of the range
