#include <stdio.h>
#include <stdlib.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>   // Touch Screen for TFT
#include <optfft.h> // FFT library
#include <math.h> // Sine and PI

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
#define RESP_RAW_SUBTASK 5
#define EKG_RAW_SUBTASK 6
#define TEMP_DISP_WIDTH 5
#define SYS_PRESS_DISP_WIDTH 3
#define DIAS_PRESS_DISP_WIDTH 6
#define PULSE_PRESS_DISP_WIDTH 3
#define RESP_DISP_WIDTH 3
#define TEMP_SCHEDULED 1
#define BP_SCHEDULED 2
#define PULSE_SCHEDULED 4
#define RESP_SCHEDULED 8
#define EKG_SCHEDULED 16
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
#define NEUTRAL 0
#define START 1
#define STOP 2


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
  unsigned int* rrRawBufPtr;
  unsigned int* ekgRawBufPtr;
  unsigned int* measurementSelectionPtr;
  unsigned char* mCountPtr; 
} MeasureData; 

typedef struct
{
   unsigned int* tempRawBufPtr;
  unsigned int* bpRawBufPtr;
  unsigned int* prRawBufPtr;
  unsigned int* rrRawBufPtr;
  unsigned int* ekgFreqBufPtr;
   Bool* tempHighPtr;
  unsigned char* bpOutOfRangePtr;
  Bool* bpHighPtr;
  unsigned char* pulseOutOfRangePtr;
  Bool* pulseLowPtr;
  unsigned char* respOutOfRangePtr;
  Bool* rrLowPtr;
  Bool* rrHighPtr;
  unsigned char* ekgOutOfRangePtr;
  Bool* ekgLowPtr;
  Bool* ekgHighPtr;
} RemComData; 

typedef struct
{
  unsigned int* tempRawBufPtr;
  unsigned int* bpRawBufPtr;
  unsigned int* prRawBufPtr;
  unsigned int* rrRawBufPtr;
  unsigned int* ekgRawBufPtr;
  unsigned char** tempCorrectedBufPtr;
  unsigned char** bpCorrectedBufPtr;
  unsigned char** prCorrectedBufPtr;
  unsigned char** rrCorrectedBufPtr;  
  unsigned int* ekgFreqBufPtr;
} ComputeData; 


typedef struct
{
  unsigned int* tempRawBufPtr;
  unsigned int* bpRawBufPtr;
  unsigned int* prRawBufPtr;
  unsigned int* rrRawBufPtr;
  unsigned int* ekgRawBufPtr;
  unsigned short* batteryState;
  unsigned char* tempOutOfRangePtr;
  Bool* tempHighPtr;
  unsigned char* bpOutOfRangePtr;
  Bool* bpHighPtr;
  unsigned char* pulseOutOfRangePtr;
  Bool* pulseLowPtr;
  unsigned char* respOutOfRangePtr;
  Bool* rrLowPtr;
  Bool* rrHighPtr;
  unsigned char* ekgOutOfRangePtr;
  Bool* ekgLowPtr;
  Bool* ekgHighPtr;
  unsigned char* ackReceived;
  unsigned int* tempColorPtr;
  unsigned int* systoColorPtr;
  unsigned int* diastoColorPtr;
  unsigned int* pulseColorPtr;
  unsigned int* respColorPtr;
  unsigned char* mCountPtr; 
} WarningAlarmData;


//DisplayData
typedef struct
{
  unsigned char** tempCorrectedBufPtr;
  unsigned char** bpCorrectedBufPtr;
  unsigned char** prCorrectedBufPtr;  
  unsigned char** rrCorrectedBufPtr;  
  unsigned int* ekgFreqBufPtr;
  unsigned short* batteryState;
  unsigned int* tempColorPtr;
  unsigned int* systoColorPtr;
  unsigned int* diastoColorPtr;
  unsigned int* pulseColorPtr;
  unsigned int* respColorPtr;
  unsigned int* ekgColorPtr;
  unsigned char* modeSelection;
  unsigned int* measurementSelectionPtr;  
  unsigned char* ackReceived;
} DisplayData;


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
void toTerminal(unsigned int inputBuffer, char taskType, char subTaskType);
// TCB executer functions
void executeTCB(TCB* taskControlBlock);
// TaskQueue Management
void insertTask(TCB* task);
void deleteTask(TCB* task);
char compareData(unsigned int oldData, unsigned int newData);
// TFT Related
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP ,XM , YM, 300);



// Globals for keypad and ack
unsigned char ackReceived[4];
unsigned char mCount[4]; 
unsigned char modeSelection = 1;  // 0 means menu   1 anounciation
// Global Variables for Measurements
unsigned int temperatureRawBuf[8] = {75,0,0,0,0,0,0,0};
unsigned int bloodPressureRawBuf[16] = {80,0,0,0,0,0,0,0,80,0,0,0,0,0,0,0};
unsigned int pulseRateRawBuf[8] = {0,0,0,0,0,0,0,0};
unsigned int respirationRateRawBuf[8] = {0,0,0,0,0,0,0,0};
unsigned int ekgRawBuf[256];
unsigned int measurementSelection = 0; // this is using a bit mask capability. See MeasureTask function for detail 
unsigned int remoteScheduled = 0;
// Initial Values for Compute
const char* initialTempDisplay = "61.25";
const char* initialSystoDisplay = "169";
const char* initialDiastoDisplay = "126.00";
const char* initialPulseDisplay = "8";
const char* initialRespDisplay = "7";
// Global Variables for Display
unsigned char* tempCorrectedBuf[8];
unsigned char* bloodPressureCorrectedBuf[16];
unsigned char* pulseRateCorrectedBuf[8];
unsigned char* respirationRateCorrectedBuf[8];
unsigned int ekgFreqBuf[16];
unsigned int tempColor = GREEN;
unsigned int systoColor = GREEN;
unsigned int diastoColor = GREEN;
unsigned int pulseColor = GREEN;
unsigned int respColor = GREEN;
unsigned int ekgColor = GREEN;
Elegoo_GFX_Button button[2];
Elegoo_GFX_Button menuButton[5];
Elegoo_GFX_Button ackButton;
// Global Variables for Function Counter
unsigned char freshTempCursor = 0;
unsigned char freshSBPCursor = 0;
unsigned char freshDBPCursor = 8;
unsigned char freshPulseCursor = 0;
unsigned char freshRespCursor = 0;
unsigned char freshEKGCursor = 0;
// Global Variables for Status
unsigned short batteryState = FULL_BATTERY;
// Global Variables for Alarm
unsigned char bpOutOfRange = 0;
unsigned char tempOutOfRange = 0;
unsigned char pulseOutOfRange = 0;
unsigned char respOutOfRange = 0;
unsigned long flasherTimer[4];
//Global Variables for Warning
Bool bpHigh = FALSE;
Bool tempHigh = FALSE;
Bool pulseLow = FALSE;
Bool rrLow = FALSE;
Bool rrHigh = FALSE;
Bool ekgLow = FALSE;
Bool ekgHigh = FALSE;
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
unsigned int flasherIndicator = 0;
// Used for Remote stuffs. 
unsigned char remoteOn = 0;
unsigned char displayOn = 1;
unsigned char commanderMode = NEUTRAL;
unsigned char measureCursor = 0;
unsigned int samplingFreq;
double samplingRate;


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
void startUpTask() {   
   //Setup the TFT display
   tft.reset();
   uint16_t identifier = tft.readID();
   if(identifier == 0x9325) {
     //Serial.println(F("Found ILI9325 LCD driver"));
   } else if(identifier == 0x9328) {
     //Serial.println(F("Found ILI9328 LCD driver"));
   } else if(identifier == 0x4535) {
     //Serial.println(F("Found LGDP4535 LCD driver"));
   }else if(identifier == 0x7575) {
     //Serial.println(F("Found HX8347G LCD driver"));
   } else if(identifier == 0x9341) {
     //Serial.println(F("Found ILI9341 LCD driver"));
   } else if(identifier == 0x8357) {
     //Serial.println(F("Found HX8357D LCD driver"));
   } else if(identifier==0x0101)
   {
       identifier=0x9341;
       //Serial.println(F("Found 0x9341 LCD driver"));
   }
   else if(identifier==0x1111)
   {
       identifier=0x9328;
       //Serial.println(F("Found 0x9328 LCD driver"));
   }
   else {
     //Serial.print(F("Unknown LCD driver chip: "));
     //Serial.println(identifier, HEX);
     //Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
     //Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
     //Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
     //Serial.println(F("If using the breakout board, it should NOT be #defined!"));
     //Serial.println(F("Also if using the breakout, double-check that all wiring"));
     //Serial.println(F("matches the tutorial."));
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
   measureData.rrRawBufPtr = &respirationRateRawBuf[0];
   measureData.ekgRawBufPtr = &ekgRawBuf[0];
   measureData.measurementSelectionPtr = &measurementSelection;
   measureData.mCountPtr = mCount;
   freshTempCursor = 0;
   freshSBPCursor = 0;
   freshDBPCursor = 8;
   freshPulseCursor = 0;
   freshRespCursor = 0;
   mCount[0] = 0; mCount[1]=0; mCount[2]=0; mCount[3]=0;
   ackReceived[0] = 0; ackReceived[1]=0; ackReceived[2]=0; ackReceived[3]=0;
   flasherTimer[0] = 0; flasherTimer[1]=0; flasherTimer[2]=0; flasherTimer[3]=0;


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
   computeData.rrRawBufPtr = &respirationRateRawBuf[0];
   computeData.ekgRawBufPtr = &ekgRawBuf[0];
   computeData.tempCorrectedBufPtr = tempCorrectedBuf;
   computeData.bpCorrectedBufPtr = bloodPressureCorrectedBuf;
   computeData.prCorrectedBufPtr = pulseRateCorrectedBuf;
   computeData.rrCorrectedBufPtr = respirationRateCorrectedBuf;
   computeData.ekgFreqBufPtr = &ekgFreqBuf[0];
   for (int i = 0; i < 8; i++) {
     tempCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   for (int i = 0; i < 16; i++) {
     bloodPressureCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   for (int i = 0; i < 8; i++) {
     pulseRateCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   for (int i = 0; i < 8; i++) {
     respirationRateCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   strcpy((char*)tempCorrectedBuf[0], initialTempDisplay);
   strcpy((char*)bloodPressureCorrectedBuf[0], initialSystoDisplay);
   strcpy((char*)bloodPressureCorrectedBuf[8], initialDiastoDisplay);
   strcpy((char*)pulseRateCorrectedBuf[0], initialPulseDisplay);
   strcpy((char*)respirationRateCorrectedBuf[0], initialRespDisplay);

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
   warningAlarmData.rrRawBufPtr = &respirationRateRawBuf[0];
   warningAlarmData.tempOutOfRangePtr = &tempOutOfRange;
   warningAlarmData.tempHighPtr = &tempHigh;
   warningAlarmData.bpOutOfRangePtr = &bpOutOfRange;
   warningAlarmData.bpHighPtr = &bpHigh;
   warningAlarmData.pulseOutOfRangePtr = &pulseOutOfRange;
   warningAlarmData.pulseLowPtr = &pulseLow;
   warningAlarmData.respOutOfRangePtr = &respOutOfRange;
   warningAlarmData.rrLowPtr = &rrLow;
   warningAlarmData.rrHighPtr = &rrHigh;
   warningAlarmData.ekgLowPtr = &ekgLow;
   warningAlarmData.ekgHighPtr = &ekgHigh;
   warningAlarmData.batteryState = &batteryState;
   warningAlarmData.ackReceived = ackReceived;
   warningAlarmData.tempColorPtr = &tempColor;
   warningAlarmData.systoColorPtr = &systoColor;
   warningAlarmData.diastoColorPtr = &diastoColor;
   warningAlarmData.pulseColorPtr = &pulseColor;
   warningAlarmData.respColorPtr = &respColor;
   warningAlarmData.mCountPtr = mCount;

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
   displayData.rrCorrectedBufPtr = respirationRateCorrectedBuf;
   displayData.ekgFreqBufPtr = &ekgFreqBuf[0];
   displayData.batteryState = &batteryState;
   displayData.tempColorPtr = &tempColor;
   displayData.systoColorPtr = &systoColor;
   displayData.diastoColorPtr = &diastoColor;
   displayData.pulseColorPtr = &pulseColor;
   displayData.respColorPtr = &respColor;
   displayData.ekgColorPtr = &ekgColor;
   displayData.modeSelection = &modeSelection;
   displayData.measurementSelectionPtr = &measurementSelection;  
   displayData.ackReceived = ackReceived;

   // TCB:
   TCB displayTaskControlBlock;
   displayTaskControlBlock.myTask = displayTask;
   displayTaskControlBlock.taskDataPtr = (void*)&displayData;
   displayTaskControlBlock.next = NULL;
   displayTaskControlBlock.prev = NULL;
   displayTCB = &displayTaskControlBlock;

   // Buttons
   char ButtonText[2][11] = {"Menu",  "Annun"};
   for (int j = 0; j < 2; j++) {
      button[j].initButton(&tft, 80 + (160*j), 177 + (40), BUTTON_W + 20, BUTTON_H, BLACK, BLACK, WHITE, ButtonText[j], 2);
   }
   char MenuText[5][11] = {"Temp", "BP", "Pulse", "Resp", "EKG"};
   for (int i = 0; i < 5; i++) { 
            menuButton[i].initButton(&tft, 155,  30 + (37 * i), BUTTON_W + 120, BUTTON_H - 5, BLUE, BLUE, WHITE, MenuText[i], 2); 
   }
   char AckText[1][4] = {"Ack"};
   ackButton.initButton(&tft, 150, 140+40, BUTTON_W, BUTTON_H - 10, MAGENTA, MAGENTA, WHITE, AckText[0], 2);

   // 5. KeyPad
   // data:
   KeypadData keypadData;
   keypadData.measurementSelectionPtr = &measurementSelection;  
   keypadData.ackReceived = ackReceived;
   keypadData.modeSelection = &modeSelection;
   // TCB:
   TCB keypadTaskControlBlock;
   keypadTaskControlBlock.myTask = keypadTask;
   keypadTaskControlBlock.taskDataPtr = (void*)&keypadData;
   keypadTaskControlBlock.next = NULL;
   keypadTaskControlBlock.prev = NULL;
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
   statusTaskControlBlock.prev = NULL;
   statusTCB = &statusTaskControlBlock;

   // 6.5 Remote Com
   RemComData remData;
   remData.tempRawBufPtr = &temperatureRawBuf[0];
   remData.bpRawBufPtr = &bloodPressureRawBuf[0];;
   remData.prRawBufPtr = &pulseRateRawBuf[0];
   remData.rrRawBufPtr = &respirationRateRawBuf[0];
   remData.ekgFreqBufPtr = &ekgFreqBuf[0];
   // TCB:
   TCB remoteComTCB;
   remoteComTCB.myTask = remoteComTask;
   remoteComTCB.taskDataPtr = (void*)&remData;
   remoteComTCB.next = NULL;
   remoteComTCB.prev = NULL;

   // 8. Schedule
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
   insertTask(&remoteComTCB);

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
ISR(TIMER2_OVF_vect) {
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
void scheduleTask(void* data) {
   SchedulerData* schedulerData = (SchedulerData*)data;
   // pick the first task to run.
   TCB* currTask = schedulerData->head;
   // forever execute each task
   while(1) {
    // check if we need to add the compute 
    if (addComputeTaskFlag == 0) {
      // compute task is completed and we can remove it now.
      deleteTask(computeTCB);  // deleteTask has protection and check if computeTCB is removed already.
    }else if (addComputeTaskFlag == 1) {
      // compute task has been added and scheduled by measureTask. add it in
            //Serial.print("add compute");
      insertTask(computeTCB);
      // increment the flag to be neutral
      addComputeTaskFlag++;
    } else {
      // neutral state
      // we have already added a compute task. don't add it again. wait for it to finish.
    }
    if (currTask ==  NULL) {
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
void insertTask(TCB* task) {
  TCB* currHead = schedulerTaskQueue->head;
  TCB* currTail = schedulerTaskQueue->tail;
  if (NULL == currHead) {
    // this is the first task ever,
    schedulerTaskQueue->head = task;
    schedulerTaskQueue->tail = task;
  } else {
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
void deleteTask(TCB* task) {
  TCB* currHead = schedulerTaskQueue->head;
  TCB* currTail = schedulerTaskQueue->tail;
  if (currHead == NULL) {
  } else if (currHead == currTail && task == currHead) {
    // deleting the only one node
    schedulerTaskQueue->head = NULL;
    schedulerTaskQueue->tail = NULL;
  } else if (task == currHead) { // so we have more than one tasks
    // we are deleteing the head. promote its next to be head
    schedulerTaskQueue->head = task->next;
  } else if (task == currTail) {
    // we are deleting the tail. drag the prev to be tail.
    TCB* itsPrev = task->prev;
    schedulerTaskQueue->tail = task->prev;
    itsPrev->next = NULL;
  } else {
    // the task is in the middle
    if (task->next == NULL && task->prev == NULL) {
      // this task is not even in the queue
      return;
    }
    // connect the prev and the next
    TCB* itsPrev = task->prev;
    TCB* itsNext = task->next;
    itsPrev->next = itsNext;
    itsNext->prev = itsPrev;
  }
  // finall, clean the task
  task->next = NULL;
  task->prev = NULL;
}

void measureTask(void* data) {
   // TIMING MECH
   if (commanderMode == STOP){
    return;
   }    
   static unsigned long timer = 0;
   if (timer!=0 && (timeElapsed-timer)<SUSPENSION) {
     return;
   }
   // It is our turn.
   timer = timeElapsed;
   //Serial.print("\nMeasureTask Starts\n");
   MeasureData* mData = (MeasureData*)data;
   unsigned int selections = *(mData->measurementSelectionPtr);
   
   // run the measurement that has been scheduled.
   if (selections & TEMP_SCHEDULED) {
     // measure temperature
     unsigned char oldTempCursor = freshTempCursor;
     freshTempCursor = (freshTempCursor + 1) % 8;
     unsigned int oldTempData = mData->tempRawBufPtr[freshTempCursor];
     requestAndReceive((char*)&(mData->tempRawBufPtr[oldTempCursor]), sizeof(unsigned int), 
     (char*)&(mData->tempRawBufPtr[freshTempCursor]),sizeof(unsigned int), MEASURE_TASK, TEMP_RAW_SUBTASK);
     if(compareData(mData->tempRawBufPtr[oldTempCursor], mData->tempRawBufPtr[freshTempCursor])) {
        mData->tempRawBufPtr[freshTempCursor] = oldTempData;
        freshTempCursor = (freshTempCursor - 1) % 8;
        //Serial.print("Rolled Back\n");
     }
     if (mData->mCountPtr[0] > 0) {
        // alarm is expecting us to count how many times we are called.
        mData->mCountPtr[0] = mData->mCountPtr[0] + 1;
     }
     addComputeTaskFlag++;  // just add one. repetition has been delt with.
     remoteScheduled = remoteScheduled | TEMP_SCHEDULED;
   }

   // Measure Blood Pressure
   if (selections & BP_SCHEDULED) {
     // Measure Systolic
     unsigned char oldSBPCursor  = freshSBPCursor;
     freshSBPCursor  = (freshSBPCursor  + 1) % 8;
     requestAndReceive((char*)&(mData->bpRawBufPtr[oldSBPCursor]), sizeof(unsigned int), 
     (char*)&(mData->bpRawBufPtr[freshSBPCursor]), sizeof(unsigned int), MEASURE_TASK, SYSTO_RAW_SUBTASK);
     // Measure Diastolic
     unsigned char oldDBPCursor  = freshDBPCursor;
     freshDBPCursor  = ((freshDBPCursor + 1) % 8) + 8;
     requestAndReceive((char*)&(mData->bpRawBufPtr[oldDBPCursor]), sizeof(unsigned int), 
     (char*)&(mData->bpRawBufPtr[freshDBPCursor]), sizeof(unsigned int), MEASURE_TASK, DIASTO_RAW_SUBTASK);
     // do the systo alarm increment
     if (mData->mCountPtr[2] > 0) {
        // alarm is expecting us to count how many times we are called.
        mData->mCountPtr[2] = mData->mCountPtr[2] + 1;
     }
     addComputeTaskFlag++;  // just add one. repetition has been delt with.
     remoteScheduled = remoteScheduled | BP_SCHEDULED;
   }

   // Measure Pulse Rate
   if (selections & PULSE_SCHEDULED) {
     // Measure Pulse rate
     unsigned char oldPulseCursor  = freshPulseCursor;
     freshPulseCursor  = (freshPulseCursor + 1) % 8;
     unsigned int oldPulseData = mData->prRawBufPtr[freshPulseCursor];
     requestAndReceive((char*)&(mData->prRawBufPtr[oldPulseCursor]), sizeof(unsigned int), 
     (char*)&(mData->prRawBufPtr[freshPulseCursor]),sizeof(unsigned int), MEASURE_TASK, PULSE_RAW_SUBTASK);
     if(compareData(mData->prRawBufPtr[oldPulseCursor], mData->prRawBufPtr[freshPulseCursor])) {
        mData->prRawBufPtr[freshPulseCursor] = oldPulseData;
        freshPulseCursor = (freshPulseCursor - 1) % 8;
        //Serial.print("Rolled Back\n");
     }
     if (mData->mCountPtr[1] > 0) {
        // alarm is expecting us to count how many times we are called.
        mData->mCountPtr[1] = mData->mCountPtr[1] + 1;
     }
     addComputeTaskFlag++;  // just add one. repetition has been delt with.
     remoteScheduled = remoteScheduled | PULSE_SCHEDULED;
   }

   // Measure Respiration Rate
   if (selections & RESP_SCHEDULED) {
     // Mesure Respiration rate
     unsigned char oldRespCursor  = freshRespCursor;
     freshRespCursor  = (freshRespCursor + 1) % 8;
     unsigned int oldRespData = mData->rrRawBufPtr[freshRespCursor];
     requestAndReceive((char*)&(mData->rrRawBufPtr[oldRespCursor]), sizeof(unsigned int), 
     (char*)&(mData->rrRawBufPtr[freshRespCursor]),sizeof(unsigned int), MEASURE_TASK, RESP_RAW_SUBTASK);
     if(compareData(mData->rrRawBufPtr[oldRespCursor], mData->rrRawBufPtr[freshRespCursor])) {
        mData->rrRawBufPtr[freshRespCursor] = oldRespData;
        freshRespCursor = (freshRespCursor - 1) % 8;
       // Serial.print("Rolled Back\n");
     }
     if (mData->mCountPtr[3] > 0) {
        // alarm is expecting us to count how many times we are called.
        mData->mCountPtr[3] = mData->mCountPtr[3] + 1;
     }
     addComputeTaskFlag++;  // just add one. repetition has been dealt with.
     remoteScheduled = remoteScheduled | RESP_SCHEDULED;
   }

   // Measure ekg
   if (selections & EKG_SCHEDULED) {
      samplingFreq = 500;
      samplingRate = 1.0 / (3.0 * samplingFreq);
      double t = 0;
      for (int i = 0; i < 256; i++) {
        ekgRawBuf[i] = (int)(10.0 * sin(2 * PI * samplingFreq * t));
        t = samplingRate + t;
      }
      addComputeTaskFlag++;  // just add one. repetition has been dealt with.
      remoteScheduled = remoteScheduled | EKG_SCHEDULED;
   }
   
   // Wrap up. clear the selections. notify that the compute task needs to be scheduled
   *(mData->measurementSelectionPtr) = 0;
   //Serial.print((mData->rrRawBufPtr[freshPulseCursor]));
   //Serial.print("  MeasureTask Completes\n ");
   return; // get out
}


void computeTask(void* data) {
   // TIMING MECH
   static unsigned long timer = 0;
   if (timer != 0 && (timeElapsed-timer) < SUSPENSION) {
     return;
   }
   
   // It is our turn. do the compute. Then tell the scheduler to remove us
   timer = timeElapsed;
   ComputeData* cData = (ComputeData*)data;
   
   // Compute temp
   double tempCorrDump;
   requestAndReceive((char*)&(cData->tempRawBufPtr[freshTempCursor]),sizeof(unsigned int),
   (char*)&tempCorrDump, sizeof(double), COMPUTE_TASK, TEMP_RAW_SUBTASK);
   dtostrf(tempCorrDump, 1, 2, (char*)(cData->tempCorrectedBufPtr[freshTempCursor]));
   
   // Compute Systo
   unsigned int systoCorrDump;
   requestAndReceive((char*)&(cData->bpRawBufPtr[freshSBPCursor]),sizeof(unsigned int),
   (char*)&systoCorrDump, sizeof(unsigned int), COMPUTE_TASK, SYSTO_RAW_SUBTASK);
   sprintf((char*)(cData->bpCorrectedBufPtr[freshSBPCursor]), "%d", systoCorrDump);
   
   //Compute Diasto
   double bpDiastoCorrDump;
   requestAndReceive((char*)&(cData->bpRawBufPtr[freshDBPCursor]),sizeof(unsigned int),
   (char*)&bpDiastoCorrDump, sizeof(double), COMPUTE_TASK, DIASTO_RAW_SUBTASK);
   dtostrf(bpDiastoCorrDump, 1, 2, (char*)(cData->bpCorrectedBufPtr[freshDBPCursor]));
   
   //Compute Pulse
   unsigned int prCorrDump;
   requestAndReceive((char*)&(cData->prRawBufPtr[freshPulseCursor]),sizeof(unsigned int),
   (char*)&prCorrDump, sizeof(unsigned int), COMPUTE_TASK, PULSE_RAW_SUBTASK);
   sprintf((char*)(cData->prCorrectedBufPtr[freshPulseCursor]), "%d", prCorrDump);
   
   //Compute Resp
   unsigned int rrCorrDump;
   requestAndReceive((char*)&(cData->rrRawBufPtr[freshRespCursor]),sizeof(unsigned int),
   (char*)&rrCorrDump, sizeof(unsigned int), COMPUTE_TASK, RESP_RAW_SUBTASK);
   sprintf((char*)(cData->rrCorrectedBufPtr[freshRespCursor]), "%d", rrCorrDump);

   ekgFreqBuf[freshEKGCursor] = computeEKG();
   freshEKGCursor++;

   
   // Done. now suicide
   addComputeTaskFlag = 0;
   return;
}


void warningAlarmTask(void* data) {
  // No timer is needed
  WarningAlarmData* wData = (WarningAlarmData*)data;
  // Temp storage
  char temporaryValue = 0;
  
  // Temp
  requestAndReceive((char*)&(wData->tempRawBufPtr[freshTempCursor]),sizeof(unsigned int), (char*)(wData->tempOutOfRangePtr),sizeof(unsigned char), ALARM_TASK , TEMP_RAW_SUBTASK );
  requestAndReceive((char*)&(wData->tempRawBufPtr[freshTempCursor]),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , TEMP_RAW_SUBTASK );
  if (temporaryValue==0) {
   *(wData->tempHighPtr) = FALSE;
  } else {
   *(wData->tempHighPtr) = TRUE;
  }
  
  // Blood Pressure
  Bool sysWarnResult;
  char sysAlarmResult;
  // two blood pressure types' two stuffs
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshSBPCursor]),sizeof(unsigned int), (char*)&sysAlarmResult,sizeof(unsigned char), ALARM_TASK, SYSTO_RAW_SUBTASK);
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshSBPCursor]),sizeof(unsigned int), &temporaryValue,sizeof(unsigned char), WARN_TASK , SYSTO_RAW_SUBTASK);
  if (temporaryValue==0) {
    sysWarnResult = FALSE;
  } else {
    sysWarnResult = TRUE;
  }
  Bool diasWarnResult;
  char diasAlarmResult;
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshDBPCursor]),sizeof(unsigned int), (char*)&diasAlarmResult, sizeof(unsigned char), ALARM_TASK, DIASTO_RAW_SUBTASK);
  requestAndReceive((char*)&(wData->bpRawBufPtr[freshDBPCursor]),sizeof(unsigned int), &temporaryValue, sizeof(unsigned char), WARN_TASK, DIASTO_RAW_SUBTASK);
  if (temporaryValue==0) {
   diasWarnResult = FALSE;
  } else {
   diasWarnResult = TRUE;
  }
  *(wData->bpOutOfRangePtr) = sysAlarmResult | diasAlarmResult;
  if (sysWarnResult == FALSE && diasWarnResult == FALSE) {
    *(wData->bpHighPtr) = FALSE;
  } else {
    *(wData->bpHighPtr) = TRUE;
  }
  
  // Pulse Rate
  requestAndReceive((char*)&(wData->prRawBufPtr[freshTempCursor]),sizeof(unsigned int), (char*)(wData->pulseOutOfRangePtr), sizeof(unsigned char), ALARM_TASK , PULSE_RAW_SUBTASK);
  requestAndReceive((char*)&(wData->prRawBufPtr[freshTempCursor]),sizeof(unsigned int), &temporaryValue, sizeof(unsigned char), WARN_TASK, PULSE_RAW_SUBTASK);
  if (temporaryValue==0) {
   *(wData->pulseLowPtr) = FALSE;
  } else {
   *(wData->pulseLowPtr) = TRUE;
  }
  
  // Respiration Rate
  requestAndReceive((char*)&(wData->rrRawBufPtr[freshTempCursor]),sizeof(unsigned int), (char*)(wData->respOutOfRangePtr), sizeof(unsigned char), ALARM_TASK, RESP_RAW_SUBTASK);
  requestAndReceive((char*)&(wData->rrRawBufPtr[freshTempCursor]),sizeof(unsigned int), &temporaryValue, sizeof(unsigned char), WARN_TASK , RESP_RAW_SUBTASK);
  if (temporaryValue==0) {
   *(wData->rrLowPtr) = FALSE;
  } else {
   *(wData->rrLowPtr) = TRUE;
  }

  //----------------------------------------------------------------------------------------------------------------
  
  // Temperature
  if (*(wData->tempOutOfRangePtr)) {
    *(wData->tempColorPtr) = ORANGE;
    if (*(wData->tempOutOfRangePtr) == 2){
      flasherIndicator = flasherIndicator | TEMP_SCHEDULED;
    } else {
      flasherIndicator = flasherIndicator & (~TEMP_SCHEDULED);
    }
  } else {
    *(wData->tempColorPtr) = GREEN;
    flasherIndicator = flasherIndicator & (~TEMP_SCHEDULED);
  }
  
  // Blood Pressure
  if (*(wData->bpOutOfRangePtr)) {
    *(wData->systoColorPtr) = ORANGE;
    *(wData->diastoColorPtr) = ORANGE;
    if (*(wData->bpOutOfRangePtr) >= 2){
      flasherIndicator = flasherIndicator | BP_SCHEDULED;
    } else {
      flasherIndicator = flasherIndicator & (~BP_SCHEDULED);
    }
  } else {
    *(wData->systoColorPtr) = GREEN;
    *(wData->diastoColorPtr) = GREEN;
    flasherIndicator = flasherIndicator & (~BP_SCHEDULED);
  }
  
  // Pulse Rate
  if (*(wData->pulseOutOfRangePtr)) {
    *(wData->pulseColorPtr) = ORANGE;
    if (*(wData->pulseOutOfRangePtr) == 2){
      flasherIndicator = flasherIndicator | PULSE_SCHEDULED;
    } else {
      flasherIndicator = flasherIndicator & (~PULSE_SCHEDULED);
    }
  } else {
    *(wData->pulseColorPtr) = GREEN;
    flasherIndicator = flasherIndicator & (~PULSE_SCHEDULED);
  }
  
  // Respiration Rate
  if (*(wData->respOutOfRangePtr)) {
    *(wData->respColorPtr) = ORANGE;
    if (*(wData->respOutOfRangePtr) == 2){
      flasherIndicator = flasherIndicator | RESP_SCHEDULED;
    } else {
      flasherIndicator = flasherIndicator & (~RESP_SCHEDULED);
    }
  } else {
    *(wData->respColorPtr) = GREEN;
    flasherIndicator = flasherIndicator & (~RESP_SCHEDULED);
  }

  
  // Special Case for Systolic
  if ((!sysAlarmResult) && (sysWarnResult == FALSE)) {
    // it is actually ok now
    *(wData->systoColorPtr) = GREEN;
    wData->ackReceived[2] = 0;
    wData->mCountPtr[2] = 0;
  } else {
    if (sysAlarmResult) {
      // it is at least out of range
      if ((wData->mCountPtr[2])>6) {
        // wait, it has been out for 5 measurements
        *(wData->systoColorPtr) = RED;
      } else {
        // it is still fine now
        *(wData->systoColorPtr) = ORANGE;
      }
    }
    if (sysWarnResult == TRUE) {
      // we are too high. have we received an ack yet?
      if (wData->ackReceived[2]) {
        // yes. so lets get the mCount going
        if (wData->mCountPtr[2] == 0) {
          // if it was zero, start it. otherwise, it has started so leave it be.
          wData->mCountPtr[2] = 1;
        }
        // make it the same fashion as out of range. so thats it
      } else {
        // not acked. make it red
        *(wData->systoColorPtr) = RED;
      } 
    }
  }

   // Special Case for Temp
  if (!(*(wData->tempOutOfRangePtr)) && (*(wData->tempHighPtr) == FALSE)) {
    *(wData->tempColorPtr) = GREEN;
    wData->ackReceived[0] = 0;
    wData->mCountPtr[0] = 0;
  } else {
    if (*(wData->tempOutOfRangePtr)) {
      // it is at least out of range
      if ((wData->mCountPtr[0])>6) {
        // wait, it has been out for 5 measurements
        *(wData->tempColorPtr) = RED;
      } else {
        // it is still fine now
        *(wData->tempColorPtr) = ORANGE;
      }
    }
    if (*(wData->tempHighPtr) == TRUE) {
      // we are too high. have we received an ack yet?
      if (wData->ackReceived[0]) {
        // yes. so lets get the mCount going
        if (wData->mCountPtr[0] == 0) {
          // if it was zero, start it. otherwise, it has started so leave it be.
          wData->mCountPtr[0] = 1;
        }
        // make it the same fashion as out of range. so thats it
      } else {
        // not acked. make it red
        *(wData->tempColorPtr) = RED;
      } 
    }
  }

   // Special Case for pulse
  if (!(*(wData->pulseOutOfRangePtr)) && (*(wData->pulseLowPtr) == FALSE)) {
    // it is actually ok now
    *(wData->pulseColorPtr) = GREEN;
    wData->ackReceived[1] = 0;
    wData->mCountPtr[1] = 0;
  } else {
    if (*(wData->pulseOutOfRangePtr)) {
      // it is at least out of range
      if ((wData->mCountPtr[1])>6) {
        // wait, it has been out for 5 measurements
        *(wData->pulseColorPtr) = RED;
      } else {
        // it is still fine now
        *(wData->pulseColorPtr) = ORANGE;
      }
    }
    if (*(wData->pulseLowPtr) == TRUE) {
      // we are too high. have we received an ack yet?
      if (wData->ackReceived[1]) {
        // yes. so lets get the mCount going
        if (wData->mCountPtr[1] == 0) {
          // if it was zero, start it. otherwise, it has started so leave it be.
          wData->mCountPtr[1] = 1;
        }
        // make it the same fashion as out of range. so thats it
      } else {
        // not acked. make it red
        *(wData->pulseColorPtr) = RED;
      } 
    }
  }
  
   // Special Case for Resp
  if (!(*(wData->respOutOfRangePtr)) && (*(wData->rrLowPtr) == FALSE)) {
    // it is actually ok now
    *(wData->respColorPtr) = GREEN;
    wData->ackReceived[3] = 0;
    wData->mCountPtr[3] = 0;
  } else {
    if (*(wData->respOutOfRangePtr)) {
      // it is at least out of range
      if ((wData->mCountPtr[3])>6) {
        // wait, it has been out for 5 measurements
        *(wData->respColorPtr) = RED;
      } else {
        // it is still fine now
        *(wData->respColorPtr) = ORANGE;
      }
    }
    if (*(wData->rrLowPtr) == TRUE) {
      // we are too high. have we received an ack yet?
      if (wData->ackReceived[3]) {
        // yes. so lets get the mCount going
        if (wData->mCountPtr[3] == 0) {
          // if it was zero, start it. otherwise, it has started so leave it be.
          wData->mCountPtr[3] = 1;
        }
        // make it the same fashion as out of range. so thats it
      } else {
        // not acked. make it red
        *(wData->respColorPtr) = RED;
      } 
    }
  }
  return; // done. get out
}



/******************************************
* function name: remoteComTask
* function inputs: a pointer to the remoteComData
* function outputs: None
* function description: This function will act as
*                       the communication unit
*                       from a remote terminal to
*                       this system
*                       * has to be form : "X?U"
*                       * Error code:  1: unkown header
*                                      2: unknown measurement option
*                                      3: unkown footer
*                                      4: request missing
*                                      5: footer missing
* author: Matt
******************************************/
void remoteComTask(void* data){
  RemComData* remData = (RemComData*) remData;
  // checks the readin stuffs from the Serial0(Serial)
  if (Serial.available() < 1) {
    // nothing to be read, get out
    return;
  }
  // there is something coming in
  char input = Serial.read();
  if (input == 'I') {
    remoteOn = 1;
    Serial.write("Connection Established!\r\n----------\r\nWhat is thy bidding, my master?\r\n\n");
  }
  // it is a request, deal with it 
  if (remoteOn == 1 && input != 'I') { 
    switch(input){
        case 'S':                                         // Case S: START MEASURE
          commanderMode = START;
          Serial.write("S: Roger. We may fire when ready -- ");
          if (measurementSelection == 0){
            if (measureCursor == 0){
                measurementSelection = measurementSelection | TEMP_SCHEDULED;
                Serial.write("Temperature neutralized. Next will be Blood Pressure\r\n");
            }else if (measureCursor == 1){
                measurementSelection = measurementSelection | BP_SCHEDULED;
                Serial.write("Blood Pressure neutralized. Next will be Pulse Rate\r\n");
            }else if (measureCursor ==2){
                measurementSelection = measurementSelection | PULSE_SCHEDULED;
                Serial.write("Pulse Rate. Next will be Respiration Rate\r\n");
            }else if (measureCursor == 3){
                measurementSelection = measurementSelection | RESP_SCHEDULED;
                Serial.write("Respiration Rate neutralized. Next will be EKG\r\n");
            }else{
                measurementSelection = measurementSelection | EKG_SCHEDULED;
                Serial.write("EKG neutralized. Next will be Temperature\r\n");
            }
            measureCursor = (measureCursor+1)%5;
          }
          break;    
        case 'P':                                         // Case P: STOP
          commanderMode = STOP;
          Serial.write("P: Roger. Executing order 66\r\n");
          break;  
        case 'D':                                         // Case D: DISPLAY    
          if (displayOn){
            displayOn = 0;
            Serial.write("D: Roger. Now they are blinded by the dark side of the force\r\n");
          } else {
            displayOn = 1;
            Serial.write("D: Roger, Now they will see through the lies of the Jedi.\r\n");
          }
          break; 
        case 'M':                                         // Case M: RETURN MEASUREED VALUES
          Serial.print("M: Ele the phantom. Ele the fen.\r\n-------------------\r\n");
          Serial.print("  T = ");
          Serial.print(remData->tempRawBufPtr[freshTempCursor]);
          Serial.print("  S = ");
          Serial.print(remData->bpRawBufPtr[freshSBPCursor]);
          Serial.print("  D = ");
          Serial.print(remData->bpRawBufPtr[freshDBPCursor]);
          Serial.print("  P = ");
          Serial.print(remData->prRawBufPtr[freshPulseCursor]);
          Serial.print("  R = ");
          Serial.print(remData->rrRawBufPtr[freshRespCursor]);
          Serial.print("  E = ");
          Serial.print(remData->ekgFreqBufPtr[freshEKGCursor]);
          Serial.print("-------------------------\r\n");
          break;
        case 'W':                                         // Case W: RETURN WARNINGS
          Serial.write("Input was W \r\n");
          break;
        default:                                          // Case 6: Default
          Serial.write("E: You don't know the Command. You must be a Rebel Scum!!\n");
          Serial.write(input);
          Serial.write(" is What we have");
          return;
    }  
  }
  }

/******************************************
* function name: displayTask
* function inputs: a pointer to the DisplayData
* function outputs: None
* function description: This function will display
*             all the data in Mega onto TFT
* author: Matt & Sabrina
******************************************/ 
void displayTask(void* data) {
   // no need for the timer
   if (!displayOn){
    // we don't display anything
    tft.fillScreen(BACKGROUND_COLOR);
    return;
   }
   DisplayData* dData = (DisplayData*)data;
   // render the bottom buttons first base on mode selection
   for (int i = 0; i < 2; i++) {
     if (*(dData->modeSelection) == i ) {
       button[i].drawButton(true);
     } else {
       button[i].drawButton(false);
     }
   }
   // now base on mode selection. decide which content to show.
   if (*(dData->modeSelection) == 0) {
       // display menu
       unsigned int selections = *(dData->measurementSelectionPtr);
       // render the buttons 
       if (selections & TEMP_SCHEDULED) {
          //render temp
         menuButton[0].drawButton(true);
       } else {
         menuButton[0].drawButton(false);
       }
       if (selections & BP_SCHEDULED) {
         // Render BP
         menuButton[1].drawButton(true);
       } else {
         menuButton[1].drawButton(false);
       }
       if (selections & PULSE_SCHEDULED) {
          //render temp
         menuButton[2].drawButton(true);
       } else {
         menuButton[2].drawButton(false);
       }
       if (selections & RESP_SCHEDULED) {
          //render temp
         menuButton[3].drawButton(true);
       } else {
         menuButton[3].drawButton(false);
       }
        if (selections & EKG_SCHEDULED) {
          //render temp
         menuButton[4].drawButton(true);
       } else {
         menuButton[4].drawButton(false);
       }
   } else if (*(dData->modeSelection) == 1) {
       // display annunc stuffs
       tft.setCursor(0,0);
       tft.println(" ");

       // Display Temperature
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" Temperature:");
       // Show the Temprature
       if (flasherIndicator & TEMP_SCHEDULED) {
          // we need to flash
          static long tempFtimer = 0;
          if (tempFtimer!=0 && (millis()-tempFtimer)<1000) {
              tft.setTextColor(*(dData->tempColorPtr),BACKGROUND_COLOR);
          } else {
              tft.setTextColor(BACKGROUND_COLOR,BACKGROUND_COLOR);
              tempFtimer = millis();
          }
       } else {
          tft.setTextColor(*(dData->tempColorPtr),BACKGROUND_COLOR);
       }
       tft.print((char*)(dData->tempCorrectedBufPtr[freshTempCursor]));
       int diff = TEMP_DISP_WIDTH - strlen((char*)(dData->tempCorrectedBufPtr[freshTempCursor]));
       for (int i = 0; i<diff; i++) {
          tft.print(" ");
       }
       tft.print(" C\n");

       // Display Systolic
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.println(" Pressure:");
       tft.print("   Systolic: ");
       if (flasherIndicator & BP_SCHEDULED) {
          // we need to flash
          static long bpSFtimer = 0;
          if (bpSFtimer!=0 && (millis()-bpSFtimer)<800) {
              tft.setTextColor(*(dData->diastoColorPtr),BACKGROUND_COLOR);
          } else {
              tft.setTextColor(BACKGROUND_COLOR,BACKGROUND_COLOR);
              bpSFtimer = millis();
          }
       } else {
          tft.setTextColor(*(dData->diastoColorPtr),BACKGROUND_COLOR);
       }
       tft.print((char*)(dData->bpCorrectedBufPtr[freshSBPCursor]));
       tft.print(".00");
       diff = SYS_PRESS_DISP_WIDTH-strlen((char*)(dData->bpCorrectedBufPtr[freshSBPCursor]));
       for (int i = 0; i<diff; i++) {
          tft.print(" ");
       }
       tft.println(" mm Hg");
       
       // Display Diastolic
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print("   Diastolic:");
       // Show the Dias Pressure
       if (flasherIndicator & BP_SCHEDULED) {
          // we need to flash
          static long bpDFtimer = 0;
          if (bpDFtimer!=0 && (millis()-bpDFtimer)<800) {
              tft.setTextColor(*(dData->diastoColorPtr),BACKGROUND_COLOR);
          } else {
              tft.setTextColor(BACKGROUND_COLOR,BACKGROUND_COLOR);
              bpDFtimer = millis();
          }
       } else {
          tft.setTextColor(*(dData->diastoColorPtr),BACKGROUND_COLOR);
       }
       
       tft.print((char*)(dData->bpCorrectedBufPtr[freshDBPCursor]));
       diff = DIAS_PRESS_DISP_WIDTH-strlen((char*)(dData->bpCorrectedBufPtr[freshDBPCursor]));
       for (int i = 0; i<diff; i++) {
         tft.print(" ");
       }
       tft.println(" mm Hg");
       
       // Display Pulse
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" Pulse rate: ");
       // Show the Dias Pressure
        if (flasherIndicator & PULSE_SCHEDULED) {
          // we need to flash
          static long pulseFtimer = 0;
          if (pulseFtimer!=0 && (millis() - pulseFtimer)<2000) {
              tft.setTextColor(*(dData->pulseColorPtr),BACKGROUND_COLOR);
          } else {
              tft.setTextColor(BACKGROUND_COLOR,BACKGROUND_COLOR);
              pulseFtimer = millis();
          }
       } else {
          tft.setTextColor(*(dData->pulseColorPtr),BACKGROUND_COLOR);
       }
       // Show the PulseRate
       tft.print((char*)(dData->prCorrectedBufPtr[freshPulseCursor]));
       diff = PULSE_PRESS_DISP_WIDTH - strlen((char*)(dData->prCorrectedBufPtr[freshPulseCursor]));
       for (int i = 0; i<diff; i++) {
         tft.print(" ");
       }
       tft.println(" BPM");

       // Display Respiration
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" Resp rate: ");
       // Show the Respiration Rate
       tft.setTextColor(*(dData->respColorPtr),BACKGROUND_COLOR);
       tft.print((char*)(dData->rrCorrectedBufPtr[freshRespCursor]));
       diff = RESP_DISP_WIDTH-strlen((char*)(dData->rrCorrectedBufPtr[freshRespCursor]));
       for (int i = 0; i<diff; i++) {
         tft.print(" ");
       }
       tft.println(" BPM");

       // Battery
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" Battery:     ");
       // Figure out the color
       if (*(dData->batteryState) <= BATTERY_LIMIT) {
          tft.setTextColor(RED, BACKGROUND_COLOR);
       } else {
          tft.setTextColor(GREEN, BACKGROUND_COLOR);
       }
       char batteryStateBuffer[9];
       sprintf(batteryStateBuffer, "%d  ",*(dData->batteryState));
       tft.print(batteryStateBuffer);
       // render the ackButton
       
        // EKG
       tft.setTextColor(STATIC_TEXT_COLOR);
       tft.print(" \n EKG:        ");
       // Figure out the color
       if (*(dData->batteryState) <= BATTERY_LIMIT) {
          tft.setTextColor(RED, BACKGROUND_COLOR);
       } else {
          tft.setTextColor(GREEN, BACKGROUND_COLOR);
       }
       char batteryStateBuffer2[9];
       sprintf(batteryStateBuffer2, "%d  ",*(dData->batteryState));
       tft.print(batteryStateBuffer2);
       // render the ackButton
       ackButton.drawButton(false);
       // Thats all
   }

   return;
}


void keypadTask(void* data) {
  static unsigned long timer = 0;
   if (timer!=0 && (timeElapsed-timer)<KEYPAD_SCAN_INTERVAL) {  // scan every 20ms
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
    if (*(kData->modeSelection) == 0) {
      // menu. use 950
      scaleFactor = 950;
    } else {
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
     if ((*(kData->modeSelection) == 0) && *(kData->measurementSelectionPtr) != 0) {
      //do nothing
      return;
    }
    // check if the menu select button is clicked
    if ((*(kData->modeSelection) == 0) && (menuButton[0].contains(x2,y2))) {
      // temp selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | TEMP_SCHEDULED;
    } else
    if ((*(kData->modeSelection) == 0) && (menuButton[1].contains(x2,y2))) {
      // blood pressure selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | BP_SCHEDULED;
    } else
    if ((*(kData->modeSelection) == 0) && (menuButton[2].contains(x2,y2))) {
      // pulse selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | PULSE_SCHEDULED;
    } else
    if ((*(kData->modeSelection) == 0) && (menuButton[3].contains(x2,y2))) {
      // respiration selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | RESP_SCHEDULED;
    }  else
    if ((*(kData->modeSelection) == 0) && (menuButton[4].contains(x2,y2))) {
      // EKG selected
      *(kData->measurementSelectionPtr) = *(kData->measurementSelectionPtr) | EKG_SCHEDULED;
    }else
    // check if the ack button is clicked
    if ((*(kData->modeSelection) == 1) && (ackButton.contains(x2,y2))) {
      //acked
      if (tempColor == RED){
        kData->ackReceived[0] = 1;
      }
      if (systoColor == RED){
        kData->ackReceived[2] = 1;
      }
      if (pulseColor == RED){
        kData->ackReceived[1] = 1;
      }
      if (respColor == RED){
        kData->ackReceived[3] = 1;
      }
    }
   }
}

void statusTask(void* data) {
    // TIMING MECH
   static unsigned long timer = 0;
   if (timer!=0 && (timeElapsed-timer)<SUSPENSION) {
     return;
   }
   // It is our turn. get the status
   timer = timeElapsed;
   //Serial.print("For Status--- \n");
   StatusData* statusData = (StatusData*)data;
   requestAndReceive((char*)(statusData->batteryState),sizeof(unsigned short), (char*)(statusData->batteryState),sizeof(unsigned short), STATUS_TASK , STATUS_TASK );
   if (*(statusData->batteryState) == 0) {
    *(statusData->batteryState) = FULL_BATTERY;  // Magical Recharge
   }
   //Serial.println(" Finished\n");
   return;
}

char compareData(unsigned int oldData, unsigned int newData) {
  char result = 0; 
  if (abs((int)(newData - oldData)) <= (0.15 * oldData)) { 
    result = 1;
  }
  return result;
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
void executeTCB(TCB* taskControlBlock) {
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
void requestAndReceive(char* inputBuffer, char inputLength , char* outputBuffer, char outputLength, char taskType, char subTaskType) {
  // write the taskType and subtype to the UNO
  Serial1.write(taskType);
  Serial1.write(subTaskType);
  // write the data that needed to be passed
  for (char i = 0; i<inputLength; i++) {
    Serial1.write(inputBuffer[i]);
  }
  // now wait for the replies
  while(Serial1.available()<outputLength) {
    // just wait
  }
  for (char j = 0; j<outputLength; j++) {
     outputBuffer[j]=Serial1.read();
  }
  return;
}

/******************************************
* function name: toTerminal
* function inputs:  char* to set input buffer,
*         char to represent input buffer length
*         char* for output buffer,
* function outputs: None
* function description: This function will write to Serial Monitor
* author: Matt
******************************************/ 
void toTerminal(unsigned int inputBuffer, char taskType, char subTaskType) {
  // write the taskType and subtype to the UNO
  Serial.write(taskType);
  Serial.write(subTaskType);
  // write the data that needed to be passed
  char dump[10];
  sprintf(dump, "%d", inputBuffer);
  for (char i = 0; i<strlen(dump); i++) {
    Serial.write(dump[i]);
  }
  Serial.write('\n');
  return;
}


unsigned int computeEKG() {
  signed int imag[256];
  for (int i = 0; i < 256; i++) {
    imag[i] = 0;
  }
  unsigned int peakIndex = optfft((signed int*)ekgRawBuf, imag);
  unsigned int output = (peakIndex * (1 / samplingRate) / 256);
  return output;
}

//  end of EE 474 code
