//  container for read data
 char inbyte;

//  EE 474 code on the uno


/* (!!!!!!!   DELETE THIS COMMENT BEFORE TURNING IN !!!!!!!!!!!!!!! )F(G^OPJ)(UB:H)(B"_&V(*^T{(^%*^%(*&^_(*&*^<(*^(*^&^*_)>&)H:^T &(^(*&^(*P)
 *  In this file, 

Implement:   All the modules that should be in MEGA (Tasks, data, scheduler)
             The entire display portion
             A TaskDispatch that: 1. sends to Uno
                                  2. and receive the data
                                 

(*&()*#&$()*#&$(*&()*#&$()#*&$^#^#*&^*N C(*#&(* # &#)(&#)%(&#{)(*&#(*$&#(* #HIUWHDKJHCI@U$Y@(*@ V) @)*&$@)(&$)@*$&@(*$&@)($&@($&@()*$@)($^@)&($*/



void setup()
{
  // running on the uno - connect to tx1 and rx1 on the mega and to rx and tx on the uno
  // start serial port at 9600 bps and wait for serial port on the uno to open:

  // initialize both serials. Serial1 for writing and reading from UNO
  // Serial is for printing in the monitor console
  // Add the TFT screen thing later
  Serial1.begin(9600);
  Serial.begin(9600);
}


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

//  end EE 474 code
