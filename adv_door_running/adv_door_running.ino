/*
changelog

27 OCT 2020 (TUE) 
- added boolean variable reset position for knob-rev2.3 (bool rPos;)
- L111-112 added if(rPos)
- L150 void motor added reset servo position (myservo.write(86);)
- L155 added 2-stage rPos reset 
- ***delayed touch (unknown reason)s

18 JUN 2021 (FRI)
- L160 added non-blocking delay for stable servo positioning

17 SEP 2021 (FRI)
- added info display for system pinouts at serial. Data can be fetch from serial monitor upon booting the system

*/



#include <SPI.h>

#include <IRremote.h>
const int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

#include <Servo.h>

Servo myservo;

//-----------------------pins-------------------------------------

const int redLed = 8;
const int greenLed = 6;
const int reedSw = A2;
const int autoTogglePin = A1;
const int touch = A0;
const int motorVss = 7;
const int servoPort = 9;

//----------------------------------------------------------------

//-----------------------variables--------------------------------

int count = 0;

bool lock = 0;
bool sBreak = 0;
bool trigEvent = 1;
bool autoLock = 1;
bool masterToggle = 1;
bool remoteToggle = 1;
bool skip = 1;
bool rPos = 0;

bool reedStatus = 1;
bool reset = 0;

unsigned long time_start = 0;

//----------------------------------------------------------------

//----------------------------RFID--------------------------------

#include <EEPROM.h>

#include <MFRC522.h> 

bool programMode = false;

uint8_t successRead;

byte storedCard[4];   
byte readCard[4];  
byte masterCard[4];

#define SS_PIN 10
#define RST_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);

//----------------------------------------------------------------

void serial(){
  Serial.println("---------------------");
  Serial.print("trigEvent : ");
  Serial.println(trigEvent);
  Serial.print("reedStatus : ");
  Serial.println(reedStatus);
  Serial.print("reset : ");
  Serial.println(reset);
  Serial.print("lock : ");
  Serial.println(lock);
  Serial.print("auto : ");
  Serial.println(reset);
}

void lSt(int g, int r){
  digitalWrite(redLed, r); 
  digitalWrite(greenLed, g);}

void aLock(){

  reedStatus = !digitalRead(reedSw);
  if(!reedStatus){reset=1;}
  masterToggle = digitalRead(autoTogglePin);
  /*
  if(masterToggle){
    if(remoteToggle||digitalRead(autoTogglePin)){autoLock=1;}
  }
  if(reset){if(autoLock && reedStatus){lock=1; trigEvent=1; reset=0;}}
*/

if(reset && masterToggle){ if(reedStatus){lock=0; trigEvent=1;reset=0; delay(100);}}
  
}

void manualTouch() {
  if(digitalRead(A0)){delay(100);lock=!lock; trigEvent=1;}
}

void opr(){ count = trigEvent + count;
 if(count > 3){trigEvent = 0; count = 0; if(rPos){rPos = 0;} if(successRead){successRead=0;}}
}

void package(){
       opr();
       rfidLess();
       motorState();
  }

void info(){
  Serial.println(" ADVDOOR INFO - WELCOME ");
  Serial.println("INPUT");
  Serial.print("Capacitive touch");
  Serial.println("    A0");
  Serial.print("Auto Toggle");
  Serial.println("    A1");
  Serial.print("Reed Switch");
  Serial.println("    A2");
  Serial.println("OUTPUT");
  Serial.print("IR RECV");
  Serial.println("    D4");
  Serial.print("LED A");
  Serial.println("    D8");
  Serial.print("LED B");
  Serial.println("    D6");
  Serial.print("MOTOR MOSFET");
  Serial.println("    D7");
  Serial.print("SERVO");
  Serial.println("    D9");
  Serial.println("------------------------------------");
  Serial.println("");
}

void setup() {  
  Serial.begin(9600);
  myservo.attach(servoPort);
  irrecv.enableIRIn();
  pinMode(motorVss, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(reedSw, INPUT);
  pinMode(touch, INPUT);
  pinMode(autoTogglePin, INPUT);
  for(int i=0;i<10;i++){
  lSt(1, 1);delay(50);lSt(0, 0);delay(50);
  }
  info();
  lock = digitalRead(reedSw);

  //------------------------RFID------------------------------------
//  pinMode(wipeB, INPUT_PULLUP);
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  SPI.begin();
  mfrc522.PCD_Init();
  

}

void motor(){
  
//  if(!rPos){if(lock){myservo.write(115);delay(200);myservo.write(87);rPos = 1;}else{myservo.write(53);delay(200);myservo.write(87);rPos = 1;} }

 int period = 500;
 
 if(!rPos){
  if(lock){
    time_start = millis();
    while(millis()-time_start<period){
      myservo.write(115);
    }
    time_start = millis();
    while(millis()-time_start<period){
      myservo.write(87);
    }
    rPos = 1;
    }
  else{
    time_start = millis();
    while(millis()-time_start<period){
      myservo.write(53);
    }
    time_start = millis();
    while(millis()-time_start<period){
      myservo.write(87);
    }
    
    rPos = 1;
    } 
  }





  digitalWrite(motorVss, 0);
  if(!trigEvent){rPos=0;}
  }

void rfidLess(){
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 if (irrecv.decode(&results)){
         switch (results.value){
          case 0xFF38C7:
           lock = !lock;
           trigEvent=1;
           delay(15);
              break;
           case 0xFF6897:
           lock = 1;
           trigEvent = 1;
           delay(15);
              break;
         }
         delay(100);
        irrecv.resume();
       }

    manualTouch();

    aLock(); 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void motorState(){
  if(trigEvent){digitalWrite(motorVss, 1);delay(100);}
      if(lock){           
        lSt(0, 1);
       motor();}
       else{
        lSt(1, 0);
       motor();}  
}  
///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  

  if (!mfrc522.PICC_IsNewCardPresent()) {
  package();
  skip=1;
  return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) { 
  package();
  skip=1;
  return 0;
  }
  for ( uint8_t i = 0; i < 4; i++) { 
        readCard[i] = mfrc522.uid.uidByte[i];
      }
        mfrc522.PICC_HaltA();
        return 1;


  // Scanned PICC's UID

}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}



void loop() {
  
//  serial();
// count = trigEvent + count;
// if(count > 3){trigEvent = 0; count = 0;if(successRead){successRead=0;}}


//----------------------------------RFID--------------------------
 successRead = getID();  
    
  if(successRead){
      if ( findID(readCard) || isMaster(readCard) ) { 
        lock=1;
        trigEvent=1;
      }}
///----------------------------------------------------------------




//if(sBreak){lSt(0, 0);}
       if(skip){
       package();
       skip=0;
       }
}

  
