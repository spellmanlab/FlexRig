#include <TrueRandom.h>
#include <elapsedMillis.h>
#include <Average.h>
elapsedMillis timeElapsed;
#include <flexRigFunctions.h>

Average<float> runAveNG(10);
Average<float> lickBias(5);
Average<float> setBias(5);
Average<float> runAve(100);
Average<float> runAveL(10);
Average<float> runAveR(10);

int session;
int ix;
int blockTrials = 1; //set to 1 to run blocks of 20 trials, 0 to do simple discrimination
int blockLength = 20;
int blockNum = 0;
float critScore = 0.8; //determines criterion score on incongruent trials needed to change blocks
float hiScore = 0;
int setID = 1;
int setChoose [] = { 1, -1};
int trialLR;
int trialChoose [] = { -1, 1};
int ITI = 0;
int trial = 0;
int trialCorrect = 0;
int totalDispensed = 0;
String sessionNames[] =  {"Shaping(4)", "Shaping(5)"};
String stim[] = {"Whisker", "Odor"};

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
int lickData[5];

int setIDOut = 52;  
int trialOut = 53;
int trialLROut = 50;
int irrelLROut = 51;

void setup() {
  initializeBoard();
  int waiting = 1;
  delay(500);
  Serial.println("Choose session:");
  Serial.println("(1) SH4"); Serial.println("(2) SH5");

  while (waiting == 1) {
    if (Serial.available() > 0) {    
      session = Serial.parseInt();       
      if (session > 0) {
        waiting = 0;
      }
    }
  }
  waiting = 1;
  delay(500);
  Serial.println("Choose Stimulus:");
  Serial.println("(1) Whisker (2) Odor");

  while (waiting == 1) {
    if (Serial.available() > 0) {    
      ix = Serial.parseInt();       
      if (ix > 0) {
        waiting = 0;
      }
    }
  }
  setID = (setChoose[ix - 1]);

  Serial.print("Begin Session ");
  Serial.println(sessionNames[session - 1] + " with " + stim[ix - 1]);

  for (int q = 0; q < 100; q++) {
    runAve.push(0);
    runAveNG.push(1);
  }

  timeElapsed = 0;
  randomSeed(analogRead(0));
  trialLR = trialChoose[random(0, 2)];

  Serial.println(" ");
  Serial.print("trial"); Serial.print(" ");
  Serial.print("blockNum"); Serial.print(" ");
  Serial.print("ITI"); Serial.print(" ");
  Serial.print("setID"); Serial.print(" ");
  Serial.print("trialLR"); Serial.print(" ");
  Serial.print("lickNumL"); Serial.print(" ");
  Serial.print("lickNumR"); Serial.print(" ");
  Serial.print("RT"); Serial.print(" ");
  Serial.print("trialCorrect"); Serial.print(" ");
  Serial.print("runAveL"); Serial.print(" ");
  Serial.print("runAveR"); Serial.print(" ");
  Serial.print("runAve"); Serial.print(" ");
  Serial.print("runAveNG"); Serial.print(" ");
  Serial.print("totalDispensed"); Serial.print(" ");
}



void loop() {

  trialLR = trialLR * (-1);
  blockNum = 0;
  if (blockTrials == 0) {
    blockLength = random(1, 5);
  }
  if (session == 1) {
      dispense(trialLR);
  }
  while (blockNum < blockLength) {

    if (totalDispensed < 1200 && runAveNG.mean() >= 0.1 ) {
      blockNum++;
      trial++;
      ITI = timeElapsed;
      trialCorrect = 0;
      digitalWrite(trialOut, HIGH);
      digitalWrite(trialLROut, trialLR == 1);
      playTone(trial_tone,500);
      timeElapsed = 0;

      //BEGIN ODOR PRESENTATION
      if (setID == -1) {
        presentOdorant(odorList[trialLR+1]);
      }
      //BEGIN WHISKER STIM PRESENTATION
      if (setID == 1) {
        playTone(stimList[trialLR+1]);
      }
      lickDetect(lickData,2,-1,2500);
      killTone();
      killOdorants();
      digitalWrite(trialLROut, LOW);

      if (session == 1) {
        lickDetect(lickData,trialLR,1,4000);
      }
      else {
        lickDetect(lickData,0,1,1500);
      }
      if((lickData[0] ==1 && trialLR == -1) || (lickData[1] == 1 && trialLR == 1) || (runAveNG.mean() <= 0.7 && runAveNG.mean() >= 0.3)){
        trialCorrect=1;
        dispense(trialLR);
        totalDispensed = totalDispensed + 4;
        }
      runAve.push(trialCorrect);
      playTone(toneList[trialLR+1],500);
      runAveNG.push(lickData[0] == 1 || lickData[1] == 1);

      
      Serial.println(" ");
      Serial.print(trial); Serial.print(" ");
      Serial.print(blockNum); Serial.print(" ");
      Serial.print(ITI); Serial.print(" ");
      Serial.print(setID); Serial.print(" ");
      Serial.print(trialLR); Serial.print(" ");
      Serial.print(lickData[0]); Serial.print(" ");
      Serial.print(lickData[1]); Serial.print(" ");
      Serial.print(lickData[4]); Serial.print(" ");
      Serial.print(trialCorrect); Serial.print(" ");
      Serial.print(runAveL.mean()); Serial.print(" ");
      Serial.print(runAveR.mean()); Serial.print(" ");
      Serial.print(runAve.mean()); Serial.print(" ");
      Serial.print(runAveNG.mean()); Serial.print(" ");
      Serial.print(totalDispensed); Serial.print(" ");

      digitalWrite(trialOut, LOW);
      if (runAve.mean() > hiScore) {
        hiScore = runAve.mean();
      }

      timeElapsed = 0;
      delay(random(3000, 4500));
    }
    else {
      for (int i = 0; i <= 10; i++) {
        Serial.println();
      }
      if (session == 1) {
        Serial.print("Session finished, "); Serial.print(totalDispensed);
        Serial.print("uL consumed. Animal has ");
        if (totalDispensed < 500) {
          Serial.print("NOT");
        }
      }
      else {
        Serial.print("Session finished, score is "); Serial.print(hiScore * 100); Serial.println("%");
        Serial.print("Animal has ");
        if (hiScore < 0.8) {  //requirement of >80% perf at session end
          Serial.print("NOT");
        }
      }

      Serial.print(" passed. Please log score in spreadsheet.");
    }
  }
}
