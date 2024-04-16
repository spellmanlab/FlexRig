#include <SPI.h>
#include <TrueRandom.h>
#include <elapsedMillis.h>
elapsedMillis timeElapsed;
#include <Average.h>
#include <flexRigFunctions.h>

int session;
int setID;
int blockLength = 1; //length of left/right licking blocks
int blockCount = 0; //progress within each block
int lickSpout = 1; //determines left/right assignment of current block
int waiting=1;
int lickTime = 0;
int startTime = 0; 
int totalDispensed = 0;
int lickData[5];

int tone_length = 500;
int trial_tone = 18;
int tone_left = 17;
int tone_right = 19;
int wstim_left = 9;
int wstim_right = 7;
int toneList[3] = {tone_left, 0, tone_right};
int stimList[3] = {wstim_left, 0, wstim_right};
int odorList[6] = {1, 0, 2, 3, 0, 4};
int lickSpoutList[3]= {-1, 0, 1};

int setIDOut = 52;
int trialOut = 53;
int trialLROut = 50;
int irrelLROut = 51;

void setup() {
  initializeBoard();
  timeElapsed = 0;
  startTime = millis();
  randomSeed(analogRead(0));
  Serial.println("Choose session:");
  Serial.println("(1) Random (HB1/2)");
  Serial.println("(2) Blocked L/R (HB3)");
  Serial.println("(3) + Lick-triggered stimuli (HB3B)");
  while (waiting == 1) {
    if (Serial.available() > 0) {    // is a character available?
      session = Serial.parseInt();       // get the character
      if (session > 0) {
        waiting = 0;
      }
    }
  }


if(session == 3){
  Serial.println("Training with which setID:");
    Serial.println("(1) Whisker");
    Serial.println("(2) Odor");
    waiting=1;
    while (waiting == 1) {
      if (Serial.available() > 0) {    // is a character available?
        setID = Serial.parseInt();       // get the character
        if (setID > 0) {
          waiting = 0;
        }
      }
    }
}
  
if(session == 1){
        Serial.println("Starting HB1/2");
      }
else if(session == 2){
        Serial.println("Starting HB3");
      }
else if(session == 3){
        Serial.print("Starting HB3B");
      }

if(session == 3){
  if(setID == 1){
    Serial.println(", whisker discrimination");
  }
  else if(setID == 2){
    Serial.println(", odor discrimination");
  }
}
  delay(1000);
  digitalWrite(52,HIGH);
  randomSeed(analogRead(A3));
}






void loop(){
  
  if(session==1) {
    blockLength = random(1, 4);
    blockCount = 0;
    lickSpout = lickSpout * (-1);
  }
  else if(session == 2) {
    if(blockLength < 20){blockLength = blockLength + 1;}
    else{blockLength = 20;}
    blockCount = 1;
    lickSpout = lickSpout * (-1);
  }
  
  else if(session == 3) {
    blockLength = 20;
    blockCount = 1;
    lickSpout = lickSpout * (-1);
  }
  
  while (blockCount < blockLength) {  
      lickDetect(lickData,lickSpout,1);    
      if(session == 3) {
        if(setID==1) {
          playTone(stimList[lickSpout+1],random(150,300));
        }
        else {
          presentOdorant(odorList[lickSpout+1],random(150,300));
        }
      }
      dispense(lickSpoutList[lickSpout+1]);
      playTone(toneList[lickSpout+1],500);
      blockCount = blockCount + 1;
      lickTime = (millis() - startTime)/1000;
      totalDispensed = totalDispensed + 4;
      Serial.println(totalDispensed);
      Serial.println(lickTime);
      Serial.println(" seconds elapsed"); 
      delay(random(1, 1000));
  }
    
}
