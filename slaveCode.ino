//FINAL
//-----------------------------------------------------------------------------------------------------------//
//                                                                                                           //
//  Slave_ELEC1601_Student_2019_v3                                                                           //
//  The Instructor version of this code is identical to this version EXCEPT it also sets PIN codes           //
//  20191008 Peter Jones                                                                                     //
//                                                                                                           //
//  Bi-directional passing of serial inputs via Bluetooth                                                    //
//  Note: the void loop() contents differ from "capitalise and return" code                                  //
//                                                                                                           //
//  This version was initially based on the 2011 Steve Chang code but has been substantially revised         //
//  and heavily documented throughout.                                                                       //
//                                                                                                           //
//  20190927 Ross Hutton                                                                                     //
//  Identified that opening the Arduino IDE Serial Monitor asserts a DTR signal which resets the Arduino,    //
//  causing it to re-execute the full connection setup routine. If this reset happens on the Slave system,   //
//  re-running the setup routine appears to drop the connection. The Master is unaware of this loss and      //
//  makes no attempt to re-connect. Code has been added to check if the Bluetooth connection remains         //
//  established and, if so, the setup process is bypassed.                                                   //
//                                                                                                           //
//-----------------------------------------------------------------------------------------------------------//

#include <SoftwareSerial.h>   //Software Serial Port
#include <Servo.h>

#define RxD 7
#define TxD 6
#define ConnStatus A1

#define DEBUG_ENABLED  1


// ##################################################################################
// ### EDIT THE LINES BELOW TO MATCH YOUR SHIELD NUMBER AND CONNECTION PIN OPTION ###
// ##################################################################################

int shieldPairNumber = 16;

// CAUTION: If ConnStatusSupported = true you MUST NOT use pin A1 otherwise "random" reboots will occur
// CAUTION: If ConnStatusSupported = true you MUST set the PIO[1] switch to A1 (not NC)

boolean ConnStatusSupported = true;   // Set to "true" when digital connection status is available on Arduino pin

// #######################################################

// The following two string variable are used to simplify adaptation of code to different shield pairs

String slaveNameCmd = "\r\n+STNA=Slave";   // This is concatenated with shieldPairNumber later

SoftwareSerial blueToothSerial(RxD,TxD);


//Wheels set up
Servo servoLeft;
Servo servoRight;

//whister
byte wLeftOld;                               // Previous loop whisker values
byte wRightOld;                              
byte counter;                                // For counting alternate corners


void setup()
{

    //Wheels set up
    servoLeft.attach(13);
    servoRight.attach(12);
    Serial.begin(9600);
    blueToothSerial.begin(38400);                    // Set Bluetooth module to default baud rate 38400
    
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    pinMode(ConnStatus, INPUT);

    // whisker code
    pinMode(5, INPUT);                         // Set left whisker pin to input
    pinMode(8, INPUT);                         // Set right whisker pin to input    
    
    tone(4, 3000, 1000);                       // Play tone for 1 second
    delay(1000);                               // Delay to finish tone
    
    servoRight.attach(12);                     // Attach right signal to pin 12
    servoLeft.attach(13);                      // Attach left signal to pin 13
    
    wLeftOld = 0;                              // Init. previous whisker states
    wRightOld = 1;  
    counter = 0;                               // Initialize counter to 0

    //  Check whether Master and Slave are already connected by polling the ConnStatus pin (A1 on SeeedStudio v1 shield)
    //  This prevents running the full connection setup routine if not necessary.

    if(ConnStatusSupported) Serial.println("Checking Slave-Master connection status.");

    if(ConnStatusSupported && digitalRead(ConnStatus)==1)
    {
        Serial.println("Already connected to Master - remove USB cable if reboot of Master Bluetooth required.");
    }
    else
    {
        Serial.println("Not connected to Master.");
        
        setupBlueToothConnection();   // Set up the local (slave) Bluetooth module

        delay(1000);                  // Wait one second and flush the serial buffers
        Serial.flush();
        blueToothSerial.flush();
    }
}

void setupBlueToothConnection()
{
    Serial.println("Setting up the local (slave) Bluetooth module.");

    slaveNameCmd += shieldPairNumber;
    slaveNameCmd += "\r\n";

    blueToothSerial.print("\r\n+STWMOD=0\r\n");      // Set the Bluetooth to work in slave mode
    blueToothSerial.print(slaveNameCmd);             // Set the Bluetooth name using slaveNameCmd
    blueToothSerial.print("\r\n+STAUTO=0\r\n");      // Auto-connection should be forbidden here
    blueToothSerial.print("\r\n+STOAUT=1\r\n");      // Permit paired device to connect me
    
    //  print() sets up a transmit/outgoing buffer for the string which is then transmitted via interrupts one character at a time.
    //  This allows the program to keep running, with the transmitting happening in the background.
    //  Serial.flush() does not empty this buffer, instead it pauses the program until all Serial.print()ing is done.
    //  This is useful if there is critical timing mixed in with Serial.print()s.
    //  To clear an "incoming" serial buffer, use while(Serial.available()){Serial.read();}

    blueToothSerial.flush();
    delay(2000);                                     // This delay is required

    blueToothSerial.print("\r\n+INQ=1\r\n");         // Make the slave Bluetooth inquirable
    
    blueToothSerial.flush();
    delay(2000);                                     // This delay is required
    
    Serial.println("The slave bluetooth is inquirable!");
}

void manualDriveHandler(char input) {
    if (input == 'W') {
        //Go forward
      servoLeft.writeMicroseconds(1400);         // Left wheel counterclockwise
      servoRight.writeMicroseconds(1600);        // Right wheel clockwise
      delay(20);
    } else if (input == 'S') {
        //Go backward
        servoLeft.writeMicroseconds(1600);         // Left wheel counterclockwise
        servoRight.writeMicroseconds(1400);        // Right wheel clockwise
      delay(20);
    } else if (input == 'A') {
        //Turn left
        servoLeft.writeMicroseconds(1400);         // Left wheel counterclockwise
        servoRight.writeMicroseconds(1400);        // Right wheel clockwise
      delay(10);
    } else if (input == 'D') {
        //Turn right
        servoLeft.writeMicroseconds(1600);         // Left wheel counterclockwise
        servoRight.writeMicroseconds(1600);        // Right wheel clockwise
      delay(10);
    }
      else if (input == 'P'){
        servoLeft.writeMicroseconds(1500);         // Left wheel counterclockwise
        servoRight.writeMicroseconds(1500);
    }
}

void forward(int time)                       // Forward function
{
  servoLeft.writeMicroseconds(1300);         // Left wheel counterclockwise
  servoRight.writeMicroseconds(1700);        // Right wheel clockwise
  delay(time);                               // Maneuver for time ms
}

void turnLeft(int time)                      // Left turn function
{
  servoLeft.writeMicroseconds(1300);         // Left wheel clockwise
  servoRight.writeMicroseconds(1300);        // Right wheel clockwise
  delay(time);                               // Maneuver for time ms
}

void turnRight(int time)                     // Right turn function
{
  servoLeft.writeMicroseconds(1700);         // Left wheel counterclockwise
  servoRight.writeMicroseconds(1700);        // Right wheel counterclockwise
  delay(time);                               // Maneuver for time ms
}

void backward(int time)                      // Backward function
{
  servoLeft.writeMicroseconds(1700);         // Left wheel clockwise
  servoRight.writeMicroseconds(1300);        // Right wheel counterclockwise
  delay(time);                               // Maneuver for time ms
}

void loop()
{
    char recvChar;

    while(1)
    {
        if(blueToothSerial.available())     // Check if there's any data sent from the remote Bluetooth shield
        {
            recvChar = blueToothSerial.read();
            Serial.println(recvChar);
            manualDriveHandler(recvChar);
             
            if  (recvChar =='B')
            {
              break;
            }
        }
    }

    while(1)
    {
        byte wLeft = digitalRead(5);               // Copy right result to wLeft    
        byte wRight = digitalRead(8);              // Copy left result to wRight
        
        if(wLeft != wRight)                        // One whisker pressed?                                       
        {
        // Alternate from last time?
          if ((wLeft != wLeftOld) && (wRight != wRightOld))  
          {                                       
            counter++;                             // Increase count by one
            wLeftOld = wLeft;                      // Record current for next rep
            wRightOld = wRight;
            if(counter == 4)                       // Stuck in a corner?
            {
              wLeft = 0;                           // Set up for U-turn
              wRight = 0;
              counter = 0;                         // Clear alternate corner count
            }
          }  
          else                                     // Not alternate from last time
          {
            counter = 0;                           // Clear alternate corner count
          }
      }  
    
      // Whisker Navigation
      if((wLeft == 0) && (wRight == 0))          // If both whiskers contact
      {
        backward(500);                          // Back up 1 second
        turnLeft(700);                           // Turn left about 120 degrees
      }
      else if(wLeft == 0)                        // If only left whisker contact
      {
        backward(500);                          // Back up 1 second
        turnRight(500);                          // Turn right about 60 degrees
      }
      else if(wRight == 0)                       // If only right whisker contact
      {
        backward(500);                          // Back up 1 second
        turnLeft(500);                           // Turn left about 60 degrees
      }
      else                                       // Otherwise, no whisker contact
      {
        forward(20);                             // Forward 1/50 of a second
      }
    }
}




