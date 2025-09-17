#define PIN_LED 7

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_LED, OUTPUT);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(PIN_LED, 0);
  delay(1000);
  for (int count = 0; count < 5; count++) {
    digitalWrite(PIN_LED, 1);
    delay(100);
    digitalWrite(PIN_LED, 0);
    delay(100);
  }
  digitalWrite(PIN_LED, 1);
  while(1) {}

}
