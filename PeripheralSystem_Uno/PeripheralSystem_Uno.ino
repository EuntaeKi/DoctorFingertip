#define BASE_TEN_BASE 10
#define PR_PIN_IN 2
#define RR_PIN_IN 2
#define DELAY_TIME 2
#define BP_PIN_IN 3
#define TEMP_INPUT A5
#define EKG_INPUT A4

#include "optfft.h"

// function headers
void setup();
void taskDispatcher(byte task, byte subtask);
void writeBack(char* data, char count);

// For measurement
unsigned int temperature(unsigned int data);
unsigned int systolicPress(unsigned int data);
unsigned int diastolicPress(unsigned int data);
unsigned int pulseRate(unsigned int data);
unsigned int respRate(unsigned int data);
unsigned short statusCheck(unsigned short data);

// For compute
double tempCorrected(unsigned int data);
unsigned int sysCorrected(unsigned int data);
double diasCorrected(unsigned int data);
unsigned int prCorrected(unsigned int data);
unsigned int rrCorrected(unsigned int data);

// For alarm
char tempRange(unsigned int data);
char sysRange(unsigned int data);
char diasRange(unsigned int data);
char prRange(unsigned int data);
char rrRange(unsigned int data);

// For warning
char tempHigh(unsigned int data);
char sysHigh(unsigned int data);
char diasHigh(unsigned int data);
char prHigh(unsigned int data);
char rrHigh(unsigned int data);


// Gloabal Variables for Uno
// Variables for measure and compute functions
int tempCount;
int tempFlag;
int tempMultiplier;
volatile byte PRcount;
volatile byte RRcount;
double BPcount;
int BPFlag;
unsigned int bloodPressureData;
int switchIn = 7; 
unsigned long startTime; 
unsigned int diasMeasure;
unsigned int sysMeasure;
unsigned long BPTimeOut = 0;
unsigned char BPFinished = 0;
unsigned int EKGRawBuffer[256];
unsigned int EKGCount = 0;
signed int real[256];
signed int imag[256];

/******************************************
* Function Name: setup
* Function Inputs: None
* Function Outputs: None
* Function Description: Start the serial port and
*            set the global variables
*           to appropriate values 
*
*
* Author: Matt, Michael, Eun Tae
******************************************/
void setup()
{
  // running on the uno - connect to tx1 and rx1 on the mega and to rx and tx on the uno
  // start serial port at 9600 bps and wait for serial port on the uno to open:
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(RR_PIN_IN), isrRR, RISING);
  detachInterrupt(digitalPinToInterrupt(RR_PIN_IN));
  attachInterrupt(digitalPinToInterrupt(PR_PIN_IN), isrPR, RISING);
  detachInterrupt(digitalPinToInterrupt(PR_PIN_IN));
  attachInterrupt(digitalPinToInterrupt(BP_PIN_IN), isrBP, FALLING);
  detachInterrupt(digitalPinToInterrupt(PR_PIN_IN));
  pinMode(PR_PIN_IN, INPUT_PULLUP); 
  pinMode(RR_PIN_IN, INPUT_PULLUP);
  pinMode(BP_PIN_IN, INPUT_PULLUP);
  pinMode(switchIn, INPUT); 
  BPFlag = 1;
  BPcount = 80.0;
  PRcount = 0; 
  RRcount = 0;
  startTime = 0; 
  tempCount = 0;
  tempMultiplier = -1;
  tempFlag = 0;
  diasMeasure = 0;
  sysMeasure = 0;
}

/******************************************
* Function Name: loop
* Function Inputs: none
* Function Outputs: none
* Function Description: Waits until Mega writes 2 task bytes
*           First read from the Serial will be 
*           the task (Measure,Compute), and second
*           byte will be the type of task.
*           Based on the received byte,
*           dispatch appropriate function.
* Author: Matt, Michael, Eun Tae
******************************************/
void loop()
{
  while(Serial.available()<2)
  {
    // just wait 
  }
  byte task = Serial.read();
  byte subtask = Serial.read();
  taskDispatcher(task, subtask);
}

/******************************************
* Function Name: taskDispatcher
* Function Inputs: bytes respresenting task and subtask
*          respectively
* Function Outputs: None
* Function Description: Based on the inputs,
*             it will read the next necessary
*           data from Serial.
*           Afterward, it will call appropriate
*           function - measure, compute, alarm,
*           warning, status
* Author: Matt, Michael, Eun Tae
******************************************/
void taskDispatcher(byte task,  byte subtask){
  // Measure = 1
  // Compute = 2
  // Alarm   = 3
  // Status  = 4
  unsigned int dataIntType;
  unsigned short dataShortType;
  unsigned int returnIntDump;
  double returnDoubleDump;
  unsigned short returnShortDump;
  char returnCharDump; 
  switch(task) {  
    case 1:                                             // Case 1: Measure the data
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
      switch(subtask){ 
        case 1:                                         // Case 1: temperatureRaw
          returnIntDump = temperature(dataIntType);
          break;
        case 2:                                         // Case 2: systolicPressRaw    
          returnIntDump = bloodPressure(dataIntType);
          break;
        case 3:                                         // Case 3: diastolicCaseRaw
          returnIntDump = bloodPressure(dataIntType);
          break;  
        case 4:                                         // Case 4: pulseRateRaw
          returnIntDump = pulseRate();
          break; 
        case 5:                                         // Case 5: respirationRateRaw
          returnIntDump = respRate();
          break;
        case 6:                                         // Case 6: EKGRaw
          returnIntDump = ekgRecord(dataIntType);
          break;
      }
      writeBack((char*)&returnIntDump, sizeof(unsigned int));
      break; 
    case 2:                                             // Case 2: Compute the data
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
      switch(subtask){                                  // Case 1: tempCorrected
        case 1:  
          returnDoubleDump = tempCorrected(dataIntType);
          writeBack((char*)&returnDoubleDump, sizeof(double));
          break;
        case 2:                                         // Case 2: sysCorrected 
          returnIntDump = sysCorrected(dataIntType); 
          writeBack((char*)&returnIntDump, sizeof(unsigned int));
          break; 
        case 3:                                         // Case 3: diasCorrected
          returnDoubleDump = diasCorrected(dataIntType); 
          writeBack((char*)&returnDoubleDump, sizeof(double)); 
          break; 
        case 4:                                         // Case 4: prCorrected
          returnIntDump = prCorrected(dataIntType); 
          writeBack((char*)&returnIntDump, sizeof(unsigned int));
          break;
        case 5:                                         // Case 5: rrCorrected
          returnIntDump = rrCorrected(dataIntType);
          writeBack((char*)&returnIntDump, sizeof(unsigned int));
          break; 
        case 6:                                         // Case 6: EKGFreqBuf
          returnIntDump = ekgCorrected();
          writeBack((char*)&returnIntDump, sizeof(unsigned int));
          break;
      }
      break;
    case 3:                                             // Case 3: Alarm if out of range
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
        switch(subtask){                                // Case 1: tempAlarm
          case 1:  
            returnCharDump = tempRange(dataIntType);
            break;
          case 2:                                       // Case 2: sysAlarm                 
            returnCharDump = sysRange(dataIntType);
            break; 
          case 3:                                       // Case 3: diasAlarm
            returnCharDump = diasRange(dataIntType);
            break; 
          case 4:                                       // Case 4: prAlarm
            returnCharDump = prRange(dataIntType);
            break; 
          case 5:                                       // Case 5: rrAlarm
            returnCharDump = rrRange(dataIntType);
            break;
        }
        writeBack(&returnCharDump, sizeof(char)); 
        break; 
    case 4:                                             // Case 4: Warning if high
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
        switch(subtask){                                
          case 1:                                       // Case 1: tempWarning
            returnCharDump = tempHigh(dataIntType);
            break;
          case 2:                                       // Case 2: sysWarning              
            returnCharDump = sysHigh(dataIntType);
            break; 
          case 3:                                       // Case 3: diasWarning
            returnCharDump = diasHigh(dataIntType);
            break; 
          case 4:                                       // Case 4: prWarning
            returnCharDump = prHigh(dataIntType);
            break; 
          case 5:                                       // Case 5: rrWarning
            returnCharDump = rrHigh(dataIntType);
            break;
        }
        writeBack(&returnCharDump, sizeof(char)); 
        break;  
     // Case 5: Status of battery
     case 5: 
          Serial.readBytes((char*)&dataShortType, sizeof(unsigned short));
          returnShortDump = statusCheck(dataShortType);
          writeBack((char*)&returnShortDump, sizeof(short)); 
          break;
  }
}

/******************************************
* Function Name: temperature
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the temperature
*           for each function call
*           based on the current value and function
*           call count.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int temperature(unsigned int data)
{
  data = map(analogRead(TEMP_INPUT), 0, 1023, 40, 50);
  return data;
}

void isrBP() 
{
  if (BPFlag == 1) { 
    BPcount = 1.1 * BPcount; 
  } else if (BPFlag == 0) { 
    BPcount = 0.9 * BPcount;  
  }
  unsigned long currTime = millis();
  if (currTime - BPTimeOut >= 7000){
    BPFinished = 1;
  }
}

/******************************************
* Function Name: bloodPressure
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the systolic
*           pressure for each function call
*           based on the current value and function
*           call count.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int bloodPressure(unsigned int data) {
  BPTimeOut = millis();
  attachInterrupt(digitalPinToInterrupt(BP_PIN_IN), isrBP, FALLING);
  BPFlag = digitalRead(switchIn); 
  BPFinished = 0;
  while(BPFinished == 0){
  if(BPcount <= 150 && BPcount >= 110 && !sysMeasure) {
      detachInterrupt(digitalPinToInterrupt(BP_PIN_IN));
      sysMeasure = 1;
      diasMeasure = 0;
      data = (unsigned int)BPcount;
      BPFinished = 1;
  } else if (BPcount <= 80 && BPcount >= 50 && !diasMeasure){
      detachInterrupt(digitalPinToInterrupt(BP_PIN_IN));
      sysMeasure = 0;
      //diasMeasure = 1;
      data = (unsigned int) BPcount;
      BPcount = 80;
      BPFinished = 1;
    }
  }
  return data;
}

/*******************************
 * Function Name:        isrPR
 * Function Inputs:      none
 * Function Outputs:     increment counter
 * Function Description: The interrupt service routine on 
 *                       positive edge. Counts the every beat on 
 *                       pulse rate.
 *                       
 ************************************/
void isrPR() 
{ 
  PRcount++;
}

/******************************************
* Function Name: pulseRate
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the pulse
*           rate for each function call
*           based on the current value and function
*           call count.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int pulseRate()
{
  attachInterrupt(digitalPinToInterrupt(PR_PIN_IN), isrPR, RISING);
  delay(1000 * DELAY_TIME);
  unsigned int pulseRateData = (60000 /(1000 * DELAY_TIME) * (PRcount));
  PRcount = 0;
  detachInterrupt(digitalPinToInterrupt(PR_PIN_IN));
  return pulseRateData;
}

/*******************************
 * Function Name:        isrRR
 * Function Inputs:      none
 * Function Outputs:     increment counter
 * Function Description: The interrupt service routine on 
 *                       positive edge. Counts the every beat on 
 *                       respiration rate.
 *                       
 ************************************/
void isrRR() {
  RRcount++;
}

unsigned int respRate() {
  attachInterrupt(digitalPinToInterrupt(RR_PIN_IN), isrRR, RISING);
  delay(1000 * DELAY_TIME);
  unsigned int respRateData = (60000 /(1000 * DELAY_TIME) * (RRcount)) - 20;
  RRcount = 0;
  detachInterrupt(digitalPinToInterrupt(RR_PIN_IN));
  return respRateData;
}

unsigned int ekgRecord(unsigned int data) {
  data = analogRead(EKG_INPUT);
  if (data == 1023) {
    EKGRawBuffer[EKGCount % 256] = millis();
    EKGCount++;
    return millis();
  } else {
    return ekgRecord(data);
  }
}

/******************************************
* Function Name: tempCorrected
* Function Inputs: Integer of raw data
* Function Outputs: Double of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in Celsius
* Author: Matt, Michael, Eun Tae
******************************************/
double tempCorrected(unsigned int data) {
  double dataCorrected = 5 + (0.75 * data);
  return dataCorrected;
}

/******************************************
* Function Name: sysCorrected
* Function Inputs: Integer of raw data
* Function Outputs: Double of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int sysCorrected(unsigned int data) {
  unsigned int dataCorrected = 9 + (2 * data);
  return dataCorrected;
}

/******************************************
* Function Name: diasCorrected
* Function Inputs: Integer of raw data
* Function Outputs: Double of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
double diasCorrected(unsigned int data) {
  double dataCorrected = 6 + (1.5 * data);
  return dataCorrected;
}

/******************************************
* Function Name: prCorrected
* Function Inputs: Integer of raw data
* Function Outputs: int of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in BPM
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int prCorrected(unsigned int data) {
  unsigned int dataCorrected = 8 + (3 * data);
  return dataCorrected;
}

/******************************************
* Function Name: rrCorrected
* Function Inputs: Integer of raw data
* Function Outputs: int of corrected data
* Function Description: perform a conversion of
*           data from measured data to
*           computed data in breath per minute
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int rrCorrected(unsigned int data) {
  unsigned int dataCorrected = 7 + (3 * data);
  return dataCorrected;
}

unsigned int ekgCorrected() {
  for (int k = 0; k < 256; k++) {
    for (int n = 0; n < 256; n++) {
      float theta = ((n*k*2.0)*(M_PI)/(float)(256));
      int r = EKGRawBuffer[n];
      real[k] += r*cos(theta);
      imag[k] -= r*sin(theta);
    }
  }
  unsigned int dataCorrected = optfft((int*)real, (int*)imag);
  return dataCorrected;
}

/******************************************
* Function Name: tempRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input temperature
*           is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char tempRange(unsigned int data) {                 
  char result = 1; 
  double dataCorrected = 5 + (0.75 * data);
  if (dataCorrected >= 36.1 && dataCorrected <= 37.8) { 
    result = 0; 
  }
  if (dataCorrected < 34.295 || dataCorrected > 39.69){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: sysRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input systolic
*           pressure is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char sysRange(unsigned int data) {                  
  char result = 1;
  unsigned int dataCorrected = 9 + (2 * data); 
  if (dataCorrected >= 120 && dataCorrected <=130) { 
    result = 0; 
  } 
  if (dataCorrected <  114 || dataCorrected > 136.5){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: diasRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input diastolic
*           pressure is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char diasRange(unsigned int data) {                 
  char result = 1;
  double dataCorrected = 6 + (1.5 * data); 
  if (dataCorrected >= 70 && dataCorrected <=80) { 
    result = 0; 
  } 
  if (dataCorrected < 66.5 || dataCorrected > 84){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: prRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input pulse rate
*           is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char prRange(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (3 * data);
  if (dataCorrected >= 60 && dataCorrected <= 100) { 
    result = 0; 
  }
  if (dataCorrected < 57 || dataCorrected > 105){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: rrRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input breath per minute data
*           is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char rrRange(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 7 + (3 * data);
  if (dataCorrected >= 12 && dataCorrected <= 25) { 
    result = 0; 
  }
  if (dataCorrected < 11.4 || dataCorrected > 26.25){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: rrRange
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input breath per minute data
*           is within the range of normal
* Author: Matt, Michael, Eun Tae
******************************************/
char ekgRange(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 7 + (3 * data);
  if (dataCorrected >= 12 && dataCorrected <= 25) { 
    result = 0; 
  }
  if (dataCorrected < 11.4 || dataCorrected > 26.25){
    result = 2;
  }
  return result; 
} 

/******************************************
* Function Name: tempHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input temperature
*             is above 37.8 Celsius
* Author: Matt, Michael, Eun Tae
******************************************/
char tempHigh(unsigned int data) {                  
  char result = 1; 
  double dataCorrected = 5 + (0.75 * data);
  if (dataCorrected <= 37.8) { 
    result = 0; 
  }
  return result; 
} 

/******************************************
* Function Name: sysHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input systolic
*             pressure is above 120 mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
char sysHigh(unsigned int data) {                  
  char result = 1;
  unsigned int dataCorrected = 9 + (2 * data); 
  if (dataCorrected <= 156) { 
    result = 0; 
  } 
  return result; 
} 

/******************************************
* Function Name: diasHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input diastolic
*             pressure is above 80 mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
char diasHigh(unsigned int data) {                 
  char result = 1;
  double dataCorrected = 6 + (1.5 * data); 
  if (dataCorrected <= 80) { 
    result = 0; 
  } 
  return result; 
} 

/******************************************
* Function Name: prHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input pulse
*             rate is above 100 BPM
* Author: Matt, Michael, Eun Tae
******************************************/
char prHigh(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (3 * data);
  if (dataCorrected <= 100) { 
    result = 0; 
  }
  return result; 
} 

/******************************************
* Function Name: rrHigh
* Function Inputs: Integer of raw data
* Function Outputs: Character, which behaves like boolean
* Function Description: Checks whether given input data
*             is above 25 breath per minute
* Author: Matt, Michael, Eun Tae
******************************************/
char rrHigh(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (7 * data);
  if (dataCorrected <= 100) { 
    result = 0; 
  }
  return result; 
} 

/******************************************
* Function Name: statusCheck
* Function Inputs: unsigned short data
* Function Outputs: unsinged short noting battery status
* Function Description: Checks whether given input systolic
*             pressure is above 120 mmHg
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned short statusCheck(unsigned short data) { 
  return --data; 
}

/******************************************
* Function Name: writeBack
* Function Inputs: string of data and char, count
* Function Outputs: none
* Function Description: writes the array of data
            input into Serial for Mega
            to take in.
* Author: Matt, Michael, Eun Tae
******************************************/
void writeBack(char* data, char count){
    for (char i = 0; i < count; i++){
      Serial.write(data[i]);
    }
}

//  end of EE 474 code
