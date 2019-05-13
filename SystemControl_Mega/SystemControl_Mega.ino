#include <stdio.h>
#include <stdlib.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>   // Touch Screen for TFT


#define SUSPENSION 2222
#define KEYPAD_SCAN_INTERVAL 20
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
#define TEMP_SCEHDULED 1
#define SYSTO_SCHEDULED 2
#define DIASTO_SCHEDULED 4
#define PULSE_SCHEDULED 8
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x05E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFE6D
#define BACKGROUND_COLOR 0xFFFF
#define STATIC_TEXT_COLOR 0x0000
#define GOOD_DATA_COLOR 0x0700
#define BAD_DATA_COLOR 0xF800
#define BUTTON_W 130
#define BUTTON_H 32
#define MINPRESSURE 5 
#define MAXPRESSURE 5000 
#define YP A3 
#define XM A2 
#define YM 9 
#define XP 8 
#define TS_MINY 70
#define TS_MAXY 920
#define TS_MINX 120
#define TS_MAXX 900

enum _myBool { FALSE = 0, TRUE = 1 };
typedef enum _myBool Bool;

typedef struct TCB
{
   void (*myTask)(void*);
   void* taskDataPtr;
   struct TCB* next;
   struct TCB* prev;
} TCB;


//MeasureData Struct
typedef struct
{
  unsigned int* tempRawBufPtr;
  unsigned int* bpRawBufPtr;
  unsigned int* prRawBufPtr;
  unsigned int* measurementSelectionPtr; 
  unsigned char* mCountPtr; 
} MeasureData; 


typedef struct
{
  unsigned int* tempRawBufPtr;
  unsigned int* bpRawBufPtr;
  unsigned int* prRawBufPtr;
  unsigned char** tempCorrectedBufPtr;
  unsigned char** bpCorrectedBufPtr;
  unsigned char** prCorrectedBufPtr;  
} ComputeData; 


typedef struct
{
  unsigned int* tempRawBufPtr;
  unsigned int* bpRawBufPtr;
  unsigned int* prRawBufPtr;
  unsigned short* batteryState;
  unsigned char* tempOutOfRangePtr;
  Bool* tempHighPtr;
  unsigned char* bpOutOfRangePtr;
  Bool* bpHighPtr;
  unsigned char* pulseOutOfRangePtr;
  Bool* pulseLowPtr;
  unsigned char* ackReceived;
  unsigned int* tempColorPtr;
  unsigned int* systoColorPtr;
  unsigned int* diastoColorPtr;
  unsigned int* pulseColorPtr;
  unsigned char* mCountPtr; 
} WarningAlarmData;


//DisplayData
typedef struct
{
   unsigned char** tempCorrectedBufPtr;
  unsigned char** bpCorrectedBufPtr;
  unsigned char** prCorrectedBufPtr;  
  unsigned short* batteryState;
  unsigned int* tempColorPtr;
  unsigned int* systoColorPtr;
  unsigned int* diastoColorPtr;
  unsigned int* pulseColorPtr;
  unsigned char* modeSelection;
  unsigned int* measurementSelectionPtr;  
  unsigned char* ackReceived;
}  DisplayData;


//Keypad Data
typedef struct
{
    unsigned int* measurementSelectionPtr;  
    unsigned char* ackReceived;
    unsigned char* modeSelection;
} KeypadData;


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
void startUpTask();
void scheduleTask(void* data);
// task functions
void measureTask(void* data);
void computeTask(void* data);
void warningAlarmTask(void* data);
void keypadTask(void* data);
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
TouchScreen ts = TouchScreen(XP, YP ,XM , YM, 300);



// Globals for keypad and ack
unsigned char ackReceived = 0;
unsigned char mCount = 0; 
unsigned char modeSelection = 1;  // 0 means menu   1 anounciation
// Global Variables for Measurements
unsigned int temperatureRawBuf[8] = {75,0,0,0,0,0,0,0};
unsigned int bloodPressureRawBuf[16] = {80,0,0,0,0,0,0,0,80,0,0,0,0,0,0,0};
unsigned int pulseRateRawBuf[8] = {0,0,0,0,0,0,0,0};
unsigned int measurementSelection = 0; // this is using a bit mask capability. See MeasureTask function for detail 
// Initial Values for Compute
const char* initialTempDisplay = "61.25";
const char* initialSystoDisplay = "169";
const char* initialDiastoDisplay = "126.00";
const char* initialPulseDisplay = "8";
// Global Variables for Display
unsigned char* tempCorrectedBuf[8];
unsigned char* bloodPressureCorrectedBuf[16];
unsigned char* pulseRateCorrectedBuf[8];
unsigned int tempColor = GREEN;
unsigned int systoColor = GREEN;
unsigned int diastoColor = GREEN;
unsigned int pulseColor = GREEN;
Elegoo_GFX_Button button[4];
Elegoo_GFX_Button menuButton[4];
Elegoo_GFX_Button ackButton;
// Global Variables for Function Counter
unsigned char freshTempCursor = 0;
unsigned char freshSBPCursor = 0;
unsigned char freshDBPCursor = 8;
unsigned char freshPulseCursor = 0;
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
TCB* measureTCB = NULL;
TCB* computeTCB = NULL;
TCB* warningAlarmTCB = NULL;
TCB* keypadTCB = NULL;
TCB* displayTCB = NULL;
TCB* statusTCB = NULL;
//Global variables for task queue management
SchedulerData* schedulerTaskQueue = NULL;
unsigned char addComputeTaskFlag = 0; // 0 for delete 1 for add, above means neutral so boldly incrementing is fine (^o^)
unsigned long timeElapsed = 0;


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
 startUpTask();
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
void startUpTask(){   
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
   // 1. Measure
   // Data:
   MeasureData measureData;
   measureData.tempRawBufPtr = &temperatureRawBuf[0];
   measureData.bpRawBufPtr = &bloodPressureRawBuf[0];;
   measureData.prRawBufPtr = &pulseRateRawBuf[0];
   measureData.measurementSelectionPtr = &measurementSelection;
   measureData.mCountPtr = &mCount;
   freshTempCursor = 0;
   freshSBPCursor = 0;
   freshDBPCursor = 8;
   freshPulseCursor = 0;
   //TCB:
   TCB measureTCBlock;
   measureTCBlock.taskDataPtr = (void*)&measureData;
   measureTCBlock.myTask = measureTask;
   measureTCBlock.next = NULL;
   measureTCBlock.prev = NULL;
   measureTCB = &measureTCBlock;


   // 2. Compute
   // Data:
   ComputeData computeData;
   computeData.tempRawBufPtr = &temperatureRawBuf[0];
   computeData.bpRawBufPtr = &bloodPressureRawBuf[0];
   computeData.prRawBufPtr = &pulseRateRawBuf[0];
   computeData.tempCorrectedBufPtr = tempCorrectedBuf;
   computeData.bpCorrectedBufPtr = bloodPressureCorrectedBuf;
   computeData.prCorrectedBufPtr = pulseRateCorrectedBuf;
   for (int i=0; i<8; i++){
     tempCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   for (int i=0; i<16; i++){
     bloodPressureCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   for (int i=0; i<8; i++){
     pulseRateCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   strcpy((char*)tempCorrectedBuf[0], initialTempDisplay);
   strcpy((char*)bloodPressureCorrectedBuf[0], initialSystoDisplay);
   strcpy((char*)bloodPressureCorrectedBuf[8], initialDiastoDisplay);
   strcpy((char*)pulseRateCorrectedBuf[0], initialPulseDisplay);

   //TCB
   TCB computeTCBlock;
   computeTCBlock.taskDataPtr = (void*)&computeData;
   computeTCBlock.myTask = computeTask;
   computeTCBlock.next = NULL;
   computeTCBlock.prev = NULL;
   computeTCB = &computeTCBlock;


   // 3. Warning Alrm Data
   WarningAlarmData warningAlarmData;
   warningAlarmData.tempRawBufPtr = &temperatureRawBuf[0];
   warningAlarmData.bpRawBufPtr = &bloodPressureRawBuf[0];
   warningAlarmData.prRawBufPtr = &pulseRateRawBuf[0];
   warningAlarmData.tempOutOfRangePtr = &tempOutOfRange;
   warningAlarmData.tempHighPtr = &tempHigh;
   warningAlarmData.bpOutOfRangePtr = &bpOutOfRange;
   warningAlarmData.bpHighPtr = &bpHigh;
   warningAlarmData.pulseOutOfRangePtr = &pulseOutOfRange;
   warningAlarmData.pulseLowPtr = &pulseLow;
   warningAlarmData.batteryState = &batteryState;
   warningAlarmData.ackReceived = &ackReceived;
   warningAlarmData.tempColorPtr = &tempColor;
   warningAlarmData.systoColorPtr = &systoColor;
   warningAlarmData.diastoColorPtr = &diastoColor;
   warningAlarmData.pulseColorPtr = &pulseColor;
   warningAlarmData.mCountPtr = &mCount;

   // TCB:
   TCB warningAlarmTCBlock;
   warningAlarmTCBlock.myTask = warningAlarmTask;
   warningAlarmTCBlock.taskDataPtr = (void*)&warningAlarmData;
   warningAlarmTCBlock.next = NULL;
   warningAlarmTCBlock.prev = NULL;
   warningAlarmTCB = &warningAlarmTCBlock;
   
    // 4. Display
   // data:
   DisplayData displayData;
   displayData.tempCorrectedBufPtr = tempCorrectedBuf;
   displayData.bpCorrectedBufPtr = bloodPressureCorrectedBuf;
   displayData.prCorrectedBufPtr = pulseRateCorrectedBuf;
   displayData.batteryState = &batteryState;
   displayData.tempColorPtr = &tempColor;
   displayData.systoColorPtr = &systoColor;
   displayData.diastoColorPtr = &diastoColor;
   displayData.pulseColorPtr = &pulseColor;
   displayData.modeSelection = &modeSelection;
   displayData.measurementSelectionPtr = &measurementSelection;  
   displayData.ackReceived = &ackReceived;

   // TCB:
   TCB displayTaskControlBlock;
   displayTaskControlBlock.myTask = displayTask;
   displayTaskControlBlock.taskDataPtr = (void*)&displayData;
   displayTaskControlBlock.next = NULL;
   displayTaskControlBlock.prev = NULL;
   displayTCB = &displayTaskControlBlock;

   // buttons
   char ButtonText[4][11] = {"Menu",  "Annun", " ", " "};
   for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        button[i*2 + j].initButton(&tft, 80 + (160 * j), 177 + (40 * i), BUTTON_W + 20, BUTTON_H, BLACK, BLACK, WHITE, ButtonText[(i * 2) + j], 2);
      }
   }
   char MenuText[4][11] = {"Temp", "Systo", "Diasto", "Pulse"};
   for (int i = 0; i < 4; i++) { 
            menuButton[i].initButton(&tft, 155,  30 + (37 * i), BUTTON_W + 120, BUTTON_H - 5, BLUE, BLUE, WHITE, MenuText[i], 2); 
   }
   char AckText[1][4] = {"Ack"};
   ackButton.initButton(&tft, 150, 140, BUTTON_W, BUTTON_H, MAGENTA, MAGENTA, WHITE, AckText[0], 2);


   // 5. KeyPad
   // data:
   KeypadData keypadData;
   keypadData.measurementSelectionPtr = &measurementSelection;  
   keypadData.ackReceived = &ackReceived;
   keypadData.modeSelection = &modeSelection;
   // TCB:
   TCB keypadTaskControlBlock;
   keypadTaskControlBlock.myTask = keypadTask;
   keypadTaskControlBlock.taskDataPtr = (void*)&keypadData;
   keypadTaskControlBlock.next = NULL;
   keypadTaskControlBlock.next = NULL;
   keypadTCB = &keypadTaskControlBlock;

   
   // 6. Status
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


   // 7. Schedule
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
   insertTask(measureTCB);
   insertTask(warningAlarmTCB);
   insertTask(displayTCB);


   // Get the timer started
   // Enable timer2 interrupt
      noInterrupts();
   TIMSK2 = (TIMSK2 & B11111110) | 0x01;
   interrupts();             // enable all interrupts

   // Start the scheduler task
   executeTCB(&scheduleTaskControlBlock);
   return;
}

// Timer2 overflow ISR. It will increment our timer by 1 everytime it interrupts. representing 1 millisec
ISR(TIMER2_OVF_vect){
   timeElapsed = timeElapsed + 1;
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
    // check if we need to add the compute 
    if (addComputeTaskFlag == 0){
      // compute task is completed and we can remove it now.
      deleteTask(computeTCB);  // deleteTask has protection and check if computeTCB is removed already.
    }else if (addComputeTaskFlag == 1){
      // compute task has been added and scheduled by measureTask. add it in
            Serial.print("add compute");

      insertTask(computeTCB);
      // increment the flag to be neutral
      addComputeTaskFlag++;
    } else {
      // neutral state
      // we have already added a compute task. don't add it again. wait for it to finish.
    }
    if (currTask ==  NULL){
      // at the end or nothing. go back to head, or spin forever
      currTask = schedulerData->head;
    } else {
      // good task, execute it
      executeTCB(currTask);
      // move on
      currTask = currTask->next;
    }
    // execute the keypad task (let the keypad task figure out its timer)
    digitalWrite(31, HIGH); 
    executeTCB(keypadTCB);
    digitalWrite(31, LOW); 
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
    Serial.print("A");
    // nothing to delete anyway
  } else if (currHead == currTail && task == currHead){
    // deleting the only one node
    schedulerTaskQueue->head = NULL;
    schedulerTaskQueue->tail = NULL;
        Serial.print(" A ");

  } else if (task == currHead){ // so we have more than one tasks
    // we are deleteing the head. promote its next to be head
        Serial.print(" B ");

    schedulerTaskQueue->head = task->next;
  } else if (task == currTail){
    // we are deleting the tail. drag the prev to be tail.
    TCB* itsPrev = task->prev;
    schedulerTaskQueue->tail = task->prev;
    itsPrev->next = NULL;

        Serial.print(" C ");

  } else {
    // the task is in the middle
    if (task->next == NULL && task->prev == NULL){
      // this task is not even in the queue
      return;
    }
    // connect the prev and the next
    TCB* itsPrev = task->prev;
    TCB* itsNext = task->next;
    itsPrev->next = itsNext;
    itsNext->prev = itsPrev;
    // and we are done
        Serial.print(" D ");

  }
  // finall, clean the task
  task->next = NULL;
  task->prev = NULL;
}

void measureTask(void* data){
   // TIMING MECH
   static unsigned long timer = 0;
   if (timer!=0 && (timeElapsed-timer)<SUSPENSION){
     return;
   }
   // It is our turn.
   timer = timeElapsed;
   Serial.print("\nMeasureTask Starts\n");
   MeasureData* mData = (MeasureData*)data;
   unsigned int selections = *(mData->measurementSelectionPtr);
   // run the measurement that has been scheduled.
   if (selections & TEMP_SCEHDULED){
     // measure temperature
     unsigned char oldTempCursor = freshTempCursor;
     freshTempCursor = (freshTempCursor + 1)%8;
     requestAndReceive((char*)&(mData->tempRawBufPtr[oldTempCursor]), sizeof(unsigned int), 
     (char*)&(mData->tempRawBufPtr[freshTempCursor]),sizeof(unsigned int), MEASURE_TASK, TEMP_RAW_SUBTASK);
        addComputeTaskFlag++;  // just add one. repetition has been delt with.

   }
   if (selections & SYSTO_SCHEDULED ){
     // Measure Systolic
     unsigned char oldSBPCursor  = freshSBPCursor ;
     freshSBPCursor  = (freshSBPCursor  + 1)%8;
     requestAndReceive((char*)&(mData->bpRawBufPtr[oldSBPCursor]), sizeof(unsigned int), 
     (char*)&(mData->bpRawBufPtr[freshSBPCursor]), sizeof(unsigned int), MEASURE_TASK, SYSTO_RAW_SUBTASK);
     // do the systo alarm increment
     if (*(mData->mCountPtr) > 0){
        // alarm is expecting us to count how many times we are called.
        *(mData->mCountPtr) = *(mData->mCountPtr)+1;
     }
        addComputeTaskFlag++;  // just add one. repetition has been delt with.

   } 
   if (selections & DIASTO_SCHEDULED ){
     // Measure Diastolic
     unsigned char oldDBPCursor  = freshDBPCursor ;
     freshDBPCursor  = ((freshDBPCursor + 1) % 8) +8;
     requestAndReceive((char*)&(mData->bpRawBufPtr[oldDBPCursor]), sizeof(unsigned int), 
     (char*)&(mData->bpRawBufPtr[freshDBPCursor]), sizeof(unsigned int), MEASURE_TASK, DIASTO_RAW_SUBTASK);
        addComputeTaskFlag++;  // just add one. repetition has been delt with.

   } 
   if (selections & PULSE_SCHEDULED ){
     // Measure Pulse rate
     unsigned char oldPulseCursor  = freshPulseCursor ;
     freshPulseCursor  = (freshPulseCursor + 1)%8;
     requestAndReceive((char*)&(mData->prRawBufPtr[oldPulseCursor]), sizeof(unsigned int), 
     (char*)&(mData->prRawBufPtr[freshPulseCursor]),sizeof(unsigned int), MEASURE_TASK, PULSE_RAW_SUBTASK);
             addComputeTaskFlag++;  // just add one. repetition has been delt with.
   }
   // Wrap up. clear the selections. notify that the compute task needs to be scheduled
   *(mData->measurementSelectionPtr) = 0;
   Serial.print((mData->prRawBufPtr[freshPulseCursor]));
   Serial.print("  MeasureTask Completes\n ");
   return; // get out
}


void computeTask(void* data){
   // TIMING MECH
   static unsigned long timer = 0;
   if (timer!=0 && (timeElapsed-timer)<SUSPENSION){
     return;
   }
   // It is our turn. do the compute. Then tell the scheduler to remove us
   timer = timeElapsed;
   Serial.print("\nComputeTask Starts\n");
   ComputeData* cData = (ComputeData*)data;
   // compute temp
   double tempCorrDump;
   requestAndReceive((char*)&(cData->tempRawBufPtr[freshTempCursor]),sizeof(unsigned int),
   (char*)&tempCorrDump, sizeof(double), COMPUTE_TASK, TEMP_RAW_SUBTASK);
   Serial.print(tempCorrDump);
   Serial.print(" is here\n");
   dtostrf(tempCorrDump, 1, 2, (char*)(cData->tempCorrectedBufPtr[freshTempCursor]));
   // compute systo
   unsigned int systoCorrDump;
   requestAndReceive((char*)&(cData->bpRawBufPtr[freshSBPCursor]),sizeof(unsigned int),
   (char*)&systoCorrDump, sizeof(unsigned int), COMPUTE_TASK, SYSTO_RAW_SUBTASK);
   sprintf((char*)(cData->bpCorrectedBufPtr[freshSBPCursor]), "%d", systoCorrDump);
   //compute diasto
   double bpDiastoCorrDump;
   requestAndReceive((char*)&(cData->bpRawBufPtr[freshDBPCursor]),sizeof(unsigned int),
   (char*)&bpDiastoCorrDump, sizeof(double), COMPUTE_TASK, DIASTO_RAW_SUBTASK);
   dtostrf(bpDiastoCorrDump, 1, 2, (char*)(cData->bpCorrectedBufPtr[freshDBPCursor]));
   //compute pulse
   unsigned int prCorrDump;
   requestAndReceive((char*)&(cData->prRawBufPtr[freshPulseCursor]),sizeof(unsigned int),
   (char*)&prCorrDump, sizeof(unsigned int), COMPUTE_TASK, PULSE_RAW_SUBTASK);
   sprintf((char*)(cData->prCorrectedBufPtr[freshPulseCursor]), "%d", prCorrDump);

   // done. now suicide
   addComputeTaskFlag = 0;
   Serial.print("nComputeTask Completes\n");
   return; // getout
}


void warningAlarmTask(void* data){
  // no timer is needed
  WarningAlarmData* wData = (WarningAlarmData*)data;
  // temp store
  char temporaryValue = 0;
  // temp
  requestAndReceive((char*)&(wData->tempRawBufPtr[freshTempCursor ]),sizeof(unsigned int), (char*)(wData->tempOutOfRangePtr),sizeof(unsigned char), ALARM_TASK , TEMP_RAW_SUBTASK );
  requestAndReceive((char*)&(wData->tempRawBufPtr[freshTempCursor ]),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , TEMP_RAW_SUBTASK );
  if (temporaryValue==0){
   *(wData->tempHighPtr) = FALSE;
  }else{
   *(wData->tempHighPtr) = TRUE;
  }
  // blood pressure
  Bool sysWarnResult;
  char sysAlarmResult;
  // two blood pressure types' two stuffs
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshSBPCursor]),sizeof(unsigned int), (char*)&sysAlarmResult,sizeof(unsigned char), ALARM_TASK , SYSTO_RAW_SUBTASK );
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshSBPCursor]),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , SYSTO_RAW_SUBTASK );
  if (temporaryValue==0){
    sysWarnResult = FALSE;
  }else{
    sysWarnResult = TRUE;
  }
  Bool diasWarnResult;
  char diasAlarmResult;
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshDBPCursor]),sizeof(unsigned int), (char*)&diasAlarmResult,sizeof(unsigned char), ALARM_TASK , DIASTO_RAW_SUBTASK );
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshDBPCursor]),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , DIASTO_RAW_SUBTASK );
  if (temporaryValue==0){
   diasWarnResult = FALSE;
  }else{
   diasWarnResult = TRUE;
  }
  *(wData->bpOutOfRangePtr) = sysAlarmResult || diasAlarmResult;
  if (sysWarnResult == FALSE && diasWarnResult == FALSE){
    *(wData->bpHighPtr) = FALSE;
  }else{
    *(wData->bpHighPtr) = TRUE;
  }
  // pulse
  requestAndReceive((char*)&(wData->prRawBufPtr[freshTempCursor ]),sizeof(unsigned int), (char*)(wData->pulseOutOfRangePtr),sizeof(unsigned char), ALARM_TASK , PULSE_RAW_SUBTASK  );
  requestAndReceive((char*)&(wData->prRawBufPtr[freshTempCursor ]),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , PULSE_RAW_SUBTASK  );
  if (temporaryValue==0){
   *(wData->pulseLowPtr) = FALSE;
  }else{
   *(wData->pulseLowPtr) = TRUE;
  }
  // NOW, lets determine what will be going on here
  // temperature
  if (*(wData->tempOutOfRangePtr)){
    // out
    *(wData->tempColorPtr) = ORANGE;
  }else{
    // good
    *(wData->tempColorPtr) = GREEN;
  }
  // general pressure
  if (*(wData->bpOutOfRangePtr)){
    // out
    *(wData->systoColorPtr) = ORANGE;
    *(wData->diastoColorPtr) = ORANGE;
  }else{
    // good
    *(wData->systoColorPtr) = GREEN;
    *(wData->diastoColorPtr) = GREEN;
  }
  // pulse rate
  if (*(wData->pulseOutOfRangePtr)){
    // out
    *(wData->pulseColorPtr) = ORANGE;
  }else{
    // good
    *(wData->pulseColorPtr) = GREEN;
  }
  // Special Case for Systolic
  if ((!sysAlarmResult) && (sysWarnResult == FALSE)){
    // it is actually ok now
    *(wData->systoColorPtr) = GREEN;
    *(wData->ackReceived) = 0;
    *(wData->mCountPtr) = 0;
  } else {
    if (sysAlarmResult){
      // it is at least out of range
      if (*(wData->mCountPtr)>6){
        // wait, it has been out for 5 measurements
        *(wData->systoColorPtr) = RED;
      }else{
        // it is still fine now
        *(wData->systoColorPtr) = ORANGE;

      }
    }
    if (sysWarnResult == TRUE){
      // we are too high. have we received an ack yet?
      if (*(wData->ackReceived)){
        // yes. so lets get the mCount going
        if (*(wData->mCountPtr) == 0){
          // if it was zero, start it. otherwise, it has started so leave it be.
          *(wData->mCountPtr) = 1;
        }
        // make it the same fashion as out of range. so thats it
      }else{
        // not acked. make it red
        *(wData->systoColorPtr) = RED;
      } 
    }
  }
  return; // done. get out
}



/******************************************
* function name: displayTask
* function inputs: a pointer to the DisplayData
* function outputs: None
* function description: This function will display
*             all the data in Mega onto TFT
* author: Matt & Sabrina
******************************************/ 
void displayTask(void* data){
   // no need for the timer

   DisplayData* dData = (DisplayData*)data;
   // render the bottom buttons first base on mode selection
   for (int i = 0; i < 4; i++) {
     if (*(dData->modeSelection) == i ){
       button[i].drawButton(true);
     }else{
       button[i].drawButton(false);
     }
   }
   // now base on mode selection. decide which content to show.
   if (*(dData->modeSelection) == 0){
       // display menu
       unsigned int selections = *(dData->measurementSelectionPtr);
       // render the buttons 
       if (selections & TEMP_SCEHDULED){
          //render temp
         menuButton[0].drawButton(true);
       }else{
         menuButton[0].drawButton(false);
       }
       if (selections & SYSTO_SCHEDULED){
          //render temp
         menuButton[1].drawButton(true);
       }else{
         menuButton[1].drawButton(false);
       }
       if (selections & DIASTO_SCHEDULED){
          //render temp
         menuButton[2].drawButton(true);
       }else{
         menuButton[2].drawButton(false);
       }
       if (selections & PULSE_SCHEDULED){
          //render temp
         menuButton[3].drawButton(true);
       }else{
         menuButton[3].drawButton(false);
       }
   }else if (*(dData->modeSelection) == 1) {
       // display annunc stuffs
       tft.setCursor(0,0);
                 tft.println(" ");

       // Display Temperature
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" Temperature:");
       // Show the Temprature
       tft.setTextColor(*(dData->tempColorPtr),BACKGROUND_COLOR);
       tft.print((char*)(dData->tempCorrectedBufPtr[freshTempCursor]));
       int diff = TEMP_DISP_WIDTH - strlen((char*)(dData->tempCorrectedBufPtr[freshTempCursor]));
       for (int i = 0; i<diff; i++){
          tft.print(" ");
       }
       tft.print(" C\n");

       // Display Blood Pressure
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.println(" Pressure:");
       tft.print("   Systolic: ");
       tft.setTextColor(*(dData->systoColorPtr),BACKGROUND_COLOR);
       tft.print((char*)(dData->bpCorrectedBufPtr[freshSBPCursor]));
       tft.print(".00");
       diff = SYS_PRESS_DISP_WIDTH-strlen((char*)(dData->bpCorrectedBufPtr[freshSBPCursor]));
       for (int i = 0; i<diff; i++){
          tft.print(" ");
       }
       tft.println(" mm Hg");
       // Display Diastolic

       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print("   Diastolic:");
       // Show the Dias Pressure
       tft.setTextColor(*(dData->diastoColorPtr),BACKGROUND_COLOR);
       tft.print((char*)(dData->bpCorrectedBufPtr[freshDBPCursor]));
       diff = DIAS_PRESS_DISP_WIDTH-strlen((char*)(dData->bpCorrectedBufPtr[freshDBPCursor]));
       for (int i = 0; i<diff; i++){
         tft.print(" ");
       }
       tft.println(" mm Hg");
       // Display Pulse
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" Pulse rate: ");
       // Show the PulseRate
       tft.setTextColor(*(dData->pulseColorPtr),BACKGROUND_COLOR);
       tft.print((char*)(dData->prCorrectedBufPtr[freshPulseCursor]));
       diff = PULSE_PRESS_DISP_WIDTH-strlen((char*)(dData->prCorrectedBufPtr[freshPulseCursor]));
       for (int i = 0; i<diff; i++){
         tft.print(" ");
       }
       tft.println(" BPM");

       // Battery
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" Battery:     ");
       // Figure out the color
       if (*(dData->batteryState) <= BATTERY_LIMIT){
          tft.setTextColor(RED, BACKGROUND_COLOR);
       }else{
          tft.setTextColor(GREEN, BACKGROUND_COLOR);
       }
       char batteryStateBuffer[9];
       sprintf(batteryStateBuffer, "%d  ",*(dData->batteryState));
       tft.print(batteryStateBuffer);
       // render the ackButton
       if (*(dData->ackReceived)){
         ackButton.drawButton(true);
       }else{
         ackButton.drawButton(false);
       }
       // Thats all
   }

   return;
}


void keypadTask(void* data){
  static unsigned long timer = 0;
   if (timer!=0 && (timeElapsed-timer)<KEYPAD_SCAN_INTERVAL){  // scan every 20ms
     return;
   }
   timer = timeElapsed;
   KeypadData* kData = (KeypadData*) data;
   // now get the pressure point
   digitalWrite(13, HIGH); 
   TSPoint p = ts.getPoint(); 
   digitalWrite(13, LOW); 
   pinMode(XM, OUTPUT); 
   pinMode(YP, OUTPUT); 
   if (p.z > MINPRESSURE && p.z < MAXPRESSURE) { 
    // touch recevied
    int scaleFactor = 1000;
    if (*(kData->modeSelection) == 0){
      // menu. use 950
      scaleFactor = 950;
    }else{
      scaleFactor = 1000;
    }
    int x2 = (int)floor((scaleFactor-p.y)*1.0/scaleFactor*320.0);
    int y2 = (int)floor((scaleFactor-p.x)*1.0/scaleFactor*250.0);
    // check if the mode button is clicked
    for (int i = 0; i < 2; i++) {
    if (button[i].contains(x2, y2)) {
      // one thing has been selected
      tft.fillScreen(BACKGROUND_COLOR);
      if(button[i].contains(x2, y2)) { 
        // this button is selected
        *(kData->modeSelection) = i;
      } 
     }
    }
     if ((*(kData->modeSelection) == 0) && *(kData->measurementSelectionPtr) != 0){
      //do nothing
      return;
    }
    // check if the menu select button is clicked
    if ((*(kData->modeSelection) == 0) && (menuButton[0].contains(x2,y2))){
      // temp selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | TEMP_SCEHDULED;
    }else
    if ((*(kData->modeSelection) == 0) && (menuButton[1].contains(x2,y2))){
      // systo selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | SYSTO_SCHEDULED;
    }else
    if ((*(kData->modeSelection) == 0) && (menuButton[2].contains(x2,y2))){
      // diasto selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | DIASTO_SCHEDULED;
    }else
    if ((*(kData->modeSelection) == 0) && (menuButton[3].contains(x2,y2))){
      // pulse selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | PULSE_SCHEDULED;
    }else
    // check if the ack button is clicked
    if ((*(kData->modeSelection) == 1) && (ackButton.contains(x2,y2))){
      //acked
      *(kData->ackReceived) = 1;
    }
   }
}


void statusTask(void* data){
    // TIMING MECH
   static unsigned long timer = 0;
   if (timer!=0 && (timeElapsed-timer)<SUSPENSION){
     return;
   }
   // It is our turn. get the status
   timer = timeElapsed;
   Serial.print("For Status--- \n");
   StatusData* statusData = (StatusData*)data;
   requestAndReceive((char*)(statusData->batteryState),sizeof(unsigned short), (char*)(statusData->batteryState),sizeof(unsigned short), STATUS_TASK , STATUS_TASK );
   if (*(statusData->batteryState)==0){
    *(statusData->batteryState) = FULL_BATTERY;  // Magical Recharge
   }
   Serial.println(" Finished\n");
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
*         char to represent input buffer length
*         char* for output buffer,
*         char to represent output buffer length
*           char to represent task type,
*         and char to represent which variable to
*         perform task on.
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
