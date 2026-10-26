
#ifndef FLEXRIGFUNCTIONS_H
#define FLEXRIGFUNCTIONS_H

    void initializeBoard();
    void dispense(int lickSpout = 0);
    void killTone();
    void playTone(int tone_num, int tone_length = -1, int toneVol = -1);
    void playTone(String tone_file, int tone_length = -1, int toneVol = -1);
    String toneFileName(int tone_num);
    void lickDetect(int lickData[], int lickSpout, int lickExit = 0, int detectDur = 300000, float lickThreshSet = -1);
    void setLickThreshold(float newLickThresh);
    void killOdorants();
    void presentOdorant(int odorant, int odorDur = -1);
    void blinkLED();
    void timerKill();    
    void photoStim(float freq, float dur, char stimType);
    void toneName();
    void changeVol(int volNum);
    void timerKillBoth();
    void timer3TickBoth();
    void photoStimBoth(int optoChannel2, float freq, float dur);
    void photoStim2(int optoChannel2, float freq, float dur, char stimType);
	

#endif
