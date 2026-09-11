/*
 * Line-Following + Obstacle-Avoidance Robot
 * ------------------------------------------
 * Two IR sensors underneath track a black line on a light floor.
 * An HC-SR04 on the front watches for obstacles. When something gets
 * in the way, the robot leaves the line, drives around it, and hunts
 * until it finds the line again.
 *
 * Board:  Arduino Uno
 * Driver: L298N dual H-bridge
 *
 * Jashan Multani
 */

// ---------------------------------------------------------------- pins
const uint8_t IR_LEFT   = 2;   // LOW  = over the line
const uint8_t IR_RIGHT  = 3;
const uint8_t TRIG_PIN  = 9;
const uint8_t ECHO_PIN  = 10;

const uint8_t ENA = 5;         // left motor speed  (PWM)
const uint8_t IN1 = 6;
const uint8_t IN2 = 7;
const uint8_t ENB = 11;        // right motor speed (PWM)
const uint8_t IN3 = 8;
const uint8_t IN4 = 12;

// ------------------------------------------------------------- tuning
// These are the values that took the most iterations to land on.
// Every chassis is different — expect to retune SPEED and the turn
// durations if your motors or wheels aren't the same as mine.
const uint8_t SPEED_CRUISE   = 150;  // straight-line speed (0-255)
const uint8_t SPEED_CORRECT  =  95;  // inside wheel while correcting
const uint8_t SPEED_PIVOT    = 140;  // both wheels during a pivot

const int  OBSTACLE_CM       = 18;   // trigger distance for avoidance
const int  SENSOR_MAX_CM     = 200;  // ignore readings beyond this

const int  BACKUP_MS         = 320;  // reverse away from the obstacle
const int  PIVOT_MS          = 400;  // ~90 degrees on my chassis
const int  BYPASS_MS         = 700;  // drive alongside the obstacle
const int  SEARCH_TIMEOUT_MS = 2500; // give up hunting and stop

// -------------------------------------------------------------- setup
void setup() {
  pinMode(IR_LEFT,  INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  Serial.begin(9600);
  halt();
  delay(1000);               // breathing room before it bolts off the table
}

// --------------------------------------------------------- motor prims
void drive(int leftSpeed, int rightSpeed) {
  // negative speed = that side runs backwards
  digitalWrite(IN1, leftSpeed  >= 0); digitalWrite(IN2, leftSpeed  <  0);
  digitalWrite(IN3, rightSpeed >= 0); digitalWrite(IN4, rightSpeed <  0);
  analogWrite(ENA, constrain(abs(leftSpeed),  0, 255));
  analogWrite(ENB, constrain(abs(rightSpeed), 0, 255));
}

void halt() { drive(0, 0); }

// ------------------------------------------------------------ sensing
bool onLine(uint8_t pin) { return digitalRead(pin) == LOW; }

int distanceCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 25 ms ceiling keeps a missing echo from stalling the control loop
  unsigned long echo = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (echo == 0) return SENSOR_MAX_CM;      // nothing came back
  return (int)(echo * 0.0343 / 2.0);
}

// -------------------------------------------------------- line follow
void followLine() {
  bool left  = onLine(IR_LEFT);
  bool right = onLine(IR_RIGHT);

  if (left && right) {
    drive(SPEED_CRUISE, SPEED_CRUISE);           // dead centre
  } else if (left && !right) {
    drive(SPEED_CORRECT, SPEED_CRUISE);          // drifting right, ease left
  } else if (!left && right) {
    drive(SPEED_CRUISE, SPEED_CORRECT);          // drifting left, ease right
  } else {
    // Both sensors off the line. Creep forward rather than stopping —
    // most of the time this is a gap in the tape, not a lost line.
    drive(SPEED_CORRECT, SPEED_CORRECT);
  }
}

// ---------------------------------------------------- obstacle bypass
void pivotLeft()  { drive(-SPEED_PIVOT,  SPEED_PIVOT); delay(PIVOT_MS); }
void pivotRight() { drive( SPEED_PIVOT, -SPEED_PIVOT); delay(PIVOT_MS); }

void avoidObstacle() {
  halt();                        delay(150);
  drive(-SPEED_CRUISE, -SPEED_CRUISE); delay(BACKUP_MS);
  halt();                        delay(100);

  pivotRight();                                  // turn off the line
  drive(SPEED_CRUISE, SPEED_CRUISE); delay(BYPASS_MS);   // go alongside it
  pivotLeft();                                   // aim back across
  drive(SPEED_CRUISE, SPEED_CRUISE); delay(BYPASS_MS);
  pivotLeft();                                   // point at the line again

  findLine();
}

// Crawl forward until a sensor sees the line, or give up so the robot
// doesn't wander off into the room forever.
void findLine() {
  unsigned long start = millis();
  while (millis() - start < SEARCH_TIMEOUT_MS) {
    if (onLine(IR_LEFT) || onLine(IR_RIGHT)) return;
    drive(SPEED_CORRECT, SPEED_CORRECT);
    delay(20);
  }
  halt();
  Serial.println(F("lost the line — stopping"));
}

// --------------------------------------------------------------- loop
void loop() {
  if (distanceCm() <= OBSTACLE_CM) {
    avoidObstacle();
  } else {
    followLine();
  }
  delay(15);   // small settle so the IR reads aren't noisy
}
