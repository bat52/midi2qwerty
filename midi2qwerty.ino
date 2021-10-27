/* 
   Arduino Pro Micro MIDI to QWERTY conversion.
   by: Marco Merlin 2019
   
   Original code:
   Pro Micro Test Code
   by: Nathan Seidle
   modified by: Jim Lindblom
   SparkFun Electronics
   date: September 16, 2013
   license: Public Domain - please use this code however you'd like.
   It's provided as a learning tool.

   This code is provided to show how to control the SparkFun
   ProMicro's TX and RX LEDs within a sketch. It also serves
   to explain the difference between Serial.print() and
   Serial1.print().
*/

#include <MIDI.h>
#include "Keyboard.h"

#define RXLED_ON  digitalWrite(RXLED, LOW)   // set the LED on
#define RXLED_OFF digitalWrite(RXLED, HIGH)  // set the LED off

// 1 turns on debug, 0 off
#define DBGSERIAL if (0) SERIAL_PORT_MONITOR
#ifdef USBCON
#define MIDI_SERIAL_PORT Serial1
#else
#define MIDI_SERIAL_PORT Serial
#endif

//button pressbutton unpressbutton pressbutton unpress
int RXLED = 17;  // The RX LED has a defined Arduino pin
// The TX LED was not so lucky, we'll need to use pre-defined
// macros (TXLED1, TXLED0) to control that.
// (We could use the same macros for the RX LED too -- RXLED1,
//  and RXLED0.)
const int buttonPin = 4;          // input pin for pushbutton

struct MySettings : public midi::DefaultSettings
{
  static const bool Use1ByteParsing = false;
  static const unsigned SysExMaxSize = 1026; // Accept SysEx messages up to 1024 bytes long.
  static const long BaudRate = 31250;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, MIDI_SERIAL_PORT, MIDIUART, MySettings);

inline uint8_t writeUARTwait(uint8_t *p, uint16_t size)
{
  // Apparently, not needed. write blocks, if needed
  //  while (MIDI_SERIAL_PORT.availableForWrite() < size) {
  //    delay(1);
  //  }
  return MIDI_SERIAL_PORT.write(p, size);
}

uint16_t sysexSize = 0;

void sysex_end(uint8_t i)
{
  sysexSize += i;
  DBGSERIAL.print(F("sysexSize="));
  DBGSERIAL.println(sysexSize);
  sysexSize = 0;
}

const uint8_t MIDI_passthru_pin=2;
bool MIDI_passthru;

enum midi_pitch {
  MIDI_B2 = 47,
  MIDI_C3 = 48,
  MIDI_D3b,
  MIDI_D3,
  MIDI_E3b,
  MIDI_E3,
  MIDI_F3,
  MIDI_G3b,
  MIDI_G3,
  MIDI_A3b,
  MIDI_A3,
  MIDI_B3b,
  MIDI_B3,
  MIDI_C4,
  MIDI_D4b,
  MIDI_D4,
  MIDI_E4b,
  MIDI_E4,
  MIDI_F4,
  MIDI_G4b,
  MIDI_G4,
  MIDI_A4b,
  MIDI_A4,
  MIDI_B4b,
  MIDI_B4,
  MIDI_C5,
  MIDI_D5b
  };

#define FIRST_MIDI_NOTE MIDI_B2
#define LAST_MIDI_NOTE  MIDI_D5b

char midi_map[] = {
  ' ', // [MIDI_B2]
  'z', // [MIDI_C3]
  's', // [MIDI_D3b]
  'x', // [MIDI_D3]
  'd', // [MIDI_E3b]
  'c', // [MIDI_E3] 
  'v', // [MIDI_F3] 
  'g', // [MIDI_G3b]
  'b', // [MIDI_G3]
  'h', // [MIDI_A3b]
  'n', // [MIDI_A3]
  'j', // [MIDI_B3b]
  'm', // [MIDI_B3]
  'q', // [MIDI_C4]
  '2', // [MIDI_D4b]
  'w', // [MIDI_D4]
  '3', // [MIDI_E4b]
  'e', // [MIDI_E4]
  'r', // [MIDI_F4]
  '5', // [MIDI_G4b]
  't', // [MIDI_G4]
  '6', // [MIDI_A4b]
  'y', // [MIDI_A4]
  '7', // [MIDI_B4b]   
  'u', // [MIDI_B4]
  'i', // [MIDI_C5]
  ' '  // [MIDI_D5b]
  };

bool midi_note_on[LAST_MIDI_NOTE - FIRST_MIDI_NOTE]  = {false};

void midi_retrigger()
{
  uint8_t pitch;
  bool trigger = false;
  
  for(pitch = 0; pitch <= (LAST_MIDI_NOTE-FIRST_MIDI_NOTE) ; pitch++)
    {
      if(midi_note_on[pitch])
        {
        Keyboard.print( midi_map[ pitch ] );
        trigger = true;
        }
    }

  // wait for extra time if a note was triggered
  // to reduce the number of characters generated
  if(trigger)
      delay(50);
}

void setup()
{
 pinMode(RXLED, OUTPUT);  // Set RX LED as an output
 // TX LED is set as an output behind the scenes

 //Serial.begin(9600); //This pipes to the serial monitor

 // make the pushButton pin an input:
 pinMode(buttonPin, INPUT);

 //if(keyboard_en)
  // initialize control over the keyboard:
 Keyboard.begin();

 // Pin 0 LOW selects MIDI pass through on
  pinMode(MIDI_passthru_pin, INPUT_PULLUP);
  MIDI_passthru = (digitalRead(MIDI_passthru_pin) == LOW);

  MIDIUART.begin(MIDI_CHANNEL_OMNI);
  if (MIDI_passthru) {
    DBGSERIAL.println("MIDI thru on");
  }
  else {
    DBGSERIAL.println("MIDI thru off");
    MIDIUART.turnThruOff();

}
 
}

void loop()
{
  uint8_t pitch, mempitch;

  //  delay(10);              // wait for 100ms 
  // midi_retrigger();

  /* MIDI UART -> MIDI USB */
  if (MIDIUART.read()) 
  {
    midi::MidiType msgType = MIDIUART.getType();
    DBGSERIAL.print(F("UART "));
    DBGSERIAL.print(msgType, HEX);
    DBGSERIAL.print(' ');
    DBGSERIAL.print(MIDIUART.getData1(), HEX);
    DBGSERIAL.print(' ');
    DBGSERIAL.println(MIDIUART.getData2(), HEX);
    switch (msgType) {
      case midi::InvalidType:
        break;
      case midi::NoteOff:
            // Keyboard.print("Note Off\n");
            pitch = MIDIUART.getData1();
            
            mempitch = pitch - FIRST_MIDI_NOTE;
            if (mempitch < (LAST_MIDI_NOTE - FIRST_MIDI_NOTE))
              midi_note_on[ pitch - FIRST_MIDI_NOTE ] = false;
              
            break;
      case midi::NoteOn:
            // Keyboard.print("Note On\n");
            pitch = MIDIUART.getData1();
            // velocity = MIDIUART.getData2();
            // Keyboard.print( midi_map[ MIDIUART.getData1() ] );
            // Keyboard.println(velocity);
            //  Keyboard.print( midi_map[ pitch - FIRST_MIDI_NOTE ] );

            mempitch = pitch - FIRST_MIDI_NOTE;
            if (mempitch < (LAST_MIDI_NOTE - FIRST_MIDI_NOTE))
              {
              midi_note_on[ mempitch ] = true;
              midi_retrigger();
              }
            
            break;
      case midi::AfterTouchPoly:
      case midi::ControlChange:
      case midi::ProgramChange:
      case midi::AfterTouchChannel:
      case midi::PitchBend:
        { 
          //  MIDIUART.getData1(),
          //  MIDIUART.getData2()
          break;
        }
      case midi::SystemExclusive:
        DBGSERIAL.print("sysex size ");
        DBGSERIAL.println(MIDIUART.getSysExArrayLength());
        break;
      case midi::TuneRequest:
      case midi::Clock:
      case midi::Start:
      case midi::Continue:
      case midi::Stop:
      case midi::ActiveSensing:
      case midi::SystemReset:
      case midi::TimeCodeQuarterFrame:
      case midi::SongSelect:
      case midi::SongPosition:
      default:
        break;
    }
  }
}
