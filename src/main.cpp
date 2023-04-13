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
int section = 1;
int startOver = 0;
int LightsPerSector = 6;
int stop = 0;
//Set up for fastled
//sets the LED_PIN too be C6 CHANGE THIS IF NOT USING THIS PIN 
int LED_PIN = 5;
//CHANGE TO NUMBER LEDS THAT ARE BEING USED
const int NUM_LEDS = 11;
#define DATA_PIN 3
CRGB leds[NUM_LEDS];

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
  userOrdA |= digitalRead(7) << 0;
  userOrdA |= mcp2.digitalRead(12) << 1;
  userOrdA |= mcp2.digitalRead(13) << 2;
  userOrdA |= mcp2.digitalRead(14) << 3;
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
} 

void readIntroInstruments(){
  //reads pin a0-a5 from the MCP23017 expander
  //puts this in a 6-bit word called introInst 
    //note that 0xb000 is that the box is not connected 
   introInstOrdA = 0;
 for(int i = 0; i < 6; i++){
    introInstOrdA |= mcp1.digitalRead(i) << i;
  }
  //har nå lest av de to første instrumentet fra de 6 første pinsa
  //all of this is the word A 
  //Reads now from A6-A7 and from B0-B3
    introInstOrdB = 0;
  for (int x = 6; x <= 11; x++)
  {
    //usikker på denne linjen
    introInstOrdB |= mcp1.digitalRead(x) << (x-6); 
  }
  //We know have two 6-bit words that we can send too rasberry-pi
  //så må dette sendes til channel 1 osv
  //lagres som controlChange men sendes ikke
  controlChange(1,introInstOrdA,introInstOrdB);
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
    //usikker på denne linjen
    VersInstOrdB |= mcp2.digitalRead(x) << (x-6); 
  }
  //We know have two 6-bit words that we can send too rasberry-pi
  //lagres som controlChange for channel 2 men sendes ikke
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
  //lagres som controlChange for channel 2 men sendes ikke
  //each MCP23017 expander now has B4-B7 aviable(4-bits)
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
  //controlChange(0, 0b001, 67); 
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


void songLights(){
  //A function that light up the leds that should tell the user where it is in the song
  //We first need to check what kind of BMP we are using(what mood)
  //A loop that lights up one and one light for each part of the song
int currentLight = 0;
int a = 0;
while(startOver == 0 && currentLight != LightsPerSector && stop == 0) 
{
  //Light up the next lights after a few BPM has passed
  int startLight = LightsPerSector * section;
  //Need to be changed into whatever position the right leds are on
  leds[startLight + a] = CRGB::Red;
  //But I cant have a delay her as it would slow the whole program down
  
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
  //Constant loop updating and sending midi-singals 
  readMainFunctions();
  readIntroInstruments();
  readVersInstrument();
  readRefrengInstrument();
  LightUp();
  delay(200);
  readSliderValues();
  sendSliderValues();
  delay(200);
}