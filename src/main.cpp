#include <Arduino.h>
//Legger til Expander MCP23017
#include <Adafruit_MCP23X17.h>
#include "MIDIUSB.h"
#include <FastLED.h>
#include <Vector.h>
//Starter mcp project 
Adafruit_MCP23X17 mcp1,mcp2,mcp3;

//Globalevariables that are the 6-bit words that should be sent from the intro
uint8_t introInstOrdA = 0;
uint8_t introInstOrdB = 0;
//And the vers
uint8_t VersInstOrdA = 0;
uint8_t VersInstOrdB = 0;
//And the refreng
uint8_t refrengInstOrdA = 0;
uint8_t refrengInstOrdB = 0;
//Main funciton
uint8_t usersMood = 0;
uint8_t userOrdA = 0;

//More Globelvariables that for the sliders
float sliderInst1= 0;
float sliderInst2= 0;
float sliderBass = 0;
float sliderDrums = 0;
float sliderVers = 0;
float sliderRefreng = 0;
float sliderIntro = 0;

//Int for what kind of BMP that are being played
int BPM = 0;
//An array of the diffrent BPM for the 6 diffrent moods
//CHANGE TO THE CORRECT BPM 
//the first number is being used as the mood is not selected yet
int arrayOfBPM[] = {0,128,125,105,108,95,0};
//The diffrent button pins that are being used 
int buttonValueLoop = 1;
int buttonValueSkipForward = 1;
int buttonValueSkipBackwards = 1;
//Stop values and stuff
int stop = 1;
bool pauseState = false;
//Need a function so that the program knows what kind of section the user is in and stuff like that 
int currentLight = 0;
//section is zero if user is in intro
int section = 0;
int startOver = 0;
int LightsPerSector = 8;
bool systemPlayingSound = 0;
//Set up for fastled
//sets the LED_PIN too be C6 CHANGE THIS IF NOT USING THIS PIN 
int LED_PIN = 5;
//CHANGE TO NUMBER LEDS THAT ARE BEING USED
#define DATA_PIN 5
const int NUM_LEDS = 167;
CRGB leds[NUM_LEDS];
//We need GlobalVariables to know if something has changed in the loop.
// global variables to store the previous values 
int changeHasCome = 0;
int prevIntroInstOrdA = 0;
int prevIntroInstOrdB = 0;
int prevVersInstOrdA = 0;
int prevVersInstOrdB = 0;
int prevRefInstOrdA = 0;
int prevRefInstOrdB = 0;
//Millies to be user in light function
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long previousMillisForLoop = 0;
unsigned long previousMillisForLights= 0;
//Globalvariables too be used in slider-leds

//CHANGE THIS TO ACCORDING MAX VALUE
int maxSliderValue = 1023;
//AND THIS
int numberOfSliderLeds = 5;

//ReadFrom here
struct PinsData {
  uint8_t ordToRead1;
  uint8_t ordToRead2;
};

//Vector of the last 100 values measuered for each word(two boxses)for each section
Vector<int> introInstOrd1Values;
Vector<int> introInstOrd2Values;
//Vector used in vers
Vector<int> versInstOrd1Values;
Vector<int> versInstOrd2Values;
//Vector used in refreng
Vector<int> refrengInstOrd1Values;
Vector<int> refrengInstOrd2Values;
//Need used in Mood
Vector<int> moodValues;

//We need prevWord to check if send midi or not
uint8_t sendMePreviousIntroOrdA;
uint8_t sendMePreviousIntroOrdB;
uint8_t sendMeIntroOrdA;
uint8_t sendMeIntroOrdB;
bool sendMeIntro1;
bool sendMeIntro2; 
//We need the same values but for vers and refreng
uint8_t sendMePreviousVersOrdA;
uint8_t sendMePreviousVersOrdB;
uint8_t sendMeVersOrdA;
uint8_t sendMeVersOrdB;
bool sendMeVers1;
bool sendMeVers2; 
//Refreng
uint8_t sendMePreviousRefrengOrdA;
uint8_t sendMePreviousRefrengOrdB;
uint8_t sendMeRefrengOrdA;
uint8_t sendMeRefrengOrdB;
bool sendMeRefreng1;
bool sendMeRefreng2; 
//Listes for moodBox
uint8_t sendMeMood;
uint8_t sendMePrevMood;
bool sendMeMoodBool;
//Light stuff
const long intervalOn = 1000; // LED on for 1 second
const long intervalOff = 500; // LED off for 0.5 seconds
boolean ledOn = false;
bool lightUpIntroLeds1 = true;
bool lightMeUp = true;
unsigned long MixBoxMillis = 0;
bool changeSection = true;

//newPausePlayButtonStuff
unsigned long LastButtonPressed = 0;
int buttonValueStop = 1;
bool loopState = true;
//Need this too turn of all leds if being pressed again
bool prevStopState= false;
unsigned long progressBarStart=0;
unsigned long currentProgressTime = 0;

void noteOn(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void progressBar(int BPM){
  //pause mode light up red

  currentProgressTime = millis();
  float MaxTimeLed = ((32.0 * 3.0) / (BPM))*60000;

  
  if (pauseState)
  {
    for (int i = 143; i < (LightsPerSector*3+143); i++)
    {
      leds[i]=CRGB::Red;
    }
    currentLight = section * LightsPerSector +143; 
    
  }
  else{

      if (currentProgressTime > progressBarStart + MaxTimeLed)
      {
        progressBarStart = currentProgressTime;

      }

    int color_list[] = {180,180,180,180, 180,180,180,180,
                        200,200,200,200, 200,200,200,200,
                        220,220,220,220, 220,220,220,220};
    float totalBrightness = 8.0*3.0;
    unsigned long my_time = (currentProgressTime-progressBarStart);
    float currentBrightness = (currentProgressTime-progressBarStart)/MaxTimeLed*totalBrightness;
    for (int i = 143; i < (LightsPerSector*3+143); i++)
    {
      if (currentBrightness >= 1)
      {
        
        leds[i] = CHSV(color_list[i-143],200,255);
        currentBrightness-=1;
      }
      else{
        leds[i] = CHSV(color_list[i-143],200, currentBrightness * 255);
        currentBrightness = 0;
      }
    }
  }
}
// First parameter is the event type (0x0B = control change). Pluss Jørgen
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
PinsData readPins(uint8_t ordToRead1,uint8_t ordToRead2, int numberOfMCP){
  PinsData data;
  int vectorMe1 = 0;
  int vectorMe2 = 0;
  //Here is the right pinorder
  //Box 1 pin1,2,3 = 7,6,5
  //Box 2 pin1,2,3 = 4,3,2
  //Box 3 pin1,2,3 = 1,0,8
  //Box 4 pin1,2,3 = 9,10,11
  if (numberOfMCP == 1)
  {
    //Serial.print("Setter i gang readFunksjon");
    ordToRead1 = 0;
      for(int i = 7; i > 1; i--){
        ordToRead1 |= mcp1.digitalRead(i) << (7-i);
        // Serial.print(mcp1.digitalRead(i));
      }
  //all of this is the word A 
  //Reads now from A6-A7 and from B0-B3
    ordToRead2 = 0;
    ordToRead2 |= mcp1.digitalRead(1) << (0);
    ordToRead2 |= mcp1.digitalRead(0) << (1);
    ordToRead2 |= mcp1.digitalRead(8) << (2);
      for (int x = 9; x <= 11; x++)
       {
        //Change to be bit 3,4,5
        ordToRead2 |= mcp1.digitalRead(x) << (x-6); 
        }
        
      // Serial.print(ordToRead2);
      data.ordToRead1 = ordToRead1;
      data.ordToRead2 = ordToRead2;

      // Serial.print("Her kommer ordToRead1");
      // Serial.print("\n");
      // Serial.print(ordToRead1);
      // Serial.print("\n");
      vectorMe1 = ordToRead1;
      vectorMe2 = ordToRead2;
      introInstOrd1Values.PushBack(vectorMe1);
      introInstOrd2Values.PushBack(vectorMe2);
      // Serial.print("Her kommer ordToRead1 in intro");
      // Serial.print("\n");
      // Serial.print(ordToRead1);
      // Serial.print("\n");
      // Serial.print("VectorSize:\n");
      //Serial.print(introInstOrd1Values.Size());
      //CAN I STILL ADD POP BACK
      vectorMe1 = 0;
      vectorMe2 = 0;
        return data;
  }
  else if (numberOfMCP == 2)
  {
  ordToRead1 = 0;
    for(int i = 7; i > 1; i--){
      ordToRead1 |= mcp2.digitalRead(i) << (7-i);
      }
      //all of this is the word A 
      //Reads now from A6-A7 and from B0-B3
  ordToRead2 = 0;
    for (int x = 9; x <= 11; x++){
        ordToRead2 |= mcp2.digitalRead(x) << (x-6); 
        // Serial.print(mcp2.digitalRead(x));
        }
      ordToRead2 |= mcp2.digitalRead(1) << (0);
      ordToRead2 |= mcp2.digitalRead(0) << (1);
      ordToRead2 |= mcp2.digitalRead(8) << (2);
      data.ordToRead1 = ordToRead1;
      data.ordToRead2 = ordToRead2;
      //I want to convert the 6-bit words too integer too be later used to check if the midi-signal should be sent
      vectorMe1 = ordToRead1;
      vectorMe2 = ordToRead2;
      // Serial.print("Her kommer ordToRead1 in vers");
      // Serial.print("\n");
      // Serial.print(ordToRead1);
      // Serial.print("\n");
      versInstOrd1Values.PushBack(vectorMe1);
      versInstOrd2Values.PushBack(vectorMe2);
      vectorMe1 = 0;
      vectorMe2 = 0;
      //  Serial.print("Her kommer ordToRead1 in vers");
      //  Serial.print("\n");
      //  Serial.print(ordToRead1);
      //  Serial.print("\n");
        return data;
  }
  else if (numberOfMCP == 3)
  {
    ordToRead1 = 0;
      for(int i = 7; i > 1; i--){
        ordToRead1 |= mcp3.digitalRead(i) << (7-i);
        }
      //all of this is the word A 
      //Reads now from A6-A7 and from B0-B3
    ordToRead2 = 0;
      for (int x = 9; x <= 11; x++)
       {
        ordToRead2 |= mcp3.digitalRead(x) << (x-6); 
        // Serial.print(mcp3.digitalRead(x));
        }
    ordToRead2 |= mcp3.digitalRead(1) << (0);
    ordToRead2 |= mcp3.digitalRead(0) << (1);
    ordToRead2 |= mcp3.digitalRead(8) << (2);
    data.ordToRead1 = ordToRead1;
    data.ordToRead2 = ordToRead2;
    vectorMe1 = ordToRead1;
    vectorMe2 = ordToRead2;
    refrengInstOrd1Values.PushBack(ordToRead1);
    refrengInstOrd2Values.PushBack(ordToRead2);
    // Serial.print("Her kommer ordToRead1 in refreng");
    // Serial.print("\n");
    // Serial.print(ordToRead2);
    // Serial.print("\n");
    vectorMe1 = 0;
    vectorMe2 = 0;
        return data;
  }

}



void controlChange(byte channel, byte control, byte value)
{
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void readMainFunctions(){
  //This funciton need to read the diffrent buttons that the arduino micro pro are connected too 
  //This is going too be sent as midi-channel 0
  usersMood = 0;
  userOrdA = 0;
  int choosenMood = 0;
  unsigned long readTime = millis();
  //Saving the button values as the first 4 values of the 7 bit word
  //We are reading the buttons now
  //The buttons are connected too pin 7 on the arduino, and then b5-b7 on the secont expander
  //Lets have the stop button on the 7 pin


  //Check if a button is pressed. The button value should be true until it gets sent
  //if the button has not been pressed or that we are in the stop section

  //userOrdA is going to get or´d with the value that humør value that gets sent by sendMidi
  if (readTime-LastButtonPressed>=400)
  {
    buttonValueStop = mcp2.digitalRead(13);
    if (buttonValueStop == 0)
    {
      pauseState = !pauseState;
      LastButtonPressed = readTime;
      //We have been in pause mode turn off all leds
      if (pauseState == true)
      {
        prevStopState = true;
      }
      
    }
  }
  userOrdA |= pauseState << 6;

if (readTime-LastButtonPressed>=355)
{
  buttonValueLoop = mcp2.digitalRead(15);
  if (buttonValueLoop==0)
  {
    loopState = !loopState;
    LastButtonPressed = readTime;
  }
}
  userOrdA |= loopState << 5;
//Can change this forward and backwards function aswell if this works 
  if (buttonValueSkipBackwards == 1)
  {
    buttonValueSkipBackwards = mcp2.digitalRead(12);
    userOrdA |= mcp2.digitalRead(12) << 3;
  }
  if (buttonValueSkipForward == 1)
  {
    buttonValueSkipForward = mcp2.digitalRead(14);
    userOrdA |= mcp2.digitalRead(14) << 4;
    //Change the userOrdA bit value and the integer too 1 
  }
   //We need to change stop too one if the stop button is being played and if it gets pressed again we need to change it too zero
  //We also need to change the loop variable if being pressed
  //Changing the section if some of the skip and backward function are being pressed and we are not in the last or first secetion
  //We need to read the pins from the First MCP23017 extander but for the unused pins B4-B6

  //CHANGE TO CORRECT ADRESS

  for(int i = 12; i <= 14; i++){
    //saving the values of the diffrents pins in userMood
    //starts with saving it at the least significant bit
    usersMood |= mcp1.digitalRead(i) << (i-12);
  }
  //changing the 3-bit word too integer

  //CHANGE THIS SO IT SAVE THE BPM AS THE LAST SENT MOOD

  choosenMood = (int)usersMood;
  //saving the read mood in the vector
  moodValues.PushBack(choosenMood);
  //saving the BPM of chellected mood
  BPM = arrayOfBPM[choosenMood];
  //Starting the led function 
  if (readTime > currentProgressTime + 250){
    progressBar(BPM);
    //Serial.println("LIghtupdated");
  }
} 

void readIntroInstruments(){
  //reads pin a0-a5 from the MCP23017 expander
  //puts this in a 6-bit word called introInst 
    //note that 0xb000 is that the box is not connected 

    //The problem with the code is that if I do not have this in lines under. The system are not able to read the mcppins.
    //But if I do the system runs a X number of times then it stops for some reason!!!

  int a = 1;
  PinsData data = readPins(introInstOrdA,introInstOrdB,a);
  introInstOrdA = data.ordToRead1;
  // Serial.print(introInstOrdA);
  introInstOrdB = data.ordToRead2;
  //We know have two 6-bit words that we can send too rasberry-pi
  //This are going to be sent of as channel 1
  //Checking if the user is playing sound from intro. 
   if (introInstOrdA != 0 || introInstOrdB != 0) {
    systemPlayingSound = 1;
  }
  else{
    systemPlayingSound = 0;
  }
   //we need to check if the user has had a change to the MixBox
   if (introInstOrdA != prevIntroInstOrdA || introInstOrdB != prevIntroInstOrdB) {
  } 
  else {
    changeHasCome = 0;
  }
  //Saving the new changes
  prevIntroInstOrdA = introInstOrdA;
  prevIntroInstOrdB = introInstOrdB;
  // Serial.print("Her kommer intro OrdA:\n");
  // Serial.print("\n");
  // Serial.print(introInstOrdA);
  // Serial.print("\n");
  //If introInstOrdA or introInstOrdB is true x out of 100 times make true@
  //We therfor need to save the last 100 values
}

void readVersInstrument(){
  //reads pin a0-a5 from the second MCP23017 expander
  //puts this in a 6-bit word
    //note that 0xb000 is that the box is not connected 
    int b = 2;
    PinsData data = readPins(introInstOrdA,introInstOrdB,b);
    VersInstOrdA = data.ordToRead1;
    VersInstOrdB = data.ordToRead2;
  //We know have two 6-bit words that we can send too rasberry-pi
  //Checking if the system is playing sound from Vers
   if (VersInstOrdA != 0 || VersInstOrdB != 0) {
    systemPlayingSound = 1;
  }
   else{
    systemPlayingSound = 0;
  }
   //we need to check if the user has made a change to the MixBox
   if (VersInstOrdA != prevVersInstOrdA || VersInstOrdB != prevVersInstOrdB) {
    changeHasCome = 1;
  } else {
    changeHasCome = 0;
  }
  //Saving the new changes
  prevVersInstOrdA = VersInstOrdA;
  prevVersInstOrdB = VersInstOrdB;
  // Serial.print("Her kommer vers \n");
  // Serial.print(VersInstOrdA);
  // Serial.print(VersInstOrdB);
  //Saving the last values
}

void readRefrengInstrument(){
  //reads pin a0-a5 from the third MCP23017 expander
  //puts this in a 6-bit word
  //note that 0xb000 is that the box is not connected 
  int c = 3;
  PinsData data = readPins(refrengInstOrdA,refrengInstOrdB,c);
  refrengInstOrdA = data.ordToRead1;
  refrengInstOrdB = data.ordToRead2;
  //We know have two 6-bit words that we can send too rasberry-pi
  //each MCP23017 expander now has B4-B7 aviable(4-bits)
  //Check if the system is playing a sound from refreng
   if (refrengInstOrdA != 0 || refrengInstOrdB != 0) {
    systemPlayingSound = 1;
  }
   else{
    systemPlayingSound = 0;
  }
   //we need to check if the user has made a change to the MixBox
   if (refrengInstOrdA != prevRefInstOrdA || refrengInstOrdB != prevRefInstOrdB) {
    changeHasCome = 1;
  } else {
    changeHasCome = 0;
  }
  //Saving the new changes
  prevRefInstOrdA = refrengInstOrdA;
  prevRefInstOrdB = refrengInstOrdB;
  // Serial.print("Her kommer refreng \n");
  // Serial.print(refrengInstOrdA);
  // Serial.print(refrengInstOrdB);
  //Saving the last values 
}

void sendMidi(){
  //Here Midisignals are being sent
  //Getting each 6-bit word from each section ready to be sent on the right channel 
  if (!sendMeIntro1)
  {
    sendMeIntroOrdA = sendMePreviousIntroOrdA;
  }
  if (!sendMeIntro2)
  {
    sendMeIntroOrdB = sendMePreviousIntroOrdB;
  }
  if (!sendMeVers1)
  {
    sendMeVersOrdA = sendMePreviousVersOrdA;
  }
  if (!sendMeVers2)
  {
    sendMeVersOrdB = sendMePreviousVersOrdB;
  }
  if (!sendMeRefreng1)
  {
    sendMeRefrengOrdA = sendMePreviousRefrengOrdA;
  }
  if (!sendMeRefreng2)
  {
    sendMeRefrengOrdB = sendMePreviousRefrengOrdB;
  }
  if (sendMeRefrengOrdB == 0 && sendMeRefrengOrdA == 0 && sendMeVersOrdB == 0 &&sendMeVersOrdA == 0 && sendMeIntroOrdA == 0 && sendMeIntroOrdB == 0)
  {
    systemPlayingSound = true; 
  }
  if (sendMeMoodBool == false)
  {
    sendMeMood = sendMePrevMood;
  }
  
  controlChange(0,sendMeMood,0);
  MidiUSB.flush();

  controlChange(1,sendMeIntroOrdA,sendMeIntroOrdB);
  MidiUSB.flush();

  controlChange(2,sendMeVersOrdA,sendMeVersOrdB);
  MidiUSB.flush();

  controlChange(3,sendMeRefrengOrdA,sendMeRefrengOrdB);
  MidiUSB.flush();


}

//Here comes the slider functions
void readSliderValues(){
    //reads value from slider that is connected to pin A9 on the arduino micro pro
    sliderInst2 = analogRead(A9)/1023.0;
    // sliderInst2 = 1.0*pow(10.0,sliderInst2)-1.0;
    sliderInst2 = (log10(20.0*sliderInst2+1.0)/log10(20.0+1.0))*127.0;
    //sliderInst2 = map(sliderInst2, 0,1023, 0, 127);
    
    //sliderInst2 = 127 * (exp(float(sliderInst2) / 1023 * log(128)) - 1) / (128 - 1);

    //sliderInst2 = round(127.0/1023.0 * sliderInst2);

    //  Serial.print("Inst2\n");
    //  Serial.print(sliderInst2);
    //  Serial.print("\n");
    
    //reads value from A8
    sliderBass = analogRead(A8)/1023.0;
    sliderBass = (log10(20.0*sliderBass+1.0)/log10(20.0+1.0))*127.0;
    // Serial.print("SliderBass\n");
    // Serial.print(sliderBass);
    // Serial.print("\n");
    //A7
    sliderDrums = analogRead(A7)/1023.0;
    sliderDrums = (log10(20.0*sliderDrums+1.0)/log10(20.0+1.0))*127.0;
    // Serial.print("SliderDrums\n");
    // Serial.print(sliderDrums);
    // Serial.print("\n");
    //A6

    //Slider inst1 er ikke kobla til denne 
    sliderInst1 = analogRead(A10)/1023.0;
    sliderInst1 = (log10(20.0*sliderInst1+1.0)/log10(20.0+1.0))*127.0;
    // Serial.print("SliderInstrument1\n");
    // Serial.print(sliderInst1);
    // Serial.print("\n");
    // //A3
    sliderRefreng = analogRead(A3)/1023.0;
    sliderRefreng = (log10(20.0*sliderRefreng+1.0)/log10(20.0+1.0))*127.0;
    //A2
    sliderVers = analogRead(A2)/1023.0;
    sliderVers = (log10(20.0*sliderVers+1.0)/log10(20.0+1.0))*127.0;
    //A1
    sliderIntro = analogRead(A1)/1023.0;
    sliderIntro = (log10(20.0*sliderIntro+1.0)/log10(20.0+1.0))*127.0;

}
void sendSliderValues(){
  controlChange(4,sliderInst1,sliderInst2);
  MidiUSB.flush();
  controlChange(5,sliderBass,sliderDrums);
  MidiUSB.flush();
  controlChange(6,sliderIntro,0);
  MidiUSB.flush();
  controlChange(7,sliderVers,0);
  MidiUSB.flush();
  controlChange(8,sliderRefreng,0);
  MidiUSB.flush();
}


//Led function for leds connected too MusicBox
void LightUp(){
  unsigned long my_time = millis();
  //We need to check if the first three and the last bits in the 6-bit words are not 111 if this is true light up leds
  //We create a mask too check this
  if((introInstOrdA & 0b000111) != 0b000111){
    //Box1 are connected light up 
    //light up the light that are connected too sendMeIntro1 in intro
    //example light up first light too red
    for (int i = 0; i < 12; i++)
    {
     leds[i] = CRGB::Blue;
       
    }
    
  }
  else if ((introInstOrdA & 0b000111) == 0b000111)
{
  for (int i = 0; i < 12; i++)
  {
    //int brightness = ((my_time%1000)/1000)*255;
    //leds[i]=CHSV(1,255,brightness);
    leds[i]=CRGB::Orange;
  }
   
   
}
//Box 2 connected light up
if ((introInstOrdA & 0b111000) != 0b111000)
{
  for (int i = 12; i < 24; i++)
  {
    leds[i] = CRGB::Yellow;
     
  }
  
}
//Box2 are not connected do something
  else if((introInstOrdA & 0b111000) == 0b111000)
  {
    for (int i = 12; i < 24; i++)
    {
      leds[i] = CRGB::Orange;
    }
  }
  ///box3 er connected 
  if((introInstOrdB & 0b000111) != 0b000111){
    for (int i = 24; i < 36; i++)
    {
      leds[i] = CRGB::Purple;
    }
    
  }
  //turn of box 3
  else if ((introInstOrdB & 0b000111) == 0b000111)
  {
    for (int i = 24; i < 36; i++)
    {
      leds[i] = CRGB::Orange;
      
    }
  }
//box 4 er connected 
  if((introInstOrdB & 0b111000) != 0b111000){
    for (int i = 36; i < 48; i++)
    {
      leds[i] = CRGB::Red;
      
    }
  }
    //box 4 not connected
  else if ((introInstOrdB & 0b111000) == 0b111000)
  {
    for (int i = 36; i < 48; i++)
    {
      leds[i] = CRGB::Orange;
    }
    
  }
  //Same but for Refreng
   if((refrengInstOrdA & 0b000111) != 0b000111){
    //Box1 are connected light up ;
    //light up the light that are connected too sendMeIntro1 in intro
    //example light up first light too red
    for (int i = 48; i < 60; i++)
    {
      //Light up Blue
     leds[i] = CRGB::Blue;
    }
    
  }

  else if ((refrengInstOrdA & 0b000111) == 0b000111)
  {
     for (int i = 48; i < 60; i++)
    {
     leds[i] = CRGB::Orange;
    }
  // turn off all LEDs
}
//Box 2 connected light up
  if ((refrengInstOrdA & 0b111000) != 0b111000)
  {
  for (int i = 60; i < 72; i++)
  {
    leds[i] = CRGB::Yellow;
    
  }
  
}
//Box2 are not connected do something
  else if((refrengInstOrdA & 0b111000) == 0b111000)
  {
    for (int i = 60; i < 72; i++)
    {
      leds[i] = CRGB::Orange;
      
    }
  }
  ///box3 er connected 
  if((refrengInstOrdB & 0b000111) != 0b000111){
    for (int i = 72; i < 84; i++)
    {
      leds[i] = CRGB::Purple;
     
    }
    
  }
  //turn of box 3
  else if ((refrengInstOrdB & 0b000111) == 0b000111)
  {
    for (int i = 72; i < 84; i++)
    {
      leds[i] = CRGB::Orange;
      
    }
  }
//box 4 er connected 
  if((refrengInstOrdB & 0b111000) != 0b111000){
    for (int i = 84; i < 96; i++)
    {
      leds[i] = CRGB::Red;
      
    }
  }
    //box 4 not connected
  else if ((refrengInstOrdB & 0b111000) == 0b111000)
  {
    for (int i = 84; i < 96; i++)
    {
      leds[i] = CRGB::Orange;
    }
    
  }
  if (usersMood != 0)
  {
    for (int i = 131; i < 131+12; i++)
    {
      leds[i] = CRGB::White;
    } 
  }
  else if(usersMood == 7){
    for (int i = 131; i < 131+12; i++)
    {
      leds[i]==CRGB::Orange;
    }
    
  }
  
}


void sliderLeds(){ 


  int slider_first_led_nr = 95;
  float slider_led_list[] = {sliderInst1, sliderInst2, sliderBass, sliderDrums, sliderRefreng, sliderVers, sliderIntro};
  int slider_color_list[] = {170, 60, 200, 1, 240, 210, 170 };
  int slider_saturation_list[] = {255, 255, 255, 255, 150, 150, 150 };

  for (int i = 0; i < 7; i++){

    float totalBrightness = 5;
    int my_led = slider_led_list[i];
    float currentBrightness = my_led/127.0*totalBrightness;


    for (int ii = slider_first_led_nr+(i+1)*5; ii > slider_first_led_nr+(i)*5; ii--){
    //leds[ii] = CRGB::White;
      

      if (currentBrightness >= 1)
      {
        leds[ii] = CHSV(slider_color_list[i],slider_saturation_list[i],255);
        currentBrightness-=1;
      }
      else{
        leds[ii] = CHSV(slider_color_list[i],slider_saturation_list[i], currentBrightness * 255);
        currentBrightness = 0;
      }
    }
  }

}

void checkIfSendMidi(){
  //Check if midiSignal are good enough and then put this send ready if so
  //If it is not send ready send previous signal
    const int numValuesToCheck = 7;
    int count = 1;
    //Check if we send Prev or newest midisignal
    sendMeIntro1 = false;
    sendMeIntro2 = false;
    //For vers:
    sendMeVers1 = false;
    sendMeVers2 = false;
    //For refreng
    sendMeRefreng1 = false;
    sendMeRefreng2 = false;
    changeHasCome = 0;
    //For mood
    sendMeMoodBool = false;


    for (int i = 1; i < introInstOrd1Values.Size(); i++) {
      //Checks if the vector with values has 10 of the same values that are not zero
      //If it has this this value should be sent a
      
      //ADD IF VALUE == 111 NOT GOOD DO NOT SEND
        if (introInstOrd1Values[i] == introInstOrd1Values[i - 1]&&(introInstOrd1Values[i] & 0b111) != 0b111 &&(introInstOrd1Values[i] >> 3) != 0b111 &&introInstOrd1Values[i] != 7 &&introInstOrd1Values[i] != 56) {
            count++;
            if (count == numValuesToCheck) {
                // Serial.print("SignalReady to be sent");
                // Serial.print("\n");
                sendMeIntro1 = true;
                //The variable we should send 
                sendMeIntroOrdA = introInstOrd1Values[i] & 0x3F;
                 if (sendMeIntroOrdA != sendMePreviousIntroOrdA)
                {
                  progressBarStart = millis();
                  changeHasCome = 1;
                }
                // Serial.print("SendMeIntroOrdA\n");
                // Serial.print(sendMeIntroOrdA);
                // Serial.print("\n");
                // Serial.print("sendMePrevious\n");
                // Serial.print(sendMePreviousIntroOrdA);
                // Serial.print("\n");
                // Serial.print("Send this signal");
                // Serial.print(sendMeIntroOrdA);
                sendMePreviousIntroOrdA = introInstOrd1Values[i] & 0x3F;
                count = 1;
                
            }
        } 
    }
//We check intro word B
    count = 1;
    for(int i = 1; i < introInstOrd2Values.Size(); i++){
      if (introInstOrd2Values[i] == introInstOrd2Values[i - 1]&&(introInstOrd2Values[i] & 0b111) != 0b111 &&(introInstOrd2Values[i] >> 3) != 0b111 &&introInstOrd2Values[i] != 7 &&introInstOrd2Values[i] != 56) {
                count++;
                if (count == numValuesToCheck) {
                    sendMeIntro2 = true;
                    sendMeIntroOrdB = introInstOrd2Values[i] & 0x3F;
                    if (sendMeIntroOrdB != sendMePreviousIntroOrdB)
                    {
                      progressBarStart = millis();
                      changeHasCome = 1;
                    }
                    sendMePreviousIntroOrdB = introInstOrd2Values[i] & 0x3F;
                    count = 1;
                }
            } 
    }  
    count = 1;
    //Need to check the exact same for the vers:
     for (int i = 1; i < versInstOrd1Values.Size(); i++) {
      //Checks if the vector with values has 10 of the same values that are not zero
      //If the 3 first bits of the word is 111 it should send previous value. It should do the same if the 3 last are 111

      //Can make a mask and check with this
      
        if (versInstOrd1Values[i] == versInstOrd1Values[i - 1]&&(versInstOrd1Values[i] & 0b111) != 0b111 &&(versInstOrd1Values[i] >> 3) != 0b111 &&versInstOrd1Values[i] != 7 &&versInstOrd1Values[i] != 56) {
            count++;
            if (count == numValuesToCheck) {
                sendMeVers1 = true;
                //The variable we should send 
                sendMeVersOrdA = versInstOrd1Values[i] & 0x3F;
                 if (sendMeVersOrdA != sendMePreviousVersOrdA)
                    {
                      progressBarStart = millis();
                      changeHasCome = 1;
                    }
                sendMePreviousVersOrdA = versInstOrd1Values[i] & 0x3F;
                count = 1;
            }
        } 
    }
  //The exact same but for word B
  count = 1;
  for (int i = 1; i < versInstOrd2Values.Size(); i++) {
        if (versInstOrd2Values[i] == versInstOrd2Values[i - 1]&&versInstOrd2Values[i]!= 0&&(versInstOrd2Values[i] & 0b111) != 0b111 &&(versInstOrd2Values[i] >> 3) != 0b111) {
            count++;
            if (count == numValuesToCheck) {
                sendMeVers2 = true;
                sendMeVersOrdB = versInstOrd2Values[i] & 0x3F;
                if (sendMeVersOrdB != sendMePreviousVersOrdB)
                    {
                      progressBarStart = millis();
                      changeHasCome = 1;
                    }
                sendMePreviousVersOrdB = versInstOrd2Values[i] & 0x3F;
                count = 1;
            }
        } 
    }
    count = 1;
    //The exact same but for refreng:
     for (int i = 1; i < refrengInstOrd1Values.Size(); i++) {
      //Checks if the vector with values has 10 of the same values that are not zero
      //If it has this this value should be sent a
        if (refrengInstOrd1Values[i] == refrengInstOrd1Values[i - 1]&&(refrengInstOrd1Values[i] & 0b111) != 0b111 &&(refrengInstOrd1Values[i] >> 3) != 0b111&&refrengInstOrd1Values[i] != 7 &&refrengInstOrd1Values[i] != 56) {
            count++;
            if (count == numValuesToCheck) {
                sendMeRefreng1 = true;
                //The variable we should send 
                sendMeRefrengOrdA = refrengInstOrd1Values[i] & 0x3F;
                if (sendMeRefrengOrdA != sendMePreviousRefrengOrdA)
                {
                  progressBarStart = millis();
                  changeHasCome = 1;
                }
                sendMePreviousRefrengOrdA = refrengInstOrd1Values[i] & 0x3F;
                count = 1;
            }
        } 
    }
  count = 1;
  for (int i = 1; i < refrengInstOrd2Values.Size(); i++) {
        if (refrengInstOrd2Values[i] == refrengInstOrd2Values[i - 1]&&(refrengInstOrd2Values[i] & 0b111) != 0b111 &&(refrengInstOrd2Values[i] >> 3) != 0b111) {
            count++;
            if (count == numValuesToCheck) {
                sendMeRefreng2 = true;
                sendMeRefrengOrdB = refrengInstOrd2Values[i] & 0x3F;
                if (sendMeRefrengOrdB != sendMePreviousRefrengOrdB)
                {
                  progressBarStart = millis();
                  changeHasCome = 1;
                }
                sendMePreviousRefrengOrdB = refrengInstOrd2Values[i] & 0x3F;
                count = 1;
            }
        } 
    }
  count = 1;
    //Check if mood good to be sent

    //NEED TO CHANGE THIS SO IT TAKES WITH THE BUTTON VALUES

    //I CAN OR IT WITH THE USERORDA

    for (int i = 1; i < moodValues.Size(); i++) {
      //Checks if the vector with values has 10 of the same values that are not zero
      //If it has this this value should be sent a
        if (moodValues[i] == moodValues[i - 1]&&moodValues[i]!= 0&&moodValues[i] != 7&&(moodValues[i] & 0b111) != 0b111 &&(moodValues[i] >> 3) != 0b111) {
            count++;
            if (count == numValuesToCheck) {
                sendMeMoodBool = true;
                //The variable we should send 
                  sendMeMood = moodValues[i] & 0x3F;
                 if (sendMeMood != sendMePrevMood)
                {
                  changeHasCome = 1;
                }

                sendMePrevMood = moodValues[i] & 0x3F;
            }
        } 
    }
    //clear all vectors before getting the next datas
    introInstOrd1Values.Clear();
    introInstOrd2Values.Clear();
    versInstOrd1Values.Clear();
    versInstOrd2Values.Clear();
    refrengInstOrd1Values.Clear();
    refrengInstOrd2Values.Clear();
    moodValues.Clear();
  }

void updateButtons(){
    if (section != 0 && buttonValueSkipBackwards == 0)
  {
    section-=1;
    buttonValueSkipBackwards = 1;
    changeSection = true;
  }
  if (section != 3 && buttonValueSkipForward == 0)
  {
    section+=1;
    //Think this can be removed should be put to zero by next loop
    buttonValueSkipForward = 1;
    changeSection = true;
  }
  //Need to add the button values to the midisignal that are being sent
  //Needs to ba added to the new signal and the prev should also be changed
  sendMeMood = (userOrdA) | sendMeMood;
  sendMePrevMood = (userOrdA) | sendMePrevMood;
  //change stop back to it has not been pressed button
  stop = 1;
  buttonValueSkipBackwards = 1;
  buttonValueSkipForward = 1;
  buttonValueLoop = 1;
  buttonValueStop = 1;
  //ADD THAT ALL BUTTON VALUES GETS SET TOO ZERO
}


void setup()
{
  Serial.begin(115200);
  //set up for expanders
  mcp1.begin_I2C(0x20); // Initialize mcp1 with default I2C address (0x20)
  //Set up is wrong for mcp2
  mcp2.begin_I2C(0x22);
  mcp3.begin_I2C(0x21); // Initialize mcp1 with default I2C address (0x20)
  //Evt legg til konfigurasjon om pinsa er pull up eller pull down
  //For å vite startverdien til pinsa
 for(int i = 0; i <= 15; i++){
    mcp1.pinMode(i, INPUT_PULLUP);
  }
  for(int i = 0; i <= 15; i++){
    mcp2.pinMode(i, INPUT_PULLUP);
  }
  for(int i = 0; i <= 15; i++){
    mcp3.pinMode(i, INPUT_PULLUP);
  }
  //set up for fastleds 
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  //starting millis
  millis();
  //Set up empty array with 20 zeroes for all the vectors
  int storageArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  //Tror vi kan drite i det her
  // introInstOrd1Values.storage(storageArray);
  // introInstOrd2Values.storage(storageArray);
  // versInstOrd1Values.storage(storageArray);
  // versInstOrd2Values.setStorage(storageArray);
  // refrengInstOrd1Values.setStorage(storageArray);
  // refrengInstOrd2Values.setStorage(storageArray);
  //LEGG TIL FOR DE ANDRE MCPOGSÅ
  if (!mcp1.begin_I2C(0x20)) {
     Serial.print("Expander onenot working");
    }
  if (!mcp2.begin_I2C(0x22))
  {
     Serial.print("Expander two not working");
  }
  if (!mcp3.begin_I2C(0x21))
  {
    Serial.print("Expander three not working");
  }
  
}

int f = 0;
void loop()
{
  const int delayVar = 3;
  const unsigned long MIDI_INTERVAL = 70 * delayVar;
  static unsigned long lastMidiTime = 0;
  //Constant loop updating  MIDI-values

  MixBoxMillis = millis();
  readMainFunctions();
  readIntroInstruments();
  readVersInstrument();
  readRefrengInstrument();
  readSliderValues();
  unsigned long currentTime = millis();
  //If it has been 30 checks since we send midiSignals check if midi are Good and send 
  sliderLeds();
  LightUp();

   if (introInstOrd1Values.Size()>10)
   {
    //Check if we should send MIDI
    checkIfSendMidi();
    //Update button values and add it too sendMeMood
    updateButtons();
    lastMidiTime = currentTime;
    sendMidi();
    sendSliderValues();
    //Looks like the lightUp takes a lot of time
    
    
  } 
  //LightUp();
  FastLED.show();
  //Constant loop sending MIDI-values
  //sendMidi();
  //Lighting up the leds

  //LightUp();
  //sliderLeds();
}