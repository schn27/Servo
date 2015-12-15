Servo controller (modified Hitec)
---------------------------------

[GPIO]

P0.0 - Bridge driver enable
P0.3 - Rs485 transmitter enable


[ADC]

P1.2 - Reference
P1.3 - Voltage
P1.4 - Position
P1.5 - Current


[PCA]

Module0 - ~Q2 (P0.1)
Module1 -  Q1 (P0.2)
Module2 -  Q3 (P1.6)
Module3 - ~Q4 (P1.7)
Module4 - Master (24 kHz)


[TIMERS]

Timer0 - Unused
Timer1 - UART (115200 bit/s)
Timer2 - Regulator cycles (1000 Hz)
Timer3 - Initiates ADC (20 kHz)



Bugs
----

1. Очень редко при старте начинаются колебания туда-сюда с большой амплитудой (лечится только рестартом).
   Не смог пока воспроизвести "на столе". Подозреваю проблему при считывании настроек с FLASH.
2. Текущая реализация использует PD регулатор, поэтому под нагрузкой есть статическая ошибка. 
   С интегратором не удалось добиться чёткой перекладки (есть выбросы).






