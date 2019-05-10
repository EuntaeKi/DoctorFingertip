#include <stdio.h>
#include <stdlib.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library


#define SUSPENSION 5000
#define TASK_TOTAL_COUNT 6
#define MAX_STR_BUF_LEN 20
#define BATTERY_LIMIT 40
#define FULL_BATTERY 200
#define MEASURE_TASK 1
#define COMPUTE_TASK 2
#define ALARM_TASK 3
#define WARN_TASK 4
#define STATUS_TASK 5
#define TEMP_RAW_SUBTASK 1
#define SYSTO_RAW_SUBTASK 2
#define DIASTO_RAW_SUBTASK 3
#define PULSE_RAW_SUBTASK 4
#define TEMP_DISP_WIDTH 5
#define SYS_PRESS_DISP_WIDTH 3
#define DIAS_PRESS_DISP_WIDTH 6
#define PULSE_PRESS_DISP_WIDTH 3
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x0700
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define BACKGROUND_COLOR 0xFFFF
#define STATIC_TEXT_COLOR 0x0000
#define GOOD_DATA_COLOR 0x0700
#define BAD_DATA_COLOR 0xF800


enum _myBool { FALSE = 0, TRUE = 1 };
typedef enum _myBool Bool;

typedef struct
{
   void (*myTask)(void*);
   void* taskDataPtr;
   TCB* next;
   TCB* prev;
} TCB;

typedef struct 
{
  unsigned int** tempRawBufPtr;
  unsigned char*** tempCorrectedBufPtr;
  unsigned char* tempOutOfRangePtr;
  Bool* tempHighPtr;
} TemperatureData;

typedef struct 
{
  unsigned int** bpRawBufPtr;
  unsigned char*** bpCorrectedBufPtr;
  unsigned char* bpOutOfRangePtr;
  Bool* bpHighPtr;
} BPData;

typedef struct 
{
  unsigned int** prRawBufPtr;
  unsigned char*** prCorrectedBufPtr;
  unsigned char* pulseOutOfRangePtr;
  Bool* pulseLowPtr;
} PRData;

//DisplayData
typedef struct
{
  unsigned TemperatureData* tempData;
  unsigned BPData* bpData;
  unsigned PRData* prData;
  unsigned short* batteryState;
}  DisplayData;

// StatusData
typedef struct
{
  unsigned short* batteryState;
} StatusData;

 // SchedulerData
typedef struct
{
  TCB* head;
  TCB* tail;
} SchedulerData;


// function headers
// major functions
void initialization();
void scheduleTask(void* data);
// task functions
void temperatureTasks(void* data);
void bloodPressureTask(void* data);
void pulseRateTask(void* data);
void displayTask(void* data);
void statusTask(void* data);
// Intrasystem Communication functions
void requestAndReceive(char* inputBuffer, char inputLength , char* outputBuffer, char outputLength, char taskType, char subTaskType);
// TCB executer functions
void executeTCB(TCB* taskControlBlock);
// TaskQueue Management
void insertTask(TCB* task);
void deleteTask(TCB* task);
// TFT Related
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


// Global Variables for Measurements
unsigned int temperatureRawBuf[8];
unsigned int bloodPressureRawBuf[16];
unsigned int pulseRateRawBuf[8];
// Global Variables for Display
unsigned char* tempCorrectedBuf[8];
unsigned char* bloodPressureCorrectedBuf[16];
unsigned char* pulseRateCorrectedBuf[8];
// Global Variables for Function Counter
unsigned char tempCount = 0;
unsigned char bpCount = 0;
unsigned char prCount = 0;
// Global Variables for Status
unsigned short batteryState = FULL_BATTERY;
// Global Variables for Alarm
unsigned char bpOutOfRange = 0;
unsigned char tempOutOfRange = 0;
unsigned char pulseOutOfRange = 0;
//Global Variables for Warning
Bool bpHigh = FALSE;
Bool tempHigh = FALSE;
Bool pulseLow = FALSE;
//Global variables for all possible tasks
TCB* temperatureTCB;
TCB* bloodPressureTCB;
TCB* pulseRateTCB;
TCB* displayTCB;
TCB* statusTCB;
SchedulerData* schedulerTaskQueue;


/******************************************
* function name: setup
* function inputs: None
* function outputs: None
* function description: This is the function
*                       for arduino to initialize
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
 initialize();
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
   //Setup the TFT display
   tft.reset();
   uint16_t identifier = tft.readID();
   if(identifier == 0x9325) {
     Serial.println(F("Found ILI9325 LCD driver"));
   } else if(identifier == 0x9328) {
     Serial.println(F("Found ILI9328 LCD driver"));
   } else if(identifier == 0x4535) {
     Serial.println(F("Found LGDP4535 LCD driver"));
   }else if(identifier == 0x7575) {
     Serial.println(F("Found HX8347G LCD driver"));
   } else if(identifier == 0x9341) {
     Serial.println(F("Found ILI9341 LCD driver"));
   } else if(identifier == 0x8357) {
     Serial.println(F("Found HX8357D LCD driver"));
   } else if(identifier==0x0101)
   {
       identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
   }
   else if(identifier==0x1111)
   {
       identifier=0x9328;
       Serial.println(F("Found 0x9328 LCD driver"));
   }
   else {
     Serial.print(F("Unknown LCD driver chip: "));
     Serial.println(identifier, HEX);
     Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
     Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
     Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
     Serial.println(F("If using the breakout board, it should NOT be #defined!"));
     Serial.println(F("Also if using the breakout, double-check that all wiring"));
     Serial.println(F("matches the tutorial."));
     identifier=0x9328;
   }
   tft.begin(identifier);
   tft.fillScreen(BACKGROUND_COLOR);
   tft.setRotation(1);
   //unsigned long start = micros();
   tft.setCursor(0, 0);
   tft.setTextColor(GOOD_DATA_COLOR); 
   tft.setTextSize(2);
   // Prepare for each task Each Tasks
   // 1. Temperature
   // Data:
   TemperatureData temperatureData;
   temperatureData.tempRawBufPtr = &temperatureRawBuf;
   temperatureData.tempCorrectedBufPtr = &tempCorrectedBuf;
   temperatureData.tempOutOfRangePtr = &tempOutOfRange;
   temperatureData.tempHighPtr = &tempHigh;
   
   // TCB:
   TCB temperatureTaskControlBlock;
   temperatureTaskControlBlock.myTask = temperatureTask;
   temperatureTaskControlBlock.taskDataPtr = (void*)&temperatureData;
   temperatureTaskControlBlock.next = NULL;
   temperatureTaskControlBlock.prev = NULL;
   temperatureTCB = &temperatureTaskControlBlock;
   
   // 2. BloodPressure
   // data:
   BPData bpData;
   bpData.bpRawBufPtr = &bloodPressureRawBuf;
   bpData.bpCorrectedBufPtr = &bloodPressureCorrectedBuf;
   bpData.bpOutOfRangePtr = &bpOutofRange;
   bpData.bpHighPtr = &bpHigh;
   // TCB:
   TCB bpTaskControlBlock;
   bpTaskControlBlock.myTask = bloodPressureTask;
   bpTaskControlBlock.taskDataPtr = (void*)&bpData;
   bpTaskControlBlock.next = NULL;
   bpTaskControlBlock.prev = NULL;
   bloodPressureTCB = &bpTaskControlBlock;
   
   // 3. Pulse Rate
   // data:
   PRData prData;
   prData.prRawBufPtr = &pulseRateRawBuf;
   prData.prCorrectedBufPtr = &pulseRateCorrectedBuf;
   prData.pulseOutOfRangePtr = &pulseOutOfRange;
   prData.pulseLowPtr = &pulseLow;
   // TCB:
   TCB prTaskControlBlock;
   prTaskControlBlock.myTask = pulseRateTask;
   prTaskControlBlock.taskDataPtr = (void*)&prData;
   prTaskControlBlock.next = NULL;
   prTaskControlBlock.prev = NULL
   pulseRateTCB = &prTaskControlBlock;
   
    // 4. Display
   // data:
   DisplayData displayData;
   displayData.tempData =&temperatureData;
   displayData.bpData = &bpData;
   displayData.prData = &prData;
   displayData.batteryState = &batteryState;
   // TCB:
   TCB displayTaskControlBlock;
   displayTaskControlBlock.myTask = displayTask;
   displayTaskControlBlock.taskDataPtr = (void*)&displayData;
   displayTaskControlBlock.next = NULL;
   displayTaskControlBlock.prev = NULL;
   displayTCB = &displayTaskControlBlock;
   
   // 5. Status
   // data:
   StatusData statusData;
   statusData.batteryState = &batteryState;
   // TCB:
   TCB statusTaskControlBlock;
   statusTaskControlBlock.myTask = statusTask;
   statusTaskControlBlock.taskDataPtr = (void*)&statusData;
   statusTaskControlBlock.next = NULL;
   statusTaskControlBlock.next = NULL;
   statusTCB = &statusTaskControlBlock;

   
   // 6. Schedule
   // data:
   SchedulerData schedulerData;
   schedulerData.head = NULL;
   schedulerData.tail = NULL;
   schedulerTaskQueue = &schedulerData;
   // TCB:
   TCB scheduleTaskControlBlock;
   scheduleTaskControlBlock.myTask = scheduleTask;
   scheduleTaskControlBlock.taskDataPtr = (void*)&schedulerData;
   // Insert the basic TCBs in
   insertTask(statusTCB);
   insertTask(displayTCB);
   // Start the scheduler task
   executeTCB(&scheduleTaskControlBlock);
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
   SchedulerData* schedulerData = (SchedulerData*)data;
   // pick the first task to run.
   TCB* currTask = schedulerData->head;
   // forever execute each task
   while(1){
    if (currTask ==  null){
      // at the end or nothing. go back to head, or spin forever
      currTask = schedulerData->head;
    } else {
      // good task, execute it
      executeTCB(currTask);
      // move on
      currTask = currTask->next;
      // future addition for the TFT touch screen
      //???????????????????????
    }
   }
   return;
}


/******************************************
* function name: insertTask
* function inputs: a pointer to a TCB block
* function outputs: None
* function description: This function will insert
*                       a TCB to the end of
*                       the task queue
* author: Matt & Sabrina
******************************************/ 
void insertTask(TCB* task){
  TCB* currHead = schedulerTaskQueue->head;
  TCB* currTail = schedulerTaskQueue->tail;
  if (NULL == currHead){
    // this is the first task ever,
    schedulerTaskQueue->head = task;
    schedulerTaskQueue->tail = task;
  }else{
    // attach it to the tail
    currTail->next = task;
    task->prev = currTail;
    schedulerTaskQueue->tail = task;
  }
  return;
}

/******************************************
* function name: deleteTask
* function inputs: a pointer to a TCB block
* function outputs: None
* function description: This function will delete
*                       the TCB block in the queue
* author: Matt & Sabrina
******************************************/
void deleteTask(TCB* task){
  TCB* currHead = schedulerTaskQueue->head;
  TCB* currTail = schedulerTaskQueue->tail;
  if (currHead == NULL){
    // nothing to delete anyway
  } else if (currHead == currTail && task == currHead){
    // deleting the only one node
    schedulerTaskQueue->head = NULL;
    schedulerTaskQueue->tail = NULL;
  } else if (task == currHead){ // so we have more than one tasks
    // we are deleteing the head. promote its next to be head
    schedulerTaskQueue->head = task->next;
  } else if (task == currTail){
    // we are deleting the tail. drag the prev to be tail.
    schedulerTaskQueue->tail = task->prev;
  } else {
    // the task is in the middle
    if (task->next == NULL || task->prev == NULL){
      // this task is not even in the queue
      return;
    }
    // connect the prev and the next
    itsPrev = task->prev;
    itsNext = task->next;
    itsPrev->next = itsNext;
    itsNext->prev = itsPrev;
    // and we are done
  }
  // finall, clean the task
  task->next = NULL;
  task->prev = NULL;
}

void temperatureTasks(void* data) {
  static unsigned long timer = 0;
  if (timer != 0 && (millis() - timer) < SUSPENSION) {
    return;
  }
  timer = millis();
  TemperatureData* temperatureData = (TemperatureData*)data;
  requestAndReceive((char*)&(*(temperatureData->tempRawBufPtr)[tempCount]), sizeof(unsigned int), 
  (char*)&(*(temperatureData->tempRawBufPtr)[tempCount]),sizeof(unsigned int), MEASURE_TASK, TEMP_RAW_SUBTASK);
  for (int iii = 0; iii< 8; iii++){
    Serial.print(temperatureRawBuf[iii]);
    Serial.print(",");    
  }
  Serial.print("\n");
  double tempCorrDump;
  requestAndReceive((char*)&(*(temperatureData->tempRawBufPtr[tempCount])),sizeof(unsigned int),
  (char*)&tempCorrDump, sizeof(double), COMPUTE_TASK, TEMP_RAW_SUBTASK);
  dtostrf(tempCorrDump, 1, 2, (char*)&(*tempCorrectedDataBuf)[tempCount]);
  tempCount = (tempCount + 1) % 8;
  Serial.print("Temperature Measurement Complete");
  return;
}


void pulseRateTasks(void* data) {
  static unsigned long timer = 0;
  if (timer != 0 && (millis() - timer) < SUSPENSION) {
    return;
  }
  timer = millis();
  BPData* bloodPressureData = (BPData*)data;
  // Measure Systolic
  requestAndReceive((char*)&(*(bloodPressureData->bpRawBufPtr)[bpCount]), sizeof(unsigned int), 
  (char*)&(*(bloodPressureData->bpRawBufPtr)[bpCount]), sizeof(unsigned int), MEASURE_TASK, SYSTO_RAW_SUBTASK);
  // Measure Diastolic
  requestAndReceive((char*)&(*(bloodPressureData->bpRawBufPtr)[bpCount + 8]), sizeof(unsigned int), 
  (char*)&(*(bloodPressureData->bpRawBufPtr)[bpCount + 8]), sizeof(unsigned int), MEASURE_TASK, DIASTO_RAW_SUBTASK);
  for (int iii = 0; iii< 8; iii++){
    Serial.print(bpRawBuf[iii]);
    Serial.print(",");    
  }
  Serial.print("\n");
  
  double bpCorrDump;
  // Compute Systolic
  requestAndReceive((char*)&(*(bloodPressureData->bpRawBufPtr[bpCount])),sizeof(unsigned int),
  (char*)&bpCorrDump, sizeof(double), COMPUTE_TASK, SYSTO_RAW_SUBTASK);
  dtostrf(bpCorrDump, 1, 2, (char*)&(*prCorrectedDataBuf)[bpCount]);
  // Compute Diastolic
  requestAndReceive((char*)&(*(bloodPressureData->bpRawBufPtr[bpCount + 8])),sizeof(unsigned int),
  (char*)&bpCorrDump, sizeof(double), COMPUTE_TASK, DIASTO_RAW_SUBTASK);
  dtostrf(bpCorrDump, 1, 2, (char*)&(*prCorrectedDataBuf)[bpCount + 8]);
  prCount = (prCount + 1) % 8;
  Serial.print("Blood Pressure Measurement Complete");
  return;
}

void pulseRateTasks(void* data) {
  static unsigned long timer = 0;
  if (timer != 0 && (millis() - timer) < SUSPENSION) {
    return;
  }
  timer = millis();
  PRData* pulseRateData = (PRData*)data;
  requestAndReceive((char*)&(*(pulseRateData->prRawBufPtr)[prCount]), sizeof(unsigned int), 
  (char*)&(*(pulseRateData->prRawBufPtr)[prCount]),sizeof(unsigned int), MEASURE_TASK, PULSE_RAW_SUBTASK);
  for (int iii = 0; iii< 8; iii++){
    Serial.print(pulseRateRawBuf[iii]);
    Serial.print(",");    
  }
  Serial.print("\n");
  double prCorrDump;
  requestAndReceive((char*)&(*(pulseRateData->prRawBufPtr[prCount])),sizeof(unsigned int),
  (char*)&prCorrDump, sizeof(double), COMPUTE_TASK, PULSE_RAW_SUBTASK);
  dtostrf(prCorrDump, 1, 2, (char*)&(*prCorrectedDataBuf)[prCount]);
  prCount = (prCount + 1) % 8;
  Serial.print("Pulse Rate Measurement Complete");
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
   if (timer!=0 && (millis()-timer)<SUSPENSION){
     return;
   }
   Serial.print("Measureing----");
   timer = millis();
   // get the data struct
   MeasureData* measureData = (MeasureData*)data;
   // get the raw temperature
   requestAndReceive((char*)(measureData->tempRawPtr),sizeof(unsigned int), (char*)(measureData->tempRawPtr),sizeof(unsigned int), MEASURE_TASK , TEMP_RAW_SUBTASK );
      Serial.print("  1 = ");
      Serial.print(*(measureData->tempRawPtr));
   // get the raw systo
   requestAndReceive((char*)(measureData->systoRawPtr), sizeof(unsigned int), (char*)(measureData->systoRawPtr), sizeof(unsigned int), MEASURE_TASK , SYSTO_RAW_SUBTASK);
   Serial.print("  2----");

   // get the raw diasto
   requestAndReceive((char*)(measureData->diastoRawPtr) , sizeof(unsigned int),(char*)(measureData->diastoRawPtr) , sizeof(unsigned int), MEASURE_TASK , DIASTO_RAW_SUBTASK);

      Serial.print("3----");
   // get the raw pulse rate
   requestAndReceive((char*)(measureData->pulseRawPtr) , sizeof(unsigned int),(char*)(measureData->pulseRawPtr) , sizeof(unsigned int), MEASURE_TASK , PULSE_RAW_SUBTASK );
   Serial.println("Finished");
   return;
}

/******************************************
* function name: computeTask
* function inputs: a pointer to the computeData
* function outputs: computation results
* function description: This function will compute
*                       the data and save the string
*                       results
* author: Matt & Sabrina
******************************************/ 
void computeTask(void* data){
   // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer!=0 && (millis()-timer)<SUSPENSION){
     return;
   }
   timer = millis();
   Serial.print("Computing---");
   ComputeData* computeData = (ComputeData*) data;
   // give the corrected data somewhere to store its string
   static unsigned char tempCorrectedDataBuf[MAX_STR_BUF_LEN];
   static unsigned char sysCorrectedDataBuf[MAX_STR_BUF_LEN]; 
   static unsigned char diasCorrectedDataBuf[MAX_STR_BUF_LEN]; 
   static unsigned char pulseCorrectedDataBuf[MAX_STR_BUF_LEN]; 
   if (*(computeData->tempCorrected) == NULL){
     *(computeData->tempCorrected) = tempCorrectedDataBuf;
   }
   if (*(computeData->sysPressCorrected) == NULL){
     *(computeData->sysPressCorrected) = sysCorrectedDataBuf;
   }
   if (*(computeData->diasCorrected) == NULL){
     *(computeData->diasCorrected) = diasCorrectedDataBuf;
   }
   if (*(computeData->prCorrected) == NULL){
     *(computeData->prCorrected) = pulseCorrectedDataBuf;
   }
   // Get the data from UNO
   // temperature measure
   double tempCorrDump;
   requestAndReceive((char*)(computeData->tempRawPtr),sizeof(unsigned int), (char*)&tempCorrDump,sizeof(double), COMPUTE_TASK , TEMP_RAW_SUBTASK );
   dtostrf(tempCorrDump, 1, 2, (char*)tempCorrectedDataBuf);
   // systo measure
   unsigned int systoCorrDump;
   requestAndReceive((char*)(computeData->systoRawPtr),sizeof(unsigned int), (char*)&systoCorrDump,sizeof(unsigned int), COMPUTE_TASK , SYSTO_RAW_SUBTASK );
   sprintf((char*)sysCorrectedDataBuf, "%d", systoCorrDump);
   // diasto measure
   double diastoCorrDump;
   requestAndReceive((char*)(computeData->diastoRawPtr),sizeof(unsigned int),(char*)&diastoCorrDump,sizeof(double), COMPUTE_TASK , DIASTO_RAW_SUBTASK );
   dtostrf(diastoCorrDump, 1, 2, (char*)diasCorrectedDataBuf);
   // pulse measure
   unsigned int pulseCorrDump;
   requestAndReceive((char*)(computeData->pulseRawPtr),sizeof(unsigned int), (char*)&pulseCorrDump,sizeof(unsigned int), COMPUTE_TASK , PULSE_RAW_SUBTASK );
   sprintf((char*)pulseCorrectedDataBuf, "%d", pulseCorrDump);
   return;
}

/******************************************
* function name: displayTask
* function inputs: a pointer to the DisplayData
* function outputs: None
* function description: This function will display
* 						all the data in Mega onto TFT
* author: Matt & Sabrina
******************************************/ 
void displayTask(void* data){
   // no need for the timer
   Serial.print("displaying---");
   DisplayData* displayData = (DisplayData*) data;
   WarningAlarmData* bindedData = displayData->warnData;
   tft.setCursor(0,0);
   tft.println("    ");
   tft.println("    ");
   // Display Temperature
   // Figure out the color
   tft.setTextColor(STATIC_TEXT_COLOR);
   tft.print("Temperature:  ");
   if ((*(bindedData->tempOutOfRange) == 1) || (*(bindedData->tempHigh) == TRUE)){
      tft.setTextColor(BAD_DATA_COLOR, BACKGROUND_COLOR);
   }else{
      tft.setTextColor(GOOD_DATA_COLOR, BACKGROUND_COLOR);
   }
   // Show the Temprature
   tft.print((char*)*(displayData->tempCorrected));
   int diff = TEMP_DISP_WIDTH-strlen((char*)*(displayData->tempCorrected));
   for (int i = 0; i<diff; i++){
    tft.print(" ");
   }
   tft.println(" C\n");
   // Display Blood Pressure
   tft.setTextColor(STATIC_TEXT_COLOR);
   tft.println("Pressure:\n");
   tft.print("   Systolic:  ");
   // Display Systolic
   // Figure out the color
   if ((*(bindedData->bpOutOfRange) == 1) || (*(bindedData->bpHigh) == TRUE)){
      tft.setTextColor(BAD_DATA_COLOR, BACKGROUND_COLOR);
   }else{
      tft.setTextColor(GOOD_DATA_COLOR, BACKGROUND_COLOR);
   }
   // Show the Pressure
   tft.print((char*)*(displayData->sysPressCorrected));
   tft.print(".00");
   diff = SYS_PRESS_DISP_WIDTH-strlen((char*)*(displayData->sysPressCorrected));
   for (int i = 0; i<diff; i++){
    tft.print(" ");
   }
   tft.println(" mm Hg");
   // Display Diastolic
   tft.setTextColor(STATIC_TEXT_COLOR);
   tft.print("   Diastolic: ");
   // Figure out the color
   if ((*(bindedData->bpOutOfRange) == 1) || (*(bindedData->bpHigh) == TRUE)){
      tft.setTextColor(BAD_DATA_COLOR, BACKGROUND_COLOR);
   }else{
      tft.setTextColor(GOOD_DATA_COLOR, BACKGROUND_COLOR);
   }
   // Show the Dias Pressure
   tft.print((char*)*(displayData->diasCorrected));
   diff = DIAS_PRESS_DISP_WIDTH-strlen((char*)*(displayData->diasCorrected));
   for (int i = 0; i<diff; i++){
    tft.print(" ");
   }
   tft.println(" mm Hg");
   // Display Pulse
   tft.setTextColor(STATIC_TEXT_COLOR);
   tft.print("Pulse rate:   ");
   // Figure out the color
   if ((*(bindedData->pulseOutOfRange) == 1) || (*(bindedData->pulseLow) == TRUE)){
      tft.setTextColor(BAD_DATA_COLOR, BACKGROUND_COLOR);
   }else{
      tft.setTextColor(GOOD_DATA_COLOR, BACKGROUND_COLOR);
   }
   // Show the PulseRate
   tft.print((char*)*(displayData->prCorrected));
   diff = PULSE_RAW_SUBTASK-strlen((char*)*(displayData->prCorrected));
   for (int i = 0; i<diff; i++){
    tft.print(" ");
   }
   tft.println(" BPM\n");
   // Display Battery State
   tft.setTextColor(STATIC_TEXT_COLOR);
   tft.print("Battery:      ");
   // Figure out the color
   if (*(displayData->batteryState) <= BATTERY_LIMIT){
      tft.setTextColor(BAD_DATA_COLOR, BACKGROUND_COLOR);
   }else{
      tft.setTextColor(GOOD_DATA_COLOR, BACKGROUND_COLOR);
   }
   // Show the Temprature
   char batteryStateBuffer[5];
   sprintf(batteryStateBuffer, "%d  ",*(displayData->batteryState));
   tft.print(batteryStateBuffer);  
   Serial.println("Finished");  
   return;
}

/******************************************
* function name: alarmAndWarningTask
* function inputs: a pointer to the WarningAlarmData
* function outputs: None
* function description: This function will check
*						whether any of the current value
*						in Mega are out of the range
*						or too high by calling Uno's
*						functions
* author: Matt & Sabrina
******************************************/ 
void alarmsAndWarningTask(void* data){
   // do not check time
   WarningAlarmData* warnData = (WarningAlarmData*)data;
   // grab the data
   char temporaryValue = 0;
   Bool sysWarnResult;
   char sysAlarmResult;
   // two blood pressure types' two stuffs
   requestAndReceive((char*)(warnData->systoRawPtr),sizeof(unsigned int), (char*)&sysAlarmResult,sizeof(unsigned char), ALARM_TASK , SYSTO_RAW_SUBTASK );
   requestAndReceive((char*)(warnData->systoRawPtr),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , SYSTO_RAW_SUBTASK );
   if (temporaryValue==0){
     sysWarnResult = FALSE;
   }else{
     sysWarnResult = TRUE;
   }
   Bool diasWarnResult;
   char diasAlarmResult;
   requestAndReceive((char*)(warnData->diastoRawPtr),sizeof(unsigned int), (char*)&diasAlarmResult,sizeof(unsigned char), ALARM_TASK , DIASTO_RAW_SUBTASK );
   requestAndReceive((char*)(warnData->diastoRawPtr),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , DIASTO_RAW_SUBTASK );
   if (temporaryValue==0){
    diasWarnResult = FALSE;
   }else{
    diasWarnResult = TRUE;
   }
   *(warnData->bpOutOfRange) = sysAlarmResult || diasAlarmResult;
   if (sysWarnResult == FALSE && diasWarnResult == FALSE){
     *(warnData->bpHigh) = FALSE;
   }else{
         *(warnData->bpHigh) = TRUE;
   }
   // tempreture's two stuffs
   requestAndReceive((char*)(warnData->tempRawPtr),sizeof(unsigned int), (char*)(warnData->tempOutOfRange),sizeof(unsigned char), ALARM_TASK , TEMP_RAW_SUBTASK );
   requestAndReceive((char*)(warnData->tempRawPtr),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , TEMP_RAW_SUBTASK );
   if (temporaryValue==0){
    *(warnData->tempHigh) = FALSE;
   }else{
    *(warnData->tempHigh) = TRUE;
   }
   // pluse's two stuffs
   requestAndReceive((char*)(warnData->pulseRawPtr),sizeof(unsigned int), (char*)(warnData->pulseOutOfRange),sizeof(unsigned char), ALARM_TASK , PULSE_RAW_SUBTASK );
   requestAndReceive((char*)(warnData->pulseRawPtr),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , PULSE_RAW_SUBTASK );
   if (temporaryValue==0){
    *(warnData->pulseLow) = FALSE;
   }else{
    *(warnData->pulseLow) = TRUE;
   }
   return;
}

void statusTask(void* data){
   // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer!=0 && (millis()-timer)<SUSPENSION){
     return;
   }
   timer = millis();
   Serial.print("For Status--- B=");
   StatusData* statusData = (StatusData*)data;
   requestAndReceive((char*)(statusData->batteryState),sizeof(unsigned short), (char*)(statusData->batteryState),sizeof(unsigned short), STATUS_TASK , STATUS_TASK );
   if (*(statusData->batteryState)==0){
    *(statusData->batteryState) = FULL_BATTERY;  // Magical Recharge
   }
   Serial.println(" Finished");
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
   taskControlBlock->myTask(taskControlBlock->taskDataPtr);
   return;
}

/******************************************
* function name: requestAndReceive
* function inputs:  char* to set input buffer,
*					char to represent input buffer length
*					char* for output buffer,
*					char to represent output buffer length
* 					char to represent task type,
*					and char to represent which variable to
*					perform task on.
* function outputs: None
* function description: This function will setup
*                       the communication between
*                       Mega and Uno to send command
*                       and grab the results
* author: Matt & Sabrina
******************************************/ 
void requestAndReceive(char* inputBuffer, char inputLength , char* outputBuffer, char outputLength, char taskType, char subTaskType){
  // write the taskType and subtype to the UNO
  Serial1.write(taskType);
  Serial1.write(subTaskType);
  // write the data that needed to be passed
  for (char i = 0; i<inputLength; i++){
    Serial1.write(inputBuffer[i]);
  }
  // now wait for the replies
  while(Serial1.available()<outputLength){
    // just wait
  }
  for (char j = 0; j<outputLength; j++){
     outputBuffer[j]=Serial1.read();
  }
  return;
}

//  end of EE 474 code
