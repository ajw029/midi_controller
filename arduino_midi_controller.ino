#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#define NUMBER_OF_SHIFT_CHIPS   2

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   1

/* You will need to change the "int" to "long" If the
 * NUMBER_OF_SHIFT_CHIPS is higher than 2.
*/
#define BYTES_VAL_T unsigned int

int ploadPin        = 8; // Green;  // Connects to Parallel load pin the 165
int clockPin        = 9; // Red; // Connects to the Clock pin the 165
int dataPin         = 10; // Yellow; // Connects to the Q7 pin the 165
int clockEnablePin  = 11; // White;  // Connects to Clock Enable pin the 165
byte note = 52 ;

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

int buttonsChannel[16]={0, 1, 2, 3, 4, 5 ,6, 7, 9, 8, 11, 10, 14, 13, 15, 12};

/* This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
    long bitVal;
    BYTES_VAL_T bytesVal = 0;

    /* Trigger a parallel Load to latch the state of the data lines,
    */
    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(ploadPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(ploadPin, HIGH);
    digitalWrite(clockEnablePin, LOW);

    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(dataPin);

        /* Set the corresponding bit in bytesVal.
        */
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockPin, LOW);
    }

    return(bytesVal);
}

void display_pin_values() {
    BYTES_VAL_T mask = 1;
    for(int i = 0; i < DATA_WIDTH; i++) {        
        int curVal = (pinValues >> i) & 1;
        int prevVal = (oldPinValues >> i) & 1;

        if (prevVal != curVal) {
          int correctNote = buttonsChannel[i] + note;
          if(curVal > 0) {
            MIDI.sendNoteOn(correctNote, 127, 1); 
          } else {
            MIDI.sendNoteOn(correctNote, 0, 1); 
          }
        }
//        Serial.print("\n");
    }
}

void setup()
{
    Serial.begin(9600);


    pinMode(ploadPin, OUTPUT);
    pinMode(clockEnablePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, INPUT);

    digitalWrite(clockPin, LOW);
    digitalWrite(ploadPin, HIGH);

    pinValues = read_shift_regs();
    display_pin_values();
    oldPinValues = pinValues;
}

void loop() {
    pinValues = read_shift_regs();

    if(pinValues != oldPinValues)
    {
        display_pin_values();
        oldPinValues = pinValues;
    }

    delay(POLL_DELAY_MSEC);
}

