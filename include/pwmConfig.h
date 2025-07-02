#ifndef PWM_CONFIG_H
#define PWM_CONFIG_H

#define PWM0 0
#define PWM1 1
#define PWM2 2
#define PWM3 3
#define PWM4 4
#define PWM5 5
#define PWM6 6
#define PWM7 7
#define PWM8 8
#define PWM9 9
#define PWM10 10
#define PWM11 11
#define PWM12 12
#define PWM13 13
#define PWM14 14
#define PWM15 15
#define PWM16 16
#define PWM17 17
#define PWM18 18
#define PWM19 19
#define PWM20 20
#define PWM21 21
#define PWM22 22
#define PWM23 23
#define PWM24 24

#define PWM_CHANNEL_0 0
#define PWM_CHANNEL_1 1
#define PWM_CHANNEL_2 2
#define PWM_CHANNEL_3 3
#define PWM_CHANNEL_4 4
#define PWM_CHANNEL_5 5
#define PWM_CHANNEL_6 6
#define PWM_CHANNEL_7 7
#define PWM_CHANNEL_8 8
#define PWM_CHANNEL_9 9
#define PWM_CHANNEL_10 10
#define PWM_CHANNEL_11 11
#define PWM_CHANNEL_12 12
#define PWM_CHANNEL_13 13
#define PWM_CHANNEL_14 14
#define PWM_CHANNEL_15 15

#ifndef MAX_NUM_PWM
#define MAX_NUM_PWM 25 // 不要修改這裡！ 請到platformio.ini去定義實際使用的PWM數量!
#endif

#ifndef ACTUAL_NUM_PWM
#define ACTUAL_NUM_PWM 1 // 不要修改這裡！ 請到platformio.ini去定義實際使用的PWM數量!
#endif

#ifndef PWM_ADDRESS_1
#define PWM_ADDRESS_1 0x40 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_2
#define PWM_ADDRESS_2 0x41 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_3
#define PWM_ADDRESS_3 0x42 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_4
#define PWM_ADDRESS_4 0x43 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_5
#define PWM_ADDRESS_5 0x44 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_6
#define PWM_ADDRESS_6 0x45 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_7
#define PWM_ADDRESS_7 0x46 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_8
#define PWM_ADDRESS_8 0x47 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_9
#define PWM_ADDRESS_9 0x48 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_10
#define PWM_ADDRESS_10 0x49 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_11
#define PWM_ADDRESS_11 0x4A // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_12
#define PWM_ADDRESS_12 0x4B // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_13
#define PWM_ADDRESS_13 0x4C // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_14
#define PWM_ADDRESS_14 0x4D // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_15
#define PWM_ADDRESS_15 0x4E // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_16
#define PWM_ADDRESS_16 0x4F // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_17
#define PWM_ADDRESS_17 0x50 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_18
#define PWM_ADDRESS_18 0x51 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_19
#define PWM_ADDRESS_19 0x52 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_20
#define PWM_ADDRESS_20 0x53 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_21
#define PWM_ADDRESS_21 0x54 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_22
#define PWM_ADDRESS_22 0x55 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_23
#define PWM_ADDRESS_23 0x56 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_24
#define PWM_ADDRESS_24 0x57 // 請到platformio.ini去pca9685定義地址！
#endif

#ifndef PWM_ADDRESS_25
#define PWM_ADDRESS_25 0x58 // 請到platformio.ini去pca9685定義地址！
#endif

#endif // PWM_CONFIG_H