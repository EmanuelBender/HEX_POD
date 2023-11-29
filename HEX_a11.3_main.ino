
#define codeRevision "a11.3"

#include <Arduino.h>
#include <Wire.h>
#include "SPI.h"
#include <WiFi.h>
#include <ArduinoBLE.h>
// #include <Credentials.h>
#include <Preferences.h>

#define sda GPIO_NUM_18
#define scl GPIO_NUM_17

#include <PCA95x5.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "SPIFFS.h"
#include "FS.h"
#include "SD.h"

// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
//AsyncWebServer server(80);
#include <ESPmDNS.h>
#include <WebServer.h>
// #include <WebSocketsServer.h>
WebServer server(80);
uint8_t webServerPollMs = 100;


// ________________  ESP32 UTILITY  ____________________
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <esp_system.h>
#include <esp_cpu.h>

#include "TaskManagerIO.h"
#include <BasicInterruptAbstraction.h>
#include <SimpleSpinLock.h>
SimpleSpinLock taskManagerLock;
BasicArduinoInterruptAbstraction interruptAbstraction1;                                                                                               // INT for buttons
taskid_t LOG, ST1, STATID, IMUID, TEMPID, INA2ID, BMEID, SGPID, SECID, NTPID, BTNID, CLKID, MENUID, WIFIID, SNSID, HOMEID, UTILID, TMID, SYSID, WEB;  // task IDs
uint32_t tmTracker;
int i;
const byte menuRowM = 26;

esp_chip_info_t chip_info;
esp_reset_reason_t resetReason;
String resetReasonString;
String wakeupReasonString;

BLEDevice peripheral;
struct BLEData {
  String result;
  int rssi;
};
int BLEresults = 40;
BLEData scanData[40];

const char* wifiStatusChar[] = {
  "Idle",
  "No SSID",
  "Scan Done",
  "Connected",
  "Connect Failed",
  "Connection Lost",
  "Disconnected",
};

String menuOptions[5] = {
  "Home",
  "Sensors",
  "Utility",
  "Task Manager",
  "System",
};



#include "time.h"  // ____________________________  TIME  __________________________
#include "esp_sntp.h"
// byte WiFistatus, WiFiRSSI;
String WiFiIP;
const char* hostname = "[HEX]POD";
const int WiFiTimeout = 8000;
String webHost, webPw;
// uint8_t MainMAC[] = { 0x30, 0xA0, 0xA5, 0x07, 0x0D, 0x66 };  // 3600
const char* ntpServer1 = "pool.ntp.org";   // NTP Time server
const char* ntpServer2 = "time.nist.gov";  // fallback
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const int SECONDS_IN_MINUTE = 60;
const int MINUTES_IN_HOUR = 60;
const int HOURS_IN_DAY = 24;
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // CET-1CEST,M3.5.0,M10.5.0/3  ,  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00  ,  WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
                                                       // ESP32Time espTime(0);
struct tm timeinfo;
String lastNTPtime, lastNTPtimeFail;
String printTime, printDate;
long int restarts, uptime;
int DS;

String uptimeString;
uint32_t lastInputTime;
uint32_t timeTracker;
double bmeTracker, sgpTracker, ina2Tracker, tempTracker, uTimeTracker, powerStTracker, loggingTracker, ntpTracker, clientTracker, statBaTracker, imuTracker;

const byte slotsSize = 24;
char taskFreeSlots[slotsSize];  // Free TaskManagerIO slots
String taskArray[slotsSize];
String TaskNames[slotsSize];

//________________________________________________________________  GENERAL  _______________________

Preferences preferences;

bool DEBUG = false;
// #define ENABLE_I2C_DEBUG_BUFFER
bool SLEEPENABLE;
bool serialPrintLOG;
bool serialPrintBME1;

SPIClass sdSPI = SPIClass(HSPI);
#define SD_MISO 13
#define SD_MOSI 14
#define SD_SCLK 12
#define SD_CS 21

#define SPI_FREQUENCY 60000000       // 80000000 tested ok
#define SPI_READ_FREQUENCY 30000000  // 40000000 tested ok
#define I2C_SPEED 400000             // 800000   tested ok

uint32_t loggingInterval = 5;    // logAll
uint16_t getNTPInterval = 1800;  // 600 = 10 mins, 1800 = 30 mins


String SDarray[50][65];
uint32_t SDIndex = 0;
bool SDinserted;
uint8_t serialColumn;

uint8_t consoleLine;
String console[55][65];

uint32_t file_system_size, file_system_used, free_size, program_size, psramSize, freePsram;
long int cpu_freq_mhz, cpu_xtal_mhz, cpu_abp_hz;
uint32_t flash_size;
int chiprevision;
bool LEDon, FANon, isFading;

String TAG = "ESP";

enum PowerState { NORMAL,
                  IDLE,
                  POWER_SAVE,
                  LIGHT_SLEEP,
                  DEEP_SLEEP };

const char* powerStateNames[] = {
  "NORMAL",
  "IDLE",
  "POWER_SAVE",
  "LIGHT_SLEEP",
  "DEEP_SLEEP"
};

PowerState currentPowerState = NORMAL;
PowerState previousPowerState = NORMAL;

uint32_t idleDelay = 10000 * 1000;        // 30 sec
uint32_t powersaveDelay = 240000 * 1000;  // 3 min
// uint32_t lightsleepDelay = 600000 * 1000; // 10 min

const double ONEMILLION = 1000000.0;
const float pi = 3.14159265358979323846264338327950;

//PCA9585_______________________________________________________________

PCA9555 io;
bool INT_TRGR;
int UP, DOWN, LEFT, RIGHT;
bool CLICK_DOWN, CLICK_LEFT, CLICK;
bool BUTTON;
bool P0[18];
uint8_t click;
int PCABITS = 0b0000000000000000;

//TFT_eSPI_______________________________________________________________
#include "TFT_eSPI.h"
TFT_eSPI tft = TFT_eSPI();  // before TFT_eSPI library updates, save 'Setup203_ST7789.h', 'User_Setup_Select.h', 'User_Setup.h'. Replace after updating.
#include <ESP32Servo.h>
ESP32PWM pwm;
float TFTbrightness;

const byte radius = 5;  // menu UI stuff
int cIndex;             // menu

bool menuTrigger, blockMenu = false;
const int menuItems = 5;
int carousel = 1, lastCarousel = 0, highlightIndex = 0, lastHighlightIndex = 1;
int sysIndex = 0;
int utilIndex = 1;
int tmIndexY = 60;
String label, value, extraSP, taskSymbol;

uint16_t Delta = (TFT_WIDTH - 1) / 12;
bool smooth = true;
uint16_t colors[12];


//OLED______________________________________________________________________
#include <U8g2lib.h>
U8G2_SH1106_128X32_VISIONOX_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
const byte oledDatum_X = 0;
const byte oledDatum_Y = 32;
bool OLEDon = false;

//LIS3______________________________________________________________________
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
int X, Y, Z;
// #define CLICKTHRESHHOLD 80
const uint8_t imuAvg = 55;  // averaging samples
uint16_t imuInterval = 1000;
uint16_t mapRange = 16384;  // accelrange 2 = 16384, 4 = 8096, 8 = 1024, 16 = 256
uint8_t imuSampleCount;

//DS18B20___________________________________________________________________

OneWire oneWire(8);
DallasTemperature tempSens(&oneWire);
DeviceAddress tempProbe1 = { 0x28, 0x12, 0xEF, 0x75, 0xD0, 0x01, 0x3C, 0x77 };
double temp1;

//BME688___________________________________________________________________
#include "bme68xLibrary.h"

#define NEW_GAS_MEAS (BME68X_GASM_VALID_MSK | BME68X_HEAT_STAB_MSK | BME68X_NEW_DATA_MSK)
bme68xData data;
Bme68x bme;

String BME_ERROR, lastBMEpoll;
uint32_t bmeInterval;  //  interval polling Sensor
const byte numProfiles = 14;
const uint16_t bmeFloorOffs = 5684;
uint32_t bme_resistance[numProfiles];  // = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint32_t bme_resistance_avg[numProfiles];
double bme_gas_avg;
uint8_t bmeProfile, bmeSamples, repeater, bmeProfilePause;
uint16_t duration, heaterTemp;
float Altitude;

uint16_t heatProf_1[] = {
  75,   // 1
  85,   // 2
  100,  // 3
  120,  // 4
  140,  // 5
  160,  // 6
  180,  // 7
  200,  // 8
  220,  // 9
  240,  // 10
  260,  // 11
  280,  // 12
  300,  // 13
  320,  // 14
};

uint8_t durProf_1[] = {
  6,  // 1
  5,  // 2
  5,  // 3
  5,  // 4
  5,  // 5
  5,  // 6
  5,  // 7
  5,  // 8
  5,  // 9
  5,  // 10
  5,  // 11
  5,  // 12
  5,  // 13
  5,  // 14
};



//SGP41___________________________________________________________________
#include <SensirionI2CSgp41.h>
#include <VOCGasIndexAlgorithm.h>
#include <NOxGasIndexAlgorithm.h>

VOCGasIndexAlgorithm voc_algorithm;
NOxGasIndexAlgorithm nox_algorithm;
SensirionI2CSgp41 sgp41;

String lastSGPpoll;
int32_t index_offset;
int32_t learning_time_offset_hours;
int32_t learning_time_gain_hours;
int32_t gating_max_duration_minutes;
int32_t std_initial;
int32_t gain_factor;

uint8_t conditioning_duration = 20;
uint16_t sgpInterval = 1000;  // needs to be at 1 for the algo ?

uint16_t sgpError, error;
char sgpErrorMsg[256];

constexpr uint16_t defaultRh = 0x8000;
constexpr uint16_t defaultT = 0x6666;
uint16_t srawVoc, srawNox;
int32_t VOC, NOX;


// INA219B_____________________________________________
#include "INA219.h"
INA219 INA2(0x40);
float BUS2_BusVoltage;
float BUS2_ShuntVoltage;
float BUS2_Current;
float BUS2_Power;
bool BUS2_OVF;
bool BUS2_CNVR;


void setup() {

  timeTracker = millis();

  //___________________________ INITIALIZE COMMS __________________________


  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(40);  //relax...

  if (DEBUG) {
    ESP_LOGI(TAG, "Serial bus Initialized.");
  }

  if (Wire.begin(sda, scl, I2C_SPEED)) {  // sda= , scl= in pins_arduino.h
    delay(20);
    if (DEBUG) {
      ESP_LOGI(TAG, "I2C bus Initialized.");
    }
  }


  preferences.begin("credentials", true);      // true : read only
  webHost = preferences.getString("webHost");  // hexpod
  webPw = preferences.getString("webPw");

  WiFi.setHostname(hostname);  //define hostname
  WiFi.begin(preferences.getString("ssid", "NaN"), preferences.getString("pw", "NaN"));
  delay(200);

  preferences.end();


  //_________________ INITIALIZE Preferences ____________________
  preferences.begin("my - app", false);

  // Sensor Prefs
  serialPrintBME1 = preferences.getBool("bmelog", 0);
  SLEEPENABLE = preferences.getBool("sleep", 0);
  OLEDon = preferences.getBool("oled", 1);
  bmeSamples = preferences.getUInt("bmeSpls", 1);
  loggingInterval = preferences.getUInt("logItvl", 15000);
  bmeProfilePause = preferences.getUInt("bmePause", 0);
  // System Prefs
  DEBUG = preferences.getBool("debug", 0);
  restarts = preferences.getUInt("counter", 0);
  restarts++;
  preferences.putUInt("counter", restarts);
  preferences.end();

  program_size = ESP.getSketchSize();
  free_size = ESP.getFlashChipSize() - program_size - file_system_size + file_system_used;
  esp_flash_get_size(NULL, &flash_size);
  esp_chip_info(&chip_info);
  cpu_freq_mhz = getCpuFrequencyMhz();
  cpu_xtal_mhz = getXtalFrequencyMhz();
  cpu_abp_hz = getApbFrequency();
  SPIFFS.begin(true);
  file_system_size = SPIFFS.totalBytes();
  file_system_used = SPIFFS.usedBytes();
  SPIFFS.end();


  //___________________________ INITIALIZE TFT _________________________
  tft.init();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  tft.setTextDatum(TC_DATUM);
  tft.setTextPadding(100);

  TAG = "SD";
  pinMode(GPIO_NUM_47, INPUT);  // SD present
  SDinserted = digitalRead(GPIO_NUM_47);
  if (SDinserted) {
    ESP_LOGI(TAG, "SD card present.");

    sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);  // MOSI 11, SCK 12, MISO 13, CS 10
    sdSPI.setFrequency(SPI_FREQUENCY / 2);
    sdSPI.setDataMode(SPI_MODE0);

    if (SD.begin(SD_CS, sdSPI, SPI_FREQUENCY / 2, "/sd", 50)) {
      ESP_LOGI(TAG, "SD card mounted.");
    } else {
      ESP_LOGI(TAG, "SD card Init Failed.");
    }
  }

  //_________________________ INITIALIZE GPIO __________________________
  void IRAM_ATTR UDLR_INT();
  void IRAM_ATTR CTR_INT();
  ESP32PWM::allocateTimer(0);

  pinMode(GPIO_NUM_0, INPUT_PULLDOWN);  // BUTTON
  attachInterrupt(GPIO_NUM_0, CTR_INT, FALLING);

  TFTbrightness = 1;
  pinMode(GPIO_NUM_1, OUTPUT);          // FAN_CTL
  pwm.attachPin(GPIO_NUM_2, 1000, 12);  // BLK, 1KHz, 10 bit
  pwm.writeScaled(TFTbrightness);

  // pinMode(GPIO_NUM_3, INPUT);
  // pinMode(GPIO_NUM_7, INPUT);  // LIS3_INT
  // attachInterrupt(GPIO_NUM_7, LIS_ISR, RISING);
  // pinMode(GPIO_NUM_8, INPUT);   // free
  // pinMode(GPIO_NUM_14, INPUT);  // free
  // pinMode(GPIO_NUM_21, INPUT);  // free
  pinMode(GPIO_NUM_38, INPUT_PULLDOWN);  // PCA_INT
  attachInterrupt(GPIO_NUM_38, UDLR_INT, FALLING);
  // pinMode(GPIO_NUM_45, INPUT);  // free, bootstrap
  //pinMode(GPIO_NUM_46, INPUT);  // free, bootstrap
  pinMode(GPIO_NUM_47, INPUT);  // SD present
  // pinMode(GPIO_NUM_48, INPUT);  // free

  //_______________________ INITIALIZE MULTIPLEXER ______________________
  io.attach(Wire, 0x20);
  io.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  //io.direction(PCA95x5::Port::P00, PCA95x5::Direction::IN); // free, ALS_1 INT solder bridge
  //io.direction(PCA95x5::Port::P01, PCA95x5::Direction::IN); // free
  //io.direction(PCA95x5::Port::P02, PCA95x5::Direction::IN); // free
  //io.direction(PCA95x5::Port::P03, PCA95x5::Direction::IN); // free
  //io.direction(PCA95x5::Port::P04, PCA95x5::Direction::IN); // free
  //io.direction(PCA95x5::Port::P05, PCA95x5::Direction::IN); // free
  //io.direction(PCA95x5::Port::P06, PCA95x5::Direction::IN); // free
  io.direction(PCA95x5::Port::P07, PCA95x5::Direction::OUT);  // OLED PWR
  io.direction(PCA95x5::Port::P10, PCA95x5::Direction::IN);   // BTN Down
  io.direction(PCA95x5::Port::P11, PCA95x5::Direction::IN);   // BTN Left
  io.direction(PCA95x5::Port::P12, PCA95x5::Direction::IN);   // BTN Right
  io.direction(PCA95x5::Port::P13, PCA95x5::Direction::IN);   // BTN Up
  io.direction(PCA95x5::Port::P14, PCA95x5::Direction::OUT);  // STATUS_2 LED
  io.direction(PCA95x5::Port::P15, PCA95x5::Direction::OUT);  // STATUS_1 LED
                                                              // io.direction(PCA95x5::Port::P16, PCA95x5::Direction::OUT);  // PER_PWR
                                                              // io.direction(PCA95x5::Port::P17, PCA95x5::Direction::OUT);  // SNS_PWR

  io.write(PCA95x5::Port::P07, OLEDon ? PCA95x5::Level::H : PCA95x5::Level::L);  // OLED
  io.write(PCA95x5::Port::P14, PCA95x5::Level::H);                               // led off
  io.write(PCA95x5::Port::P15, PCA95x5::Level::H);                               // led off
  // io.write(PCA95x5::Port::P16, PCA95x5::Level::L);  // PER_PWR On
  // io.write(PCA95x5::Port::P17, PCA95x5::Level::L);  // SNS_PWR On

  //___________________________ INITIALIZE LIS3DH ______________________
  /* lis.settings.adcEnabled = 0;
  lis.settings.tempEnabled = 1;
  lis.settings.accelSampleRate = imuSR;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  lis.settings.accelRange = accelRange;  //   Can be: 2, 4, 8, 16
  lis.settings.xAccelEnabled = 1;
  lis.settings.yAccelEnabled = 1;
  lis.settings.zAccelEnabled = 1;
  // lis.configureFreeFallInterrupt(true); */

  lis.begin(0x18);
  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // lis.setClick(2, CLICKTHRESHHOLD);  // THRESHOLD 80
  lis.setRange(LIS3DH_RANGE_2_G);
  // lis.setDataRate(LIS3DH_DATARATE_LOWPOWER_5KHZ);
  lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);

  //___________________________ INITIALIZE DS18B20 _____________________
  tempSens.begin();
  DS = tempSens.getDeviceCount();
  tempSens.setWaitForConversion(false);
  tempSens.setResolution(10);                 // set global resolution to 9, 10 (default), 11, or 12 bits
  tempSens.setHighAlarmTemp(tempProbe1, 60);  // Dallas tempProbe Alarm Thresholds
  tempSens.setLowAlarmTemp(tempProbe1, -2);


  //___________________________ INITIALIZE INA219 _____________________
  if (!INA2.begin()) {
    // Serial.println("could not connect INA219. Fix and Reboot");
  } else {
    INA2.setMaxCurrentShunt(1, 0.001);  //  maxCurrent, shunt
    INA2.setBusVoltageRange(16);        // 16,32
    INA2.setGain(8);                    // 1,2,4,8
    //INA2.setBusADC(3);
    //INA2.setShuntADC(3);
    // INA2.setMode(7);
    // INA2.setModeShuntBusTrigger();
  }

  //___________________________ INITIALIZE BME688 _____________________

  bme.begin(0x77, Wire);

  bmeInterval = loggingInterval;

  if (bme.checkStatus()) {
    if (bme.checkStatus() == BME68X_ERROR) {
      BME_ERROR = "Error: " + bme.statusString();
      return;
    } else if (bme.checkStatus() == BME68X_WARNING) {
      BME_ERROR = "Warning: " + bme.statusString();
    }
  }

  // Set BME68X settings
  bme.setTPH(BME68X_OS_2X, BME68X_OS_8X, BME68X_OS_4X);
  bme.setFilter(3);

  pollBME();
  // pollBME();
  // bme.setOpMode(BME68X_SLEEP_MODE);

  //___________________________ INITIALIZE SGP41 _____________________
  sgp41.begin(Wire);
  configSGP();

  //___________________________ INITIALIZE OLED ________________________
  if (OLEDon) {
    u8g2.begin();
    u8g2.setFontDirection(0);
    u8g2.setFontMode(0);
    u8g2.setFont(u8g2_font_logisoso28_tn);  // u8g2_font_u8glib_4_tf

    if (DEBUG) {
      u8g2.clearBuffer();
      u8g2.drawStr(oledDatum_X, oledDatum_Y, "OLED Good.");
      u8g2.sendBuffer();
    }
  }

  getNTP();
  setupWebInterface();

  //___________________________ TASK MANAGER ________________________

  launchUtility();  // launch utility Menu, setup tasks
  lastInputTime = micros();
  //___________________________ END REPORT _____________________________

  if (DEBUG) {
    WiFiIP = WiFi.localIP().toString();
    Serial.println();
    Serial.println(WiFiIP);
    Serial.println();

    TAG = "ESP";
    ESP_LOGI(TAG, "Reset Reason: %s", getResetReason());
    ESP_LOGI(TAG, "%d Restarts:", restarts);
    ESP_LOGI(TAG, "Setup took: %.2lfs\n\n", (millis() - timeTracker) / 1000);
  }
}




void loop() {

  taskManager.runLoop();
}
