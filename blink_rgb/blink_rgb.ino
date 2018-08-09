

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  //Serial.begin(115200);
  

}

// common anode
//#define A HIGH
//#define B LOW

// common cathode
#define A LOW
#define B HIGH


void loop() {
  int d = analogRead(0);
  d >>= 2;
  d += 1;
  //Serial.println(d);

  //int d = 300;
  digitalWrite(0, B);  
  digitalWrite(2, A);  
  delay(d);                       
  digitalWrite(0, A);   
  digitalWrite(1, B);
  delay(d);
  digitalWrite(1, A);
  digitalWrite(2, B);   
  delay(d);
}



