//NOTE: the following third party libraries are required: elapsedMillis, Average, math, DFRobot_DF1201S
#include <elapsedMillis.h>
    elapsedMillis timeElapsed_lib;
#include <Average.h>
#include <math.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <DFRobot_DF1201S.h>
    DFRobot_DF1201S DF1201S;

int solenoidOpenTime = 25;             //open time on water solenoid, in ms (~25ms @ ~3psi = ~4uL)
float lickThresh = 16;                 //threshold voltage difference for lick detection
int H2OVol = 4;			//amount of water dispensed each time (uL)
int lickPrev=0;
int lickedL=0;                        
int lickedR=0;
int lickNumL=0;
int lickNumR=0;
int RT=0;

int LEDChannel = 40;
int optoChannel = 38; //switch to 44 for sinusoidal
volatile float phase = 0.0;
volatile float phaseIncrement = 0.0;
volatile int ledState = LOW;
volatile int timer3Count = 0;
volatile unsigned long timer3Target = 0;
int H2OL = 7;
int H2OR = 6;
int odorAirSupply = 5;
int odorPair = 10;
int odorAB = 11;
int odorCD = 8;
int rewardOut = 46;
int lickingLOut = 48;
int lickingROut = 49;

int optoChannelBoth2 = -1;
int optoChannelSolo2 = -1;

void initializeBoard(){
    Serial.begin(9600);
    Serial1.begin(115200);
    delay(100);
    for (int i=0; i<=53; i++){  
      pinMode(i, OUTPUT);
      digitalWrite(i, LOW);
    }
      while(!DF1201S.begin(Serial1)){
        Serial.println("Init failed, please check the wire connection!");
        delay(1000);
      }
      DF1201S.setPrompt(false);
      DF1201S.setLED(false);
      DF1201S.setVol(/*VOL = */19); //Sets audio card volume 20 max.
      DF1201S.switchFunction(DF1201S.MUSIC); //This command makes the speakers say "Music"
      DF1201S.setPlayMode(DF1201S.SINGLE); //Makes it so the player will stop if it reaches the end of a file
}


void dispense(int lickSpout = 0){
  if (lickSpout != 0) {
	if(lickSpout == -1){
	  digitalWrite(H2OL,HIGH);
	  digitalWrite(rewardOut,HIGH);
    	  delay(solenoidOpenTime);
    	  digitalWrite(H2OL,LOW);
	  digitalWrite(rewardOut,LOW);
	}
	else{
	  digitalWrite(H2OR,HIGH);
	  digitalWrite(rewardOut,HIGH);
    	  delay(solenoidOpenTime);
   	  digitalWrite(H2OR,LOW);
	  digitalWrite(rewardOut,LOW);
	}
  }
  else {
    digitalWrite(H2OL,HIGH);
    digitalWrite(H2OR,HIGH);
    digitalWrite(rewardOut,HIGH);
    delay(solenoidOpenTime);
    digitalWrite(H2OL,LOW);
    digitalWrite(H2OR,LOW);
    digitalWrite(rewardOut,LOW);
  }
}



void killTone(){
  DF1201S.setPlayTime(/*Play Time = */100);       //kill tone by advancing to the end of the file
};



static void setToneVol(int toneVol){
  if(toneVol >= 0){
    DF1201S.setVol(toneVol);
  }
}

String toneFileName(int tone_num){
  switch(tone_num){
    case 1:  return "/09K.wav";
    case 2:  return "/9KL.wav";
    case 3:  return "/9KR.wav";
    case 4:  return "/035.wav";
    case 5:  return "/083.wav";
    case 6:  return "/155.wav";
    case 7:  return "/210.wav";
    case 8:  return "/CK2.wav";
    case 9:  return "/CLK.wav";
    case 10: return "/DM1.wav";
    case 11: return "/DM2.wav";
    case 12: return "/HSW.wav";
    case 13: return "/LSW.wav";
    case 14: return "/MD2.wav";
    case 15: return "/MOD.wav";
    case 16: return "/SWP.wav";
    case 17: return "/WHL.wav";
    case 18: return "/WHN.wav";
    case 19: return "/WHR.wav";
    case 20: return "/WN2.wav";
    case 21: return "/WND.wav";
    case 22: return "/WNU.wav";
    case 23: return "/Z1HZ30.wav";
    case 24: return "/Z2HZ80.wav";
    case 25: return "/Z3HZ130.wav";
    case 26: return "/Z4HZ180.wav";
    case 27: return "/Z5HZ230.wav";
    case 28: return "/Z6HZ280.wav";
    case 29: return "/Z7HZ330.wav";
    case 30: return "/Z8HZ130_2.wav";
    case 31: return "/Z8HZ180_2.wav";
    case 32: return "/Z8HZ230_2.wav";
    case 33: return "/Z8HZ280_2.wav";
    case 34: return "/Z9HZWhiteNoise.wav";
    default: return "";
  }
}

void playTone(int tone_num, int tone_length = -1, int toneVol = -1){
  setToneVol(toneVol);
  DF1201S.playFileNum(tone_num);
  DF1201S.setPlayTime(/*Play Time = */0);
  if(tone_length!=-1){
    delay(tone_length);
    killTone();
  };
};

void playTone(String tone_file, int tone_length = -1, int toneVol = -1){
  if(tone_file.length() == 0){
    return;
  }
  setToneVol(toneVol);
  DF1201S.playSpecFile(tone_file);
  DF1201S.setPlayTime(/*Play Time = */0);
  if(tone_length!=-1){
    delay(tone_length);
    killTone();
  };
};

void changeVol(int volNum){
    DF1201S.setVol(volNum);
};


void toneName(){
  String currentFileName = DF1201S.getFileName();
  Serial.print("Current file name: ");
  Serial.println(currentFileName);
}



void setLickThreshold(float newLickThresh){
  lickThresh = newLickThresh;
}

void lickDetect(int lickData[], int lickSpout, int lickExit = 0, int detectDur = 300000, float lickThreshSet = -1) {
      //input arguments:
      //1) lickData (lickedL, lickedR, lickNumL, lickNumR, RT)
      //2) lickSpout (-1=left, 1=right, 2=both)
      //3) lickExit (whether a lick terminates the function; 0=no, 1=yes)
      //4)duration of lick detection in ms
      //5) lickThreshSet (optional; >=0 replaces lickThresh for this call and later calls)
      if(lickThreshSet >= 0){
        lickThresh = lickThreshSet;
      }
      //returns the array lickData:
      //1) lickedL (0/1)
      //2) lickedR (0/1)
      //3) lickNumL (number of left licks, when lickExit !=0)
      //4) lickNumR (number of right licks, when lickExit !=0)
      //5) RT (reaction time; latency to first lick)
      Average<float> lickAveR(3);
      Average<float> lickAveL(3);   
      lickedL=0;
      lickedR=0;
      lickNumL=0;
      lickNumR=0;
      lickPrev=0;
      RT=0;
      timeElapsed_lib=0;
      int exit=0;
      while (exit == 0 && timeElapsed_lib <= detectDur) {
        lickAveR.push(analogRead(A5) - analogRead(A0));
        lickAveL.push(analogRead(A0) - analogRead(A5));
        if (lickSpout != -1 && lickAveR.minimum() > lickThresh) {
          lickedR = 1;
          RT = timeElapsed_lib;
          digitalWrite(lickingROut, HIGH);
          delay(5);
          digitalWrite(lickingROut, LOW);
            if(lickExit==1){exit=1;}
        }
        if (lickSpout != 1 && lickAveL.minimum() > lickThresh) {
          lickedL = 1;
          RT = timeElapsed_lib;
          digitalWrite(lickingLOut, HIGH);
          delay(5);
          digitalWrite(lickingLOut, LOW);
            if(lickExit==1){exit=1;}
        }

        if (lickPrev == 0 && lickedR == 1) {
          lickNumR = lickNumR + 1;
        }
        if (lickPrev == 0 && lickedL == 1) {
          lickNumL = lickNumL + 1;
        }
        if (lickedR == 1 || lickedL == 1) {
           lickPrev = 1;
        }
        else {
          lickPrev = 0;
        }
	lickedL = 0;
	lickedR = 0;
        delay(5);
      }
	if(lickNumL>0){lickedL=1;}
	if(lickNumR>0){lickedR=1;}
    lickData[0] = lickedL;
    lickData[1] = lickedR;
    lickData[2] = lickNumL;
    lickData[3] = lickNumR;
    lickData[4] = RT;
	return lickData;
}



void presentOdorant(int odorant, int odorDur = -1){
  digitalWrite(odorAirSupply, HIGH);
  
  if(odorant == 2){
    digitalWrite(odorAB,HIGH);
  }
  if(odorant == 3){
    digitalWrite(odorPair,HIGH);
  }
  if(odorant == 4){
    digitalWrite(odorPair,HIGH);
    digitalWrite(odorCD,HIGH);
  }

  if(odorDur != -1){
    delay(odorDur);
      digitalWrite(odorAirSupply,LOW);
      digitalWrite(odorPair,LOW);
      digitalWrite(odorAB,LOW);
      digitalWrite(odorCD,LOW);
  }
}



void killOdorants(){
  digitalWrite(odorAirSupply,LOW);
  digitalWrite(odorPair,LOW);
  digitalWrite(odorAB,LOW);
  digitalWrite(odorCD,LOW);
}



void blinkLED(){
  ledState = !ledState;
  digitalWrite(optoChannel,ledState); 
}

// Forward declarations
void timerKill();
void timerKillContinuous();
void timerKillAnalog();
void timerKillAnalogContinuous();
void timerKillBoth();
void timerKillContinuous2();
void timer3TickContinuous2();

void timer3Tick(){
  timer3Count++;
  if(timer3Count >= timer3Target) {
    timerKill();
  }
}

void timer3TickContinuous(){
  timer3Count++;
  if(timer3Count >= timer3Target) {
    timerKillContinuous();
  }
}

void timerKill(){
  Timer1.detachInterrupt();
  Timer3.detachInterrupt();
  Timer3.stop();
  digitalWrite(optoChannel,LOW);
  ledState=LOW;
  timer3Count = 0;
  timer3Target = 0;
}

void timerKillContinuous(){
  Timer3.detachInterrupt();
  Timer3.stop();
  digitalWrite(optoChannel,LOW);
  ledState=LOW;
  timer3Count = 0;
  timer3Target = 0;
}



void timer3TickAnalog(){
  timer3Count++;
  if(timer3Count >= timer3Target) {
    timerKillAnalog();
  }
}

void timer3TickAnalogContinuous(){
  timer3Count++;
  if(timer3Count >= timer3Target) {
    timerKillAnalogContinuous();
  }
}

void timerKillAnalog() {
  Timer1.detachInterrupt();
  Timer3.detachInterrupt();
  Timer1.stop();
  Timer3.stop();
  analogWrite(optoChannel, 0);
  timer3Count = 0;
  timer3Target = 0;
}

void timerKillAnalogContinuous() {
  Timer3.detachInterrupt();
  Timer3.stop();
  analogWrite(optoChannel, 0);
  timer3Count = 0;
  timer3Target = 0;
}

void timer3TickContinuous2(){
  timer3Count++;
  if(timer3Count >= timer3Target) {
    timerKillContinuous2();
  }
}

void timerKillContinuous2(){
  Timer3.detachInterrupt();
  Timer3.stop();
  if (optoChannelSolo2 >= 0) {
    digitalWrite(optoChannelSolo2, LOW);
  }
  ledState = LOW;
  timer3Count = 0;
  timer3Target = 0;
  optoChannelSolo2 = -1;
}



void sineGenerator() {
  float s = sin(phase);       // -1..+1
  float scaled = (s + 1.0f) * 127.5f; // 0..255
  analogWrite(optoChannel, (int)(scaled + 0.5f));
  phase += phaseIncrement;
  if (phase >= TWO_PI) {
    phase -= TWO_PI;
  }
}

void timer3TickBoth(){
  timer3Count++;
  if (timer3Count >= timer3Target) {
    timerKillBoth();
  }
}

void timerKillBoth(){
  Timer1.detachInterrupt();
  Timer3.detachInterrupt();
  Timer3.stop();
  digitalWrite(optoChannel, LOW);
  if (optoChannelBoth2 >= 0) {
    digitalWrite(optoChannelBoth2, LOW);
  }
  ledState = LOW;
  timer3Count = 0;
  timer3Target = 0;
  optoChannelBoth2 = -1;
}



void photoStim(float freq, float dur, char stimType){
	//freq = frequency (Hz); 0 = constant illumination
	//dur = duration (ms)
	//stimType = "d" (digital, ie square pulses), or "a" (analog, ie sinusoids)
	ledState=LOW;
    digitalWrite(optoChannel, LOW);
    timer3Count = 0;
    if (stimType == 'd') {
        if(freq==0){
          Timer1.stop();
          Timer1.detachInterrupt();
          Timer3.stop();
          Timer3.detachInterrupt();
          delay(2);
          digitalWrite(optoChannel,HIGH);
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          delay(1);
          Timer3.attachInterrupt(timer3TickContinuous);
          Timer3.restart();
        }
        else{
          Timer3.stop();
          Timer3.detachInterrupt();
          Timer1.stop();
          Timer1.detachInterrupt();
          delay(2);
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          Timer3.attachInterrupt(timer3Tick);
          Timer3.restart();
          delay(1);
          Timer1.initialize((unsigned long)((1000.0/freq)/2.0*1000.0));
          Timer1.attachInterrupt(blinkLED);
          Timer1.restart();
        }
    }
    else if (stimType == 'a') {
        if (freq == 0) {
          Timer1.stop();
          Timer1.detachInterrupt();
          Timer3.stop();
          Timer3.detachInterrupt();
          delay(2);
          analogWrite(optoChannel, 255);
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          delay(1);
          Timer3.attachInterrupt(timer3TickAnalogContinuous);
          Timer3.restart();
        }
        else {
          // Setup a timer to kill the analog stimulus after 'dur' ms
          Timer3.stop();
          Timer3.detachInterrupt();
          Timer1.stop();
          Timer1.detachInterrupt();
          delay(2);
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          Timer3.attachInterrupt(timer3TickAnalog);
          Timer3.restart();
          delay(1);
          int samplesPerCycle = 64;
          float sampleRate = freq * samplesPerCycle;
          float sampleIntervalUs = 1e6 / sampleRate;
          Timer1.initialize((unsigned long)sampleIntervalUs);
          Timer1.attachInterrupt(sineGenerator);
          Timer1.restart();
          phaseIncrement = TWO_PI / samplesPerCycle;
          phase = -M_PI / 2;
        }
      }
}

void photoStim2(int optoChannel2, float freq, float dur, char stimType){
	//freq = frequency (Hz); 0 = constant illumination
	//dur = duration (ms)
	//stimType = "d" (digital, ie square pulses), or "a" (analog, ie sinusoids)
	ledState=LOW;
    digitalWrite(optoChannel2, LOW);
    timer3Count = 0;
    if (stimType == 'd') {
        if(freq==0){
          Timer1.stop();
          Timer1.detachInterrupt();
          Timer3.stop();
          Timer3.detachInterrupt();
          delay(2);
          digitalWrite(optoChannel2,HIGH);
          optoChannelSolo2 = optoChannel2;
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          delay(1);
          Timer3.attachInterrupt(timer3TickContinuous2);
          Timer3.restart();
        }
        else{
          Timer3.stop();
          Timer3.detachInterrupt();
          Timer1.stop();
          Timer1.detachInterrupt();
          delay(2);
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          Timer3.attachInterrupt(timer3Tick);
          Timer3.restart();
          delay(1);
          Timer1.initialize((unsigned long)((1000.0/freq)/2.0*1000.0));
          Timer1.attachInterrupt(blinkLED);
          Timer1.restart();
        }
    }
    else if (stimType == 'a') {
        if (freq == 0) {
          Timer1.stop();
          Timer1.detachInterrupt();
          Timer3.stop();
          Timer3.detachInterrupt();
          delay(2);
          analogWrite(optoChannel2, 255);
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          delay(1);
          Timer3.attachInterrupt(timer3TickAnalogContinuous);
          Timer3.restart();
        }
        else {
          // Setup a timer to kill the analog stimulus after 'dur' ms
          Timer3.stop();
          Timer3.detachInterrupt();
          Timer1.stop();
          Timer1.detachInterrupt();
          delay(2);
          timer3Count = 0;
          timer3Target = (unsigned long)dur;
          Timer3.initialize(1000);
          Timer3.attachInterrupt(timer3TickAnalog);
          Timer3.restart();
          delay(1);
          int samplesPerCycle = 64;
          float sampleRate = freq * samplesPerCycle;
          float sampleIntervalUs = 1e6 / sampleRate;
          Timer1.initialize((unsigned long)sampleIntervalUs);
          Timer1.attachInterrupt(sineGenerator);
          Timer1.restart();
          phaseIncrement = TWO_PI / samplesPerCycle;
          phase = -M_PI / 2;
        }
      }
}

void photoStimBoth(int optoChannel2, float freq, float dur){
  // Channel 1 (optoChannel): pulsed stimulation
  // Channel 2 (optoChannel2): constant HIGH for same duration

  ledState = LOW;
  digitalWrite(optoChannel, LOW);
  digitalWrite(optoChannel2, LOW);
  timer3Count = 0;
  optoChannelBoth2 = optoChannel2;

  Timer3.stop();
  Timer3.detachInterrupt();
  Timer1.stop();
  Timer1.detachInterrupt();
  delay(2);

  // Turn on MD suppression channel continuously
  digitalWrite(optoChannel2, HIGH);

  // Kill both after dur ms
  timer3Target = (unsigned long)dur;
  Timer3.initialize(1000);
  Timer3.attachInterrupt(timer3TickBoth);
  Timer3.restart();
  delay(1);

  // Pulse L6CT channel
  Timer1.initialize((unsigned long)((1000.0 / freq) / 2.0 * 1000.0));
  Timer1.attachInterrupt(blinkLED);   // blinkLED toggles optoChannel
  Timer1.restart();
}






