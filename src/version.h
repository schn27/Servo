#undef BOARD_REV
#define BOARD_REV 2

#undef DEVICE_STRING

#if BOARD_REV == 1
#define DEVICE_STRING "Servo,1.14,r1"
#elif BOARD_REV == 2
#define DEVICE_STRING "Servo,1.14,r2"
#endif
