#include <Wire.h>
#include <Servo.h>
#define SLAVE_ADDRESS 0x08

#define TRAV_PIN 9
#define LV8_PIN 10
#define LOAD_PIN 8
#define MOT_A1_PIN 4
#define MOT_A2_PIN 5
#define MOT_B1_PIN 6
#define MOT_B2_PIN 7

// Definitions Arduino pins connected to input H Bridge
int IN1 = 4; //in 1 & 2 control out 1 & 2 (motor on same side of in pins)
int IN2 = 5;
int IN3 = 6; //ditto for in 3 & 4 and out 3 & 4
int IN4 = 7;
int ls_x_last = 0;
int ls_y_last = 0;
int rs_x_last = 0;
int rs_y_last = 0;
int lt_last = 0;
int rt_last = 0;
int traverseSpeed = 90;
int elevateSpeed = 90;

byte data_to_echo = 0;
int state = 0;

Servo lv8;
Servo traverse;
Servo autoloader;

void setup() 
{
  lv8.attach(LV8_PIN);
  traverse.attach(TRAV_PIN);
  autoloader.attach(LOAD_PIN);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);


}

void loop() {
  int rightServoSpeed = (ls_y_last * 4) - 255;
  int leftServoSpeed = (ls_y_last * 4) - 255;
  bool loadOn = (rt_last > 40);
  rightServoSpeed = rightServoSpeed + 1.2 * (ls_x_last - 64);
  leftServoSpeed = leftServoSpeed - 1.2 * (ls_x_last - 64);
  //leftServoSpeed = 0;
  //rightServoSpeed = 0;
  leftServoSpeed = -1 * leftServoSpeed;
  spin_and_wait(rightServoSpeed, leftServoSpeed, 10); // sets speed for 0.01 sec
  if (rs_x_last > 88) {
    traverseSpeed--;
    if (traverseSpeed >= 180) {
      traverseSpeed = 180;
    }
  } else if (rs_x_last < 40) {
    traverseSpeed++;
    if (traverseSpeed <= 0) {
      traverseSpeed = 0;
    }
  }
  if (rs_y_last > 88) {
    elevateSpeed++;
    if (elevateSpeed >= 130) {
      elevateSpeed = 130;
    }
  } else if (rs_y_last < 40) {
    elevateSpeed--;
    if (elevateSpeed <= 55) {
      elevateSpeed = 55;
    }
  }
  if (loadOn) {
    autoloader.write(180);
  } else {
    autoloader.write(82);
  }
  lv8.write(elevateSpeed);
  traverse.write(traverseSpeed);
}

void spin_and_wait(int pwm_A, int pwm_B, int duration)
{
  set_motor_currents(pwm_A, pwm_B);
  delay(duration);
}

void set_motor_currents(int pwm_A, int pwm_B)
{
  set_motor_pwm(pwm_A, MOT_A1_PIN, MOT_A2_PIN);
  set_motor_pwm(pwm_B, MOT_B1_PIN, MOT_B2_PIN);

  // Print a status message to the console.
  Serial.print("Set motor A PWM = ");
  Serial.print(pwm_A);
  Serial.print(" motor B PWM = ");
  Serial.println(pwm_B);
}

void set_motor_pwm(int pwm, int IN1_PIN, int IN2_PIN)
{
  if (pwm < 0) {  // reverse speeds
    analogWrite(IN1_PIN, -pwm);
    digitalWrite(IN2_PIN, LOW);

  } else { // stop or forward
    digitalWrite(IN1_PIN, LOW);
    analogWrite(IN2_PIN, pwm);
  }
}


void receiveData(int bytecount)
{
  for (int i = 0; i < bytecount; i++) {
    data_to_echo = Wire.read();
  }
  if (state == 1) {
    ls_x_last = data_to_echo;
    state++;
  } else if (state == 2) {
    ls_y_last = data_to_echo;
    state++;
  } else if (state == 3) {
    rs_x_last = data_to_echo;
    state++;
  } else if (state == 4) {
    rs_y_last = data_to_echo;
    state++;
  } else if (state == 5) {
    rt_last = data_to_echo;
    state = 0;
  } else if ((state == 0) && (data_to_echo == 255)) {
    state++;
  }
  Serial.println(state);
  Serial.print(" ");
  Serial.print(data_to_echo);
}

void sendData()
{
  Wire.write(data_to_echo);
}
