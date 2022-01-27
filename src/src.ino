#include <IRremote.h>
#include <Servo.h>

// IR
int RECV_PIN = 6, IR_BUTTON_CODE = 0xFFFFA25D;
IRrecv irrecv(RECV_PIN);
decode_results results;

// Servo
Servo servo;
int SERVO_PIN = 5, GATE_CLOSE_POSITION = 90, GATE_OPEN_POSITION = 180, is_gate_open = false, time_since_opening = 0;
unsigned long last_time_open = 0;

// Ultrasonic sensor
int ECHO_PIN = 2, TRIG_PIN = 3, distance;
long duration;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  irrecv.enableIRIn(); // Start the receiver
  servo.attach(SERVO_PIN); // Attach the Servo object to the pin
}

void loop() {
  // Measure the distance using the Ultrasonic Sensor
  initSensor();
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = usToCm(duration);
  
  // Get the input from the IR remote
  if (irrecv.decode(&results)) {
    // Get the value representing the button that was pressed
    int button_pressed = results.value;
    // Uncomment to get value of your remote's button
    // After that, update the IR_BUTTON_CODE constant in line 5
    // Serial.println(results.value, HEX);

    // Check if the pressed button matches the required button
    if (button_pressed == IR_BUTTON_CODE) {
      // If the gate is open, try closing it
      if (is_gate_open == true) { tryClosingGate(); }
      
      // If the gate is closed, open it
      else { openGate(); }
      }
    }
    
  // Resume the IR Receiver
  irrecv.resume();
  
  // If gate is open, update the time since its last opening
  if (is_gate_open) {
    time_since_opening = (millis() - last_time_open) / 1000;
  }

  // If the time since gate's last opening exceeds the provided threshold, try closing the gate
  if (time_since_opening >= 5) {
    tryClosingGate();
  }

  // Uncomment these lines for debugging
  // delay(100);
  // Serial.println("time_since_opening = " + String(time_since_opening));
  // Serial.println("is_gate_open = " + String(is_gate_open));
  // Serial.println("last_time_open = " + String(last_time_open / 1000));
}

// Opens the gate and updates all the necessary variables
void openGate() {
  servo.write(GATE_OPEN_POSITION);
  is_gate_open = true;
  last_time_open = millis();
}

// Closes the gate and updates all the necessary variables
void closeGate() {
  servo.write(GATE_CLOSE_POSITION);
  is_gate_open = false;
  time_since_opening = 0;
}

// Returns false if something is blocking the gate, based on the provided distance threshold
// Else return true
bool isGateFreeToGo() {
  if (distance < 10) {
    return false;
  }
  else {
    return true;
  }
}

// Tries closing the gate
// If fails, calls openGate()
// If succeeds, calls closeGate()
void tryClosingGate() {
  if (isGateFreeToGo() ) {
    servo.write(GATE_CLOSE_POSITION);
    is_gate_open = false;
    time_since_opening = 0;
  }
  else {
    Serial.println("Object blocking the gate!");
    openGate();
  }
}

// Converts Ultrasonic Sensor output to distance (in centimeters)
// Ultrasonic Sensor provides the number of microseconds that have passed since the signal left the sensor, bounced from the object and got back into the sensor
// Based on this we can calculate the distance traveled, and divide it by two to get the distance to the nearest object 
int usToCm(long microseconds) {
  int centimeters = microseconds * 0.034 / 2;
  return centimeters;
}

// Perform the necessary actions for the Ultrasonic Sensor to work
// Each time we want to measure the distance, we need to:
// 1. Clear the Trigger Pin
// 2. Wait a few ms
// 3. Generate the ultrasound by setting the Trigger Pin to HIGH
// 4. Wait around 10ms
// 5. Clear the Trigger Pin
// After that we can get the number of microseconds from Echo Pin
void initSensor() {
  // Clear the TRIG_PIN
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Generate the ultrasound
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
}
