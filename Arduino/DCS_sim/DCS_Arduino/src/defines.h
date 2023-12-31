// PWM pins
#define LeftPWM_pin 3
#define RightPWM_pin 6
#define air_PWM_pin 11
// Digital pins
#define LeftLL_pin 4
#define LeftUL_pin 5
#define RightLL_pin 8
#define RightUL_pin 7
#define man_right_pin 10
#define man_left__pin 9
#define LED_auto_pin 22
#define LED_man_pin 24
#define auto_pin 26
#define man_pin 28
#define man_mode_pin 30
#define air_force_on_pin 34
#define air_off_pin 32
#define left__PB_pin 36
#define LED_red_pin 44
#define LED_blu_pin 46
#define LED_grn_pin 45
// Analog pins
#define left__pos_pin 0
#define right_pos_pin 1
#define man_speed_pin 2
#define air_speed_pin 4
#define scale_pin 3
// calibrations
#define left__pos_0 512
#define right_pos_0 490
#define left__min_pos -420
#define left__max_pos 380
#define right_min_pos -420
#define right_max_pos 380
#define pos_ofset -14 //
#define dist_to_start_slowing 40
// motors control
#define KS_left 12
#define KS_right 12
#define KP 7 // X10  5 means 0.5
#define DB 0 // allowable error
#define PWM_zero 90
#define PWM_range_per_side 85
#define max_pwr 100 // in %
#define air_zero_pwr 20

// communication
#define baud_rate 500000
#define COM0 0         // hardware Serial Port
#define START_BYTE '[' // Start Byte for serial commands
#define END_BYTE ']'   // End Byte for serial commands
