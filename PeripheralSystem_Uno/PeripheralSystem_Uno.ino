//  container for read data

//  EE 474 code on the uno




/* (!!!!!!!   DELETE THIS COMMENT BEFORE TURNING IN !!!!!!!!!!!!!!! )F(G^OPJ)(UB:H)(B"_&V(*^T{(^%*^%(*&^_(*&*^<(*^(*^&^*_)>&)H:^T &(^(*&^(*P)
 *  In this file, 

Implement:   All the modules that should be in UNO
             A TaskDispatch that: 1. receives the data from Mega
                                  2. figure out what task the Mega wants
                                  3. do that task and write data back to Mega
                                 

(*&()*#&$()*#&$(*&()*#&$()#*&$^#^#*&^*N C(*#&(* # &#)(&#)%(&#{)(*&#(*$&#(* #HIUWHDKJHCI@U$Y@(*@ V) @)*&$@)(&$)@*$&@(*$&@)($&@($&@()*$@)($^@)&($*/

// Global Variables for Function Call Counter
int tempCount;
int systCount;
int diastCount;
int pulseCount;
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
  switch(task) {  
    case 1:                                             // Case 1: Measure the data
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
      switch(subtask){ 
        case 1:                                         // Case 1: temperatureRaw
          Serial.write(temperature(dataIntType));
          break;
        case 2:                                         // Case 2: systolicPressRaw    
          Serial.write(systolicPress(dataIntType)); 
          break;    
        case 3:                                         // Case 3: diastolicCaseRaw
          Serial.write(diastolicPress(dataIntType)); 
          break;  
        case 4:                                         // Case 4: pulseRateRaw
          Serial.write(pulseRate(dataIntType)); 
          break; 
      }
      break; 
    case 2:                                             // Case 2: Compute the data
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
      switch(subtask){                                  // Case 1: tempCorrected
        case 1:  
          Serial.write(tempCorrected(dataIntType)); 
          break;
        case 2:                                         // Case 2: sysCorrected 
          Serial.write(sysCorrected(dataIntType)); 
          break; 
        case 3:                                         // Case 3: diasCorrected
          Serial.write(diasCorrected(dataIntType)); 
          break; 
        case 4:                                         // Case 4: prCorrected
          Serial.write(prCorrected(dataIntType)); 
          break; 
      }
        break;
    case 3:                                             // Case 3: Alarm if out of range
      Serial.readBytes((char*)&dataIntType, sizeof(unsigned int));
        switch(subtask){                                // Case 1: tempAlarm
          case 1:  
            Serial.write(tempRange(dataIntType)); 
            break;
          case 2:                                       // Case 2: sysAlarm                 
            Serial.write(sysRange(dataIntType));
            break; 
          case 3:                                       // Case 3: diasAlarm
            Serial.write(diasRange(dataIntType));
            break; 
          case 4:                                       // Case 4: prAlarm
            Serial.write(prRange(dataIntType));
            break; 
        }
        break; 

        // Case 4: Status of battery
        case 4: 
          Serial.readBytes((char*)&dataShortType, sizeof(unsigned short));
          Serial.write(statusCheck(dataShortType)); 
          break;
  }
}

unsigned int temperature(unsigned int data)
{
  bool flag = true;
  if(data < 50 && flag) {
    if(data > 50) {
      flag = false;
    }
    if(tempCount % 2 == 0) {
      data += 2;
    } else {
      data--;
    }
  } else if (data > 15 && !flag){
    if(data < 15) {
      flag = true;
    }
    if(tempCount % 2 == 0) {
      data--;
    } else {
      data += 2;
    }
  }
  tempCount++;
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
  if(data < 100) {
    diastolicFlag = false;
    if(diastCount % 2 == 0) {
      data += 3;
    } else {
      data--;
    }
  } else {
    diastolicFlag = true;
  }
  diastCount++;
  return data;
}

unsigned int pulseRate(unsigned int data)
{
  bool flag = true;
  if(data <= 40 && flag) {
    if(data > 40) {
      flag = false;
    }
    if(tempCount % 2 == 0) {
      data--;
    } else {
      data += 3;
    }
  } else if (data > 15 && !flag){
    if(data < 15) {
      flag = true;
    }
    if(tempCount % 2 == 0) {
      data += 3;
    } else {
      data--;
    }
  }
  return data;
}

unsigned int tempCorrected(unsigned int data) {
  unsigned int dataCorrected = 5 + (0.75 * data);
  return data;
}

unsigned int sysCorrected(unsigned int data) {
  unsigned int dataCorrected = 9 + (2 * data);
  return data;
}

unsigned int diasCorrected(unsigned int data) {
  unsigned int dataCorrected = 6 + (1.5 * data);
  return data;
}

unsigned int prCorrected(unsigned int data) {
  unsigned int dataCorrected = 8 + (3 * data);
  return data;
}

char tempRange(unsigned int data) {                 // Alarm off "0" if in range 
  char result = 1; 
  if (data >= 36.1 && data <= 37.8) { 
    result = 0; 
  }
  return result; 
} 

char sysRange(unsigned int data) {                  // Alarm off "0" if in range
  char result = 1; 
  if (data == 120) { 
    result = 0; 
  } 
  return result; 
} 

char diasRange(unsigned int data) {                 // Alarm off "0" if in range
  char result = 1; 
  if (data == 80) { 
    result = 0; 
  } 
  return result; 
} 

char prRange(unsigned int data) { 
  char result = 1; 
  if (data >= 60 && data <= 100) { 
    result = 0; 
  }
  return result; 
} 

unsigned short statusCheck(unsigned short data) { 
  return data--; 
}

//  end EE 474 code
