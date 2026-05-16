#include <Servo.h>
#include "HX711.h"
#define DOUT_PIN      3
#define CLK_PIN       2
#define S0_PIN        4
#define S1_PIN        5
#define S2_PIN        6
#define S3_PIN        7
#define COLOR_OUT_PIN 8
#define IR_PIN        9
#define ENA_PIN       10
#define IN1_PIN       12
#define IN2_PIN       11
#define SERVO_A_PIN   A0
#define SERVO_B_PIN   A1
const float FACTOR       = 1837.5;
const float WEIGHT_HIGH  = 36;
const float NOISE_FLOOR  = 3.0;
const int   MOTOR_SPEED  = 255;
const long  IR_TIMEOUT   = 45000;
const int  COLOR_GAP     = 20;
const long PULSE_TIMEOUT = 200000;
HX711 scale;
Servo servoA;
Servo servoB;
float finalWeight   = 0.0;
byte  detectedColor = 0;
void sendWaiting() {
  Serial.println(F("STATUS:WAIT"));
}
void sendReady() {
  Serial.println(F("STATUS:READY"));
}
void sendToDisplay(float weight, byte color) {
  String wLabel = (weight >= WEIGHT_HIGH) ? "HIGH" : "LOW";
  String cLabel;
  if      (color == 1) cLabel = "Red";
  else if (color == 3) cLabel = "Blue";
  else                 cLabel = "Unknown";
  Serial.print(F("W:"));
  Serial.print(wLabel);
  Serial.print(F(",C:"));
  Serial.println(cLabel);
}
void setup() {
  Serial.begin(9600);
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  analogWrite(ENA_PIN, 0);
  pinMode(IR_PIN, INPUT);
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(COLOR_OUT_PIN, INPUT);
  digitalWrite(S0_PIN, HIGH);
  digitalWrite(S1_PIN, LOW);
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
  servoA.attach(SERVO_A_PIN);
  servoB.attach(SERVO_B_PIN);
  servoA.write(0);
  servoB.write(0);
  delay(800);
  servoA.detach();
  servoB.detach();
  scale.begin(DOUT_PIN, CLK_PIN);
  delay(500);
  scale.set_scale(FACTOR);
  scale.power_up();
  scale.tare();
  delay(300);
}
void startMotor() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  analogWrite(ENA_PIN, MOTOR_SPEED);
}
void stopMotor() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, HIGH);
  analogWrite(ENA_PIN, 0);
  delay(150);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  delay(2000);
}
byte detectColor() {
  long redPulse;
  long greenPulse;
  long bluePulse;
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
  delay(50);
  redPulse = pulseIn(COLOR_OUT_PIN, LOW, PULSE_TIMEOUT);
  digitalWrite(S2_PIN, HIGH);
  digitalWrite(S3_PIN, HIGH);
  delay(50);
  greenPulse = pulseIn(COLOR_OUT_PIN, LOW, PULSE_TIMEOUT);
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, HIGH);
  delay(50);
  bluePulse = pulseIn(COLOR_OUT_PIN, LOW, PULSE_TIMEOUT);
  if (redPulse == 0 && greenPulse == 0 && bluePulse == 0) {
    return 0;
  }
  if (redPulse == 0)   redPulse   = 999999L;
  if (greenPulse == 0) greenPulse = 999999L;
  if (bluePulse == 0)  bluePulse  = 999999L;
  if (redPulse < greenPulse && redPulse < bluePulse) {
    if ((min(greenPulse, bluePulse) - redPulse) > COLOR_GAP) {
      return 1;
    }
  }
  if (greenPulse < redPulse && greenPulse < bluePulse) {
    if ((min(redPulse, bluePulse) - greenPulse) > COLOR_GAP) {
      return 2;
    }
  }
  if (bluePulse < redPulse && bluePulse < greenPulse) {
    if ((min(redPulse, greenPulse) - bluePulse) > COLOR_GAP) {
      return 3;
    }
  }
  return 0;
}
float readWeight() {
  delay(400);
  scale.power_up();
  delay(800);
  float avg = 0;
  for (int i = 0; i < 10; i++) {
    avg += scale.get_units(5);
    delay(50);
  }
  float reading = avg / 10.0;
  if (reading < 0) {
    reading = 0;
  }
  if (reading < NOISE_FLOOR) {
    reading = 0;
  }
  Serial.println(reading);
  return reading;
}
void moveServos(int a, int b) {
  servoA.attach(SERVO_A_PIN);
  servoB.attach(SERVO_B_PIN);
  servoA.write(a);
  servoB.write(b);
  delay(1200);
}
void sweepServoA(int fromAngle, int toAngle, int stepDelay) {
  servoA.attach(SERVO_A_PIN);
  int step = (toAngle > fromAngle) ? 1 : -1;
  for (int pos = fromAngle; pos != toAngle; pos += step) {
    servoA.write(pos);
    delay(stepDelay);
  }
  servoA.write(toAngle);
}
void sweepServoB(int fromAngle, int toAngle, int stepDelay) {
  servoB.attach(SERVO_B_PIN);
  int step = (toAngle > fromAngle) ? 1 : -1;
  for (int pos = fromAngle; pos != toAngle; pos += step) {
    servoB.write(pos);
    delay(stepDelay);
  }
  servoB.write(toAngle);
}
void resetServos() {
  servoA.write(0);
  servoB.write(0);
  delay(600);
  servoA.detach();
  servoB.detach();
}
bool sortObject(byte color, float weight) {
  bool high = (weight >= WEIGHT_HIGH);
  if (!high) {
    if (color == 1) {
      startMotor();
      servoA.attach(SERVO_A_PIN);
      servoA.write(90);
      delay(400);
      sweepServoA(140, 0, 25);
      delay(400);
      return true;
    }
    else if (color == 3) {
      moveServos(140, 0);
      startMotor();
    }
  }
  else {
    if (color == 1) {

      moveServos(0, 42);
    }
    else if (color == 3) {
    }
  }
  return false;
}
void loop() {
  sendReady();
  startMotor();
  unsigned long t = millis();
  while (digitalRead(IR_PIN) == HIGH) {
    delay(750);

    if (millis() - t > IR_TIMEOUT) {

      stopMotor();

      return;
    }
  }
  stopMotor();
  delay(3000);
  sendWaiting();
  detectedColor = detectColor();
  finalWeight = readWeight();
  sendToDisplay(finalWeight, detectedColor);
  bool beltAlreadyOn = sortObject(detectedColor, finalWeight);
  if (!beltAlreadyOn) {

    startMotor();
  }
  delay(5000);
  resetServos();
  delay(800);
  stopMotor();
  delay(300);
  scale.power_up();
  delay(500);
  scale.tare();
  delay(1000);
}