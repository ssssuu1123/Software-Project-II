// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25      // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // coefficent to convert duration to distance

unsigned long last_sampling_time;   // unit: msec

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar 
  
  // initialize serial port
  Serial.begin(57600);
}

void loop() { 
  float distance;

  // wait until next sampling time. // polling
  // millis() returns the number of milliseconds since the program started.
  //    will overflow after 50 days.
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance

  // --- 밝기 계산 & 출력 (PWM) ---
  // 의미상의 밝기: 0(최소) ~ 255(최대). 200mm에서 255, 100/300mm에서 0
  if (distance <= 0.0 || distance < _DIST_MIN || distance > _DIST_MAX) {
    // 범위 밖/타임아웃 → 소등 (active-low: 255 = 꺼짐)
    analogWrite(PIN_LED, 255);
  } else {
    // 중앙(200mm)에서 최대, 가장자리(100/300mm)에서 최소가 되도록 선형 보간
    float diff = (distance > 200.0f) ? (distance - 200.0f) : (200.0f - distance); // |distance-200|
    if (diff > 100.0f) diff = 100.0f;                     // 안전 클램프 (0~100)
    float norm = 1.0f - (diff / 100.0f);                  // 200mm→1.0, 가장자리→0.0
    int desired = (int)(norm * 255.0f + 0.5f);            // 0~255 (200mm=255)

    // active-low: PWM을 반전해서 출력 (0=가장 밝음, 255=꺼짐)
    analogWrite(PIN_LED, 255 - desired);
  }


  // output the distance to the serial port
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.println("");
  
  // do something here
  delay(50); // Assume that it takes 50ms to do something.
  
  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm

  // Pulse duration to distance conversion example (target distance = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout) returns microseconds (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 하려면 * 1,000이 필요 ==>  SCALE = 0.001 * 0.5 * SND_VEL
  //
  // - 예, pusseIn()이 100,000 이면 (= 0.1초, 왕복 거리 34.6m)
  //        = 100,000 micro*sec * 0.001 milli/micro * 0.5 * 346 meter/sec
  //        = 100,000 * 0.001 * 0.5 * 346
  //        = 17,300 mm  ==> 17.3m
}
