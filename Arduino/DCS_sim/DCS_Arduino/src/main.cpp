#include <Arduino.h>
#include <Servo.h>
#include <defines.h>

float filtered_wanted;
float led_phase;
bool encoer_fault = 0;
bool LeftLL = 0;
bool LeftUL = 0;
bool RightLL = 0;
bool RightUL = 0;
bool man_right = 0;
bool man_left = 0;
bool auto_mode = 0;
bool man_mode = 0;
bool mid_switch = 0;
bool air_force_on = 0;
bool air_off = 0;
bool left__PB = 0;
bool enable_auto_motion = 0;
bool home_in_progress = 0;

int man_speed = 0;
int air_speed = 0;
int motion_amplitude_scale = 0;
int left__pos_A = 0;
int right_pos_A = 0;
int man_pos = 0;
int air_PWM = 0;
int in_home_counter = 0;
int left__percent_power = 0;
int right_percent_power = 0;
int left__pos_W = 0;
int right_pos_W = 0;
int air_speed_W = 0;
int prev_left__pos_W = 0;
int prev_right_pos_W = 0;
int no_change_counter = 0;

int BufferEnd[2] = {-1};           // Rx Buffer end index for each of the two comm ports
unsigned int RxByte[2] = {0};      // Current byte received from each of the two comm ports
unsigned int RxBuffer[5][2] = {0}; // 5 byte Rx Command Buffer for each of the two comm ports
unsigned int CommsTimeout = 0;     // used to reduce motor power if there has been no comms for a while
byte errorcount = 0;               // serial receive error detected by invalid packet start/end bytes

unsigned long last_sent_tele = 0;
unsigned long last_run = 0;
unsigned long time_turn_on = 0;
unsigned long time_turn_off = 0;
unsigned long time_started_homing = 0;
unsigned long millis_on = 200;
unsigned long millis_off = 800;

bool led_on = 0;
byte grn_LED_pwr = 0;
byte blu_LED_pwr = 0;
byte red_LED_pwr = 0;

Servo left__motor;
Servo right_motor;
Servo air_motor;

int limit(int val, int limits)
{
  if (val > limits)
    val = limits;
  if (val < -limits)
    val = -limits;
  return val;
}

int range(int val, int lower, int upper)
{
  if (val > upper)
    val = upper;
  if (val < lower)
    val = lower;
  return val;
}

int sign(int val)
{
  if (val > 0)
    return 1;
  if (val < 0)
    return -1;
  return 0;
}

int dead_band(int val, int db)
{
  if (val > db)
    val -= db;
  else if (val < -db)
    val += db;
  else
    val = 0;
  return val;
}

void wait_for_loop_t()
{
  while ((millis() - last_run) < 1)
    ;
  last_run = millis();
  return;
}

void reset_wanted()
{
  left__pos_W = 0;
  right_pos_W = 0;
  air_speed_W = 0;
}

void ParseCommand(int ComPort)
{
  CommsTimeout = 0; // reset the comms timeout counter to indicate we are getting packets
  switch (RxBuffer[0][ComPort])
  {
  case 'A':
    left__pos_W = int(RxBuffer[1][ComPort] * 256 + RxBuffer[2][ComPort]); // range 0...1023
    left__pos_W = (left__pos_W - 512) * 4 / 5;                            //-400...400
    left__pos_W = range((left__pos_W * motion_amplitude_scale / 20) + pos_ofset, left__min_pos, left__max_pos);
    break;
  case 'B':
    right_pos_W = int(RxBuffer[1][ComPort] * 256 + RxBuffer[2][ComPort]);
    right_pos_W = (right_pos_W - 512) * 4 / 5;
    right_pos_W = range((right_pos_W * motion_amplitude_scale / 20) + pos_ofset, right_min_pos, right_max_pos);
    break;
  case 'C':
    air_speed_W = int(RxBuffer[1][ComPort] * 256 + RxBuffer[2][ComPort]);
    air_speed_W = (air_speed_W - 512) / 5;
    if (air_speed_W > 0)
      air_speed_W += 10;                                          // 0....100
    air_speed_W = range((air_speed_W * air_speed / 100), 0, 120); //
    break;
  case 'S':
    enable_auto_motion = 1;
    home_in_progress = 0;
    reset_wanted();
    break;
  case 'E':
    enable_auto_motion = 0;
    home_in_progress = 1;
    reset_wanted();
    break;
  }
}

void read_data_from_serial()
{
  while (Serial.available())
  {
    if (BufferEnd[COM0] == -1)
    {
      RxByte[COM0] = Serial.read();
      if (RxByte[COM0] != START_BYTE)
      {
        BufferEnd[COM0] = -1;
        errorcount++;
      }
      else
      {
        BufferEnd[COM0] = 0;
      }
    }
    else
    {
      RxByte[COM0] = Serial.read();
      RxBuffer[BufferEnd[COM0]][COM0] = RxByte[COM0];
      BufferEnd[COM0]++;
      if (BufferEnd[COM0] > 3)
      {
        if (RxBuffer[3][COM0] == END_BYTE)
        {
          ParseCommand(COM0);
        }
        else
        {
          errorcount++;
        }
        BufferEnd[COM0] = -1;
      }
    }
  }
}

void send_tele()
{
  if (millis() - last_sent_tele > 50)
  {
    Serial.print(" W: ");
    Serial.print(man_pos);
    Serial.print(" LP: ");
    Serial.print(left__pos_A);
    Serial.print(" RP: ");
    Serial.print(right_pos_A);
    Serial.print(" L%: ");
    Serial.print(left__percent_power);
    Serial.print(" R%: ");
    Serial.print(right_percent_power);
    Serial.print(" mn: ");
    Serial.print(man_pos);
    // Serial.print(" lft LL UL, rgt LL UL: ");
    // Serial.print(LeftLL);
    // Serial.print(LeftUL);
    // Serial.print(RightLL);
    // Serial.print(RightUL);
    // Serial.print(" AirP: ");
    // Serial.print(air_PWM);
    // Serial.print(" spd: ");
    // Serial.print(man_speed);
    // Serial.print(" air: ");
    // Serial.print(air_speed);
    // Serial.print(" motion_amplitude_scale: ");
    // Serial.print(motion_amplitude_scale);
    // Serial.print(" lft, rgt: ");
    // Serial.print(man_left);
    // Serial.print(man_right);
    Serial.println(" ");
    last_sent_tele = millis();
  }
  return;
}

void read_Arduino_IO()
{
  // digitals
  LeftLL = 1 - digitalRead(LeftLL_pin);
  LeftUL = 1 - digitalRead(LeftUL_pin);
  RightLL = 1 - digitalRead(RightLL_pin);
  RightUL = 1 - digitalRead(RightUL_pin);

  auto_mode = 1 - digitalRead(auto_pin);
  man_mode = 1 - digitalRead(man_pin);
  air_force_on = 1 - digitalRead(air_force_on_pin);
  air_off = 1 - digitalRead(air_off_pin);
  mid_switch = 1 - digitalRead(man_mode_pin);

  // analogs
  left__pos_A = analogRead(left__pos_pin);
  right_pos_A = 1023 - analogRead(right_pos_pin);
  if ((left__pos_A < 20 || left__pos_A > 1000 || right_pos_A < 20 || right_pos_A > 1000) && auto_mode)
  {
    encoer_fault = 1;
  }
  left__pos_A -= left__pos_0;
  right_pos_A -= right_pos_0;

  air_speed = range((analogRead(air_speed_pin) - 23) / 10, 0, 100);  // 0 ... 100
  motion_amplitude_scale = range(analogRead(scale_pin) / 50, 0, 20); // 0 ... 20

  // manual mode inputs
  if (!auto_mode)
  {
    left__PB = 1 - digitalRead(left__PB_pin);
    man_left = 1 - digitalRead(man_left__pin);
    man_right = 1 - digitalRead(man_right_pin);
    man_pos = limit((analogRead(man_speed_pin) - 465), 460); // -400 ... 400
    filtered_wanted = 0.99 * filtered_wanted + 0.01 * man_pos;
    man_pos = int(filtered_wanted);
    man_speed = man_pos / 4; // -100 ... 100
  }
  return;
}

bool data_is_changing()
{
  if (left__pos_W == prev_left__pos_W && right_pos_W == prev_right_pos_W)
    no_change_counter++;
  else
    no_change_counter = 0;
  prev_left__pos_W = left__pos_W;
  prev_right_pos_W = right_pos_W;
  if (no_change_counter > 500)
  {
    no_change_counter = 501;
    return (0);
  }
  else
    return (1);
}

void operate_motors(int left__percent, int right_percent)
{
  int left__PWM, right_PWM;
  right_percent = limit(right_percent, max_pwr);
  left__percent = limit(left__percent, max_pwr);

  if (RightUL && right_percent > 0)
    right_percent = 0;
  if (RightLL && right_percent < 0)
    right_percent = 0;
  if (LeftUL && left__percent > 0)
    left__percent = 0;
  if (LeftLL && left__percent < 0)
    left__percent = 0;

  left__PWM = (left__percent * PWM_range_per_side) / 100 + PWM_zero;
  right_PWM = (right_percent * PWM_range_per_side) / 100 + PWM_zero;

  left__motor.write(left__PWM);
  right_motor.write(right_PWM);
}

void calc_motors_pwr_to_pos(int left__W, int right_W)
{
  int left__err = range(left__W, left__min_pos, left__max_pos) - left__pos_A;
  int right_err = range(right_W, right_min_pos, right_max_pos) - right_pos_A;
  if (abs(left__err) > DB)
  {
    left__percent_power = left__err * KP / 10;
    left__percent_power = limit(left__percent_power + KS_left * sign(left__percent_power), 100);
  }
  if (abs(right_err) > DB)
  {
    right_percent_power = right_err * KP / 10;
    right_percent_power = limit(right_percent_power + KS_right * sign(right_percent_power), 100);
  }
}

void homing()
{
  calc_motors_pwr_to_pos(0, 0);
  if (abs(left__pos_A) < 25 && abs(right_pos_A) < 25)
    in_home_counter++;
  else
    in_home_counter = 0;
  if (in_home_counter > 500 || millis() - time_started_homing > 3000)
  {
    home_in_progress = 0;
    enable_auto_motion = 0;
    left__percent_power = 0;
    right_percent_power = 0;
  }
}

void LED_set_timing(int time_on, int time_off)
{
  millis_on = time_on;
  millis_off = time_off;
}

void LED_set_color_rgb(byte r, byte g, byte b)
{
  red_LED_pwr = r;
  grn_LED_pwr = g;
  blu_LED_pwr = b;
}

void LED_set_color_hsv(int H, int S, int V) //  0..360, 0..100, 0..100
{
  float s = float(S) / 100;
  float v = float(V) / 200;
  float C = s * v;
  float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
  float m = v - C;
  float r, g, b;
  if (H >= 0 && H < 60)
  {
    r = C, g = X, b = 0;
  }
  else if (H >= 60 && H < 120)
  {
    r = X, g = C, b = 0;
  }
  else if (H >= 120 && H < 180)
  {
    r = 0, g = C, b = X;
  }
  else if (H >= 180 && H < 240)
  {
    r = 0, g = X, b = C;
  }
  else if (H >= 240 && H < 300)
  {
    r = X, g = 0, b = C;
  }
  else
  {
    r = C, g = 0, b = X;
  }

  grn_LED_pwr = 20 * (r + m);
  blu_LED_pwr = 20 * (g + m);
  red_LED_pwr = 20 * (b + m);
}

void set_LEDS_direct(bool r, bool g, bool b)
{
  digitalWrite(LED_red_pin, 1 - r);
  digitalWrite(LED_grn_pin, 1 - g);
  digitalWrite(LED_blu_pin, 1 - b);
}

void set_LEDs(bool on)
{
  static unsigned long last_led_cycle_start_time;

  byte time_in_cycle = (millis() - last_led_cycle_start_time);

  if (on)
  {
    digitalWrite(LED_red_pin, (time_in_cycle > red_LED_pwr));
    digitalWrite(LED_grn_pin, (time_in_cycle > grn_LED_pwr));
    digitalWrite(LED_blu_pin, (time_in_cycle > blu_LED_pwr));
  }
  else
  {
    digitalWrite(LED_red_pin, 1);
    digitalWrite(LED_grn_pin, 1);
    digitalWrite(LED_blu_pin, 1);
  }
  if (time_in_cycle > 20)
  {
    last_led_cycle_start_time = millis();
  }
}

void operate_LEDs()
{
  digitalWrite(LED_man_pin, man_mode);
  digitalWrite(LED_auto_pin, auto_mode);
  if (led_on == 1)
  {
    if (millis() - time_turn_on > millis_on)
    {
      time_turn_off = millis();
      led_on = 0;
    }
  }
  else
  {
    if (millis() - time_turn_off > millis_off)
    {
      time_turn_on = millis();
      led_on = 1;
    }
  }
  set_LEDs(led_on);
}

void operate_blower()
{
  if (!air_force_on && !air_off) // mid pos - auto air
  {
    if (enable_auto_motion)
      air_speed = max(0, air_speed_W);
    else
      air_speed = 0;
  }
  if (air_off || millis() < 15000)
    air_speed = 0;
  encoer_fault = 0;
  air_PWM = air_zero_pwr + air_speed;
  air_motor.write(air_PWM);
}

void operate_manual_mode()
{
  send_tele();
  LED_set_color_rgb(20, 0, 20);
  int speed = dead_band(man_speed, 15);
  if (mid_switch)
  {
    LED_set_timing(200, 300);
    if (man_left)
      calc_motors_pwr_to_pos(man_pos, man_pos);
    if (man_right)
      calc_motors_pwr_to_pos(man_pos, -man_pos);
  }
  else
  {
    LED_set_timing(200, 1300);
    if (man_left)
      left__percent_power = speed;
    if (man_right)
      right_percent_power = speed;
  }
  if (left__PB) // home
  {
    calc_motors_pwr_to_pos(0, 0);
  }
  enable_auto_motion = 0;
  encoer_fault = 0;
}

void operate_demo_mode()
{
  LED_set_color_rgb(20, 15, 0);
  int speed = dead_band(man_speed, 15);
  if (man_left || man_right)
  {
    LED_set_timing(200, 100);
    left__percent_power = speed;
    right_percent_power = speed;
  }
  else if (left__PB) // home
  {
    LED_set_timing(200, 100);
    calc_motors_pwr_to_pos(0, 0);
  }
  else
  {
    LED_set_timing(300, 700);
  }
  enable_auto_motion = 0;
}

void operate_auto_mode()
{
  bool data_is_changing_b = data_is_changing();

  read_data_from_serial(); // fills left__pos_W and right_pos_W
  if (encoer_fault)
  {
    left__percent_power = 0;
    right_percent_power = 0;
    LED_set_color_rgb(20, 0, 0);
    LED_set_timing(500, 500);
    return;
  }
  else if (!mid_switch) //  motion is off
  {
    left__percent_power = 0;
    right_percent_power = 0;
    LED_set_color_rgb(0, 20, 20);
    LED_set_timing(500, 500);
    air_speed /= 2;
  }
  else if (enable_auto_motion)
  {
    LED_set_color_rgb(0, 20 * data_is_changing_b, 20 - 20 * data_is_changing_b); // green while moving, blue when not
    LED_set_timing(50, 250);

    if (data_is_changing_b)
    {
      calc_motors_pwr_to_pos(left__pos_W, right_pos_W);
    }
    else
    {
      left__percent_power = 0;
      right_percent_power = 0;
    }

    if (home_in_progress)
      homing();
    else
      time_started_homing = millis();
    air_speed /= 2;
  }
  else
  {
    led_phase += 0.002;
    if (led_phase >= 6.283)
      led_phase = 0;
    LED_set_color_rgb(0, sin(led_phase) * 10.0 + 10.0, 0);
    LED_set_timing(5000, 0);
  }
}

void calc_motors_pwr()
{
  left__percent_power = 0;
  right_percent_power = 0;
  if (auto_mode)
  { // auto mode
    operate_auto_mode();
    return;
  }
  if (man_mode)
    operate_manual_mode();
  else
    operate_demo_mode();
}

void setup()
{
  Serial.begin(baud_rate);
  for (int i = 0; i < 38; i++)
    pinMode(i, INPUT_PULLUP);

  pinMode(LeftPWM_pin, OUTPUT);
  pinMode(RightPWM_pin, OUTPUT);
  pinMode(air_PWM_pin, OUTPUT);
  pinMode(LED_auto_pin, OUTPUT);
  pinMode(LED_man_pin, OUTPUT);
  pinMode(LED_red_pin, OUTPUT);
  pinMode(LED_blu_pin, OUTPUT);
  pinMode(LED_grn_pin, OUTPUT);

  digitalWrite(LeftPWM_pin, LOW);
  digitalWrite(RightPWM_pin, LOW);
  digitalWrite(air_PWM_pin, LOW);
  digitalWrite(LED_auto_pin, LOW);
  digitalWrite(LED_man_pin, LOW);

  digitalWrite(LED_grn_pin, HIGH);
  digitalWrite(LED_blu_pin, HIGH);
  digitalWrite(LED_red_pin, HIGH);

  left__motor.attach(LeftPWM_pin);
  right_motor.attach(RightPWM_pin);
  air_motor.attach(air_PWM_pin);
  air_motor.write(10);

  set_LEDS_direct(1, 0, 0);
  digitalWrite(LED_auto_pin, HIGH);
  digitalWrite(LED_man_pin, LOW);
  delay(300);

  set_LEDS_direct(0, 1, 0);
  digitalWrite(LED_auto_pin, LOW);
  digitalWrite(LED_man_pin, HIGH);
  delay(300);

  set_LEDS_direct(0, 0, 1);
  digitalWrite(LED_auto_pin, LOW);
  digitalWrite(LED_man_pin, LOW);
  delay(300);
}

void loop()
{
  wait_for_loop_t();
  read_Arduino_IO();
  calc_motors_pwr();
  operate_motors(left__percent_power, right_percent_power);
  operate_blower();
  operate_LEDs();
}