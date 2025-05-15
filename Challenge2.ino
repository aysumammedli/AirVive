ch 2

#include <SimpleFOC.h>

BLDCMotor motor = BLDCMotor(7, 0.24, 360, 0.000133);
BLDCDriver3PWM driver = BLDCDriver3PWM(11, 10, 9, 6, 5, 3);

float target_position = 0;
float position_step = 0.3;
float voltage_step = 0.5;
float simulated_response = 0;

unsigned long last_plot_time = 0;
unsigned long plot_interval = 100; 

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

  Serial.println("Motor ready! Use w/s for angle, +/- for voltage.");
  delay(1000);
}

void loop() {
  motor.move(target_position);

  simulated_response = 0.9 * simulated_response + 0.1 * (target_position * motor.voltage_limit);

  if (millis() - last_plot_time > plot_interval) {
    Serial.print("Target:");
    Serial.print(target_position);
    Serial.print("\tVoltage:");
    Serial.print(motor.voltage_limit);
    Serial.print("\tResponse:");
    Serial.println(simulated_response);
    last_plot_time = millis();
  }

  if (Serial.available()) {
    char key = Serial.read();
    switch (key) {
      case 'w':
        target_position += position_step;
        break;
      case 's':
        target_position -= position_step;
        break;
      case '+':
        motor.voltage_limit += voltage_step;
        break;
      case '-':
        motor.voltage_limit -= voltage_step;
        if (motor.voltage_limit < 0.5) motor.voltage_limit = 0.5;
        break;
    }
  }
}