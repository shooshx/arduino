#include <Servo.h>

Servo servo1, servo2, servo3;


// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  servo1.attach(9);
  servo2.attach(10);
  servo3.attach(11);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  
  Serial.println("Ready");
  
}

String inWords[3];
int atWord = 0;
int pos[3];

void readLine() {

  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (inChar == '\n') {
      pos[0] = inWords[0].toInt();
      pos[1] = inWords[1].toInt();
      pos[2] = inWords[2].toInt();
      inWords[0] = "";
      inWords[1] = "";
      inWords[2] = "";
      atWord = 0;
      Serial.print(pos[0]);
      Serial.print(" ");
      Serial.print(pos[1]);
      Serial.print(" ");
      Serial.print(pos[2]);      
      Serial.println(" OK");
      servo1.write(pos[0]);
      servo2.write(pos[1]);
      servo3.write(pos[2]);
    }
    else if (inChar == ' ') {
      ++atWord;
    }
    else {
      if (atWord < 3)
        inWords[atWord] += (char)inChar;
    }

  }

}

void loop() {
  readLine();
  

}
