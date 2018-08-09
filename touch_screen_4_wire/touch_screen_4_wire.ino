/* DOCUMENTATION BY CHANDAN KUMAR 01/03/2015
INTERFACE 4 WIRE RESISTIVE TOUCH SCREEN WITH ARDUINO

            ________________
           |                |
           |                |
           |     TOUCH      |
           |                |
           |                |
           |     SCREEN     |
           |    (YL1083)    |
           |                |
           |                |
            ________________

           ||||||||
           __   __   __   __
          |X1| |Y1| |X2| |Y2|
           ||   ||   ||   ||
           A0   A1   A2   A3
              
FOR Y-AXIS:

X1 GND
X2 +5V
MAKE Y1 AND Y2 AS INPUT MEANS HIGH IMPEDANCE
NOW READ ANALOG VALUE ON Y2

FOR X-AXIS:

Y1 GND
Y2 +5V
MAKE X1 AND X2 AS INPUT MEANS HIGH IMPEDANCE
NOW READ ANALOG VALUE ON Y2

*/              

const int X1=A0 ,Y1=A1, X2=A2, Y2=A3;


void setup() 
{
    Serial.begin(9600);
}

void loop()
{
    int x_cord=0, y_cord=0;
    pinMode(X1,OUTPUT);
    pinMode(X2,OUTPUT);
    digitalWrite(X1,LOW);
    digitalWrite(X2,HIGH);
    pinMode(Y1,INPUT);
    pinMode(Y2,INPUT);
    y_cord=analogRead(Y2);
    
    pinMode(Y1,OUTPUT);
    pinMode(Y2,OUTPUT);
    digitalWrite(Y1,LOW);
    digitalWrite(Y2,HIGH);
    pinMode(X1,INPUT);
    pinMode(X2,INPUT);
    
    x_cord=analogRead(X2);
       
   Serial.print("X = ");  
   Serial.print(x_cord);
   Serial.print(" Y = ");
   Serial.println(y_cord);
   delay(100);
}


