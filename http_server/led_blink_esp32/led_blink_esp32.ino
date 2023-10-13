
int LED_BUILTIN = 2;

void setup(void){
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void){
  digitalWrite(LED_BUILTIN, LOW);  
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(500);
}
