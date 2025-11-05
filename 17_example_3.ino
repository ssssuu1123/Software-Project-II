#include <Servo.h>

// Arduino pin assignment
#define PIN_IR    A0         
#define PIN_LED   9
#define PIN_SERVO 10

// Servo pulse (us)
#define _DUTY_MIN 1000
#define _DUTY_NEU 1600
#define _DUTY_MAX 2200

// Distance limits (mm)
#define _DIST_INVALID 60.0   
#define _DIST_MIN    100.0   
#define _DIST_MAX    250.0   

// Filter & loop
#define EMA_ALPHA   0.30     
#define LOOP_INTERVAL 30     

Servo myservo;
unsigned long last_loop_time;   

float dist_prev = _DIST_MIN;
float dist_ema  = _DIST_MIN;

void setup()
{
  pinMode(PIN_LED, OUTPUT);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(1000000);        
  last_loop_time = millis();    
}

void loop()
{
  unsigned long time_curr = millis();
  if (time_curr < (last_loop_time + LOOP_INTERVAL)) return;
  last_loop_time += LOOP_INTERVAL;

  int duty;
  float a_value, dist_raw, dist_filtered;

  // 1) ADC read (분모 보호)
  a_value = analogRead(PIN_IR);
  if (a_value <= 10) a_value = 10;   

  // 2) 거리 근사식 (mm)
  dist_raw = ((6762.0f / (a_value - 9.0f)) - 4.0f) * 10.0f;

  // 3) Range filter (+ 0~6cm 무시) & LED 표시
  bool in_range = (dist_raw >= _DIST_INVALID) && (dist_raw >= _DIST_MIN) && (dist_raw <= _DIST_MAX);
  if (in_range) {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
    digitalWrite(PIN_LED, LOW);   
  } else {
    dist_filtered = dist_prev;     
    digitalWrite(PIN_LED, HIGH);    
  }

  // 4) EMA filter
  dist_ema = EMA_ALPHA * dist_filtered + (1.0f - EMA_ALPHA) * dist_ema;

  // 5) map() 없이 선형 변환 + 클램프
  float duty_f = (dist_ema - _DIST_MIN) * (_DUTY_MAX - _DUTY_MIN)
                 / (_DIST_MAX - _DIST_MIN) + _DUTY_MIN;
  if (duty_f < _DUTY_MIN) duty_f = _DUTY_MIN;
  if (duty_f > _DUTY_MAX) duty_f = _DUTY_MAX;
  duty = (int)(duty_f + 0.5f);   

  // 6) Servo write
  myservo.writeMicroseconds(duty);

  // 7) Debug
  Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
  Serial.print(",_DIST_MIN:"); Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");     Serial.print(duty);
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println();
}
