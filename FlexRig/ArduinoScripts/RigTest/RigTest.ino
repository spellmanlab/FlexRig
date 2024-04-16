#include <SPI.h>
#include <TrueRandom.h>
#include <elapsedMillis.h>
elapsedMillis timeElapsed;
#include <Average.h>
#include <flexRigFunctions.h>

int waiting = 1;                        
int session = 0;
int runLoop = 1;

void setup() {
  initializeBoard();
  timeElapsed = 0;
  randomSeed(analogRead(0));
  Serial.println("How many test loops?");
  while (waiting == 1) {
    if (Serial.available() > 0) {   // is a character available?
      runLoop = Serial.parseInt();  // get the character
      if (runLoop > 0) {
        waiting = 0;
      }
    }
  }
  waiting=1;
  Serial.println("Choose action:");
  Serial.println("(1) Test Water Lines");
  Serial.println("(2) Play Tones");
  Serial.println("(3) Test Odorant Lines");
  Serial.println("(4) Test Lick Detectors");
  Serial.println("(5) Opto Pulses");
}

void loop() {

  while (waiting == 1) {
    if (Serial.available() > 0) {   // is a character available?
      session = Serial.parseInt();  // get the character
      if (session > 0) {
        waiting = 0;
      }
    }
  }
  waiting=1;

  if (session == 1) {
    //TEST WATER SPOUTS
    for (int i=1; i<=runLoop; i++){
      Serial.println("Testing left water spout");
      dispense(-1);
      delay(1000);
      Serial.println("Testing right water spout");
      dispense(1);
      delay(1000);
    }
  }

  else if (session == 2) {
    Serial.println("Testing tones");
    for (int i=1; i<=runLoop; i++){
    // TEST TONES//
      playTone(17,500);
      delay(500);
      playTone(19,500);
      delay(500);
      playTone(7,500);
      delay(500);
    }    
  } 
  
    else if (session == 3) {
      Serial.println("Testing odorant lines");
      for (int i=1; i<=runLoop; i++){
        ////// TEST ODORANT LINES //
        for (int j=1; j<=4; j++){
          presentOdorant(j, 1000);
        }
        delay(1000);
      }
    } 

  else if (session == 4) {
    timeElapsed = 0;
    Serial.println("Testing lick detectors!");
    //// TEST LICK DETECTORS //
//    Serial.println(analogRead(A0) - analogRead(A5));
//    delay(10);
    int lickData[5];
    lickDetect(lickData,0,0,5000);
    Serial.print("licked left: ");
    Serial.println(lickData[0]);
    Serial.print("licked right: ");
    Serial.println(lickData[1]);
    Serial.print("number of left licks: ");
    Serial.println(lickData[2]);
    Serial.print("number of right licks: ");
    Serial.println(lickData[3]);
    Serial.print("reaction time (ms): ");
    Serial.println(lickData[4]);
  }

  else if (session == 5) {
    Serial.println("Testing Opto Pulses!");
    for (int i=1; i<=runLoop; i++){
      ///// TEST OPTO PULSES //////
      photoStim(5,3000);
    }
  }

  delay(1000);

}
