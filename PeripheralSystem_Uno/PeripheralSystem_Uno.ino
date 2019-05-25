#define BASE_TEN_BASE 10
#define PRpinIn 2
#define RRpinIn 3
#define delayTime 3

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
int systCount;
int diastCount;
bool systolicFlag;
bool diastolicFlag;
bool systoInitialized;
bool diasInitialized;
unsigned int systoInitial;
unsigned int diastoInitial;
volatile byte PRcounter;
volatile byte RRcounter;
unsigned long passedTime; 
unsigned int pulseRateData;
unsigned int respRateData; 


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
  attachInterrupt(digitalPinToInterrupt(RRpinIn), isrRR, RISING);
  detachInterrupt(digitalPinToInterrupt(RRpinIn));
  attachInterrupt(digitalPinToInterrupt(PRpinIn), isrPR, RISING);
  detachInterrupt(digitalPinToInterrupt(PRpinIn));
  pinMode(PRpinIn, INPUT_PULLUP); 
  pinMode(RRpinIn, INPUT_PULLUP);
  PRcounter = 0; 
  RRcounter = 0;
  pulseRateData = 0;
  respRateData = 0; 
  passedTime = 0; 
  tempCount = 0;
  systCount = 0;
  diastCount = 0;
  systolicFlag = false;
  diastolicFlag = false;
  tempMultiplier = -1;
  tempFlag = 0;
  systoInitialized = 0;
  diasInitialized = 0;
  systoInitial = 0;
  diastoInitial = 0;
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
          returnIntDump = systolicPress(dataIntType);
          break;    
        case 3:                                         // Case 3: diastolicCaseRaw
          returnIntDump = diastolicPress(dataIntType);
          break;  
        case 4:                                         // Case 4: pulseRateRaw
          returnIntDump = pulseRate(dataIntType);
          break; 
        case 5:                                         // Case 5: respirationRateRaw
          returnIntDump = pulseRate(dataIntType);
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
  // What should happen when it is over 50. it should start ticking down right?

  // start with going up. Hit 50->reverse Hit15->reverse
  if ((data>50 || data<15) && tempFlag == 1){ // reverse
    tempMultiplier = -tempMultiplier;
    tempFlag = 0;
  }
  if ( data < 50 && data > 15){
    tempFlag = 1;
  }
  if (tempCount % 2 == 0){
    data += 2*tempMultiplier;
  }else{
    data -= 1*tempMultiplier;
  }
  tempCount++;
  tempCount=tempCount % BASE_TEN_BASE;  // prevent overflow, but not very necessary
  return data; 
}

/******************************************
* Function Name: systolicPress
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the systolic
*           pressure for each function call
*           based on the current value and function
*           call count.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int systolicPress(unsigned int data) {
  if (systoInitialized == 0){
    systoInitial = data;
    systoInitialized = 1; 
  }
  if(data > 100) {
    data = systoInitial;
  }
  systolicFlag = false;
  if(systCount % 2 == 0) {
    data += 3;
  } else {
    data--;
  }
  systCount++;
  return data;
}

/******************************************
* Function Name: diastolicPress
* Function Inputs: Integer of raw data
* Function Outputs: Integer of processed data
* Function Description: Increase or decrease the diastolic
*           pressure for each function call
*           based on the current value and function
*           call count.
* Author: Matt, Michael, Eun Tae
******************************************/
unsigned int diastolicPress(unsigned int data) {
  //if (diastolicFlag == false || sys 
  if (diasInitialized == 0){
    diastoInitial = data;
    diasInitialized = 1; 
  }
  if(data < 40) {
    data = diastoInitial;
  }
  diastolicFlag = false;
  if(diastCount % 2 == 0) {
    data -= 2;
  } else {
    data++;
  }
  diastCount++;
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
  PRcounter++; 
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
unsigned int pulseRate(unsigned int data)
{
  attachInterrupt(digitalPinToInterrupt(PRpinIn), isrPR, RISING);
  delay(1000 * delayTime);
  pulseRateData = (60000 /(1000 * delayTime) * (PRcounter)) - 20;
  PRcounter = 0;
  detachInterrupt(digitalPinToInterrupt(PRpinIn));
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
  RRcounter++;
}

unsigned int respRate(unsigned int data) {
  attachInterrupt(digitalPinToInterrupt(RRpinIn), isrRR, RISING);
  delay(1000 * delayTime);
  pulseRateData = (60000 /(1000 * delayTime) * (RRcounter)) - 20;
  RRcounter = 0;
  detachInterrupt(digitalPinToInterrupt(RRpinIn));
  return respRateData;
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
