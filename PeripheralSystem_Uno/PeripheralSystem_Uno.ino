//  container for read data

//  EE 474 code on the uno




/* (!!!!!!!   DELETE THIS COMMENT BEFORE TURNING IN !!!!!!!!!!!!!!! )F(G^OPJ)(UB:H)(B"_&V(*^T{(^%*^%(*&^_(*&*^<(*^(*^&^*_)>&)H:^T &(^(*&^(*P)
 *  In this file, 

Implement:   All the modules that should be in UNO
             A TaskDispatch that: 1. receives the data from Mega
                                  2. figure out what task the Mega wants
                                  3. do that task and write data back to Mega
                                 

(*&()*#&$()*#&$(*&()*#&$()#*&$^#^#*&^*N C(*#&(* # &#)(&#)%(&#{)(*&#(*$&#(* #HIUWHDKJHCI@U$Y@(*@ V) @)*&$@)(&$)@*$&@(*$&@)($&@($&@()*$@)($^@)&($*/
#define BASE_TEN_BASE 10





// REMEMBER TO PUT FUNCTION DEF HERE ACCORDING TO CODE STANDARD
void writeBack(char* data, char count);


// Global Variables for Function Call Counter
int tempCount;
int tempFlag;
int tempMultiplier;
int systCount;
int diastCount;
int pulseCount;
int pulseFlag;
int pulseMultiplier;
bool systolicFlag;
bool diastolicFlag;


// Initialize all the values needed for measure()
void setup()
{
  // running on the uno - connect to tx1 and rx1 on the mega and to rx and tx on the uno
  // start serial port at 9600 bps and wait for serial port on the uno to open:
  Serial.begin(9600);
  tempCount = 0;
  systCount = 0;
  diastCount = 0;
  pulseCount = 0;
  systolicFlag = false;
  diastolicFlag = false;
  tempMultiplier = -1;
  pulseMultiplier = -1;
  tempFlag = 0;
  pulseFlag = 0;
  //struct ComputeData cd;
}


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
        }
        writeBack(&returnCharDump, sizeof(char)); 
        break; 
    case 4:                                             // Case 3: Warm if high
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
        switch(subtask){                                // Case 1: tempAlarm
          case 1:  
            returnCharDump = tempHigh(dataIntType);
            break;
          case 2:                                       // Case 2: sysAlarm                 
            returnCharDump = sysHigh(dataIntType);
            break; 
          case 3:                                       // Case 3: diasAlarm
            returnCharDump = diasHigh(dataIntType);
            break; 
          case 4:                                       // Case 4: prAlarm
            returnCharDump = prHigh(dataIntType);
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

unsigned int systolicPress(unsigned int data) {
  //if (systolicFlag == false || dia  
  if(data < 100) {
    systolicFlag = false;
    if(systCount % 2 == 0) {
      data += 3;
    } else {
      data--;
    }
  } else {
    systolicFlag = true;
  }
  systCount++;
  return data;
}

unsigned int diastolicPress(unsigned int data) {
  //if (diastolicFlag == false || sys 
  if(data >40) {
    diastolicFlag = false;
    if(diastCount % 2 == 0) {
      data -= 2;
    } else {
      data++;
    }
  } else {
    diastolicFlag = true;
  }
  diastCount++;
  return data;
}

unsigned int pulseRate(unsigned int data)
{
  if ((data>40 || data<15) && pulseFlag == 1){ // reverse
    pulseMultiplier = -pulseMultiplier;
    pulseFlag = 0;
  }
  if (data<40 && data>15){
    pulseFlag = 1;
  }
  if (pulseCount % 2 == 0){
    data -= 1*pulseMultiplier;
  }else{
    data += 3*pulseMultiplier;
  }
  pulseCount++;
  pulseCount=pulseCount % BASE_TEN_BASE;  // prevent overflow, but not very necessary
  return data; 
}

double tempCorrected( unsigned int data) {
  double dataCorrected = 5 + (0.75 * data);
  return dataCorrected;
}

unsigned int sysCorrected(unsigned int data) {
  unsigned int dataCorrected = 9 + (2 * data);
  return dataCorrected;
}

double diasCorrected( unsigned int data) {
  double dataCorrected = 6 + (1.5 * data);
  return dataCorrected;
}

unsigned int prCorrected(unsigned int data) {
  unsigned int dataCorrected = 8 + (3 * data);
  return dataCorrected;
}

char tempRange(unsigned int data) {                 // Alarm off "0" if in range 
  char result = 1; 
  double dataCorrected = 5 + (0.75 * data);
  if (dataCorrected >= 36.1 && dataCorrected <= 37.8) { 
    result = 0; 
  }
  return result; 
} 

char sysRange(unsigned int data) {                  // Alarm off "0" if in range
  char result = 1;
  unsigned int dataCorrected = 9 + (2 * data); 
  if (dataCorrected <= 120) { 
    result = 0; 
  } 
  return result; 
} 

char diasRange(unsigned int data) {                 // Alarm off "0" if in range
  char result = 1;
  double dataCorrected = 6 + (1.5 * data); 
  if (dataCorrected <= 80) { 
    result = 0; 
  } 
  return result; 
} 

char prRange(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (3 * data);
  if (dataCorrected >= 60 && dataCorrected <= 100) { 
    result = 0; 
  }
  return result; 
} 

char tempHigh(unsigned int data) {                 // Alarm off "0" if in range 
  char result = 1; 
  double dataCorrected = 5 + (0.75 * data);
  if (dataCorrected <= 37.8) { 
    result = 0; 
  }
  return result; 
} 

char sysHigh(unsigned int data) {                  // Alarm off "0" if in range
  char result = 1;
  unsigned int dataCorrected = 9 + (2 * data); 
  if (dataCorrected <= 120) { 
    result = 0; 
  } 
  return result; 
} 

char diasHigh(unsigned int data) {                 // Alarm off "0" if in range
  char result = 1;
  double dataCorrected = 6 + (1.5 * data); 
  if (dataCorrected <= 80) { 
    result = 0; 
  } 
  return result; 
} 

char prHigh(unsigned int data) { 
  char result = 1; 
  unsigned int dataCorrected = 8 + (3 * data);
  if (dataCorrected <= 100) { 
    result = 0; 
  }
  return result; 
} 

unsigned short statusCheck(unsigned short data) { 
  return --data; 
}

void writeBack(char* data, char count){
    for (char i = 0; i<count; i++){
      Serial.write(data[i]);
    }
}




//  end EE 474 code
