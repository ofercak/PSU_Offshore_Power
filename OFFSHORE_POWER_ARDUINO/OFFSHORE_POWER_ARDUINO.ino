#include <Wire.h>
#include <Adafruit_INA219.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ENCODER: DEFINE VARIABLES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Current Sensor
Adafruit_INA219 ina219;

// volatile unsigned long  prevTime;
volatile long             prevTRtime;
volatile long             currTRtime;
volatile unsigned long    curTime;
volatile unsigned long    initTime;
unsigned long             totalTimeSec;
unsigned long             totalTimeMicro;
unsigned long             resPoints;
volatile unsigned short   cnt = 0;
volatile unsigned short   total = 0;
float                     bus;
float                     shunt;
float                     current;
bool                      inputFlag = true;
bool                      initFlag = false;
bool                      intOK = false;
bool                      interupt_print = false;
int                       stopSeconds = 3;
int                       trigger = 0;
int                       trigger_curr = LOW;
int                       trigger_prev = LOW;
int                       wiperValue;
int                       digiStart;
int                       encSteps;
int                       digiInt;
int                       digiEnd;
int                       start;
int                       type;

int   resistorPin[29][3]= {{10, 11, {}}, { 9, 10, {}}, { 8, 10,  7}, {11, {}, {}}, 
                           { 9, {}, {}}, { 7,  8, {}}, { 8, {}, {}}, { 6,  7, {}}, 
                           { 3,  5,  6}, { 7, {}, {}}, { 4,  6, {}}, { 3,  5, A2},
                           { 6, A1, {}}, { 5, A3, {}}, { 6, {}, {}}, { 5, {}, {}},
                           { 4, {}, {}}, { 3, 13, {}}, { 3, A1, {}}, { 3, A3, {}},
                           {13, A1, {}}, {A0, A3, {}}, {A1, A3, {}}, { 3, {}, {}},
                           {13, {}, {}}, {A0, {}, {}}, {A1, {}, {}}, {A2, {}, {}},
                           {A3, {}, {}}};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INITIALIZE/SETUP
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  ina219.begin();                     // initialize ina 219 Current Sensor
  Serial.begin(2000000);              // set baud rate

  pinMode(2, INPUT);    pinMode(12, INPUT);
  pinMode(3, OUTPUT);   pinMode(4, OUTPUT);  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);   pinMode(7, OUTPUT);  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);   pinMode(10, OUTPUT); pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);  pinMode(A0, OUTPUT); pinMode(A1, OUTPUT); 
  pinMode(A2, OUTPUT);  pinMode(A3, OUTPUT);
  
  setupLoop ();

  if (type == 0) {
    while (trigger == 0) {
      read();
      if (trigger == 1) {
        print();
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN LOOP
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() { 

  // FIXED RESISTOR LOOP
  if (type == 0) {
    fixedMain();
  }

  // SWEEP RESISTOR LOOP
  else {
    sweepMain();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setupLoop () {
// INPUT PARAMETERS
 //////////////////////////////// 
  while (inputFlag == true) {     
    inputParameters();                // request user input for type of data collection
  }

  // INITIALIZE PARAMATERS
  ////////////////////////////////
  if (initFlag == true) {
    intOK       = true;
    wiperValue  = digiStart;                            // initialize the  wiper value variable
    digitalWrite(resistorPin[wiperValue - 1][0], HIGH); // write first  resistor pin to HIGH
    digitalWrite(resistorPin[wiperValue - 1][1], HIGH); // write second resistor pin to HIGH
    digitalWrite(resistorPin[wiperValue - 1][2], HIGH); // write third  resistor pin to HIGH
    initTime    = micros();                             // initialize the  previous time variable
    prevTRtime  = micros();                             // initialize the  previous time variable
    currTRtime  = micros();                             // initialize the  previous time variable
    initFlag    = false;                                // break out of init loop
  }

  if (type == 1) {
    // Initialize Interrupt Service Routine (ISR) for Row A - [Turbine Motors have 400 increments per rotation]
    attachInterrupt (digitalPinToInterrupt(2), turbineRotation, RISING);  
  }
}

void inputParameters () {
  Serial.println("Fixed [0] or Sweep [1] Resistance Values?"); 
  while (Serial.available() == 0) {}  
  type = Serial.readString().toInt();
  Serial.println(type);

  // FIXED INPUT LOOP
  if (type == 0) {
    Serial.println("Enter Resistance Value [1-29:ohms]!"); 
    while (Serial.available() == 0) {}  
    digiStart = Serial.readString().toInt();
    Serial.println(digiStart); 

    Serial.println("PRESS [0] TO START DATA COLLECTION or [1] to CHANGE PARAMETERS!");
    while (Serial.available() == 0) {}  
    start = Serial.readString().toInt();
    Serial.println(start);

    if (start == 0) {
      while (Serial.available() == 0) {}  
      Serial.readString();
      inputFlag = false;
      initFlag = true;
    }
  } 

  // SWEEP INPUT LOOP    
  else if (type == 1) {
    Serial.println("Number of Encoder Steps for Interrupt [1 rev = 400 steps]!"); 
    while (Serial.available() == 0) {}  
    encSteps = Serial.readString().toInt();
    Serial.println(encSteps); 

    Serial.println("Enter Number of Collection Points per Resistor Value [points]!"); 
    while (Serial.available() == 0) {}  
    resPoints = Serial.readString().toInt();
    Serial.println(resPoints);
    
    Serial.println("Enter Digipot START Value [1-29:ohms]!");
    while (Serial.available() == 0) {}  
    digiStart = Serial.readString().toInt();
    Serial.println(digiStart); 

    Serial.println("Enter Digipot INCREMENT Value [1-29]!"); 
    while (Serial.available() == 0) {}  
    digiInt = Serial.readString().toInt();
    Serial.println(digiInt); 

    Serial.println("Enter Digipot END Value [1-29:ohms]!");
    while (Serial.available() == 0) {}  
    digiEnd = Serial.readString().toInt();
    Serial.println(digiEnd); 

    Serial.println("PRESS [0] TO START DATA COLLECTION or [1] to CHANGE PARAMETERS!");
    while (Serial.available() == 0) {}  
    start = Serial.readString().toInt();
    Serial.println(start);

    if (start == 0) {
      while (Serial.available() == 0) {} 
      Serial.readString();
      inputFlag = false;
      initFlag = true;
    }
  } 
  
  // ERROR LOOP
  else {
    Serial.println("**LET'S TRY THAT AGAIN!**");
    delay(1500);
  }
}

void turbineRotation (){
  cnt++;
  if (cnt >= encSteps && intOK == true) {
    cnt = 0;
    total++;
    print();
    }
  }

void fixedMain() {
  curTime = micros();               // log fixed resistance value data
  fixedEnd();                       // end fixed resistance value data
  read();
  print();
}

void sweepMain() {
  curTime = micros();
  wiperSweep();                     // log sweep resistance value data
  sweepEnd();                       // end sweep resistance value data
  read();
}

void read() {
  currTRtime    = micros();
  bus           = ina219.getBusVoltage_V();
  shunt         = ina219.getShuntVoltage_mV();
  current       = ina219.getCurrent_mA();
  trigger_curr  = digitalRead(12);
  if (trigger_curr == HIGH && trigger_prev == LOW) {
    trigger = 1;
    prevTRtime  = micros();}
  else {
    trigger = 0;}
  trigger_prev = trigger_curr;
}

void print() {
  Serial.print(micros()); Serial.print(" ");
  Serial.print(wiperValue); Serial.print(" ");
  Serial.print(bus); Serial.print(" "); 
  Serial.print(shunt); Serial.print(" ");
  Serial.print(current); Serial.print(" ");
  Serial.println(trigger);
}

void wiperSweep() {
    if (total >= resPoints) {
    wiperValue = wiperValue + digiInt;
    digitalWrite(resistorPin[wiperValue - 1][0], HIGH);  // write first  resistor pin to HIGH
    digitalWrite(resistorPin[wiperValue - 1][1], HIGH);  // write second resistor pin to HIGH
    digitalWrite(resistorPin[wiperValue - 1][2], HIGH);  // write third  resistor pin to HIGH
    total = 0;
  }
}

void fixedEnd() {
  if ((currTRtime - prevTRtime) > stopSeconds*pow(10, 6)) {
    intOK = false;
    digitalWrite(resistorPin[wiperValue - 1][0], LOW);                          // write first  resistor pin to LOW
    digitalWrite(resistorPin[wiperValue - 1][1], LOW);                          // write second resistor pin to LOW
    digitalWrite(resistorPin[wiperValue - 1][2], LOW);                          // write third  resistor pin to LOW
    Serial.print("\nEncoder Steps: "); Serial.println(encSteps);
    Serial.print("Resistance: "); Serial.println(digiStart);
    Serial.print("Logging Time:"); Serial.println(totalTimeSec);
    Serial.println("\nDATA COLLECTION SUCCESSFUL! SAVING DATA...");
    delay(5);
    exit(0);
  }
}

void sweepEnd() {
  if (wiperValue > digiEnd || wiperValue > 29) {
    intOK = false;
    digitalWrite(resistorPin[wiperValue - 1][0], LOW);                        // write first  resistor pin to LOW
    digitalWrite(resistorPin[wiperValue - 1][1], LOW);                        // write second resistor pin to LOW
    digitalWrite(resistorPin[wiperValue - 1][2], LOW);                        // write third  resistor pin to LOW
    Serial.print("\nEncoder Steps: "); Serial.println(encSteps);
    Serial.print("Resistance Points: "); Serial.println(resPoints);
    Serial.print("Resistance Start: "); Serial.println(digiStart);
    Serial.print("Resistance Increment: "); Serial.println(digiInt);
    Serial.print("Resistance End: "); Serial.println(digiEnd);
    Serial.println("\nDATA COLLECTION SUCCESSFUL! SAVING DATA...");
    delay(5);
    exit(0);
  }
}
