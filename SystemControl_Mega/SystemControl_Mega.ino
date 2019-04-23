/* (!!!!!!!   DELETE THIS COMMENT BEFORE TURNING IN !!!!!!!!!!!!!!! )F(G^OPJ)(UB:H)(B"_&V(*^T{(^%*^%(*&^_(*&*^<(*^(*^&^*_)>&)H:^T &(^(*&^(*P)
 *  In this file, 

Implement:   All the modules that should be in MEGA (Tasks, data, scheduler)
             The entire display portion
             A TaskDispatch that: 1. sends to Uno
                                  2. and receive the data
                                 

(*&()*#&$()*#&$(*&()*#&$()#*&$^#^#*&^*N C(*#&(* # &#)(&#)%(&#{)(*&#(*$&#(* #HIUWHDKJHCI@U$Y@(*@ V) @)*&$@)(&$)@*$&@(*$&@)($&@($&@()*$@)($^@)&($*/





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
   
} WarningAlarmData;

// StatusData
typedef struct
{
  unsigned short* batteryState;
   
} StatusData;

 // SchedulerData
typedef struct
{
  int none;
   
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
void Initialization();
void Schedule(TCB* tasks[], int taskCount);
// task functions
void Measure(void* data);
void Compute(void* data);
void Display(void* data);
void Status(void* data);
void AlarmsAndWarning(void* data);
// Intrasystem Communication functions





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
unsigned char tempOutoRange = 0;
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
}

/******************************************
* function name: loop
* function inputs: None
* function outputs: None
* function description: This is the function
*                       our Mega will run forever
* author: Matt & Sabrina
******************************************/ 
void loop(){
  
}



//  end EE 474 code
