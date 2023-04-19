#include <Arduino.h>
//Legger til Expander MCP23017
#include <Adafruit_MCP23X17.h>
#include "MIDIUSB.h"
#include <FastLED.h>
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
int sliderInst1= 0;
int sliderInst2= 0;
int sliderBass = 0;
int sliderDrums = 0;
int sliderVers = 0;
int sliderIntro = 0;
int sliderRefreng = 0;

//Int for what kind of BMP that are being played
int BPM = 0;
//An array of the diffrent BPM for the 6 diffrent moods
//CHANGE TO THE CORRECT BPM 
//the first number is being used as the mood is not selected yet
int arrayOfBPM[] = {0,2,4,8,16,32,64};
//The diffrent button pins that are being used 
int buttonValueLoop = 0;
int buttonValueStop = 0;
int buttonValueSkipForward = 0;
int buttonValueSkipBackwards = 0;
//Need a function so that the program knows what kind of section the user is in and stuff like that 
int currentLight = 0;
//section is zero if user is in intro
int section = 0;
int startOver = 0;
int LightsPerSector = 6;
int stop = 0;
int systemPlayingSound = 0;
//Set up for fastled
//sets the LED_PIN too be C6 CHANGE THIS IF NOT USING THIS PIN 
int LED_PIN = 5;
//CHANGE TO NUMBER LEDS THAT ARE BEING USED
const int NUM_LEDS = 11;
#define DATA_PIN 3
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

// First parameter is the event type (0x0B = control change). Pluss Jørgen
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

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
  //Saving the button values as the first 4 values of the 7 bit word
  //We are reading the buttons now
  //The buttons are connected too pin 7 on the arduino, and then b5-b7 on the secont expander
  //Lets have the stop button on the 7 pin

 //I am asuming that the digitalread gives out a high integer when the buttons are being pressed!!!

  userOrdA |= digitalRead(7) << 0;
   //We need to change stop too one if the stop button is being played and if it gets pressed again we need to change it too zero
  stop = digitalRead(7);
  userOrdA |= mcp2.digitalRead(12) << 1;
  buttonValueSkipForward = mcp2.digitalRead(12);
  userOrdA |= mcp2.digitalRead(13) << 2;
  buttonValueSkipBackwards = mcp2.digitalRead(14);
  userOrdA |= mcp2.digitalRead(14) << 3;
  //We also need to change the loop variable if being pressed
  buttonValueLoop = mcp2.digitalRead(14);
  //Changing the section if some of the skip and backward function are being pressed and we are not in the last or first secetion
  if (section != 0)
  {
    section-=buttonValueSkipBackwards;
    buttonValueSkipBackwards = 0;
  }
  if (section != 3)
  {
    section+=buttonValueSkipForward;
    //Think this can be removed should be put to zero by next loop
    buttonValueSkipForward = 0;
  }
  
  //We need to read the pins from the First MCP23017 extander but for the unused pins B4-B6
  for(int i = 12; i <= 14; i++){
    //saving the values of the diffrents pins in userMood
    //starts with saving it as the least significant bit and then most significant bit 
    usersMood |= mcp1.digitalRead(i) << (i-12);
    userOrdA  |= mcp1.digitalRead(i) << (i-8);
  }
  //changing the 3-bit word too integer
  choosenMood = (int)usersMood;
  //saving the BPM of chellected mood
  BPM = arrayOfBPM[choosenMood];
  //Starting the led function 
  songLights(BPM);
} 

void readIntroInstruments(){
  //reads pin a0-a5 from the MCP23017 expander
  //puts this in a 6-bit word called introInst 
    //note that 0xb000 is that the box is not connected 
   introInstOrdA = 0;
 for(int i = 0; i < 6; i++){
    introInstOrdA |= mcp1.digitalRead(i) << i;
  }
  //all of this is the word A 
  //Reads now from A6-A7 and from B0-B3
    introInstOrdB = 0;
  for (int x = 6; x <= 11; x++)
  {
    introInstOrdB |= mcp1.digitalRead(x) << (x-6); 
  }
  //We know have two 6-bit words that we can send too rasberry-pi
  //This are going to be sent of as channel 1
  //Checking if the user is playing sound from intro. 
   if (introInstOrdA != 0 || introInstOrdB != 0) {
    systemPlayingSound = 1;
  }
   //we need to check if the user has had a change to the MixBox
   if (introInstOrdA != prevIntroInstOrdA || introInstOrdB != prevIntroInstOrdB) {
    changeHasCome = 1;
  } else {
    changeHasCome = 0;
  }
  //Saving the new changes
  prevIntroInstOrdA = introInstOrdA;
  prevIntroInstOrdB = introInstOrdB;
}

void readVersInstrument(){
  //reads pin a0-a5 from the second MCP23017 expander
  //puts this in a 6-bit word
    //note that 0xb000 is that the box is not connected 
    VersInstOrdA = 0;
 for(int i = 0; i < 6; i++){
    VersInstOrdA |= mcp2.digitalRead(i) << i;
  }
  //Reads now from A6-A7 and from B0-B3
    VersInstOrdB = 0;
  for (int x = 6; x <= 11; x++)
  {
    VersInstOrdB |= mcp2.digitalRead(x) << (x-6); 
  }
  //We know have two 6-bit words that we can send too rasberry-pi
  //Checking if the system is playing sound from Vers
   if (VersInstOrdA != 0 || VersInstOrdB != 0) {
    systemPlayingSound = 1;
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
}

void readRefrengInstrument(){
  //reads pin a0-a5 from the third MCP23017 expander
  //puts this in a 6-bit word
  //note that 0xb000 is that the box is not connected 
   refrengInstOrdA = 0;
 for(int i = 0; i < 6; i++){
    refrengInstOrdA |= mcp3.digitalRead(i) << i;
  }
  //Reads now from A6-A7 and from B0-B3
    refrengInstOrdB = 0;
  for (int x = 6; x <= 11; x++)
  {
    //usikker på denne linjen
    refrengInstOrdB |= mcp3.digitalRead(x) << (x-6); 
  }
  //We know have two 6-bit words that we can send too rasberry-pi
  //each MCP23017 expander now has B4-B7 aviable(4-bits)
  //Check if the system is playing a sound from refreng
   if (refrengInstOrdA != 0 || refrengInstOrdB != 0) {
    systemPlayingSound = 1;
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
}

void sendMidi(){
  //Here Midisignals are being sent
  //Getting each 6-bit word from each section ready to be sent on the right channel 
  controlChange(0,userOrdA,0);
  //Sending the midi-signal
  MidiUSB.flush();
  controlChange(1,introInstOrdA,introInstOrdB);
    //sending the midi-signal
   MidiUSB.flush();
  controlChange(2,VersInstOrdA,VersInstOrdB);
  //sending the midi-signal
   MidiUSB.flush();
  controlChange(3,refrengInstOrdA,refrengInstOrdB);
  //sending the midi-signal
  MidiUSB.flush();
}

//Here comes the slider functions
void readSliderValues(){
    //reads value from slider that is connected to pin A9 on the arduino micro pro
    sliderInst1 = analogRead(A9);
    //reads value from A8
    sliderInst2 = analogRead(A8);
    //A7
    sliderBass = analogRead(A7);
    //A6
    sliderDrums = analogRead(A6);
    //A3
    sliderVers = analogRead(A3);
    //A2
    sliderIntro = analogRead(A2);
    //A1
    sliderRefreng = analogRead(A1);
}

void sendSliderValues(){
  controlChange(4,sliderInst1,sliderInst2);
  MidiUSB.flush();
  controlChange(5,sliderBass,sliderDrums);
  MidiUSB.flush();
  controlChange(6,sliderIntro,sliderVers);
  MidiUSB.flush();
  controlChange(7,sliderRefreng,0);
  MidiUSB.flush();
}


//Here comes the led function 
void LightUp(){
  //We need to check if the first three and the last bits in the 6-bit words are not 111 if this is true light up leds
  //We create a mask too check this
  if((introInstOrdA & 0xb111000) != 0xb111000){
    //light up the light that are connected too inst 2 in intro
    //example light up first light too red
    leds[0] = CRGB::Red;
    FastLED.show();
  }
  if ((introInstOrdA & 0xb000111) != 0xb000111)
  {
    leds[1] = CRGB::Blue;
    FastLED.show();
  }
  if((introInstOrdB & 0xb111000) != 0xb111000){
    leds[2] = CRGB::Red;
    FastLED.show();
  }
  if ((introInstOrdB & 0xb000111) != 0xb000111)
  {
    leds[3] = CRGB::Blue;
    FastLED.show();
  }
  if((VersInstOrdA & 0xb111000) != 0xb111000){
    leds[4] = CRGB::Red;
    FastLED.show();
  }
  if ((VersInstOrdA & 0xb000111) != 0xb000111)
  {
    leds[5] = CRGB::Blue;
    FastLED.show();
  }
  if((VersInstOrdB & 0xb111000) != 0xb111000){
    leds[6] = CRGB::Red;
    FastLED.show();
  }
  if ((VersInstOrdB & 0xb000111) != 0xb000111)
  {
    leds[7] = CRGB::Blue;
    FastLED.show();
  }
  if((refrengInstOrdA & 0xb111000) != 0xb111000){
    leds[8] = CRGB::Red;
    FastLED.show();
  }
if ((refrengInstOrdA & 0xb000111) != 0xb000111)
  {
    leds[9] = CRGB::Blue;
    FastLED.show();
  }
 if((refrengInstOrdB & 0xb111000) != 0xb111000){
    leds[10] = CRGB::Red;
    FastLED.show();
  }
if ((refrengInstOrdB & 0xb000111) != 0xb000111)
  {
    leds[11] = CRGB::Blue;
    FastLED.show();
  }
}


void songLights(int BPM){
  //A function that light up the leds that should tell the user where it is in the song
  //We first need to ger what kind of BMP we are using(what mood)
  //A loop that lights up one and one light for each part of the song
  static int currentLight = -1; 
  int interval = (BPM/60)*4;
  int startSectionLight = section * LightsPerSector; 
  //This lights up the next light after it has gone some time
  //checking if the system is playing sound and that the mood MixBox is added
  //Does only light up leds if the system is playing a sound and it has not happened a recent change in the melody, and if the stop button has been pressed
  if (systemPlayingSound == 1 && BPM != 0 && stop == 0 && changeHasCome == 0)
  {
    //only if the system is playing sound are we going to update currentMillis 
    currentMillis = millis();
    //Lighting up the first led in the right section 
    if (currentLight < startSectionLight) {
      currentLight = startSectionLight;
      leds[currentLight]=CRGB::Green;
      FastLED.show();
      //we are not in the intro lets light up the leds before 
      if (currentLight != 0)
      {
        for (int i = 0; i < currentLight; i++)
        {
          leds[i]=CRGB::Green;
        }
        
      }
      
    }
    //After the beat has gone through 4 beats we are going to light up the next light
    else if (currentMillis - previousMillis >= interval)
    {
       previousMillis = currentMillis;
      currentLight += 1;
      if (currentLight >= LightsPerSector*(section+1) && buttonValueLoop == 1)
      {
       //We have know comed too the last led and need to restart the loop
       for (int i = 0; i <LightsPerSector*section; i++)
       {
        //turning of all the lights
      //Maybe the board should give the user some way to know they are in a loop?????????
        leds[i]=CRGB::Black;
       }
       //Setting currentLight so that the loop can repeat itself
       currentLight=-1;
      }
      leds[currentLight]=CRGB::Green;
      FastLED.show();
    }
  }
  else if (stop == 1)
  {
    for (int i = 0; i < LightsPerSector*3; i++)
    {
      //Making the leds glow red if the button is pressed
      leds[i]==CRGB::Red;
    }
    
  }
  else if (changeHasCome == 1)
  {
    for (int i = 0; i < LightsPerSector*3; i++)
    {
      //turning all of the leds off and the light up section will repeat it self in the next loop
      leds[i]==CRGB::Black;
    }  
  }
  
}





void setup()
{
  Serial.begin(115200);
  //set up for expanders
 for(int i = 0; i <= 11; i++){
    mcp1.pinMode(i, INPUT);
  }
  for(int i = 0; i <= 11; i++){
    mcp2.pinMode(i, INPUT);
  }
  for(int i = 0; i <= 11; i++){
    mcp3.pinMode(i, INPUT);
  }
  //set up for fastleds 
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
}

void testEspen(){
  introInstOrdB = 0b0000100;
  VersInstOrdB = 0b0001010;
  refrengInstOrdB = 0b0000110;
}
void NewTestEspen(){
  introInstOrdB = 0b0000010;
  VersInstOrdB = 0b0000010;
  refrengInstOrdB = 0b0000010;
}

void loop()
{
  //Constant loop updating  MIDI-values
  readMainFunctions();
  readIntroInstruments();
  readVersInstrument();
  readRefrengInstrument();
  delay(200);
  //Constant loop sending MIDI-values
  sendMidi();
  sendSliderValues();
  delay(200);
  //Lighting up the leds
  //LightUp();
}