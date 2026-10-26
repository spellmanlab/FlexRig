Spellman Lab - Timothy Spellman - UConn Health
# flexRigFunctions

Library for the FlexRig head-fixed behavior board: DFPlayer audio, water solenoids, capacitive lick detection, odor valves, and optogenetic timing.


## Additional Library Installation
Required third-party libraries: elapsedMillis, Average, TimerOne, TimerThree, DFRobot_DF1201S.

#include <elapsedMillis.h>
#include <Average.h>
#include <math.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <DFRobot_DF1201S.h>


## Installation
Download the required libraries into the Arduino libraries directory, then open the examples folder in this library and run cleanlines to test the rig.


## Board pins
Water left / right: 7 / 6. Solenoid open time is 25 ms.
Reward sync: 46.
Lick sync left / right: 48 / 49.
Lick sense: A0 is the left spout, A5 is the right spout. A lick is the difference between those two channels.
Odor air supply: 5. Odor A/B valve: 11. Odor-pair valve: 10. Odor C/D valve: 8.
Optogenetic output: 38.
DFPlayer serial: Serial1 at 115200. USB serial: Serial at 9600.


## Functions

void initializeBoard()
	Sets pins 0-53 to outputs and drives them low.
	Starts the DFPlayer, turns prompts and its LED off, sets volume to 19, switches to music mode, and sets single-file playback. In single-file mode a track stops when the file ends.


void dispense(int lickSpout = 0)
	Opens a water solenoid for solenoidOpenTime (25 ms) and pulses the reward sync pin.
	lickSpout -1 = left (pin 7). Any other non-zero value = right (pin 6). 0 = both.


void playTone(int tone_num, int tone_length = -1, int toneVol = -1)
void playTone(String tone_file, int tone_length = -1, int toneVol = -1)
	Plays a DFPlayer file. The integer form plays by copy order (playFileNum). The string form plays a card path (playSpecFile). The path must start with '/' and include the extension, for example "/CLK.wav". An empty string returns without playing.
	tone_length -1, or omitting it, plays until the file ends or something else calls killTone. Any other value waits that many milliseconds, then calls killTone. The wait blocks the sketch. A duration longer than the file lets the file finish and then keeps the sketch inside playTone until the wait is over.
	toneVol of 0 or greater sets the DFPlayer volume before playback. The new volume stays in effect until changeVol or another playTone call sets it. Omit toneVol, or pass a negative value, to leave the current volume alone. To set volume without cutting the file short, pass -1 for tone_length: playTone(18, -1, 12) or playTone("/CLK.wav", -1, 12).


String toneFileName(int tone_num)
	Returns the card path for a file number, so playTone(toneFileName(n)) plays that named file instead of whatever currently occupies slot n. An unknown number returns "", and the string form of playTone then plays nothing.
	1 /09K.wav    2 /9KL.wav    3 /9KR.wav    4 /035.wav    5 /083.wav
	6 /155.wav    7 /210.wav    8 /CK2.wav    9 /CLK.wav    10 /DM1.wav
	11 /DM2.wav   12 /HSW.wav   13 /LSW.wav   14 /MD2.wav   15 /MOD.wav
	16 /SWP.wav   17 /WHL.wav   18 /WHN.wav   19 /WHR.wav   20 /WN2.wav
	21 /WND.wav   22 /WNU.wav
	23-29 /Z1HZ30.wav through /Z7HZ330.wav
	30-33 /Z8HZ130_2.wav, /Z8HZ180_2.wav, /Z8HZ230_2.wav, /Z8HZ280_2.wav
	34 /Z9HZWhiteNoise.wav
	Slots used by the set-shifting sketches: 7 right whisker (/210.wav), 9 left whisker (/CLK.wav), 17 left-correct (/WHL.wav), 18 trial tone (/WHN.wav), 19 right-correct (/WHR.wav). Those sketches call playTone with the integer, so this table applies only when the card was copied in this order.


void killTone()
	Seeks the DFPlayer to 100 seconds, which cuts off the current file.


void changeVol(int volNum)
	Sets the DFPlayer volume. The value persists until the next changeVol or a playTone call that passes toneVol.


void toneName()
	Reads the file currently selected on the DFPlayer and prints it to Serial as "Current file name: ...".


void setLickThreshold(float newLickThresh)
	Sets the lick-detection threshold without running lickDetect. The startup value is 16. The new value persists until the next setLickThreshold or a lickDetect call that passes lickThreshSet.


void lickDetect(int lickData[], int lickSpout, int lickExit = 0, int detectDur = 300000, float lickThreshSet = -1)
	Watches the lick sensors for detectDur milliseconds (default 300000) and writes five values into lickData:
		[0] lickedL, 1 if any left lick was detected
		[1] lickedR, 1 if any right lick was detected
		[2] lickNumL, left lick count
		[3] lickNumR, right lick count
		[4] RT, milliseconds from the start of this call to the most recent detected lick
	Left is analogRead(A0) - analogRead(A5). Right is analogRead(A5) - analogRead(A0). A side counts when the minimum of its last three samples exceeds lickThresh. Each detected side pulses its sync pin (48 left, 49 right) for 5 ms.
	lickSpout -1 = left only. 1 = right only. Any other value, including 0 and 2, watches both spouts.
	lickExit 1 returns on the first lick. lickExit 0 keeps sampling until detectDur.
	lickThreshSet of 0 or greater replaces lickThresh for this call and for later calls. Omit it, or pass a negative value, to keep the current threshold.
	Example: lickDetect(lickData, 2, 1, 1500, 12);


void presentOdorant(int odorant, int odorDur = -1)
	Turns on the odor air supply (pin 5), then opens valves for the requested odorant.
		1 = air on, odor A/B valve left low
		2 = odor A/B valve high (pin 11)
		3 = odor-pair valve high (pin 10)
		4 = odor-pair valve and odor C/D valve high (pins 10 and 8)
	odorDur -1 leaves the valves on until killOdorants. Any other value waits that many milliseconds, then turns the air and all odor valves off. The wait blocks the sketch.


void killOdorants()
	Turns off the odor air supply and all odor valves.


void photoStim(float freq, float dur, char stimType)
	Drives pin 38. freq is Hz; 0 is steady light. dur is milliseconds. stimType 'd' is a digital square wave. stimType 'a' is an analog sine, or full-scale analog output when freq is 0. Timing is interrupt-driven and the call returns while the stimulus is still running. A 1 ms tick turns the output off after dur.


void photoStim2(int optoChannel2, float freq, float dur, char stimType)
	Same arguments as photoStim, plus a pin number.
	Steady digital (stimType 'd', freq 0) holds optoChannel2 high for dur milliseconds, then turns that pin off.
	Pulsed digital still blinks pin 38, through blinkLED, rather than optoChannel2.
	Steady analog writes optoChannel2. The sine mode writes pin 38.


void photoStimBoth(int optoChannel2, float freq, float dur)
	Pulses pin 38 at freq Hz and holds optoChannel2 high for the same dur milliseconds, then turns both off.


void blinkLED()
	Toggles pin 38. Called by the pulse timers inside photoStim, photoStim2, and photoStimBoth.


void timerKill()
	Stops Timer1 and Timer3 and drives pin 38 low. Used to end a photoStim pulse train.


void timerKillBoth()
void timer3TickBoth()
	Timer helpers for photoStimBoth. timer3TickBoth counts 1 ms ticks and timerKillBoth turns off pin 38 and the second opto pin when dur is reached.
