//  container for read data

//  EE 474 code on the uno




/* (!!!!!!!   DELETE THIS COMMENT BEFORE TURNING IN !!!!!!!!!!!!!!! )F(G^OPJ)(UB:H)(B"_&V(*^T{(^%*^%(*&^_(*&*^<(*^(*^&^*_)>&)H:^T &(^(*&^(*P)
 *  In this file, 

Implement:   All the modules that should be in UNO
             A TaskDispatch that: 1. receives the data from Mega
                                  2. figure out what task the Mega wants
                                  3. do that task and write data back to Mega
                                 

(*&()*#&$()*#&$(*&()*#&$()#*&$^#^#*&^*N C(*#&(* # &#)(&#)%(&#{)(*&#(*$&#(* #HIUWHDKJHCI@U$Y@(*@ V) @)*&$@)(&$)@*$&@(*$&@)($&@($&@()*$@)($^@)&($*/

int tempCount;
int systCount;
int diastCount;
int pulseCount;
bool systolicFlag;

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
}


void loop()
{
  //  read incoming byte from the mega
  while(Serial.available()<2)
  {
    // just wait
  }
  byte inbyte1 = Serial.read();
  byte inbyte2 = Serial.read();
  switch(){
    
  }
  //  write the data back
  Serial.write(inbyte2+inbyte1);
}

void temperature(struct* md)
{
  bool flag = true;
  if(md.temperatureRaw < 50 && flag) {
    if(md.temperatureRaw > 50) {
      flag = false;
    }
    if(tempCount % 2 == 0) {
      md.temperatureRaw += 2;
    } else {
      md.temperatureRaw--;
    }
  } else if (md.temperatureRaw > 15 && !flag){
    if(md.temperatureRaw < 15) {
      flag = true;
    }
    if(tempCount % 2 == 0) {
      md.temperatureRaw--;
    } else {
      md.temperatureRaw += 2;
    }
  }
}

void systolicPress(struct* md) {
  if(md.systolicPressRaw < 100) {
    systolicFlag = false;
    if(systCount % 2 == 0) {
      md.systolicPressRaw += 3;
    } else {
      md.systolicPressRaw--;
    }
  } else {
    systolicFlag = true;
  }
}

void diastolicPress(struct* md) {
  if(md.diastsolicPressRaw < 100) {
    diastolicFlag = false;
    if(diastCount % 2 == 0) {
      md.diastolicPressRaw += 3;
    } else {
      md.diastolicPressRaw--;
    }
  } else {
    diastolicFlag = true;
  }
}

void pulseRate(struct* md)
{
  bool flag = true;
  if(md.pulseRateRaw <= 40 && flag) {
    if(md.temperatureRaw > 40) {
      flag = false;
    }
    if(tempCount % 2 == 0) {
      md.temperatureRaw--;
    } else {
      md.temperatureRaw += 3;
    }
  } else if (md.temperatureRaw > 15 && !flag){
    if(md.temperatureRaw < 15) {
      flag = true;
    }
    if(tempCount % 2 == 0) {
      md.temperatureRaw += 3;
    } else {
      md.temperatureRaw--;
    }
  }
}

void compute(struct* md) {
  
}

//  end EE 474 code
