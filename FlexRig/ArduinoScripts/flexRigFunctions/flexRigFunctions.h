
#ifndef FLEXRIGFUNCTIONS_H
#define FLEXRIGFUNCTIONS_H

    void initializeBoard();
    void dispense(int lickSpout = 0);
    void killTone();
    void playTone(int tone_num, int tone_length = -1);
    void lickDetect(int lickData[], int lickSpout, int lickExit = 0, int detectDur = 300000);
    void killOdorants();
    void presentOdorant(int odorant, int odorDur = -1);
    void blinkLED();
    void timerKill();    
    void photoStim(float freq, float dur);
    
    

#endif
