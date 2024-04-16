//NOTE: the following third party libraries are required: elapsedMillis, Average, DFRobot_DF1201S
#include <elapsedMillis.h>
    elapsedMillis timeElapsed_lib;
#include <Average.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <DFRobot_DF1201S.h>
    DFRobot_DF1201S DF1201S;

int solenoidOpenTime = 25;             //open time on water solenoid, in ms (~25ms @ ~3psi = ~4uL)
float lickThresh = 20;                 //threshold voltage difference for lick detection
int H2OVol = 4;			//amount of water dispensed each time (uL)
int lickPrev=0;
int lickedL=0;                        
int lickedR=0;
int lickNumL=0;
int lickNumR=0;
int RT=0;

int LEDChannel = 40;
int optoChannel = 38;
int ledState = LOW;
int H2OL = 7;
int H2OR = 6;
int odorAirSupply = 5;
int odorPair = 10;
int odorAB = 11;
int odorCD = 8;
int rewardOut = 46;
int lickingLOut = 48;
int lickingROut = 49;



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



void playTone(int tone_num, int tone_length = -1){
  DF1201S.playFileNum(tone_num);
  DF1201S.setPlayTime(/*Play Time = */0);
  if(tone_length!=-1){
    delay(tone_length);
    DF1201S.setPlayTime(/*Play Time = */3);     //kill tone by advancing to the end of the file
  };
};



void killTone(){
  DF1201S.setPlayTime(/*Play Time = */3);       //kill tone by advancing to the end of the file
};



void lickDetect(int lickData[], int lickSpout, int lickExit = 0, int detectDur = 300000) {
      //input arguments:
      //1) lickData (lickedL, lickedR, lickNumL, lickNumR, RT)
      //2) lickspout (-1=left, 1=right, 0=both)
      //3) lickExit (whether a lick terminates the function; 0=no, 1=yes)
      //4)duration of lick detection in ms
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
  if(ledState==LOW){ledState=HIGH;}
  else{ledState=LOW;}
  digitalWrite(optoChannel,ledState); 
}

void timerKill(){
  Timer1.detachInterrupt();
  digitalWrite(optoChannel,LOW);
  ledState=LOW;
}

void photoStim(float freq, float dur){
	//freq = frequency (Hz); 0 = constant illumination
	//dur = duration (ms)
	ledState==LOW;
	if(freq==0){digitalWrite(optoChannel,HIGH);}
	else{ 
	  Timer3.attachInterrupt(timerKill);
	  Timer3.initialize(dur*1000);
	  Timer3.restart();
	  delay(1);
	  Timer1.attachInterrupt(blinkLED);	  
	  Timer1.initialize((1000/freq)/2*1000);
	  Timer1.restart();
	  
	}
}





