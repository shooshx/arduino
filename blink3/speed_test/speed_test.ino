

unsigned long startTime = 0;
unsigned long count = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  
  startTime = millis();

  

}

// common anode
//#define A HIGH
//#define B LOW

// common cathode
#define A LOW
#define B HIGH


void loop() {
  ++count;

  if ((count & 0xff) == 0) {
    unsigned long now = millis();
    if (now - startTime > 1000) {
      Serial.println(count);

      count = 0;
      startTime = now;
    }
  }  
  //int d = 2/000;
  //digitalWrite(0, B);  
 /* digitalWrite(2, A);  
  //delay(d);                       
  digitalWrite(0, A);   
  digitalWrite(1, B);
  //delay(d);
  digitalWrite(1, A);
  digitalWrite(2, B);   
  //delay(d);*/
}



