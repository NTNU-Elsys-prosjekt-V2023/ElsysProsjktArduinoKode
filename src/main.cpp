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
#define DATA_PIN 5
const int NUM_LEDS = 11;
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
//Globalvariables too be used in slider-leds

//CHANGE THIS TO ACCORDING MAX VALUE
int maxSliderValue = 1023;
//AND THIS
int numberOfSliderLeds = 6;

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

//We need prevWord to check if send midi or not
uint8_t sendMePreviousIntroOrdA;
uint8_t sendMePreviousIntroOrdB;
uint8_t sendMeIntroOrdA;
uint8_t sendMeIntroOrdB;
bool sendMeIntro1;
bool sendMeIntro2; 



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
      leds[i]=CRGB::Black;
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
      for (int x = 11; x >= 9; x--)
       {
        ordToRead2 |= mcp1.digitalRead(x) << (11-x); 
        }
        
      ordToRead2 |= mcp1.digitalRead(1) << (3);
      ordToRead2 |= mcp1.digitalRead(0) << (4);
      ordToRead2 |= mcp1.digitalRead(8) << (5);
      data.ordToRead1 = ordToRead1;
      data.ordToRead2 = ordToRead2;

      //Serial.print("Her kommer ordToRead1");
     // Serial.print("\n");
      //Serial.print(ordToRead1);
      //Serial.print("\n");
      vectorMe1 = ordToRead1;
      vectorMe2 = ordToRead2;
      introInstOrd1Values.push_back(vectorMe1);
      introInstOrd2Values.push_back(vectorMe2);
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
    for (int x = 11; x >= 9; x--){
        ordToRead2 |= mcp2.digitalRead(x) << (11-x); 
        Serial.print(mcp2.digitalRead(x));
        }
      ordToRead2 |= mcp2.digitalRead(1) << (3);
      ordToRead2 |= mcp2.digitalRead(0) << (4);
      ordToRead2 |= mcp2.digitalRead(8) << (5);
      data.ordToRead1 = ordToRead1;
      data.ordToRead2 = ordToRead2;
      //I want to convert the 6-bit words too integer too be later used to check if the midi-signal should be sent
      vectorMe1 = ordToRead1;
      vectorMe2 = ordToRead2;
      versInstOrd1Values.push_back(vectorMe1);
      versInstOrd2Values.push_back(vectorMe2);
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
      for (int x = 11; x >= 9; x--)
       {
        ordToRead2 |= mcp3.digitalRead(x) << (11-x); 
        Serial.print(mcp3.digitalRead(x));
        }
    ordToRead2 |= mcp3.digitalRead(1) << (3);
    ordToRead2 |= mcp3.digitalRead(0) << (4);
    ordToRead2 |= mcp3.digitalRead(8) << (5);
    data.ordToRead1 = ordToRead1;
    data.ordToRead2 = ordToRead2;
    vectorMe1 = ordToRead1;
    vectorMe2 = ordToRead2;
    refrengInstOrd1Values.push_back(ordToRead1);
    refrengInstOrd2Values.push_back(ordToRead2);
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

  //CHANGE TO CORRECT ADRESS

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

    //The problem with the code is that if I do not have this in lines under. The system are not able to read the mcppins.
    //But if I do the system runs a X number of times then it stops for some reason!!!

  int a = 1;
  PinsData data = readPins(introInstOrdA,introInstOrdB,a);
  introInstOrdA = data.ordToRead1;
  Serial.print(introInstOrdA);
  introInstOrdB = data.ordToRead2;
  //We know have two 6-bit words that we can send too rasberry-pi
  //This are going to be sent of as channel 1
  //Checking if the user is playing sound from intro. 
   if (introInstOrdA != 0 || introInstOrdB != 0) {
    systemPlayingSound = 1;
  }
   //we need to check if the user has had a change to the MixBox
   if (introInstOrdA != prevIntroInstOrdA || introInstOrdB != prevIntroInstOrdB) {
    changeHasCome = 1;
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
   if (!mcp2.begin_I2C()) {
    Serial.println("Error mc2 not connected.");
    return;
  }
    int b = 2;
    PinsData data = readPins(introInstOrdA,introInstOrdB,b);
    VersInstOrdA = data.ordToRead1;
    VersInstOrdB = data.ordToRead2;
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
  Serial.print("Her kommer vers \n");
  Serial.print(VersInstOrdA);
  Serial.print(VersInstOrdB);
  //Saving the last values
}

void readRefrengInstrument(){
  //reads pin a0-a5 from the third MCP23017 expander
  //puts this in a 6-bit word
  //note that 0xb000 is that the box is not connected 
 if (!mcp3.begin_I2C()) {
    Serial.println("Error mc3 not connected.");
    return;
  }
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
   //we need to check if the user has made a change to the MixBox
   if (refrengInstOrdA != prevRefInstOrdA || refrengInstOrdB != prevRefInstOrdB) {
    changeHasCome = 1;
  } else {
    changeHasCome = 0;
  }
  //Saving the new changes
  prevRefInstOrdA = refrengInstOrdA;
  prevRefInstOrdB = refrengInstOrdB;
  Serial.print("Her kommer refreng \n");
  Serial.print(refrengInstOrdA);
  Serial.print(refrengInstOrdB);
  //Saving the last values 
}

void sendMidi(){
  //Here Midisignals are being sent
  //Getting each 6-bit word from each section ready to be sent on the right channel 
  //controlChange(0,userOrdA,0);
  //Sending the midi-signal
  //MidiUSB.flush();
  if (!sendMeIntro1)
  {
    sendMeIntroOrdA = prevIntroInstOrdA;
  }
  if (!sendMeIntro2)
  {
    sendMeIntroOrdB = prevIntroInstOrdB;
  }
  // Serial.print("Sender midiSignaler");
  // Serial.print("Her kommer først ord som blir sendt");
  // Serial.print("\n");
  // Serial.print(sendMeIntroOrdA);
  //Serial.print(sendMeIntroOrdB);
  controlChange(1,sendMeIntroOrdA,sendMeIntroOrdB);

    //sending the midi-signal
   MidiUSB.flush();
  //controlChange(2,VersInstOrdA,VersInstOrdB);
  //sending the midi-signal
  //MidiUSB.flush();
  //controlChange(3,refrengInstOrdA,refrengInstOrdB);
  //sending the midi-signal
  //MidiUSB.flush();
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


//Led function for leds connected too MusicBox
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

void sliderLeds(){ 
  int stepValue = maxSliderValue/numberOfSliderLeds;
  for (int i = 0; i < numberOfSliderLeds; i++)
  {
  if (sliderInst1 >= stepValue * i)
  {
    //change 30 too whatever led this is
   leds[i+30]=CRGB::Green;
  }
  if (sliderBass >= stepValue * i)
  {
    leds[i + 60]=CRGB::Green;
  }
   if (sliderDrums >= stepValue * i)
  {
    //change this to whatever led it is connected too
    leds[i + 60]=CRGB::Green;
  }
   if (sliderInst2 >= stepValue * i)
  {
        //change this to whatever led it is connected too
    leds[i + 60]=CRGB::Green;
  }
   if (sliderIntro >= stepValue * i)
  {
        //change this to whatever led it is connected too
    leds[i + 60]=CRGB::Green;
  }

  }
}




void setup()
{
  Serial.begin(115200);
  //set up for expanders
  mcp1.begin_I2C(0); // Initialize mcp1 with default I2C address (0x20)
  mcp2.begin_I2C(1);
  mcp3.begin_I2C(2); // Initialize mcp1 with default I2C address (0x20)
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
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  //starting millis
  millis();
  //Set up empty array
  int storageArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  introInstOrd1Values.setStorage(storageArray);
  //LEGG TIL FOR DE ANDRE MCPOGSÅ
  if (!mcp1.begin_I2C()) {
     Serial.print("Expander one not working");
    }
  if (!mcp2.begin_I2C())
  {
     Serial.print("Expander two not working");
  }
  
}

void checkIfSendMidi(){
  //Check if midiSignal are good enough and then put this send ready if so
  //If it is not send ready send previous signal
    const int numValuesToCheck = 15;
    int count = 1;
    //Check if we send Prev or newest midisignal
    bool sendMeIntro1 = false;
    bool sendMeIntro2 = false;

    for (int i = 1; i < introInstOrd1Values.size(); i++) {
      //Checks if the vector with values has 10 of the same values that are not zero
      //If it has this this value should be sent a
        if (introInstOrd1Values[i] == introInstOrd1Values[i - 1]&&introInstOrd1Values[i]!= 0&&introInstOrd1Values[i] != 7 &&introInstOrd1Values[i] != 56) {
            count++;
            if (count == numValuesToCheck) {
                sendMeIntro1 = true;
                //The variable we should send 
                sendMeIntroOrdA = introInstOrd1Values[i] & 0x3F;
                sendMePreviousIntroOrdA = introInstOrd1Values[i] & 0x3F;
            }
        } else {
          //If the vector does not have this it should return false
          sendMeIntro1 = false;
        }
    }
  for (int i = 1; i < introInstOrd2Values.size(); i++) {
        if (introInstOrd2Values[i] == introInstOrd2Values[i - 1]&&introInstOrd2Values[i]!= 0) {
            count++;
            if (count == numValuesToCheck) {
                sendMeIntro2 = true;
                sendMeIntroOrdB = introInstOrd1Values[i] & 0x3F;
                sendMePreviousIntroOrdB = introInstOrd1Values[i] & 0x3F;
            }
        } else {
          sendMeIntro2 = false;
        }
    }
  
  }

void sendToespen(){
  controlChange(1,0b000001,0b000010);
  MidiUSB.flush();
}

void loopFunction(){
  int delayVar = 140;
  

  //Constant loop updating  MIDI-values
  //readMainFunctions();
  readIntroInstruments();
  //readVersInstrument();
  //readRefrengInstrument();
  //readSliderValues();
  
  //If it has been 50 checks since we send midiSignals check if midi are Good and send 
  //BURDE ENDRE LISTEN TIL 20 OG SJEKK OM GJENTAR SEG 15 GANGER
  if (millis()-previousMillisForLoop >= delayVar*20)
  {
    Serial.print("Hei");
    Serial.print(introInstOrdA);
    checkIfSendMidi();
    previousMillisForLoop = millis();
    sendMidi();
    //sendToespen();
  }
  
  delay(delayVar);

    
  //Constant loop sending MIDI-values
  //sendMidi();
  //sendSliderValues();
  //Lighting up the leds

  //LightUp();
  //sliderLeds();
  
}



void loop()
{
  int delayVar = 140;
  

  //Constant loop updating  MIDI-values
  //readMainFunctions();
  readIntroInstruments();
  //readVersInstrument();
  //readRefrengInstrument();
  //readSliderValues();
  
  //If it has been 50 checks since we send midiSignals check if midi are Good and send 
  //BURDE ENDRE LISTEN TIL 20 OG SJEKK OM GJENTAR SEG 15 GANGER
  if (millis()-previousMillisForLoop >= delayVar*20)
  {
    Serial.print("Hei");
    Serial.print(introInstOrdA);
    checkIfSendMidi();
    previousMillisForLoop = millis();
    sendMidi();
    //sendToespen();
  }
  
  delay(delayVar);

    
  //Constant loop sending MIDI-values
  //sendMidi();
  //sendSliderValues();
  //Lighting up the leds

  //LightUp();
  //sliderLeds();
}