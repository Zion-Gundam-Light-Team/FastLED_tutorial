# FastLED 用戶文檔
***此專案使用 PlatformIO IDE！ 不支援ArduinoIDE***

---

## <span style="color: green;">✅ 重點 ✅ 新增env的checklist</span>
1. 改``UART_VIDEO_ADDR`` (如果要用mini mon)
2. 改``SLAVE_ID``
3. 改``I2C_SLAVE_ADDR`` ⚠️ **地址必須從 0x10 開始，依序遞增 (0x10, 0x11, 0x12...)**
4. 改``ACTUAL_SLAVE_NUM``
5. 改``WIFI_NAME``
6. 改``ACTUAL_NUM_PWM``
7. 改``PWM_ADDRESS``
8. 改``NUM_LEDS_RGB1_X`` (彩色LED燈珠數目)
9. 第一次燒晶片，記得要行``pio run --target uploadfs -e XXX``

---

## ESP32(S3)mini 開發板 GPIO
![Alt Text](./images/esp32s3.jpg)

## 電路圖
![Alt Text](./images/i2cDiagram.png)


## 前置要求
此專案主要使用 FastLED 來控制 RGB 燈光。
建議先學習 FastLED 的基礎知識。
- 教學影片：
  https://www.youtube.com/watch?v=4Ut4UK7612M&list=PLgXkGn3BBAGi5dTOCuEwrLuFtfz0kGFTC
- 文件：
  https://fastled.io/docs/

-----------

## 程式碼庫與環境
此專案包含三個獨立的程式碼庫和環境，每個都有不同的功能：

1. **master**：
   - **env**：`master`
   - **描述**：負責整體運作，處理API call。透過I2C與slave溝通燈效。
   - **進入點**：`firmware/master/src/main.cpp`
   - **命令列**：`pio run -e master -t upload`

2. **slave1**：
   - **env**：`slave1`
   - **描述**：負責顯示燈效，經I2C與master溝通並給予回覆
   - **進入點**：`firmware/slave/src/main_slave.cpp`
   - **命令列**：`pio run -e slave1 -t upload`

上傳`/data`裡的資料到晶片（用於 wifi-update 網頁）：⚠️ **忽略這步將會使晶片無法開啓wifi網頁**
`pio run --target uploadfs -e master -e slave1`
編譯並燒錄晶片：
`pio run --target upload -e master -e slave1`

## PlatformIO 設定
`platformio.ini` 檔案用於設定專案的建置環境：

**[env:master]**：`master` 環境的設定。
`upload_port`：請改成你正在使用的Port no
`monitor_port`：請設定成與上面upload_port一樣

**[env:slave1]**：`slave1` 環境設定
`-D SLAVE_ID=1;` ⚠️ **重要**⚠️ **請確保每個從屬裝置都被分配了唯一的 SLAVE_ID，否則會出Bugs**
`-D ACTUAL_NUM_PWM=5`⚠️ **重要**⚠️ **請輸入實際的pca9685數量，否則會出Bugs**
`-D PWM_ADDRESS_1=0x40` 請輸入實際pca9658的地址
`-D PWM_ADDRESS_2=0x41` 請輸入實際pca9658的地址
`-D PWM_ADDRESS_3=0x42` 請輸入實際pca9658的地址
`-D PWM_ADDRESS_4=0x43` 請輸入實際pca9658的地址
`-D PWM_ADDRESS_5=0x44` 請輸入實際pca9658的地址
`-D PWM_ADDRESS_6=0x45` 請輸入實際pca9658的地址
`-D PWM_ADDRESS_7=0x46` 請輸入實際pca9658的地址
`-D PWM_ADDRESS_8=0x47` 請輸入實際pca9658的地址
⚠️ **重要**⚠️ **pca9685地址不能重複，否則會有bugs!**
`-D NUM_LEDS_RGB1=100` 彩色LED燈珠數目（根據實際需求調整）
`-D NUM_LEDS_RGB2=50` 彩色LED燈珠數目（根據實際需求調整）
`-D NUM_LEDS_RGB3=10` 彩色LED燈珠數目（根據實際需求調整）
`-D NUM_LEDS_RGB4=10` 彩色LED燈珠數目（根據實際需求調整）
`upload_port`：請改成你正在使用的Port no
`monitor_port`：請設定成與上面upload_port一樣


⚠️ **重要**⚠️ **如果每個slave的pca9685數量不同/燈效不同，有機會造成slave與slave之間燈效時差**


## Serial Monitor
用來監示晶片的log
1. **master**：`pio device monitor -p /dev/cu.usbmodem21201`
2. **slave1**：`pio device monitor -p /dev/cu.usbmodem21301`
**需要手動更改port no**

## 進入點
- **`main.cpp` `main_slave.cpp`**：這是專案的主要進入點。它初始化硬體、設定網頁伺服器，並處理模式執行的主迴圈。
- **`setup()`**：在程式開始時被呼叫一次。它初始化 LED 燈條、設定 Wi-Fi、網頁伺服器、I2C 等。
- **`loop()`**：在 `setup()` 初始化完成後，不斷循環執行。

---------

## Story Mode 故事模式燈效
Story Mode 是預設的燈效播放系統，允許 master 控制所有 slave 播放同步的燈效。

### 模式介紹
系統內建 4 種燈效模式：
- **Mode 0**：亮點模式 - 隨機亮點效果
- **Mode 1**：正常模式 - 標準燈效展示
- **Mode 2**：長著模式 - 漸進式成長效果
- **Mode 3**：呼吸模式 - 呼吸燈效果
- **Dev Mode**：開發模式 - 藍紫色系漸變效果（slave 斷線時自動進入）

### 播放模式
Story Mode 有兩種播放方式：
1. **Loop Mode**（`isRepeatMode = 0`）
   - 燈效播放一次後停止，等待 master 下一個指令
   - 適合需要精確控制的場景
   
2. **Single Run Mode**（`isRepeatMode = 1`）
   - 燈效持續循環播放，不會停止
   - 適合持續展示的場景

### Slave 斷線處理
- 當 slave 超過 10 秒沒有收到 master 訊號，會自動進入 Dev Mode
- Dev Mode 會顯示藍紫色系漸變效果（#00b8ff → #001eff → #bd00ff → #d600ff），方便識別斷線的 slave
- 當 master 重新連接後，slave 會自動退出 Dev Mode

### 開發提示
- Story mode 函數需要回傳 boolean（true = 完成，false = 進行中）

-------

## Wi-Fi 設定
1. **初始連接**：當長按按鈕超過 10 秒時，它會以存取點（AP）模式運作。尋找名為 *"green_pepper_master"* 的 Wi-Fi 網路並使用您的裝置連接。
2. **IP 位址**：連接後，ESP32 將被分配一個 IP 位址。使用此 IP 位址來呼叫 API 以測試燈光效果。
3. 您可以在以下連結找到分配的 IP 位址：```green_pepper_master/```

## API (master晶片）：
- **GET** `/`獲取晶片資料（例如ip address)
- **GET** `/getBrightness`獲取當前亮度
- **GET** `/upload`上傳新的程式碼
- **POST** `/setSlaveWiFi`呼叫所有slave開啓wifi功能
- **POST** `/setDisconnect`關掉 WiFi
  
## API (slave晶片）：
- **GET** `/`：獲取晶片資料（例如ip address)
- **GET** `/getBrightness`獲取當前亮度
- **GET** `/upload`上傳新的程式碼
- **POST** `/setDisconnect`關掉 WiFi

新程式碼（.bin 的fireware檔案）上傳成功後，晶片會自動restart並運行新的程式碼