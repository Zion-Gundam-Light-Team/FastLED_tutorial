# FastLED 用戶文檔
***此專案使用 PlatformIO IDE！ 不支援ArduinoIDE***

---

## <span style="color: red;">⚠️ 重要⚠️ 關於第三方庫：`Adafruit_PWMServoDriver`</span>
為了提高pca9685的燈效效能，我們要對它稍作修改，而這部份需要手動去update
步驟如下：
打開`Adafruit_PWMServoDriver.h` (這個檔案在`.pio/libdeps`裡)
第line97，更新為以下的code:
```cpp
//...
private:
  friend class Custom_PWMServoDriver; //請手動加入這行CODE！！
  uint8_t _i2caddr;
  TwoWire *_i2c;
  Adafruit_I2CDevice *i2c_dev = NULL;
//...
```
留意一點： 每個env裡都會有各自生成的 `Adafruit_PWMServoDriver.h`，需要手動逐個修正。
**如無按照以上做法，project會無法順利編譯**

---

## <span style="color: green;">✅ 重點 ✅  checklist</span>
1. 請檢查：根目錄是否有一個名為`platformio_local.ini`的檔案，如沒有，你需要自行create，並複製貼上以下內容：
```ini
[env:esp32dev]
monitor_port = /dev/cu.usbmodem11201 #用來監聽log的port
```
2. 在 `platformio.ini` 中設定你的 `ACTUAL_NUM_PWM`
3. 在 `platformio.ini` 中設定你的 `PWM_ADDRESS`
4. 記得在``Adafruit_PWMServoDriver.h``內加一行code

---
## 線路圖
![Alt Text](./data/images/diagram.png)

## ESP32(S3)mini 開發板 GPIO
![Alt Text](./data/images/esp32s3.png)

## ESP32(Cs3) 開發板 GPIO
![Alt Text](./data/images/esp32c3.png)

## 前置要求
此專案主要使用 FastLED 來控制 RGB 燈光。
建議先學習 FastLED 的基礎知識。
- 教學影片：
  https://www.youtube.com/watch?v=4Ut4UK7612M&list=PLgXkGn3BBAGi5dTOCuEwrLuFtfz0kGFTC
- 文件：
  https://fastled.io/docs/


## PlatformIO 設定
`platformio.ini` 檔案包含所有開發者共用的設定
**[env:esp32dev]**：`esp32dev` 環境的設定。


## Serial Monitor
用來監示晶片的log
`pio device monitor -p /dev/cu.usbmodem21201`
**需要手動更改port no**
