//  container for read data

//  EE 474 code on the uno




/* (!!!!!!!   DELETE THIS COMMENT BEFORE TURNING IN !!!!!!!!!!!!!!! )F(G^OPJ)(UB:H)(B"_&V(*^T{(^%*^%(*&^_(*&*^<(*^(*^&^*_)>&)H:^T &(^(*&^(*P)
 *  In this file, 

Implement:   All the modules that should be in UNO
             A TaskDispatch that: 1. receives the data from Mega
                                  2. figure out what task the Mega wants
                                  3. do that task and write data back to Mega
                                 

(*&()*#&$()*#&$(*&()*#&$()#*&$^#^#*&^*N C(*#&(* # &#)(&#)%(&#{)(*&#(*$&#(* #HIUWHDKJHCI@U$Y@(*@ V) @)*&$@)(&$)@*$&@(*$&@)($&@($&@()*$@)($^@)&($*/


void setup()
{
  // running on the uno - connect to tx1 and rx1 on the mega and to rx and tx on the uno
  // start serial port at 9600 bps and wait for serial port on the uno to open:
  Serial.begin(9600);

}


void loop()
{
   // EXAMPLE ON HOW TO READ FROM MEGA
  
  //  read incoming byte from the mega
  while(Serial.available()<2)
  {
    // just wait
    }
  byte inbyte1 = Serial.read();
   byte inbyte2 = Serial.read();

  //  write the data back
  Serial.write(inbyte2+inbyte1);
  Serial.write(inbyte2-inbyte1);

}

//  end EE 474 code
