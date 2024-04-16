//INCLUDE RELEVANT LIBRARIES
#include <TrueRandom.h>
#include <elapsedMillis.h>
#include <Average.h>
elapsedMillis timeElapsed;
#include <flexRigFunctions.h>


//SET SESSION-SPECIFIC PARAMETERS
float prop = 10;                //sets proportion of trials that are incongruent (this number divided by 20)
float critScore = 0.8;          //sets criterion score needed to change blocks, set >1 to never reach
int LBCorrect = 1;              //actively correct for side bias? 1=Y; 0=N;


//INITIALIZE SESSION-SPECIFIC PLACEHOLDER VARIABLES
int session;                    //index within the corresponding sessionNames array
String sessionNames[] =  {"Simple Discrimination", "Compound Discrimination", "Intradimensional Shift", "Reversal", "Extradimensional shift", "Intradimensional shift B" , "Serial Extradimensional Shifts"};
int lightStimYN;                //whether to deliver optogenetic stimuli, 1 = No; 2 = Yes
int keyDetect;                  //will be used to input data at session outset
int A;                          //will be used to input data at session outset
int seqNum = 0;                 //set to 0 to start from beginning of set sequence in setChoose
int setChoose [] = {1, -1};     //assign set -- 1 for whisker, -1 for odor
int blockMax = 1;               //maximum number of blocks (sets) in a session
int mapID = 1;                  //used to reverse the left-right stimulus mapping on and after the Reversal session
int odorID = 1;                 //0=A,1=B
int toneID = 1;
int trialMax = 350;                     //maximum length of a session
int toneDur = 500;                      //duration of trial initiation tone
int trialChoose [] = { -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1};  //used to select left/right (-1/1) reward location for each trial
int irrelChoose [] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // choose irrelevant stim -- -1 for incongruent, 1 for congruent
int crit = 0;                           //criterion reached (1) or not (0)
int waiting = 1;                        //used in while-loops


//INITIALIZE TRIAL VARIABLES
int toneLR;
int odorLR;
int blockNum = 0;
int setID;
int trialLR;
int stimLR;
int irrelLR;
int blockTrial;
int lickData[5];


//ASSIGN VARIABLES TO ARDUINO PINS
int setIDOut = 52;
int trialOut = 53;
int trialLROut = 50;
int irrelLROut = 51;


//INITIALIZE PERFORMANCE VARIABLES
Average<float> runAveNG(10);    //initialize a moving window for tracking motivation
Average<float> lickBias(10);    //initialize a moving window for tracking side bias
Average<float> runAveL(10);     //initialize a moving window for tracking left trial accuracy
Average<float> runAveR(10);     //initialize a moving window for tracking right trial accuracy
Average<float> runAve(30);      //initialize a moving window for tracking overall accuracy
int ITI = 0;
int trial = 0;
int trialCorrect = 0;
int totalDispensed = 0;

int tone_length = 500;
int trial_tone = 18;
int tone_left = 17;
int tone_right = 19;
int wstim_left = 9;
int wstim_right = 7;
int toneList[3] = {tone_left, 0, tone_right};
int stimList[3] = {wstim_left, 0, wstim_right};
int odorList[6] = {1, 0, 2, 3, 0, 4};



//SETUP SECTION
void setup() {
  initializeBoard();
  randomSeed(analogRead(0));
  for (int q = 0; q < 10; q++) {
    runAveNG.push(1);
  }

  //CHOOSE SESSION
  waiting = 1;
  Serial.println("");
  Serial.println("Choose session:");
  Serial.println("(0) SD"); Serial.println("(1) CD"); Serial.println("(2) IDS"); Serial.println("(3) Rev"); Serial.println("(4) EDS"); Serial.println("(5) IDSB"); Serial.println("(6) SEDS");
  while (waiting == 1) {
    if (Serial.available() > 0) {    // is a character available?
      session = Serial.parseInt();       // get the character
      if (session >= 0 && session <= 6) {
        waiting = 0;
      }
    }
  }

  if(session<6){
    waiting = 1;
    delay(500);
    Serial.println("Choose Stimulus:");
    Serial.println("(1) Whisker (2) Odor");
    while (waiting == 1) {
      if (Serial.available() > 0) {    
       int ix = Serial.parseInt();       
        if (ix > 0) {
          if(ix == 1){setID=1;}
          else{setID=-1;}
          waiting = 0;
        }
      }
    }
  }

  delay(500);
  waiting = 1;
  Serial.println("");
  Serial.println("Opto stimulation during ITI ?");
  Serial.println("(1) No"); Serial.println("(2) Yes");
  while (waiting == 1) {
    if (Serial.available() > 0) {    // is a character available?
      lightStimYN = Serial.parseInt();       // get the character
      if (lightStimYN == 1 || lightStimYN == 2) {
        waiting = 0;
      }
    }
  }


  //SET SESSION-SPECIFIC PARAMETERS
  if (session >= 2) {         //ID shift, new whisker stimuli
    wstim_left=4;
    wstim_right=6;
  }
  if (session >= 3) {
    mapID = -1;               //Reversal and all sessions after
  }
  if (session >= 4) {
    seqNum = 1;               //ED shift 1
  }
  if (session < 6) {          // set icg/cg proportion
    for (int i = 0; i < 11; i++) {
      irrelChoose[i] =  irrelChoose[i] * (-1);
    }
  }
  else {                      //for SEDS sessions
    for (int i = 0; i < prop; i++) {
      irrelChoose[i] = irrelChoose[i] * (-1);
    }
    blockMax = 20;
    seqNum = 0;               //start with whisker
    trialMax = 1000;
    setID = setChoose[seqNum];
  }
  Serial.print("Begin Session ");
  Serial.println(sessionNames[session]);
  Serial.println("Press any key to begin");
  Serial.println("");
  waiting = 1;
  while (waiting == 1) {
    if (Serial.available()) {    // is a character available?
      waiting = 0;
    }
  }


  Serial.println("");
  Serial.print("n"); Serial.print("\t");
  Serial.print("blknum"); Serial.print("\t");
  Serial.print("iti"); Serial.print("\t");
  Serial.print("setID"); Serial.print("\t");
  Serial.print("trialLR"); Serial.print("\t");
  Serial.print("stim"); Serial.print("\t");
  Serial.print("irrelLR"); Serial.print("\t");
  Serial.print("lnL"); Serial.print("\t");
  Serial.print("lnR"); Serial.print("\t");
  Serial.print("rt"); Serial.print("\t");
  Serial.print("resp"); Serial.print("\t");
  Serial.print("corr"); Serial.print("\t");
  Serial.print("rAL"); Serial.print("\t");
  Serial.print("rAR"); Serial.print("\t");
  Serial.print("rANG"); Serial.print("\t");
  Serial.print("rA"); Serial.print("\t");
  Serial.print("H2O"); Serial.print("\t");
  timeElapsed = 0;
}














//START TASK
void loop() {

  crit = 0;
  blockNum++;
  blockTrial = 0;


  //SET RELEVANT STIMULUS MODALITY
  if (blockNum > 1) {
    setID = setID * -1;
  }
  digitalWrite(setIDOut, setID == 1);


  //RE-INITIALIZE PERFORMANCE TRACKING VARIABLES
  for (int q = 0; q < 30; q++) {
    runAveL.push(0);
    runAveR.push(0);
    runAve.push(0);
  }


  //TRIAL BLOCK
  while (crit == 0 || session < 6) {
    if (trial <= trialMax && totalDispensed < 1200 && runAveNG.mean() >= 0.2 && blockNum <= blockMax && (crit == 0 || session < 6)) {
      trial++;
      blockTrial++;


      //CHOOSE TRIAL STIMULI
      if (LBCorrect == 1) {
        trialLR = trialChoose[round(random(3 + 2 * lickBias.mean(), 9 + 2 * lickBias.mean()))];
      }
      else {
        trialLR = trialChoose[random(0, 2)];
      }
      
      stimLR = trialLR * mapID;                         //because of reversal, stimLR and trialLR are separate variables
      irrelLR = irrelChoose[random(0, 20)] * stimLR;    //set trial to be congruent or incongruent
      trialCorrect = 0;
      if (setID == -1) {
        odorLR = stimLR;
        toneLR = irrelLR;
      }
      else {
        odorLR = irrelLR;
        toneLR = stimLR;
      }


      //BEGIN TRIAL
      playTone(trial_tone, tone_length);
      digitalWrite(trialOut, HIGH);
      digitalWrite(trialLROut, trialLR==1);
      digitalWrite(irrelLROut, irrelLR==1);
     

      //BEGIN ODOR PRESENTATION
      if (session > 0){
        presentOdorant(odorList[odorLR+1 + (session>=5)*3]);
      }
      

      //BEGIN WHISKER STIM PRESENTATION
      playTone(stimList[toneLR+1]);


      //DETECT LICKS DURING STIMULUS PRESENTATION
      lickDetect(lickData, 2, 0, 2500);
      int lickNumL = lickData[2];
      int lickNumR = lickData[3];

      
      //STOP STIMULI
      killTone();
      killOdorants();
      digitalWrite(trialLROut, LOW);
      digitalWrite(irrelLROut, LOW);


      //WAIT FOR RESPONSE
      lickDetect(lickData, 2, 1, 1500);


      //RECORD RESULT AND DELIVER REWARD OR NOT
      if (trialLR == 1) {     //if the reward is on the right
        playTone(tone_right);
        if (lickData[1] == 1) {
          trialCorrect = 1;
          dispense(1);
          totalDispensed = totalDispensed + 4;
          runAveR.push(1);
        }
        else if (lickData[0] == 1) {
          runAveR.push(0);
        }
      }
      else if (trialLR == -1) {
        playTone(tone_left);
        if (lickData[0] == 1) {
          trialCorrect = 1;
          dispense(-1);
          totalDispensed = totalDispensed + 4;
          runAveL.push(1);
        }
        else if (lickData[1] == 1) {
          runAveL.push(0);
        }
      }

      runAveNG.push(lickData[0] == 1 || lickData[1] == 1);
      runAve.push(trialCorrect);
      


      //KEEP TRACK OF SIDE BIAS
      if (trialLR == -1) {
        if (trialCorrect == 0) {
          lickBias.push(-1);lickBias.push(-1);
        }
        else {
          lickBias.push(0);
        }
      }
      else {
        if (trialCorrect == 0) {
          lickBias.push(1);lickBias.push(1);
        }
        else {
          lickBias.push(0);
        }
      }


      //CHECK FOR END OF BLOCK/SESSION
      if (crit == 0 && runAve.mean() >= 0.8 && runAveL.mean() >= 0.5 && runAveR.mean() >= 0.5 && runAveNG.mean() >= 0.8) {
        crit = 1;
      }


      //PRINT TRIAL DATA
      Serial.println(" ");
      Serial.print(trial); Serial.print("\t");
      Serial.print(blockNum); Serial.print("\t");
      Serial.print(ITI); Serial.print("\t");
      Serial.print(setID); Serial.print("\t");
      Serial.print(trialLR); Serial.print("\t");
      Serial.print(stimLR); Serial.print("\t");
      Serial.print(irrelLR); Serial.print("\t");
      Serial.print(lickNumL); Serial.print("\t");
      Serial.print(lickNumR); Serial.print("\t");
      Serial.print(lickData[4]); Serial.print("\t");
      Serial.print(lickData[0] == 1 || lickData[1] == 1); Serial.print("\t");
      Serial.print(trialCorrect); Serial.print("\t");
      Serial.print(runAveL.mean()); Serial.print("\t");
      Serial.print(runAveR.mean()); Serial.print("\t");
      Serial.print(runAveNG.mean()); Serial.print("\t");
      Serial.print(runAve.mean()); Serial.print("\t");
      Serial.print(totalDispensed); Serial.print("\t");


      //END TRIAL, BEGIN ITI
      digitalWrite(trialOut, LOW);
      timeElapsed = 0;
      ITI = random(8000, 10000);
      if (lightStimYN == 2){
        photoStim(20, ITI - 2000);
      }
      delay(ITI);

    }   


    //END SESSION
    else {
      for (int i = 0; i <= 10; i++) {
        Serial.println();
      }
      Serial.print("Session finished. ");
      if (session < 6) {
        Serial.print("  Animal has ");
        if (crit == 0 || trial < 100) {
          Serial.print("NOT ");
        }
        Serial.print("PASSED! Please log score in spreadsheet.");
        Serial.println(" Give ");
        Serial.print(1100 - totalDispensed);
        Serial.print(" uL water to supplement.");
      }
      else {
        Serial.print("Animal has completed ");
        Serial.print(blockNum - 1);
        Serial.print(" trial blocks. Please log score in spreadsheet.");
        Serial.println(" Give ");
        Serial.print(1100 - totalDispensed);
        Serial.print(" uL H2O to supplement.");
      }
    while(1){;}
    }
  }
}
