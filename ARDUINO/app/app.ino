int door = 4;
int lights = 5;
int lock = 6;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(door, OUTPUT);
  pinMode(lights, OUTPUT);
  pinMode(lock, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    char input = (char)Serial.read();
    if(input == 'a')
      digitalWrite(lights, !digitalRead(lights));
    else if (input == 'b')
      digitalWrite(door, !digitalRead(door));
    else if (input == 'c')
      digitalWrite(lock, !digitalRead(lock));
  }
  delay(250);
}
