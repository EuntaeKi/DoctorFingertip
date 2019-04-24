/* (!!!!!!!   DELETE THIS COMMENT BEFORE TURNING IN !!!!!!!!!!!!!!! )F(G^OPJ)(UB:H)(B"_&V(*^T{(^%*^%(*&^_(*&*^<(*^(*^&^*_)>&)H:^T &(^(*&^(*P)
 *  In this file, 

Implement:   All the modules that should be in MEGA (Tasks, data, scheduler)
             The entire display portion
             A TaskDispatch that: 1. sends to Uno
                                  2. and receive the data
                                 

(*&()*#&$()*#&$(*&()*#&$()#*&$^#^#*&^*N C(*#&(* # &#)(&#)%(&#{)(*&#(*$&#(* #HIUWHDKJHCI@U$Y@(*@ V) @)*&$@)(&$)@*$&@(*$&@)($&@($&@()*$@)($^@)&($*/
#define TASK_TOTAL_COUNT 6;
#define MEASURE_TASK 1;
#define TEMP_RAW_SUBTASK 1;
#define SYSTO_RAW_SUBTASK 2;
#define DIASTO_RAW_SUBTASK 3;
#define PULSE_RAW_SUBTASK 4;


//MeasureData Struct
typedef struct
{
  unsigned int* tempRawPtr;
  unsigned int* systoRawPtr;
  unsigned int* diastoRawPtr;
  unsigned int* pulseRawPtr;
} MeasureData; 
 
 //ComputeData Struct
 
typedef struct
{
  unsigned int* tempRawPtr;
  unsigned int* systoRawPtr;
  unsigned int* diastoRawPtr;
  unsigned int* pulseRawPtr;
  unsigned char** tempCorrected;
  unsigned char** sysPressCorrected;
  unsigned char** diasCorrected;
  unsigned char** prCorrected;
  
} ComputeData; 

 //DisplayData
typedef struct
{
  unsigned char** tempCorrected;
  unsigned char** sysPressCorrected;
  unsigned char** diasCorrected;
  unsigned char** prCorrected;
  unsigned short* batteryState;
} DisplayData;

 // WarningAlarmData
typedef struct
{
  unsigned int* tempRawPtr;
  unsigned int* systoRawPtr;
  unsigned int* diastoRawPtr;
  unsigned int* pulseRawPtr;
  unsigned short* batteryState;
  unsigned char* bpOutOfRange;
  unsigned char* tempOutOfRange;
  unsigned char* pulseOutOfRange;
  Bool* bpHigh;
  Bool* tempHigh;
  Bool* pulseLow;


  unsigned char* bpOutOfRange;

} WarningAlarmData;

// StatusData
typedef struct
{
  unsigned short* batteryState;
   
} StatusData;

 // SchedulerData
typedef struct
{
  TCB** tasks[];
  unsigned int taskCount;
} SchedulerData;

typedef struct
{
   void (*myTask)(void*);
   void* taskDataPtr;
} TCB;
 
enum _myBool { FALSE = 0, TRUE = 1 };
typedef enum _myBool Bool;





// function headers
// major functions
void initialization();
void scheduleTask(void* data);
// task functions
void measureTask(void* data);
void computeTask(void* data);
void displayTask(void* data);
void statusTask(void* data);
void alarmsAndWarningTask(void* data);
// Intrasystem Communication functions
void requestAndReceive(char* data);
// TCB executer functions
void executeTCB(TCB* taskControlBlock);






// Global Variables for Measurements
unsigned int temperatureRaw = 75;
unsigned int systolicPressRaw = 80;
unsigned int diastolicPressRaw = 80;
unsigned int pulseRateRaw = 50;
// Global Variables for Display
unsigned char* tempCorrected = NULL;
unsigned char* systolicPressCorrected = NULL;
unsigned char* diastolicPressCorrected = NULL;
unsigned char* pulseRateCorrected = NULL;
// Global Variables for Status
unsigned short batteryState = 200;
// Global Variables for Alarm
unsigned char bpOutOfRange = 0;
unsigned char tempOutofRange = 0;
unsigned char pulseOutOfRange = 0;
//Global Variables for Warning
Bool bpHigh = FALSE;
Bool tempHigh = FALSE;
Bool pulseLow = FALSE;


/******************************************
* function name: setup
* function inputs: None
* function outputs: None
* function description: This is the function
*                       for arduino to initializa
*                       the processor
* author: Matt & Sabrina
******************************************/ 
void setup()
{
  // initialize the communication between Mega and Uno
  Serial1.begin(9600);
  Serial.begin(9600);
  return;
}

/******************************************
* function name: loop
* function inputs: None
* function outputs: None
* function description: This is the function
*                       our Mega will run forever
* author: Matt & Sabrina
******************************************/ 
void loop()
{
  // EXAMPLE FOR SENDING MESSAGE TO UNO
  //  write a two bytes to UNO, receive it in reverse
  Serial.println("Sending 13,35");
  byte a = 13;
  byte b = 35;
  Serial1.write(a);
  Serial1.write(b);
  // now wait for the replies
  while(Serial1.available()<2)
  {
    // just wait
  }
  byte recv1 = Serial1.read();
  byte recv2 = Serial1.read();
  Serial.print("Received :");
  Serial.print(recv1);
  Serial.print(", ");
  Serial.println(recv2);
  delay(1000);
  return;
}

/******************************************
* function name: initialize
* function inputs: None
* function outputs: None
* function description: This function will prepare
*                       all the tasks and then start
*                       the scheduler
* author: Matt & Sabrina
******************************************/ 
void initialize(){
   // Prepare for each task Each Tasks
   // 1. Measure
   // data:
   MeasureData measureData;
   measureData.tempRawPtr = &temperatureRaw;
   measureData.systoRawPtr = &systolicPressRaw;
   measureData.diastoRawPtr = &diastolicPressRaw;
   measureData.pulseRawPtr = &pulseRateRaw;
   // TCB:
   TCB measureTaskControlBlock;
   measureTaskControlBlock.mytask = measureTask;
   measureTaskControlBlock.taskDataPtr = (void*)&measureData;
   // 2. Compute
   // data:
   ComputeData computeData;
   computeData.tempRawPtr = &temperatureRaw;
   computeData.systoRawPtr = &systolicPressRaw;
   computeData.diastoRawPtr = &diastolicPressRaw;
   computeData.pulseRawPtr = &pulseRateRaw;
   computeData.tempCorrected = &tempCorrected;
   computeData.sysPressCorrected = &systolicPressCorrected;
   computeData.diasCorrected = &diastolicPressCorrected;
   computeData.prCorrected = &pulseRateCorrected;
   // TCB:
   TCB computeTaskControlBlock;
   computeTaskControlBlock.mytask = computeTask;
   computeTaskControlBlock.taskDataPtr = (void*)&computeData;
   // 3. Display
   // data:
   DisplayData displayData;
   displayData.tempCorrected = &tempCorrected;
   displayData.sysPressCorrected = &systolicPressCorrected;
   displayData.diasCorrected = &diastolicPressCorrected;
   displayData.prCorrected = &pulseRateCorrected;
   displayData.batteryState = &batteryState;
   // TCB:
   TCB displayTaskControlBlock;
   displayTaskControlBlock.mytask = displayTask;
   displayTaskControlBlock.taskDataPtr = (void*)&displayData;
   // 4. Warning and Alarm
   // data:
   WarningAlarmData warnalarmData;
   warnalarmData.tempRawPtr = &temperatureRaw;
   warnalarmData.systoRawPtr = &systolicPressRaw;
   warnalarmData.diastoRawPtr = &diastolicPressRaw;
   warnalarmData.pulseRawPtr = &pulseRateRaw;
   warnalarmData.batterySstate = &batteryState;
   warnalarmData.bpOutOfRange = &bpOutOfRange;
   warnalarmData.tempOutOfRange = &tempOutOfRange;
   warnalarmData.pulseOutOfRange = &pulseOutOfRange;
   warnalarmData.bpHigh = &bpHigh;
   warnalarmData.tempHigh = &tempHigh;
   warnalarmData.pulseLow = &pulseLow;
   // TCB:
   TCB warnAndAlarmTaskControlBlock;
   warnAndAlarmTaskControlBlock.mytask = alarmsAndWarningTask;
   warnAndAlarmTaskControlBlock.taskDataPtr = (void*)&warnalarmData;
   // 5. Status
   // data:
   StatusData statusData;
   statusData.batterySstate = &batteryState;
   // TCB:
   TCB statusTaskControlBlock;
   statusTaskControlBlock.mytask = statusTask;
   statusTaskControlBlock.taskDataPtr = (void*)&statusData;
   // 6. Schedule
   // data:
   TCB* allTasks[6];
   allTask[0] = &measureTaskControlBlock;
   allTask[1] = &computeTaskControlBlock;
   allTask[2] = &displayTaskControlBlock;
   allTask[3] = &warnAndAlarmTaskControlBlock;
   allTask[4] = &statusTaskControlBlock;
   allTask[5] = NULL;
   SchedulerData schdulerData;
   schedulerData.tasks = allTasks;
   schedulerData.taskCount = TASK_TOTAL_COUNT;
   // TCB:
   TCB scheduleTaskControlBlock;
   scheduleTaskControlBlock.mytask = scheduleTask;
   scheduleTaskControlBlock.taskDataPtr = (void*)&scheduleData;
   // Start the scheduler task
   ExecuteTCB(&scheduleTaskControlBlock);
   return;
}


/******************************************
* function name: scheduleTask
* function inputs: a pointer to the SchedulerData
* function outputs: None
* function description: This function will execute
*                       each task in order forever.
* author: Matt & Sabrina
******************************************/ 
void scheduleTask(void* data){
   SchedulerData* schedulerData = (SchdulerData*)data;
   unsigned int totalCount = schedulerData->taskCount;
   unsigned int counter = 0;
   // forever execute each task
   while(1){
    // figure out the current task index
    counter = counter % totalCount;
    // if it is a good task, run it
    TCB* nextTask= (scheudlerData->tasks)[counter];
    if (nextTask != NULL){
      executeTCB(nextTask);
    }
   }
   return;
}

/******************************************
* function name: measureTask
* function inputs: a pointer to the MeasureData
* function outputs: stores the measurement reading
* function description: This function will store each 
*                       Measurements from the reading.
* author: Matt & Sabrina
******************************************/ 
void measureTask(void* data){
   // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer>0 && ((millis()-timer)<5000)){
     return;
   }
   timer = millis();
   // get the data struct
   MeasureData* measureData = (MeasureData*)data;
   // get the raw temperature
   unsigned int temperatureMeasured;
   requestAndReceive((char*)&temperatureMeasured, MEASURE_TASK , TEMP_RAW_SUBTASK , sizeof(unsigned int));
   *(measureData->tempRawPtr) = temperatureMeasured;
   // get the raw systo
   unsigned int systoMeasured;
   requestAndReceive((char*)&systoMeasured, MEASURE_TASK , SYSTO_RAW_SUBTASK , sizeof(unsigned int));
   *(measureData->systoRawPtr) = systoMeasured;
   // get the raw diasto
   unsigned int diastoMeasured;
   requestAndReceive((char*)&diastoMeasured, MEASURE_TASK , DIASTO_RAW_SUBTASK , sizeof(unsigned int));
   *(measureData->diastoRawPtr) = diastoMeasured;
   // get the raw diasto
   unsigned int pulseMeasured;
   requestAndReceive((char*)&pulseMeasured, MEASURE_TASK , PULSE_RAW_SUBTASK  , sizeof(unsigned int));
   *(measureData->pulseRawPtr) = pulseMeasured;
   return;
}

/******************************************
* function name: measureTask
* function inputs: a pointer to the MeasureData
* function outputs: stores the measurement reading
* function description: This function will store each 
*                       Measurements from the reading.
* author: Matt & Sabrina
******************************************/ 
void computeTask(void* data){
   // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer>0 && ((millis()-timer)<5000)){
     return;
   }
   timer = millis();
   
   return;
}

void displayTask(void* data){
  // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer>0 && ((millis()-timer)<5000)){
     return;
   }
   timer = millis();
   DisplayData* displayData = (DisplayData*)data;
   

   
   return;
}


void warnAndAlarmTask(void* data){
  // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer>0 && ((millis()-timer)<5000)){
     return;
   }
   timer = millis();


   
   return;
}

void statusTask(void* data){
  // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer>0 && ((millis()-timer)<5000)){
     return;
   }
   timer = millis();


   
   return;
}




/******************************************
* function name: executeTCB
* function inputs: The pointer to TCB to be run
* function outputs: None
* function description: This function will take
*                       the task control block
*                       and run it.
* author: Matt & Sabrina
******************************************/ 
void executeTCB(TCB* taskControlBlock){
   taskControlBlock->mytask(taskControlBlock->taskDataPtr);
   return;
}

/******************************************
* function name: requestAndReceive
* function inputs: 
* function outputs: None
* function description: This function will setup
*                       the communication between
*                       Mega and Uno to send command
*                       and grab the results
* author: Matt & Sabrina
******************************************/ 
void requestAndReceive(char* outputBuffer, char taskType, char subTaskType, char outputLength){
   for (char i = 0; i<outputLength){
     outputBuffer[i] = subTaskType;
   }
}


//  end EE 474 code
