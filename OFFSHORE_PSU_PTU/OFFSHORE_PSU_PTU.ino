// CODE TAKES INPUT SIGNAL FROM PROGRAMMABLE TIMING UNIT (PTU) FROM LAVISION
// AND CAN ASSIGN MULTIPLES OF THAT SIGNAL AS OUTPUTS FOR SUB-SAMPLING
// PORTLAND STATE UNIVERSITY [PSU], ONDREJ FERCAK, 9/15/2023

int mult_A;             // Declaring multiplication interval for A
int mult_B;             // Declaring multiplication interval for B
int mult_C;             // Declaring multiplication interval for C
int cnt_A   = 0;        // Counter for use on Output A
int cnt_B   = 0;        // Counter for use on Output A
int cnt_C   = 0;        // Counter for use on Output A
int hl_cnt  = 0;        // High - Low Counter for Trigger Duration
bool start  = true;     // Flag for starting loop after user input
bool first  = true;     // Flag for first loop only

void setup() {
  DDRD &= !B00001000;   // pin [D2, PD2]  as digital input
  DDRD |=  B00000100;   // pin [D2, PB4]  as digital output
  DDRD |=  B00010000;   // pin [D4, PB4]  as digital output
  DDRB |=  B00010000;   // pin [D12, PB4] as digital output

  Serial.begin(2000000);                                          // begin serial for user defined inputs
  attachInterrupt(digitalPinToInterrupt(3), PTU_Input, CHANGE);   // assign trigger interrupt to digital pin 3
}

void loop() {
  if (start == true){                                             // run only at first pass
  inputParameters();                                              // assign trigger input multiples to outputs
  }
}

/////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////

// Assign Multiples of Input Signal to Outputs A, B, and C
void inputParameters() {
  Serial.println("A [1x POWER] Trigger Signal Multiple?"); 
  while (Serial.available() == 0) {}  
  mult_A = Serial.readString().toInt();
  Serial.print("A = Input x "); Serial.println(mult_A);

  Serial.println("B [8x CAMERA] Trigger Signal Multiple?"); 
  while (Serial.available() == 0) {}  
  mult_B = Serial.readString().toInt();
  Serial.print("B = Input x "); Serial.println(mult_B);

  Serial.println("C [2x AUX] Trigger Signal Multiple?"); 
  while (Serial.available() == 0) {}  
  mult_C = Serial.readString().toInt();
  Serial.print("C = Input x "); Serial.println(mult_C);

  Serial.println("\nPSU_PTU CONFIGURED! READY FOR INPUT!...");
  start = false;
}

// Interupt Routine for Input Signal
void PTU_Input() {
  hl_cnt++;
  cnt_A++;
  cnt_B++;
  cnt_C++;

  if (first == true) {
    PORTD      |= B00010000;          // output pin [Arduino Nano D12, PB4] to HIGH
    PORTB      |= B00010000;          // output pin [Arduino Nano D12, PB4] to HIGH
    PORTD      |= B00000100;          // output pin [Arduino Nano D12, PB4] to HIGH
    first = false;
    cnt_A = 0;
    cnt_B = 0;
    cnt_C = 0;
  }

  else {
    // Serial.print("A = "); Serial.println(cnt_A);
    // Serial.print("B = "); Serial.println(cnt_B);
    // Serial.print("C = "); Serial.println(cnt_C);

    A_on();
    B_on();
    C_on();
  }

  All_off();
}

// Function for Sending Output Signal A
void A_on() {
  if ((hl_cnt == 1 && cnt_A >= 2*mult_A)) {
    PORTD      |= B00010000;          // output pin [Arduino Nano D12, PB4] to HIGH
    cnt_A = 0;
  }
}

// Function for Sending Output Signal B
void B_on() {
  if ((hl_cnt == 1 && cnt_B >= 2*mult_B)) {
    PORTB      |= B00010000;          // output pin [Arduino Nano D12, PB4] to HIGH
    cnt_B = 0;
  }
}

// Function for Sending Output Signal C
void C_on() {
  if ((hl_cnt == 1 && cnt_C >= 2*mult_C)) {
    PORTD      |= B00000100;          // output pin [Arduino Nano D12, PB4] to HIGH
    cnt_C = 0;
  }
}

// Function for Turning Off Output Signals
void All_off() {
  if (hl_cnt >= 2) {
    PORTD      &= !B00000100;         // output pin [Arduino Nano D2, PD2]  to LOW  
    PORTD      &= !B00010000;         // output pin [Arduino Nano D4, PD4]  to LOW 
    PORTB      &= !B00010000;         // output pin [Arduino Nano D12, PB4] to LOW 
    hl_cnt = 0;
  }
}
