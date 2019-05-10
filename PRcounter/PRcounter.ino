const int PRpinIn = 2; 
const int delayTime = 5; 
volatile byte PRcounter;
unsigned long passedTime; 
unsigned int pulseRate; 

void setup() 
{
  Serial.begin(9600); 
  attachInterrupt(digitalPinToInterrupt(PRpinIn), isr, RISING);
  pinMode(PRpinIn, INPUT_PULLUP); 
  PRcounter = 0; 
  pulseRate = 0; 
  passedTime = 0; 
}

void loop() 
{
  Serial.println(measurePulseRate());
}

/*******************************
 * Function Name:        isr
 * Function Inputs:      none
 * Function Outputs:     increment counter
 * Function Description: The interrupt service routine on 
 *                       positive edge. Counts the every beat on 
 *                       pulse rate.
 *                       
 ************************************/
void isr() 
{ 
  PRcounter++; 
}

/*******************************
 * Function Name:        measurePulseRate
 * Function Inputs:      none
 * Function Outputs:     pulseRate
 * Function Description: Counts number of rising edges generated
 *                       by the function generator and converts to
 *                       beats per minute.
 *                       
 ************************************/
unsigned int measurePulseRate()
{
  delay(1000 * delayTime); 
  detachInterrupt(digitalPinToInterrupt(PRpinIn)); 
  pulseRate = 60 / delayTime * PRcounter;  
  PRcounter = 0; 
  //Serial.println(pulseRate);
  attachInterrupt(digitalPinToInterrupt(PRpinIn), isr, RISING);
  return pulseRate;
}
