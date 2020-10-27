#include <IRremote.h>
const int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

#include <Servo.h>

Servo myservo;

int motorVss = 7;
int count = 0;

bool lock = 0;
bool sBreak = 0;
bool trigEvent = 1;

void serial(){
  Serial.println("---------------------");
  Serial.print("trigEvent : ");
  Serial.println(trigEvent);
  Serial.print("status : ");
  Serial.println(lock);
  Serial.print("count : ");
  Serial.println(count);
}

void lSt(int g, int r){
  digitalWrite(8, r); 
  digitalWrite(6, g);}

void manualTouch() {
  if(digitalRead(A0)){delay(100);lock=!lock; trigEvent=1;}
}

void setup() {  
  Serial.begin(9600);
  myservo.attach(9);
  irrecv.enableIRIn();
  pinMode(motorVss, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(A0, INPUT);
  lSt(1, 1);delay(1000);lSt(0, 0);
}

void motor(){
  if(lock){myservo.write(108);}else{myservo.write(65);} 
  digitalWrite(motorVss, 0);
  //trigEvent = 0;
  }

void loop() {
  serial();
 count = trigEvent + count;
 if(count > 3){trigEvent = 0; count = 0;}
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
       
//if(sBreak){lSt(0, 0);}
  if(trigEvent){digitalWrite(motorVss, 1);delay(100);}
      if(lock){           
        lSt(0, 1);
       motor();}
       else{
        lSt(1, 0);
       motor();}
       }
       
