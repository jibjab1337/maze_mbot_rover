#include <Servo.h>

// Left wheel motor
int motorL = 6;
int motorL_dir = 7;

// Right wheel motor
int motorR = 5;
int motorR_dir = 4;

//Servo
int servoPin = 15;
Servo panServo;

// Press Button
int buttonPin = A7;
bool button = true;
int x = 0;

//endsong
int buzzer = 8; // change this to whichever pin you want to use for buzzer

//ultrasound
int pingPin = 17;
int distF, distR, distL;

// Infra-red
int IR_L = 9;
int IR_R = 10;
int leftSensor, rightSensor;
int startLeftSensor, startRightSensor;  // intial reading of color of surface(floor)
bool playSong;

//Bluetooth
byte a;

//CHANGE THESE VALUES IF YOU HAVE TO FOR PAN ANGLES
int panLeft = 155;
int panRight = 15;
int turnDuration = 270; //365;-@150turn//570;//delay time in ms for turning 90 degrees
int roverSpeed = 240;   //speed of rover motors
int speedOfPan=5;       //speed of panning servo
// Also in the turn() function change the speed of each wheel if rover doesnt go straight
// As default the left wheel is slower than the right wheel so change if needed


void setup() {
  // pin initialization
  Serial.begin(115200);
  startLeftSensor = digitalRead(IR_L);
  startRightSensor = digitalRead(IR_R);

  pinMode(buttonPin, INPUT);    //Button Pin
  
  pinMode(motorL_dir, OUTPUT);  // Left motor Analog Pin
  pinMode(motorL, INPUT);       // Left motor digital Pin
  pinMode(motorR_dir, OUTPUT);  // Right motor Analog Pin
  pinMode(motorR, INPUT);       // Right motor digital Pin
  
  panServo.attach(servoPin);
  pan(90);                      // Sets the ulrasonic sensor to look straight ahead at 90 deg
}

void loop() {
  if (buttonPress()!= true && infraredCheck()!=true) { 
    playSong = true;            // just a temp variable to make endSong() play once
    automation();    
  }
  else if(playSong!=true && infraredCheck()==true){ // infrared is true means it disables infrared 
    BluetoothFunction();                            // this will run when rover finishes maze and detects white/black line
  }
  else {
    Stop();
  }
}

//*******BUTTON ON ROVER*********
// Press button to initiate automation function
bool buttonPress() {
  int buttonState = analogRead(buttonPin);
  if ((0 ^ (buttonState > 10 ? 0 : 1))) {
    startLeftSensor = digitalRead(IR_L);  // this is not part of the button code but it is here 
    startRightSensor = digitalRead(IR_R); // to detect the surface and determine what color line it should 
    if (x < 10) {                         // detect later. It is inside here because rover will most likely
      if (x % 2 == 0) {                   // even be on the ground to record the color of the surface when pressing button
        button = false;                   // turn off rover
      } else if (x & 0x01) {              // odd
        button = true;                    // turn On rover
      }
      x++;
    } else {
      x = 0;
      button = false;
    }
  }
  return button;
  delay(50);
}

//*********AUTOMATION********
// Using on sensors to move and complete a maze
void automation() {
  pan(90);                              // turn head forward
  distF = ultrasonic();                 // check forward for any obstacles
                                        // distF = 7 at pwm 240 turn 110
  if (distF > 10) {                     // if no obstacle in front
    Forward(roverSpeed, 40);            // then move rover straight ahead
  }
  else if (distF <= 10) {
    Stop();                             // Stop if close to an object in front
                                        // check both sides for obstacles
    pan(panLeft);                       // change to 165 for your rover. I put extension on head so it can turn more
    distL = ultrasonic();               // measure left side distance
    pan(panRight);                      // pan right
    distR = ultrasonic();               // measure right side distance
    pan(90);                            // pan forward
    distF = ultrasonic();               // measure the front distance
    delay(100);
    if (distF <= 10 && distR <= 15) {   // if theres a wall in front and on the right side then
      turnLeft(turnDuration);           // turn left by 90 deg
    }
    else if (distF <= 10 && distL <= 15) {  // if theres a wall in front and on the left side then
      turnRight(turnDuration);              // turn right by 90 deg
    }
    else if (distF <= 10 && distR < 15 && distL < 15) { //
      Reverse(roverSpeed, 40);          // Reverse and
      turnRight(turnDuration * 2);      // turn right by 180 deg
    }
    else if (distF <= 10 && distR > 15 && distL > 15) {
      if (distR > distL) {
        turnRight(turnDuration);        // turn right by 90 deg
      }
      else if (distR < distL) {
        turnLeft(turnDuration);         // turn left by 90 deg
      }
    }
  }
}

//*****TURN LEFT***********
void turnLeft(int angle) {
  analogWrite(motorL, 200);   //110 default, 150 @ 240
  digitalWrite(motorL_dir, HIGH);
  analogWrite(motorR, 200);   //110
  digitalWrite(motorR_dir, HIGH);
  delay(angle);               //use 350
}

//****TURN RIGHT****************
void turnRight(int angle) {
  analogWrite(motorL, 200);   //110 default
  digitalWrite(motorL_dir, LOW);
  analogWrite(motorR, 200);   //110
  digitalWrite(motorR_dir, LOW);
  delay(angle);
}

//*********FORWARD MOVEMENT************
void Forward(int pwm, int duration) {
  analogWrite(motorL, pwm - 14);  //-8 at pwm 210
  digitalWrite(motorL_dir, LOW);  //-12 at pwm 220
  analogWrite(motorR, pwm);
  digitalWrite(motorR_dir, HIGH);
  delay(duration);
}

//******************REVERSE MOVEMENT************
void Reverse(int pwm, int duration) {
  analogWrite(motorL, pwm);
  digitalWrite(motorL_dir, HIGH);
  analogWrite(motorR, pwm);
  digitalWrite(motorR_dir, LOW);
  delay(duration);              //use 40
}

//******************STOP MOVEMENT************
void Stop() {
  analogWrite(motorL, 0);
  digitalWrite(motorL_dir, LOW);
  analogWrite(motorR, 0);
  digitalWrite(motorR_dir, LOW);
  delay(40);
}

//****ULTRASONIC SENSOR******
int ultrasonic() {
  int duration, cm;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(5);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  //Read from same pin. The duration of the HIGH pulse tells the distance
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  //convert time into a distance
  cm = duration / 58;
  //Serial.print(cm);
  //Serial.println("cm");
  delay(20);
  return cm;
}

//****PANNING OF SERVO MOTOR********
void pan(int deg) {
  int initial = panServo.read();
  if (deg > initial) {
    for (int angle = initial; angle < deg; angle += speedOfPan) {
      panServo.write(angle);
      delay(20);
    }
  } else if (deg < initial) {
    for (int angle = initial; angle > deg; angle -= speedOfPan) {
      panServo.write(angle);
      delay(20);
    }
  }
}

//*****INFRARED SENSOR*********
bool infraredCheck() {
  bool colorOfLine;
  if (startLeftSensor == HIGH && startRightSensor == HIGH) {  // if your floor is white it will detect a black  line
    leftSensor = digitalRead(IR_L);
    rightSensor = digitalRead(IR_R);
    if (leftSensor == HIGH && rightSensor == HIGH) {
       colorOfLine = false;
    } else if (leftSensor == LOW && rightSensor == LOW) {     // when it detects a black line it will stop and play song
      //delay(1000);
      Stop();                                                 // delays so rover moves past the colored line and then stops
      if (playSong == true) {                                 // this if condition is so that the song
        endSong();                                            // doesn't repeat after it plays once
        playSong = false;
      }
      colorOfLine = true;
    }
  }
  if (startLeftSensor == LOW && startRightSensor == LOW) {    // if your floor is white it will detect a black line
    leftSensor = digitalRead(IR_L);
    rightSensor = digitalRead(IR_R);
    if (leftSensor == LOW && rightSensor == LOW ) {
      colorOfLine = false;
    } else if (leftSensor == HIGH   && rightSensor == HIGH) {
      //delay(1000);
      Stop();                                                 // delays so rover moves past the colored line and then stops
      //BluetoothFunction();
      if (playSong == true) {                                 // this if condition is so that the song
        endSong();                                            // doesn't repeat after it plays once
        playSong = false;                                     // it turns back true inside automation()
        }
      colorOfLine = true;
    }
  }
  return colorOfLine;
  
}

//*****BLUETOOTH FUNCTION******
void BluetoothFunction() {

  char b;
  bool Quit = false; 
  
  Serial.println("Enter commands ['A': automation, 'F': forward, 'B': reverse, 'L': turn left, 'R': turn right, 'S': stop, 'E': exit] to start rover's functions");
  delay(100);
   
  while (!Quit){  
    Stop();
    if (Serial.available() > 0) { // if, while
      b = Serial.read();
      switch (b){
        case 65:     // Letter A
          Serial.print("The rover will begin maze automation procedure.");
           automation();// ADD YOUR FUNCTION THAT MAKES IT GO ON AUTOMATION
          break;
        case 70:     // Letter F
          Serial.print("The rover moves forward!");
           Forward(150, 700);// ADD YOUR FUNCTION THAT MAKES IT GO FORWARD FOR A SHORT AMOUNT OF TIME
          break;
        case 66:     // Letter B
          Serial.print("The rover moves in reverse!");
          Reverse(150, 500);// ADD YOUR FUNCTION THAT MAKES IT REVERSE FOR A SHORT AMOUNT OF TIME
          break;
        case 76:     // Letter L
          Serial.print("The rover turns left!");
          turnLeft(turnDuration/2);// ADD YOUR FUNCTION THAT MAKES IT TURN LEFT 
          Stop();
          break;
        case 82:     // Letter R
          Serial.print("The rover turns right!");
          turnRight(turnDuration/2); // ADD YOUR FUNCTION THAT MAKES IT TURN RIGHT 
          Stop();
          break;
        case 83:     // Letter S
          Serial.print("The rover will stop.");
          Stop();// ADD YOUR FUNCTION THAT MAKES IT STOP
          delay(500);
          break;
        case 69:     // Letter E
          Serial.print("Exiting the menu.");
          Quit = true;
           button= true; // might need fixing
          break;
        case 68:     // Letter D
          Serial.print("Delay");
           delay(1000); // ADD DELAY
           break;
      }
      delay(100);
    }
  }
}

//****SHORT SONG PLAYS AT THE END OF MAZE******
void endSong(){
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0


// change this to make the song slower or faster
int tempo = 114;

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

  // Never Gonna Give You Up - Rick Astley
  // Score available at https://musescore.com/chlorondria_5/never-gonna-give-you-up_alto-sax
  // Arranged by Chlorondria


  NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,
  NOTE_FS5,-8, NOTE_FS5,-8, NOTE_E5,-4, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,
  NOTE_A5,4, NOTE_CS5,8, NOTE_D5,-8, NOTE_CS5,16, NOTE_B4,8, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,

  NOTE_D5,4, NOTE_E5,8, NOTE_CS5,-8, NOTE_B4,16, NOTE_A4,4, NOTE_A4,8,  //23
  NOTE_E5,4, NOTE_D5,2, REST,4,


};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }
    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(buzzer);
  }
}
