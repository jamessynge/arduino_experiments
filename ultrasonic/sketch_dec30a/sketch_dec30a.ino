// So far, found that the stability of the Maxbotix LV-EZ1 isn't so good (very jumpy).
// But, have since found that this may be due to power supply fluctuations, which
// probably effect both the LV-EZ1, and the Arduino analogRead.
// I've compensated by adding smoothing of the data (see gatherData()), and by
// using non-USB power (which added a large fraction of the power fluctuations).

// This sketch is an attempt to further stabilize the readings by See if I can stabilize 

#include <ColorLCDShield.h>

LCDShield lcd;  // Creates an LCDShield, named lcd

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

char char_buffer[16];

int loopDelay = 300;

#define MAX_SAMPLES 3
int samples[MAX_SAMPLES];
int num_valid_samples = 0;
int next_sample = 0;
int vcc;

// mvcc is Vcc in millivolts.
int gatherData(int mvcc) {
  int sensor = analogRead(0);
  float millivolt_per_inch = mvcc / 512.0;
  int inches = int(sensor / millivolts_per_inch);

  samples[next_sample] = inches;
  next_sample = (next_sample + 1) % MAX_SAMPLES;
  if (num_valid_samples < MAX_SAMPLES) {
    ++num_valid_samples;
  }
  int sum = 0;
  for (int i = 0; i < num_valid_samples; ++i) {
    sum += samples[i];
  }
  return sum / num_valid_samples;
}

void lcdSetInt(int value, int x, int y, int fColor, int bColor) {
  sprintf(char_buffer, "%6d", value);
  lcd.setStr(char_buffer, x, y, fColor, bColor);
}

void drawValue(int value) {
  lcdSetInt(value, 2, 20, BLUE, WHITE);
}

void handleButtons(int s1, int s2, int s3) {
  if (s1) {
    loopDelay = 500;
    lcd.setStr("     Slow  ", 100, 0, BLUE, WHITE);
  } else if (s2) {
    loopDelay = 150;
    lcd.setStr("     Medium", 100, 0, BLUE, WHITE);
  } else if (s3) {
    loopDelay = 50;
    lcd.setStr("     Fast  ", 100, 0, BLUE, WHITE);
  }
}


//*******************************************************
//				Button Pin Definitions
//*******************************************************
#define	kSwitch1_PIN	3
#define	kSwitch2_PIN	4
#define	kSwitch3_PIN	5

void checkInputs() {
  int s1, s2, s3; // Make integers for all the swiches
  s1 = !digitalRead(kSwitch1_PIN);
  s2 = !digitalRead(kSwitch2_PIN);
  s3 = !digitalRead(kSwitch3_PIN);

  handleButtons(s1, s2, s3);
}

void setup() {
//  lcd.init(EPSON);  // Initializes lcd, using an EPSON driver
  lcd.init(PHILLIPS);  // Initializes lcd, using an PHILLIPS driver
  lcd.contrast(40);  // 40's usually a good contrast value
  lcd.clear(WHITE);

  handleButtons(0, 1, 0);
  
  analogRead(0);
}


void loop() {
  int vcc = readVcc();
  int inches = gatherData(vcc);

  lcdSetInt(inches, 2, 20, BLUE, WHITE);
  lcdSetInt(vcc, 40, 20, RED, WHITE);
  
  checkInputs();

  // pause before taking the next reading
  delay(loopDelay);  
}

