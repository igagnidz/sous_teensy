#include <TimerOne.h>
#include <math.h>

const boolean SOUS_DEBUG = true;  // Enable/Didsable global debug mode

const int SSR1 = PIN_B0;  // pin for solid-state relay 1
const int SSR2 = PIN_B1;  // pin for solid-state relay 2
const int SSR3 = PIN_B2;  // pin for solid-state relay 3
const int SSR4 = PIN_B3;  // pin for solid-state relay 4
const int SSRS[] = { SSR1, SSR2, SSR3, SSR4 };  // array for SSRs

const int TMP1 = 38;  // pin for solid-state relay 1
const int TMP2 = 39;  // pin for solid-state relay 2
const int TMP3 = 40;  // pin for solid-state relay 3
const int TMP4 = 41;  // pin for solid-state relay 4
const int TMPS[] = { TMP1, TMP2, TMP3, TMP4 };  // array for TMPs

const int DEBUG_PINS[] = { PIN_D0, PIN_D1, PIN_D2, PIN_D3 };  // debug pins
const int TOTAL_SSRS = 4;  // number of ssrs
volatile int Temp[] = { 0, 0, 0, 0 }; // use volatile for shared variables

void turnOffSsrs(void) {
  for (int i = 0; i < TOTAL_SSRS; i = i + 1) {
    digitalWrite(SSRS[i], LOW);
  }  
}

void enableIOPins(void) {
  for (int i = 0; i < TOTAL_SSRS; i = i + 1) {
    pinMode(SSRS[i], OUTPUT);
    //pinMode(TMPS[i], INPUT);
    pinMode(DEBUG_PINS[i], INPUT);
  }  
}

void setupISR(void) {
  Timer1.initialize(1000000); // 1000ms
  Timer1.attachInterrupt(ensureTemperature); // ensureTemperature every 0.5 seconds
}

int readTemp(int idx) {
  double celsius = -1;
  double fahrenheit = -1;
  double code = analogRead(TMPS[idx]);

  celsius = 0.0962914 * code - 22.7964107;
  fahrenheit = celsius * 1.8 + 32;
  return fahrenheit;
}

void ensureTemperature(void) {
  for (int i = 0; i < TOTAL_SSRS; i = i + 1) {
    double setTemp = Temp[i];
    double currentTemp = readTemp(i);
    
    if (currentTemp >= setTemp) {  // Turn off heater
      digitalWrite(SSRS[i], LOW);
    } 
    else if (currentTemp < setTemp && currentTemp > setTemp - 1) { // Toggle heater if almost setTemp
      digitalWrite(SSRS[i], digitalRead(SSRS[i]) ^ HIGH);
    }   
    else if (currentTemp < setTemp) {
      digitalWrite(SSRS[i], HIGH);
    }   
  }      
}

void setup(void) {
  enableIOPins();
  turnOffSsrs();
  setupISR();
  Temp[0] = 70;
  Temp[1] = 90;
  Serial.begin(9600);
}

// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void) {
  char buf[255];
  uint8_t n;

  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.
  noInterrupts();
  interrupts();
  n = recv_str(buf, sizeof(buf));
  if (n > 0) {
    Serial.println("got data");
  }
  if (SOUS_DEBUG) {
    for (int i = 0; i < TOTAL_SSRS; i = i + 1) {
      String message = "Pin";
      Serial.print(message + " SSR " + i + " = ");
      Serial.println(digitalRead(DEBUG_PINS[i]));
      Serial.print(message + " TMP " + i + " = ");
      Serial.println(readTemp(i));
    }
  }    

  delay(1000);
}

// Receive a string from the USB serial port.  The string is stored
// in the buffer and this function will not exceed the buffer size.
// A carriage return or newline completes the string, and is not
// stored into the buffer.
// The return value is the number of characters received
//
uint8_t recv_str(char *buf, uint8_t size)
{
  int16_t r;
  uint8_t count=0;

  while (Serial.available() > 0) {               // Read data if it is there
    r = Serial.read();
    if (r == '\r' || r == '\n') return count;     // Break on new line
    if (r >= ' ' && r <= '~' && count < size) {   // Skip if not word char or buff full 
      *buf++ = r;                                 // Store data in buffer
      Serial.write(r);                           // Send same data back
      count++;                                    // Increment count
    }
  }
  return count;
}


