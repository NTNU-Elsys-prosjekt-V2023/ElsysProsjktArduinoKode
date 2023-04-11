#include <Arduino.h>
//Legger til Expander MCP23017
#include <Adafruit_MCP23X17.h>
#include "MIDIUSB.h"
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

//More Globelvariables that for the sliders
int sliderInst1= 0;
int sliderInst2= 0;
int sliderBass = 0;
int sliderDrums = 0;
int sliderVers = 0;
int sliderIntro = 0;
int sliderRefreng = 0;


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


void setup()
{
  Serial.begin(115200);
  //setter opp digital inputs(A0-A5)
 for(int i = 0; i <= 11; i++){
    mcp1.pinMode(i, INPUT);
  }
  for(int i = 0; i <= 11; i++){
    mcp2.pinMode(i, INPUT);
  }
  for(int i = 0; i <= 11; i++){
    mcp3.pinMode(i, INPUT);
  }
}

void loop()
{
  //Legger til at den skal 
  readIntroInstruments();
  readVersInstrument();
  readRefrengInstrument();
  sendMidi();
  delay(100);
  readSliderValues();
  sendSliderValues();
  delay(100);
}