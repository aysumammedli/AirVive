#include <SimpleFOC.h>

BLDCMotor motor = BLDCMotor(7, 0.24, 360, 0.000133);
BLDCDriver3PWM driver = BLDCDriver3PWM(11, 10, 9, 6, 5, 3);

float target_position = 0;
float position_step = 0.3;      
float voltage_step = 0.5;

float voltage_presets[] = {2.0, 2.5, 3.0, 3.5};
float velocity_presets[] = {2.0, 3.0, 4.0, 5.0};
bool auto_tune_done = false;

void autoTuneMotor() {
  Serial.println("Starting auto-tuning...");
  for (int i = 0; i < sizeof(voltage_presets) / sizeof(float); i++) {
    for (int j = 0; j < sizeof(velocity_presets) / sizeof(float); j++) {
      motor.voltage_limit = voltage_presets[i];
      motor.velocity_limit = velocity_presets[j];

      Serial.print("Voltage = ");
      Serial.print(motor.voltage_limit);
      Serial.print(" V | Velocity = ");
      Serial.print(motor.velocity_limit);
      Serial.println(" rad/s");

      motor.move(1.0);
      delay(1000);
      motor.move(-1.0);
      delay(1000);
    }
  }
  Serial.println("Auto-tuning complete.");
  auto_tune_done = true;
}

void setup() {
  Serial.begin(115200);
  SimpleFOCDebug::enable(&Serial);

  
  driver.voltage_power_supply = 12;
  driver.voltage_limit = 6;
  driver.init();
  motor.linkDriver(&driver);

  motor.voltage_limit = 3.0;  
  motor.velocity_limit = 5.0;
  motor.controller = MotionControlType::angle_openloop;
  motor.init();

  Serial.println("Motor ready!");
  Serial.println("Use keys:");
  Serial.println("  w - move forward");
  Serial.println("  s - move backward");
  Serial.println("  + - increase voltage");
  Serial.println("  - - decrease voltage");
  Serial.println("  q - auto-tune");
}

void loop() {

  motor.move(target_position);

  if (Serial.available()) {
    char key = Serial.read();

    switch (key) {
      case 'w':
        target_position += position_step;
        Serial.print("Target angle: ");
        Serial.println(target_position);
        break;

      case 's':
        target_position -= position_step;
        Serial.print("Target angle: ");
        Serial.println(target_position);
        break;

      case '+':
        motor.voltage_limit += voltage_step;
        Serial.print("Voltage limit increased to: ");
        Serial.println(motor.voltage_limit);
        break;

      case '-':
        motor.voltage_limit -= voltage_step;
        if (motor.voltage_limit < 0.5) motor.voltage_limit = 0.5;
        Serial.print("Voltage limit decreased to: ");
        Serial.println(motor.voltage_limit);
        break;

      case 'q':
        if (!auto_tune_done) {
          autoTuneMotor();
        } else {
          Serial.println("Auto-tuning already done.");
        }
        break;

      default:
        Serial.print("Unknown key: ");
        Serial.println(key);
        break;
    }
  }
}