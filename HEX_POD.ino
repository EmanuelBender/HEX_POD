
/*                                               */ #define codeRevision "a11.3" /*                                                    */
/*                                                       [HEX]POD | DevKit                                                             */
/*                                                         Emanuel Bender                                                              */
/*                                                                                                                                     */
/*                                                                                                                                     */
/*                                                                                                                                     */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoBLE.h>
#include <Preferences.h>

#include <FS.h>
#include <LittleFS.h>
#include <SD.h>

#include <TFT_eSPI.h>
#include <U8g2lib.h>
#include <PCA95x5.h>
#include <Adafruit_Sensor.h>
#include "INA219.h"
#include "bme68xLibrary.h"
#include "SensirionI2CSgp41.h"
#include "VOCGasIndexAlgorithm.h"
#include "NOxGasIndexAlgorithm.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "Adafruit_LIS3DH.h"
#include "SparkFun_SCD30_Arduino_Library.h"


// ________________  ESP32 UTILITY  ____________________
#include <stdio.h>
#include <sdkconfig.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <esp_cpu.h>
#include <stdexcept>
#include <sstream>
#include <inttypes.h>
#include <sstream>

#include <TaskManagerIO.h>
#include <TimeLib.h>
#include <time.h>
#include <ESP32Servo.h>

#include <WebServer.h>
#include <ESPmDNS.h>
#include <esp_sntp.h>
  WebServer server(80);
uint16_t webServerPollMs = 80;

uint16_t i;
#define pi 3.14159265358979323846264338327950
#define ONEMILLION 1000000
#define ONETHOUSAND 1000
const double KILOBYTE = 1024.0;
const double ONEMILLIONB = KILOBYTE * KILOBYTE;
#define ZERO 0



//___________________________________  GENERAL::USER  ______________________________

// #define ENABLE_I2C_DEBUG_BUFFER
bool DEBUG = false;   // stored in prefs
bool LOGGING = true;  // stored in prefs

// !! Enter Credentials before very first upload !! once entered, they will be stored in preferences.
String wifiSSID = "";                   // your WiFi SSID
String wifiPW = "";                     // your WiFi Password
String hostname = "[HEX]POD - Center";  // web server Title
String local_domain = "hex";            // web server MDNS IP: hex.local
String webHost = "hexpod";              // web server host admin name
String webPw = "";                      // web server password

#define SD_MISO 13  // SPI pins for SD card
#define SD_MOSI 14
#define SD_SCLK 12
#define SD_CS 21
SPIClass sdSPI = SPIClass(HSPI);
// #define SPI_FREQUENCY 60000000       // 80000000 tested ok
// #define SPI_READ_FREQUENCY 30000000  // 40000000 tested ok

#define sda GPIO_NUM_18
#define scl GPIO_NUM_17
#define I2C_SPEED 200000  // 800k tested ok, max 200k(100k) with SCD30

uint16_t loggingInterval = 60000;                // in ms, stored in Prefs
uint16_t getNTPInterval = 1800;                  // in ms, 600 = 10 mins, 1800 = 30 mins
uint32_t idleDelay = 30000 * ONETHOUSAND;        // in us
uint32_t powersaveDelay = 240000 * ONETHOUSAND;  // in us, 3 min
// uint32_t lightsleepDelay = 600000 * ONETHOUSAND; // 10 min


// ___________________________________  TIME & WiFi  ________________________________
#define WiFiTimeout 8000  // in ms
String WiFiIP;
int RSSI;
String RSSIsymbol;
const char* ntpServer1 = "0.pool.ntp.org";  // NTP Time server
const char* ntpServer2 = "time.nist.gov";   // fallback
const char* ntpServer3 = "1.pool.ntp.org";  // fallback
#define SECONDS_IN_MINUTE 60
#define MINUTES_IN_HOUR 60
#define HOURS_IN_DAY 24
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // CET-1CEST,M3.5.0,M10.5.0/3  ,  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00  ,  WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00

struct tm timeinfo;
String lastNTPtime, lastNTPtimeFail;
String printTime, printDate;
long int restarts, uptime;

String uptimeString, lastRestart;
uint32_t lastInputTime;
uint32_t timeTracker;
double elapsedTime;
uint16_t lastSDInterrupt;


// ___________________________   LOG & Chart & Data _______________________________
String TAG = "ESP";
multi_heap_info_t deviceInfo;
esp_chip_info_t chip_info;
String resetReasonString;
String wakeupReasonString;
Preferences preferences;

size_t free_flash_size, flash_size, flash_used, program_size, program_free, program_used, SPIFFS_size, SPIFFS_used, SPIFFS_free, total_heap, free_heap, min_free_heap, min_free_int_heap;
double percentLeftLFS, percentUsedLFS, program_UsedP, program_LeftP, flash_UsedP, flash_LeftP, CPUTEMP, free_RAM_p, used_RAM_p;
uint32_t cpu_freq_mhz, cpu_xtal_mhz, cpu_abp_hz, flash_speed;
int chiprevision;
bool LEDon, FANon, OLEDon, SDinserted;
uint8_t filesCount, directoryCount, fileId;
String logFilePath, rootHexPath = "/.sys";

int batteryCharge, batteryChargeGauge;
float batteryVoltage;
uint8_t batteryStateByte = B0000;
float FANvalue;
bool LEDenable = true;

uint8_t consoleLine;
#define consoleColumns 20
#define consoleRows 54
#define menuRowM 26
String console[consoleRows][consoleColumns];

#define SHRT 0
#define LNG 1
#define DBL 2

bool pastLOGGINGstate = !LOGGING;
bool SLEEPENABLE;
bool serialPrintLOG;

const String logHeader = "Time, BME_0, BME_1, BME_2, BME_3, BME_4, BME_5, BME_6, BME_7, BME_8, BME_9, BME_10, BME_11, BME_12, BME_13, BME_T, BME_H, BME_P, SGP_VOC, SGP_NOX, SGP_rVOC, SGP_rNOX, SCD_CO2, SCD_T, SCD_H\n";
const String logColumns[] = { "Time", "BME_0", "BME_1", "BME_2", "BME_3", "BME_4", "BME_5", "BME_6", "BME_7", "BME_8", "BME_9", "BME_10", "BME_11", "BME_12", "BME_13", "BME_T", "BME_H", "BME_P", "SGP_VOC", "SGP_NOX", "SGP_rVOC", "SGP_rNOX", "SCD_CO2", "SCD_T", "SCD_H" };
#define log_idx_bme1_temp 15   // index in log file
#define log_idx_bme1_humid 16  // index in log file
#define log_idx_bme1_press 17  // index in log file
#define log_idx_sgp_voc 18
#define log_idx_sgp_nox 19
#define log_idx_scd_co2 22
#define log_idx_scd_temp 23
#define log_idx_scd_humid 24

uint16_t chart_data_range = 3600;  // in seconds from now
#define COLOR_TEAL 0x66cce8
#define COLOR_ORANGE 0xff7f17
#define COLOR_BLUE 0x5496ff

enum PowerState { NORMAL,
                  IDLE,
                  POWER_SAVE,
                  LIGHT_SLEEP,
                  DEEP_SLEEP };

const char* powerStateNames[] = {
  "&#9655;&#9655;&#9655;",  // NORMAL
  "&#9655;&#9655;",         // IDLE
  "&#9655;",                // POWER SAVE
  "&#9214;",
  "DEEP_SLEEP"
};


PowerState currentPowerState = NORMAL;
PowerState previousPowerState = NORMAL;

const char* menuOptions[] = {
  "Home",
  "Sensors",
  "Utility",
  "Task Manager",
  "System",
};

BLEDevice peripheral;
struct BLEData {
  String result;
  int rssi;
};
const byte BLEresults = 40;
BLEData scanData[BLEresults];

const char* wifiStatusChar[] = {
  "Idle",
  "No SSID",
  "Scan Done",
  "Connected",
  "Connect Failed",
  "Connection Lost",
  "Disconnected",
};


//PCA9585__________________________________________________________________
PCA9555 io;
bool UP, DOWN, LEFT, RIGHT, CLICK_DOWN, CLICK_LEFT, CLICK, BUTTON, INT_TRGR;
bool P0[18];
uint16_t PCABITS = 0b0000000000000000;

//TFT_eSPI_________________________________________________________________
TFT_eSPI tft = TFT_eSPI();  // before TFT_eSPI library updates, save 'Setup203_ST7789.h', 'User_Setup_Select.h', Replace after updating.
ESP32PWM pwm;
float TFTbrightness;

bool menuTrigger, blockMenu;
const byte tmIndexY = 60, radius = 5, menuItems = 5;
byte carousel = 1, lastCarousel = 0, highlightIndex = 0, lastHighlightIndex = 1, sysIndex = 0, utilIndex = 1;
String label, value;

//OLED______________________________________________________________________
U8G2_SH1106_128X32_VISIONOX_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
const byte oledDatum_X = 0;
const byte oledDatum_Y = 32;

//LIS3______________________________________________________________________
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
int16_t X, Y, Z;  // 16bit
uint16_t imuInterval;
uint8_t imuSampleCount;
const uint8_t imuAvg = 55;        // for cube demo
const uint16_t mapRange = 16384;  // accelrange 2 = 16384, 4 = 8096, 8 = 1024, 16 = 256

//DS18B20___________________________________________________________________
OneWire oneWire(GPIO_NUM_8);
DallasTemperature tempSens(&oneWire);
byte DTdevice;

struct probeStruct {
  // const byte index;
  const char* name;
  float temperature;
  DeviceAddress address;
};

probeStruct DTprobe[] = {
  { "ESP32_prox", float(), { 0x28, 0x12, 0xEF, 0x75, 0xD0, 0x01, 0x3C, 0x77 } },
  { "PCB#1_prox", float(), { 0x28, 0x30, 0x14, 0x75, 0xD0, 0x01, 0x3C, 0x79 } }
};


//BME688___________________________________________________________________
#define NEW_GAS_MEAS (BME68X_GASM_VALID_MSK | BME68X_HEAT_STAB_MSK | BME68X_NEW_DATA_MSK)
bme68xData bme1_data;
Bme68x bme1;

String BME_ERROR, lastBMEpoll;
const byte numProfiles = 14;
uint32_t bme_resistance[numProfiles], bme_resistance_avg[numProfiles];
uint16_t bmeInterval, bme_gas_avg;
uint8_t bmeProfile, bmeSamples, repeater, bmeProfilePause, bmeFilter;
uint16_t duration, heaterTemp;
double Altitude, samplingDelta;
#define offsetDelta 5684

const uint16_t heatProf_1[numProfiles] = {
  90,   // 0
  100,  // 1
  120,  // 2
  140,  // 3
  160,  // 4
  180,  // 5
  200,  // 6
  220,  // 7
  240,  // 8
  260,  // 9
  280,  // 10
  300,  // 11
  320,  // 12
  330,  // 13
};

const uint16_t durProf_1[numProfiles] = {
  8,  // 0
  8,  // 1
  8,  // 2
  8,  // 3
  8,  // 4
  8,  // 5
  8,  // 6
  8,  // 7
  8,  // 8
  8,  // 9
  8,  // 10
  8,  // 11
  8,  // 12
  8,  // 13
};



//SGP41__________________________________________________________________________________
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

uint8_t conditioning_duration = 30;
uint16_t sgpInterval = 1000;  // needs to be 1sec for the Algo

uint16_t sgpError, error;
char sgpErrorMsg[256];

constexpr uint16_t defaultRh = 0x8000;
constexpr uint16_t defaultT = 0x6666;
uint16_t srawVoc, srawNox;
int32_t VOC, NOX;

// SCD30 _______________________________
SCD30 scd30;

String lastSCDpoll;
float tempSCD, humidSCD, dewPoint;
uint16_t scdInterval = 60 * ONETHOUSAND;  // in s, stored in prefs
uint16_t co2SCD;
const float Da = 17.271, Db = 237.7;


// INA219B   __________________________________________________________________________
INA219 INA2(0x40);
float BUS2_BusVoltage;
float BUS2_ShuntVoltage;
float BUS2_Current;
float BUS2_Power;
bool BUS2_OVF;
bool BUS2_CNVR;
bool INA2_iscalibrated;

// Task Manager _______________________________________________________________________
taskid_t LOG, ST1, STATID, IMUID, TEMPID, INA2ID, BMEID, SGPID, SECID, NTPID, BTNID, CLKID, MENUID, WIFIID, SNSID, HOMEID, UTILID, TMID, SYSID, WEB, CUBEID, BLEID, SCDID, PSID;  // task IDs
double tmTracker, bmeTracker, sgpTracker, ina2Tracker, tempTracker, uTimeTracker, powerStTracker, loggingTracker, ntpTracker, clientTracker, statBaTracker, imuTracker, systemPageTracker, homePageTracker, sensorPageTracker, scdTracker, psTracker;

#define slotsSize 24
char taskFreeSlots[slotsSize];  // TaskManagerIO slots & status
String taskArray[slotsSize];    // converted String status of tasks

struct TaskData {
  const char* taskName;
  double* tracker;
  taskid_t* taskId;
};

TaskData tasks[] = {
  // System Tasks
  { "psWD", &psTracker, &PSID },
  { "updateTime", &uTimeTracker, &SECID },
  { "updateStat", &statBaTracker, &STATID },
  { "powerStates", &powerStTracker, &ST1 },
  { "pollServer", &clientTracker, &WEB },
  { "getNTP", &ntpTracker, &NTPID },
  { "pollTemp", &tempTracker, &TEMPID },
  { "pollINA_2", &ina2Tracker, &INA2ID },
  { "pollBME", &bmeTracker, &BMEID },
  { "pollSGP41", &sgpTracker, &SGPID },
  { "pollSCD30", &scdTracker, &SCDID },
  { "pollIMU", &imuTracker, &IMUID },
  { "logging", &loggingTracker, &LOG },
  { "systemPage", &systemPageTracker, &SYSID },
  { "homePage", &homePageTracker, &HOMEID },
  { "sensorPage", &sensorPageTracker, &SNSID },
};


void setup() {  // ________________ SETUP ___________________

  timeTracker = millis();

  //___________________________ INITIALIZE COMMS __________________________

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  /*
  if (DEBUG) {
    delay(50);  //relax...
    ESP_LOGI(TAG, "Serial bus Initialized.");
  }
*/

  if (Wire.begin(sda, scl, I2C_SPEED)) {  // sda= , scl= in pins_arduino.h
    delay(20);
    if (DEBUG) {
      ESP_LOGI(TAG, "I2C bus Initialized.");
    }
  }

  //_________________ INITIALIZE Preferences & WiFi ____________________

  // Credential Prefs
  preferences.begin("credentials", false);              // true: read-only,  false: read-write
  webHost = preferences.getString("webHost", webHost);  // hexpod
  webPw = preferences.getString("webPw", webPw);
  // WiFi.setHostname(hostname);
  WiFi.begin(preferences.getString("ssid", wifiSSID), preferences.getString("pw", wifiPW));
  preferences.end();

  // Sensor Prefs
  preferences.begin("my - app", false);                               // false: read-write
  loggingInterval = preferences.getUInt("logItvl", loggingInterval);  // in millis
  // serialPrintBME1 = preferences.getBool("bmelog", 0);
  bmeSamples = preferences.getUInt("bmeSpls", 1);
  bmeFilter = preferences.getUInt("bmeFilter", 0);
  bmeProfilePause = preferences.getUInt("bmePause", 0);

  // System Prefs
  LOGGING = preferences.getBool("logging", false);
  DEBUG = preferences.getBool("debug", 0);
  SLEEPENABLE = preferences.getBool("sleep", 0);
  OLEDon = preferences.getBool("oled", 1);

  restarts = preferences.getUInt("restarts", 0);
  restarts++;
  preferences.putUInt("restarts", restarts);
  preferences.end();

  getNTP();

  setupWebInterface();

  //___________________________ INITIALIZE TFT _________________________

  tft.init();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  tft.setTextDatum(TC_DATUM);
  tft.setTextPadding(100);

  //_________________________ INITIALIZE GPIO __________________________

  void IRAM_ATTR UDLR_ISR();
  void IRAM_ATTR CTR_ISR();
  void IRAM_ATTR SD_ISR();
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  pinMode(GPIO_NUM_0, INPUT_PULLUP);  // BUTTON
  attachInterrupt(GPIO_NUM_0, CTR_ISR, FALLING);
  pwm.attachPin(GPIO_NUM_1, ONETHOUSAND, 10);  // FAN, 1KHz, 10 bit
  pwm.writeScaled(FANvalue = 0.0);
  pwm.attachPin(GPIO_NUM_2, ONETHOUSAND, 10);  // BLK, 1KHz, 10 bit
  pwm.writeScaled(TFTbrightness = 1.0);
  pinMode(GPIO_NUM_3, OUTPUT);     // Key trigger for Power supply module workaround
  digitalWrite(GPIO_NUM_3, HIGH);  // set key to default state
                                   // pinMode(GPIO_NUM_7, INPUT_PULLDOWN);  // LIS3_INT
                                   // attachInterrupt(GPIO_NUM_7, CTR_ISR, RISING);
                                   // pinMode(GPIO_NUM_8, INPUT);   // temp
  pinMode(GPIO_NUM_12, INPUT);     // free
  // pinMode(GPIO_NUM_14, INPUT);  // free
  // pinMode(GPIO_NUM_21, INPUT);  // free
  pinMode(GPIO_NUM_38, INPUT_PULLUP);  // PCA_INT
  attachInterrupt(GPIO_NUM_38, UDLR_ISR, FALLING);
  pinMode(GPIO_NUM_39, INPUT_PULLDOWN);  // bat indicator lines - 25%
  pinMode(GPIO_NUM_40, INPUT_PULLDOWN);  // bat indicator lines - 50%
  pinMode(GPIO_NUM_41, INPUT_PULLDOWN);  // bat indicator lines - 75%
  pinMode(GPIO_NUM_42, INPUT_PULLDOWN);  // bat indicator lines - 100%
  // pinMode(GPIO_NUM_45, INPUT);  // free, bootstrap
  //pinMode(GPIO_NUM_46, INPUT);  // free, bootstrap
  pinMode(GPIO_NUM_47, INPUT_PULLUP);  // SD present
  attachInterrupt(GPIO_NUM_47, SD_ISR, CHANGE);
  pinMode(GPIO_NUM_48, INPUT);  // Battery Charge PIN


  //___________________________ INITIALIZE SD _________________________
  SDinserted = !digitalRead(GPIO_NUM_47);
  if (SDinserted) {  // DOES NOT WORK - suspect wiring or incompatibility with something else
    // sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    // sdSPI.setHwCs(true);
    // sdSPI.setFrequency(40000000);
    // sdSPI.setDataMode(SPI_MODE0);
    // mountSD();  // --> DataMgmt
  }

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
  // io.write(PCA95x5::Port::P16, PCA95x5::Level::L);  // free
  // io.write(PCA95x5::Port::P17, PCA95x5::Level::L);  // free

  //___________________________ INITIALIZE LIS3DH ______________________

  lis.begin(0x18);
  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // lis.setClick(2, 80);  // THRESHOLD 80
  lis.setRange(LIS3DH_RANGE_2_G);
  // lis.setDataRate(LIS3DH_DATARATE_LOWPOWER_5KHZ);
  lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);

  //___________________________ INITIALIZE DS18B20 _____________________

  tempSens.begin();
  tempSens.setWaitForConversion(false);
  tempSens.setResolution(10);  // set global resolution to 9, 10 (default), 11, or 12 bits
  // tempSens.setHighAlarmTemp(DTprobe[0].address, 65);  // Dallas tempProbe Alarm Thresholds
  // tempSens.setLowAlarmTemp(DTprobe[0].address, -1);

  //___________________________ INITIALIZE INA219 _____________________

  if (INA2.begin()) {
    INA2.setMaxCurrentShunt(1.0, 0.001);  //  maxCurrent, shunt
    INA2.setBusVoltageRange(16);          // 16,32
    INA2.setGain(4);                      // 1,2,4,8
    INA2.setMode(7);                      // 3 setModeShuntBusTrigger()
    INA2_iscalibrated = INA2.isCalibrated();

    // INA2.setModeADCOff();
    // INA2.setBusADC(3);
    // INA2.setShuntADC(3);
    // INA2.setMode(7);
    // INA2.setModeShuntBusTrigger();
  }

  //___________________________ INITIALIZE BME688 _____________________

  bme1.begin(0x77, Wire);

  if (bme1.checkStatus()) {
    if (bme1.checkStatus() == BME68X_ERROR) {
      BME_ERROR = "Error: " + bme1.statusString();
      return;
    } else if (bme1.checkStatus() == BME68X_WARNING) {
      BME_ERROR = "Warning: " + bme1.statusString();
    }
  }

  bmeInterval = loggingInterval;
  bme1.setTPH(BME68X_OS_2X, BME68X_OS_8X, BME68X_OS_4X);
  bme1.fetchData();
  bme1.getData(bme1_data);
  bme1.setOpMode(BME68X_SLEEP_MODE);

  //___________________________ INITIALIZE SGP41 _____________________

  sgp41.begin(Wire);
  configSGP();  //  /funct.ino -> bottom
  sgp41.turnHeaterOff();


  //___________________________ INITIALIZE SCD30 _____________________

  if (scd30.begin(Wire, false, true)) {
    scdInterval = loggingInterval / ONETHOUSAND;
    if (DEBUG) { scd30.enableDebugging(Serial); }
    scd30.setMeasurementInterval(scdInterval);
    scd30.readMeasurement();
  } else {
    Serial.println("SCD30 not detected. Please check wiring.");
  }
  if (!LOGGING) { scd30.StopMeasurement(); }

  //___________________________ INITIALIZE OLED ________________________

  if (u8g2.begin()) {
    if (OLEDon) {
      u8g2.setFontDirection(0);
      u8g2.setFontMode(0);
      u8g2.setFont(u8g2_font_logisoso28_tn);  // u8g2_font_u8glib_4_tf

      if (DEBUG) {
        u8g2.clearBuffer();
        u8g2.drawStr(oledDatum_X, oledDatum_Y, "OLED Good.");
        u8g2.sendBuffer();
      }
    } else {
      u8g2.setPowerSave(1);
    }
  }
  //___________________________ TASK MANAGER ________________________

  esp_chip_info(&chip_info);
  getDeviceInfo();
  initTM();  // setup task Manager Tasks

  triggerPSwatchdog();

  lastInputTime = micros();
  lastRestart = printTime + " " + printDate;
  logFilePath = rootHexPath + "/LOG_" + printDate + ".csv";
  resetReasonString = print_wakeup_reason();

  addLOGmarker("[R]", resetReasonString);

  //___________________________ END REPORT _____________________________
  TAG = "ESP";
  if (DEBUG) {
    ESP_LOGI(TAG, "Reset Reason: %s", resetReasonString);
    ESP_LOGI(TAG, "%d Restarts:", restarts);
    ESP_LOGI(TAG, "Setup took: %.3fs\n\n", (micros() - timeTracker) / ONEMILLION);
    ESP_LOGI(TAG, "WiFi IP: %s", WiFiIP);
  }
}



void loop() {
  taskManager.runLoop();
}
