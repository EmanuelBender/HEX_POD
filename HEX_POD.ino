
#define codeRevision "a11.3"

#include <Arduino.h>
#include <Wire.h>
#include "SPI.h"
#include <WiFi.h>
#include <ArduinoBLE.h>
#include <Preferences.h>
#include "time.h"

#include "FS.h"
#include <LittleFS.h>
#include "SD.h"

#include "TFT_eSPI.h"
#include <U8g2lib.h>
#include <PCA95x5.h>
#include <ESP32Servo.h>

#include <Adafruit_Sensor.h>
#include "INA219.h"
#include "bme68xLibrary.h"
#include <SensirionI2CSgp41.h>
#include <VOCGasIndexAlgorithm.h>
#include <NOxGasIndexAlgorithm.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_LIS3DH.h>

#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp_sntp.h"
WebServer server(80);
uint16_t webServerPollMs = 120;

// ________________  ESP32 UTILITY  ____________________

#include <stdio.h>
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "driver/i2c.h"
// #include <esp_system.h>
#include "esp_log.h"
#include <esp_cpu.h>
#include <stdexcept>
#include <sstream>
#include <inttypes.h>

#include "TaskManagerIO.h"
#include <TimeLib.h>

esp_chip_info_t chip_info;
esp_reset_reason_t resetReason;
String resetReasonString;
String wakeupReasonString;

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



// ___________________________________  TIME  ________________________________
const char* hostname = "[HEX]POD";
const int WiFiTimeout = 5000;
String WiFiIP;
String webHost, webPw;
const char* ntpServer1 = "0.pool.ntp.org";  // NTP Time server
const char* ntpServer2 = "time.nist.gov";   // fallback
const char* ntpServer3 = "1.pool.ntp.org";  // fallback
const byte SECONDS_IN_MINUTE = 60;
const byte MINUTES_IN_HOUR = 60;
const byte HOURS_IN_DAY = 24;
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

//___________________________________  GENERAL  ______________________________
Preferences preferences;

// #define ENABLE_I2C_DEBUG_BUFFER
bool DEBUG = false;
bool LOGGING = true;
bool pastLOGGINGstate = !LOGGING;
bool SLEEPENABLE;
bool serialPrintLOG;
bool serialPrintBME1;

SPIClass sdSPI = SPIClass(HSPI);
#define SD_MISO 13
#define SD_MOSI 14
#define SD_SCLK 12
#define SD_CS 21
// #define SPI_FREQUENCY 60000000       // 80000000 tested ok
// #define SPI_READ_FREQUENCY 30000000  // 40000000 tested ok

#define sda GPIO_NUM_18
#define scl GPIO_NUM_17
#define I2C_SPEED 400000  // 800000   tested ok

uint32_t loggingInterval;        // stored in Prefs
uint16_t getNTPInterval = 1800;  // 600 = 10 mins, 1800 = 30 mins

const byte consoleColumns = 20;
const byte consoleRows = 54;
const byte menuRowM = 26;

uint8_t consoleLine;
String console[consoleRows][consoleColumns];
// String SDarray[consoleRows][consoleColumns];

const uint32_t ONEMILLION = 1000000;
const uint16_t ONETHOUSAND = 1000;
const double KILOBYTE = 1024.0;
const double ONEMILLIONB = KILOBYTE * KILOBYTE;

multi_heap_info_t deviceInfo;
uint32_t free_flash_size, flash_size, program_size, program_free, program_used, SPIFFS_size, SPIFFS_used, SPIFFS_free, out_size;
double percentLeftLFS, percentUsedLFS, program_UsedP, program_LeftP, flash_UsedP, flash_LeftP, CPUTEMP;
long int cpu_freq_mhz, cpu_xtal_mhz, cpu_abp_hz, flash_speed;
int chiprevision;
bool LEDon, FANon, isFading, OLEDon, SDinserted;
uint8_t filesCount, directoryCount, fileId;
String logFilePath, rootHexPath = "/.sys";

String TAG = "ESP";

const String logHeader = "Time, BME_0, BME_1, BME_2, BME_3, BME_4, BME_5, BME_6, BME_7, BME_8, BME_9, BME_10, BME_11, BME_12, BME_13, BME_T, BME_H, BME_P, SGP_VOC, SGP_NOX, SGP_rVOC, SGP_rNOX\n";
const String logColumns[22] = { "Time", "BME_0", "BME_1", "BME_2", "BME_3", "BME_4", "BME_5", "BME_6", "BME_7", "BME_8", "BME_9", "BME_10", "BME_11", "BME_12", "BME_13", "BME_T", "BME_H", "BME_P", "SGP_VOC", "SGP_NOX", "SGP_rVOC", "SGP_rNOX" };
const byte log_idx_bme1_temp = 15;   // index in log file
const byte log_idx_bme1_humid = 16;  // index in log file
const byte log_idx_bme1_press = 17;  // index in log file
const byte log_idx_sgp_voc = 18;
const byte log_idx_sgp_nox = 19;
uint16_t chart_max_data = 60;
String restartHeader;

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

const char* menuOptions[] = {
  "Home",
  "Sensors",
  "Utility",
  "Task Manager",
  "System",
};

uint32_t idleDelay = 20000 * ONETHOUSAND;
uint32_t powersaveDelay = 240000 * ONETHOUSAND;  // 3 min
// uint32_t lightsleepDelay = 600000 * ONETHOUSAND; // 10 min

const float pi = 3.14159265358979323846264338327950;
uint16_t i;

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
const byte menuItems = 5;
byte carousel = 1, lastCarousel = 0, highlightIndex = 0, lastHighlightIndex = 1;
byte sysIndex = 0;
byte utilIndex = 1;
const byte tmIndexY = 60;
const byte radius = 5;
String label, value;

//OLED______________________________________________________________________
U8G2_SH1106_128X32_VISIONOX_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
const byte oledDatum_X = 0;
const byte oledDatum_Y = 32;

//LIS3______________________________________________________________________
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
int16_t X, Y, Z;  // 16bit
// #define CLICKTHRESHHOLD 80
uint16_t imuInterval;
uint8_t imuSampleCount;
const uint8_t imuAvg = 55;        // for cube demo
const uint16_t mapRange = 16384;  // accelrange 2 = 16384, 4 = 8096, 8 = 1024, 16 = 256

//DS18B20___________________________________________________________________
OneWire oneWire(GPIO_NUM_8);
DallasTemperature tempSens(&oneWire);
DeviceAddress DTprobe_1 = { 0x28, 0x12, 0xEF, 0x75, 0xD0, 0x01, 0x3C, 0x77 };
byte DTdevice;

float tempValues[2];
const char* tempID[] = {
  "ESP_prox",
  ""
};


//BME688___________________________________________________________________
#define NEW_GAS_MEAS (BME68X_GASM_VALID_MSK | BME68X_HEAT_STAB_MSK | BME68X_NEW_DATA_MSK)
bme68xData data;
Bme68x bme;

String BME_ERROR, lastBMEpoll;
const byte numProfiles = 14;
uint32_t bme_resistance[numProfiles], bme_resistance_avg[numProfiles];
uint32_t bmeInterval, bme_gas_avg, offsetDelta;
uint8_t bmeProfile, bmeSamples, repeater, bmeProfilePause, bmeFilter;
uint16_t duration, heaterTemp;
double Altitude, samplingDelta;

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
  10,  // 0
  10,  // 1
  10,  // 2
  10,  // 3
  10,  // 4
  10,  // 5
  10,  // 6
  10,  // 7
  10,  // 8
  10,  // 9
  10,  // 10
  10,  // 11
  10,  // 12
  10,  // 13
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
taskid_t LOG, ST1, STATID, IMUID, TEMPID, INA2ID, BMEID, SGPID, SECID, NTPID, BTNID, CLKID, MENUID, WIFIID, SNSID, HOMEID, UTILID, TMID, SYSID, WEB;  // task IDs
double bmeTracker, sgpTracker, ina2Tracker, tempTracker, uTimeTracker, powerStTracker, loggingTracker, ntpTracker, clientTracker, statBaTracker, imuTracker;
uint32_t tmTracker;

const byte slotsSize = 24;
char taskFreeSlots[slotsSize];  // TaskManagerIO slots & status
String taskArray[slotsSize];    // converted String status of tasks

struct TaskData {
  const char* taskName;
  double* tracker;
  taskid_t* taskId;
  //String* lastPoll;
};

TaskData tasks[] = {
  { "updateTime", &uTimeTracker, &SECID },
  { "updateStat", &statBaTracker, &STATID },
  { "getNTP", &ntpTracker, &NTPID },
  { "pollTemp", &tempTracker, &TEMPID },
  { "pollINA", &ina2Tracker, &INA2ID },
  { "powerStates", &powerStTracker, &ST1 },
  { "pollServer", &clientTracker, &WEB },
  { "pollIMU", &imuTracker, &IMUID },
  { "pollBME", &bmeTracker, &BMEID },  // &lastBMEpoll
  { "pollSGP", &sgpTracker, &SGPID },  // &lastSGPpoll
  { "logging", &loggingTracker, &LOG }
};


void setup() {  // ________________ SETUP ___________________

  timeTracker = millis();

  //___________________________ INITIALIZE COMMS __________________________

  if (DEBUG) {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    delay(50);  //relax...
    ESP_LOGI(TAG, "Serial bus Initialized.");
  }


  if (Wire.begin(sda, scl, I2C_SPEED)) {  // sda= , scl= in pins_arduino.h
    delay(20);
    if (DEBUG) {
      ESP_LOGI(TAG, "I2C bus Initialized.");
    }
  }

  //_________________ INITIALIZE Preferences & WiFi ____________________

  // Credential Prefs
  preferences.begin("credentials", true);      // true : read only
  webHost = preferences.getString("webHost");  // hexpod
  webPw = preferences.getString("webPw");      //
  WiFi.setHostname(hostname);                  //define hostname
  WiFi.begin(preferences.getString("ssid", "NaN"), preferences.getString("pw", "NaN"));
  preferences.end();

  // Sensor Prefs
  preferences.begin("my - app", false);                        // read-only bool
  loggingInterval = preferences.getUInt("logItvl", 30000);     // in microsec
  conditioning_duration = (loggingInterval / ONEMILLION) * 3;  // in sec
  serialPrintBME1 = preferences.getBool("bmelog", 0);
  bmeSamples = preferences.getUInt("bmeSpls", 1);
  bmeFilter = preferences.getUInt("bmeFilter", 0);
  bmeProfilePause = preferences.getUInt("bmePause", 0);

  // System Prefs
  LOGGING = preferences.getBool("logging", false);
  DEBUG = preferences.getBool("debug", 1);
  SLEEPENABLE = preferences.getBool("sleep", 0);
  OLEDon = preferences.getBool("oled", 1);
  restarts = preferences.getUInt("restarts", 0);
  restarts++;
  preferences.putUInt("restarts", restarts);
  preferences.end();

  getDeviceInfo();

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

  pinMode(GPIO_NUM_0, INPUT_PULLUP);  // BUTTON
  attachInterrupt(GPIO_NUM_0, CTR_ISR, FALLING);

  pinMode(GPIO_NUM_1, OUTPUT);                 // FAN_CTL
  pwm.attachPin(GPIO_NUM_2, ONETHOUSAND, 12);  // BLK, 1KHz, 12 bit
  pwm.writeScaled(TFTbrightness = 1.0);

  // pinMode(GPIO_NUM_3, INPUT);
  // pinMode(GPIO_NUM_7, INPUT_PULLDOWN);  // LIS3_INT
  // attachInterrupt(GPIO_NUM_7, CTR_ISR, RISING);
  // pinMode(GPIO_NUM_8, INPUT);   // free
  // pinMode(GPIO_NUM_14, INPUT);  // free
  // pinMode(GPIO_NUM_21, INPUT);  // free
  pinMode(GPIO_NUM_38, INPUT_PULLUP);  // PCA_INT
  attachInterrupt(GPIO_NUM_38, UDLR_ISR, FALLING);
  // pinMode(GPIO_NUM_45, INPUT);  // free, bootstrap
  //pinMode(GPIO_NUM_46, INPUT);  // free, bootstrap
  pinMode(GPIO_NUM_47, INPUT_PULLUP);  // SD present
  attachInterrupt(GPIO_NUM_47, SD_ISR, CHANGE);
  // pinMode(GPIO_NUM_48, INPUT);  // free


  //___________________________ INITIALIZE SD _________________________

  if (SDinserted) {  // DOES NOT WORK - suspect wiring or incompatibility with something else
    sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    // sdSPI.setHwCs(true);
    // sdSPI.setFrequency(40000000);
    // sdSPI.setDataMode(SPI_MODE0);
    mountSD();  // --> DataMgmt
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
  // io.write(PCA95x5::Port::P16, PCA95x5::Level::L);  // PER_PWR On
  // io.write(PCA95x5::Port::P17, PCA95x5::Level::L);  // SNS_PWR On

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
  tempSens.setResolution(9);                 // set global resolution to 9, 10 (default), 11, or 12 bits
  tempSens.setHighAlarmTemp(DTprobe_1, 65);  // Dallas tempProbe Alarm Thresholds
  tempSens.setLowAlarmTemp(DTprobe_1, -1);

  DTdevice = tempSens.getDeviceCount();

  //___________________________ INITIALIZE INA219 _____________________

  if (INA2.begin()) {
    INA2.setMaxCurrentShunt(1, 0.001);  //  maxCurrent, shunt
    INA2.setBusVoltageRange(16);        // 16,32
    INA2.setGain(2);                    // 1,2,4,8
    INA2.setMode(7);                    // 3 setModeShuntBusTrigger()
    INA2_iscalibrated = INA2.isCalibrated();

    // INA2.setModeADCOff();
    // INA2.setBusADC(3);
    // INA2.setShuntADC(3);
    // INA2.setMode(7);
    // INA2.setModeShuntBusTrigger();
  }

  //___________________________ INITIALIZE BME688 _____________________

  bme.begin(0x77, Wire);

  if (bme.checkStatus()) {
    if (bme.checkStatus() == BME68X_ERROR) {
      BME_ERROR = "Error: " + bme.statusString();
      return;
    } else if (bme.checkStatus() == BME68X_WARNING) {
      BME_ERROR = "Warning: " + bme.statusString();
    }
  }

  bmeInterval = loggingInterval;
  bme.setTPH(BME68X_OS_2X, BME68X_OS_8X, BME68X_OS_4X);
  bme.fetchData();
  bme.getData(data);
  bme.setOpMode(BME68X_SLEEP_MODE);

  //___________________________ INITIALIZE SGP41 _____________________

  sgp41.begin(Wire);
  configSGP();  //  /funct.ino -> bottom

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

  //___________________________ TASK MANAGER ________________________

  launchUtility();  // setup tasks, launch utility Menu

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




// Templates

String formatTime(const int value1, const int value2, const int value3, const char seperator) {  // format values into Time or date String with seperator

  char buffer[9];
  sprintf(buffer, "%02d%c%02d%c%02d", value1, seperator, value2, seperator, value3);

  return buffer;
}


template<typename T>
T convertSecToTimestamp(const uint32_t pass_sec) {  // Convert a seconds value to HH:MM:SS
  if (pass_sec == 0) {
    throw std::invalid_argument("Invalid seconds value");
  }
  char buffer[9];
  int hours = pass_sec / 3600;
  int minutes = (pass_sec % 3600) / MINUTES_IN_HOUR;
  int seconds = pass_sec % SECONDS_IN_MINUTE;

  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);

  return T(buffer);
}


template<typename T, size_t N>
T findSmallestValue(const T (&array)[N]) {  // find smalles value in a 1D Array
  if (N <= 0) {
    throw std::out_of_range("Array index out of bounds");
  }

  T smallestValue = array[0];  // Initialize with the first element

  for (size_t i = 1; i < N; ++i) {
    if (array[i] < smallestValue) {
      smallestValue = array[i];
    }
  }

  return smallestValue;
}



template<typename T, size_t Rows, size_t Columns>
void empty2DArray(T (&array)[Rows][Columns]) {
  if (Rows == 0 || Columns == 0) {
    throw std::invalid_argument("Invalid array dimensions");
  }

  for (size_t row = 0; row < Rows; ++row) {
    for (size_t column = 0; column < Columns; ++column) {
      array[row][column] = T();  // Use default constructor if available
    }
  }
}


template<size_t Rows, size_t Columns>
void empty2DArray(String (&array)[Rows][Columns]) {
  if (Rows == 0 || Columns == 0) {
    throw std::invalid_argument("Invalid array dimensions");
  }

  for (size_t row = 0; row < Rows; ++row) {
    for (size_t column = 0; column < Columns; ++column) {
      array[row][column] = String();  // Use String's default constructor
    }
  }
}