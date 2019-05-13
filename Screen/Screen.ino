#include <stdio.h>
#include <stdlib.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>   // Touch Screen for TFT


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
#define BUTTON_W 130
#define BUTTON_H 35
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
  unsigned char* bpOutOfRange;
  unsigned char* tempOutOfRangePtr;
  Bool* tempHighPtr;
  unsigned char* bpOutOfRangePtr;
  Bool* bpHighPtr;
  unsigned char* pulseOutOfRangePtr;
  Bool* pulseLowPtr;
} WarningAlarmData;


//DisplayData
typedef struct
{
  unsigned char** tempCorrectedBufPtr;
  unsigned char** bpCorrectedBufPtr;
  unsigned char** prCorrectedBufPtr;  
  unsigned short* batteryState;
  WarningAlarmData* warnData;
}  DisplayData;


//Keypad Data


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
void temperatureTask(void* data);
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
TouchScreen ts = TouchScreen(XP, YP ,XM , YM, 300);
Elegoo_GFX_Button button[4];


// Global Variables for Measurements
unsigned int temperatureRawBuf[8] = {75,0,0,0,0,0,0,0};
unsigned int bloodPressureRawBuf[16] = {80,0,0,0,0,0,0,0,80,0,0,0,0,0,0,0};
unsigned int pulseRateRawBuf[8] = {0,0,0,0,0,0,0,0};
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
   for (int i=0; i<8; i++){
     tempCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   tempCorrectedBuf[0] = "32.00";
   
   // 2. BloodPressure
   // data:
   for (int i=0; i<16; i++){
     bloodPressureCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   bloodPressureCorrectedBuf[0] = "120";
   bloodPressureCorrectedBuf[8] = "120";
   
   // 3. Pulse Rate
   // data:
   for (int i=0; i<8; i++){
     pulseRateCorrectedBuf[i] = (unsigned char*)malloc(MAX_STR_BUF_LEN);
   }
   pulseRateCorrectedBuf[0] = "0.00";
   
   // 4. Display
   // data:
   DisplayData displayData;
   displayData.tempCorrectedBufPtr = tempCorrectedBuf;
   displayData.bpCorrectedBufPtr = bloodPressureCorrectedBuf;
   displayData.prCorrectedBufPtr = pulseRateCorrectedBuf;
   displayData.batteryState = &batteryState;
   
   TCB displayTaskControlBlock;
   displayTaskControlBlock.next = NULL; 
   displayTaskControlBlock.prev = NULL; 
   displayTaskControlBlock.myTask = displayTask;
   displayTaskControlBlock.taskDataPtr = (void*)&displayData;
   
   // 5. Status
   // data:
   StatusData statusData;
   statusData.batteryState = &batteryState;
   
  // 6. Schedule
   // data:
   SchedulerData schedulerData;
   schedulerData.head = NULL;
   schedulerData.tail = NULL;
   schedulerTaskQueue = &schedulerData;
   // TCB:
   TCB scheduleTaskControlBlock;
   scheduleTaskControlBlock.prev = NULL; 
   scheduleTaskControlBlock.next = NULL; 
   scheduleTaskControlBlock.myTask = scheduleTask;
   scheduleTaskControlBlock.taskDataPtr = (void*)&schedulerData;
   // Insert the basic TCBs in
   insertTask(&displayTaskControlBlock);
   
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
    touchLocation();
    if (currTask ==  NULL){
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
    TCB* itsPrev = task->prev;
    TCB* itsNext = task->next;
    itsPrev->next = itsNext;
    itsNext->prev = itsPrev;
    // and we are done
  }
  // finall, clean the task
  task->next = NULL;
  task->prev = NULL;
}

void temperatureTask(void* data) {
  return;
}


void bloodPressureTask(void* data) {
   return;
}

void pulseRateTask(void* data) {
  return;
}

int touchLocation() {
  digitalWrite(13, HIGH); 
  TSPoint p = ts.getPoint(); 
  digitalWrite(13, LOW); 
  pinMode(XM, OUTPUT); 
  pinMode(YP, OUTPUT); 
  int output = 0;

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) { 
    //p.x = map(p.x, 0, tft.width(), tft.width(), 0); 
    //p.y = tft.height() - map(p.y, 0, tft.height(), tft.height(), 0); 
    Serial.print(p.x);
      Serial.print(",");
      Serial.print(p.y);
      Serial.print(" == ");
    int x2 = (int)floor((1000-p.y)*1.0/1000*320.0);
    int y2 = (int)floor((1000-p.x)*1.0/1000*250.0);
    for (int i = 0; i < 4; i++) {
    if (button[i].contains(x2, y2)) {
      button[i].press(true);
      button[i].drawButton(true);
      Serial.print(p.x);
      Serial.print(",");
      Serial.print(p.y);
      Serial.print(" == ");
      output = i + 1;
    }
    else{
      button[i].press(false);
            button[i].drawButton(false);

    }
  }
  }
  return output;
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
  
  DisplayData* displayData = (DisplayData*) data;
  tft.setCursor(0,0);
  tft.println("    ");
  
  // Display Temperature
  tft.setTextColor(STATIC_TEXT_COLOR);
  tft.print(" Temperature: ");
  // Show the Temprature
  tft.print((char*)(displayData->tempCorrectedBufPtr[tempCount]));
  int diff = TEMP_DISP_WIDTH - strlen((char*)*(displayData->tempCorrectedBufPtr[tempCount]));
  for (int i = 0; i<diff; i++){
    tft.print(" ");
  }
  tft.println(" C\n");

  // Display Blood Pressure
  tft.setTextColor(STATIC_TEXT_COLOR);
  tft.println(" Pressure:");
  tft.print("   Systolic: ");
  tft.print((char*)(displayData->bpCorrectedBufPtr[bpCount]));
  tft.print(".00");
  diff = SYS_PRESS_DISP_WIDTH-strlen((char*)*(displayData->bpCorrectedBufPtr[bpCount]));
  for (int i = 0; i<diff; i++){
    tft.print(" ");
  }
  tft.println(" mm Hg");
  // Display Diastolic
  tft.setTextColor(STATIC_TEXT_COLOR);
  tft.print("   Diastolic:");
  // Show the Dias Pressure
  tft.print((char*)(displayData->bpCorrectedBufPtr[bpCount + 8]));
  diff = DIAS_PRESS_DISP_WIDTH-strlen((char*)*(displayData->bpCorrectedBufPtr[bpCount + 8]));
    for (int i = 0; i<diff; i++){
    tft.print(" ");
  }
  tft.println(" mm Hg");
  // Display Pulse
  tft.setTextColor(STATIC_TEXT_COLOR);
  tft.print(" Pulse rate: ");
  // Show the PulseRate
  tft.print((char*)(displayData->prCorrectedBufPtr[prCount]));
  diff = PULSE_RAW_SUBTASK-strlen((char*)*(displayData->prCorrectedBufPtr));
  for (int i = 0; i<diff; i++){
    tft.print(" ");
  }
  tft.println(" BPM\n");
  static int flag = 0;
  if (flag == 0) {
    char ButtonText[4][11] = {"Menu",  "Annun", " ", " "};
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        button[i*2 + j].initButton(&tft, 80 + (160 * j), 170 + (45 * i), BUTTON_W + 20, BUTTON_H, BLACK, BLACK, WHITE, ButtonText[(i * 2) + j], 2);
        button[i*2+ j].drawButton();
      }
    }
  }
  flag = 1;
  return;
}



void statusTask(void* data){
   // check the timer to see if it is time to go
   static unsigned long timer = 0;
   if (timer!=0 && (millis()-timer)<SUSPENSION){
     return;
   }
   timer = millis();
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
