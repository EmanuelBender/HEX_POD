
#include <pgmspace.h>


void pollServer() {
  TAG = "pollWeb()      ";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  server.handleClient();

  taskManager.yieldForMicros(ONETHOUSAND);

  debugF(timeTracker);
  clientTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}


void IRAM_ATTR CTR_ISR() {
  BUTTON = true;
  BTNID = taskManager.schedule(onceMicros(3), pollButtons);
}
void IRAM_ATTR UDLR_ISR() {
  INT_TRGR = true;
  BTNID = taskManager.schedule(onceMicros(1), pollButtons);
}
void IRAM_ATTR SD_ISR() {
  if (millis() - lastSDInterrupt > 110) {
    lastSDInterrupt = millis();
    SDinserted = !digitalRead(GPIO_NUM_47);
    if (DEBUG) ESP_LOGI("SD", "%s", SDinserted ? "Inserted" : "Removed");
  }
}



void initTM() {
  TAG = "initTM()    ";
  taskManager.reset();
  if (DEBUG) ESP_LOGI(TAG, "%s", "taskManager Reset");

  lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);

  LOG = ST1 = STATID = IMUID = TEMPID = INA2ID = BMEID = SGPID = SECID = NTPID = BTNID = CLKID = MENUID = WIFIID = SNSID = HOMEID = UTILID = TMID = SYSID = WEB = BLEID = CUBEID = SCDID = ZERO;
  bmeTracker = sgpTracker = ina2Tracker = tempTracker = uTimeTracker = powerStTracker = loggingTracker = ntpTracker = clientTracker = statBaTracker = imuTracker = systemPageTracker = homePageTracker = sensorPageTracker = scdTracker = ZERO;

  SECID = taskManager.schedule(repeatMillis(994), updateTime);
  STATID = taskManager.schedule(repeatMillis(996), statusBar);
  ST1 = taskManager.schedule(repeatSeconds(1), PowerStates);
  WEB = taskManager.schedule(repeatMillis(webServerPollMs), pollServer);
  NTPID = taskManager.schedule(repeatSeconds(getNTPInterval), getNTP);
  TEMPID = taskManager.schedule(repeatMillis(984), pollTemp);
  INA2ID = taskManager.schedule(repeatMillis(500), pollINA2);

  initAirSensorTasks();

  if (!blockMenu) taskManager.schedule(onceMicros(1), reloadMenu);
}



void initAirSensorTasks() {

  if (LOGGING) {
    BMEID = taskManager.schedule(repeatMillis(bmeInterval / bmeSamples), pollBME);
    SGPID = taskManager.schedule(repeatMillis(sgpInterval), pollSGP);
    SCDID = taskManager.schedule(repeatMillis(loggingInterval), pollSCD30);
    LOG = taskManager.schedule(repeatMillis(loggingInterval), logging);
    if (LOGGING != pastLOGGINGstate) {  // initialize after LOGGING toggle
      pastLOGGINGstate = LOGGING;
      conditioning_duration = 30;
      taskManager.schedule(onceSeconds(1), pollBME);  // conditioning testing
                                                      // taskManager.schedule(onceSeconds(5), pollBME);
      taskManager.schedule(onceSeconds(10), pollBME);
      // taskManager.schedule(onceSeconds(20), pollBME);
      taskManager.schedule(onceSeconds(35), pollBME);
      // taskManager.schedule(onceSeconds(50), pollBME);
      taskManager.schedule(onceSeconds(10), pollSCD30);
      taskManager.schedule(onceSeconds(20), pollSCD30);
      taskManager.schedule(onceSeconds(30), pollSCD30);
      taskManager.schedule(onceSeconds(50), pollSCD30);
    }
  }
}


void PowerStates() {
  TAG = "PowerStates()";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  /* if (timeTracker - lastInputTime > deepsleepDelay) {
    currentPowerState = DEEP_SLEEP;
  } else */
  /* if (timeTracker - lastInputTime > lightsleepDelay) {
    currentPowerState = LIGHT_SLEEP;
  } else */
  if (timeTracker - lastInputTime > powersaveDelay) {
    currentPowerState = POWER_SAVE;
  } else if (timeTracker - lastInputTime > idleDelay) {
    currentPowerState = IDLE;
  } else {
    currentPowerState = NORMAL;
  }


  if (currentPowerState != previousPowerState) {

    if (currentPowerState == LIGHT_SLEEP) {
      Wire.end();
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      u8g2.clearBuffer();
      u8g2.sleepOn();
      sgp41.turnHeaterOff();
      lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // 1 = High, 0 = Low
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_38, 0);
      // rtc_gpio_hold_en(GPIO_NUM_1);  // fan
      esp_light_sleep_start();

    } else if (currentPowerState == POWER_SAVE) {
      setCpuFrequencyMhz(80);
      webServerPollMs = 600;
      taskManager.cancelTask(WEB);
      WEB = taskManager.schedule(repeatMillis(webServerPollMs), pollServer);
      while (TFTbrightness > 0.0) {
        pwm.writeScaled(TFTbrightness -= 0.01);
        delay(3);
      }

    } else if (currentPowerState == IDLE) {
      setCpuFrequencyMhz(160);
      while (TFTbrightness > 0.3) {
        pwm.writeScaled(TFTbrightness -= 0.01);
        delay(1);
      }

    } else if (currentPowerState == NORMAL) {
      setCpuFrequencyMhz(240);
      webServerPollMs = 100;
      taskManager.cancelTask(WEB);
      pwm.writeScaled(TFTbrightness = 1.0);
      WEB = taskManager.schedule(repeatMillis(webServerPollMs), pollServer);
    }
  }

  previousPowerState = currentPowerState;

  debugF(timeTracker);
  powerStTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}




void pollButtons() {

  pollMultiplexer();
  TAG = "pollButtons()";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  if (micros() - lastInputTime <= 250000) {  // 250ms in microseconds
    return;
  }

  if (INT_TRGR) {
    UP = P0[11];
    DOWN = P0[8];
    LEFT = P0[9];
    RIGHT = P0[10];
    INT_TRGR = false;
  }

  if (UP || DOWN || LEFT || RIGHT || BUTTON || CLICK) {
    menuTrigger = true;
    lastInputTime = micros();
    taskManager.schedule(onceMicros(500), statusBar);

    if (LEFT) {
      LEFT = CLICK_LEFT = false;
      lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);
      if (blockMenu) {
        switch (carousel) {
          case 1:
            taskManager.cancelTask(HOMEID);
            homePageTracker = HOMEID = 0;
            break;
          case 2:
            taskManager.cancelTask(SNSID);
            taskManager.cancelTask(IMUID);
            delay(100);
            imuTracker = sensorPageTracker = SNSID = IMUID = 0;
            break;
          case 3:
            UTILID = 0;
            break;
          case 5:
            SYSID = systemPageTracker = 0;
            break;
        }
      }
      blockMenu = false;
      tft.fillScreen(TFT_BLACK);
      taskManager.schedule(onceMicros(1), reloadMenu);
      return;
    }

    if (!blockMenu) {
      if (DOWN || CLICK_DOWN) {
        DOWN = CLICK_DOWN = false;
        lastCarousel = carousel;
        carousel = (carousel % menuItems) + 1;
        taskManager.schedule(onceMicros(2), reloadMenu);
        return;
      }
      if (UP) {
        UP = false;
        lastCarousel = carousel;
        carousel = (carousel - 2 + menuItems) % menuItems + 1;
        taskManager.schedule(onceMicros(2), reloadMenu);
        return;
      }
      if (RIGHT || BUTTON || CLICK) {
        RIGHT = BUTTON = CLICK = false;
        blockMenu = true;
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(TL_DATUM);

        switch (carousel) {
          case 1:
            HOMEID = taskManager.schedule(repeatMillis(985), homePage);
            taskManager.schedule(onceMicros(5), homePage);
            break;
          case 2:
            for (i = 1; i < 9; i++) {
              tft.drawRoundRect(5, 22 + (i * menuRowM), TFT_WIDTH - 14, 24, radius, TFT_DARKGREY);
            }
            tft.drawString("Sensors", 15, 15, 4);
            lis.setDataRate(LIS3DH_DATARATE_LOWPOWER_1K6HZ);
            IMUID = taskManager.schedule(repeatMillis(5), pollIMU);
            SNSID = taskManager.schedule(repeatMillis(50), sensorPage);
            break;
          case 3:
            tft.drawString("Utility", 15, 15, 4);
            for (i = 1; i <= 9; i++) {
              tft.drawRoundRect(16, 25 + (i * menuRowM), TFT_WIDTH - 32, 24, radius, TFT_DARKGREY);
            }
            UTILID = taskManager.schedule(onceMicros(10), utilPage);
            break;
          case 4:
            tft.drawString("TaskManager", 15, 15, 4);
            taskManager.schedule(onceMicros(50), taskM);
            break;
          case 5:
            SYSID = taskManager.schedule(onceMicros(10), systemPage);
            break;
          default:
            break;
        }
      }
    } else {
      if (carousel == 3) UTILID = taskManager.schedule(onceMicros(10), utilPage);
      if (carousel == 5) SYSID = taskManager.schedule(onceMicros(10), systemPage);
    }
  }
  debugF(timeTracker);
}





void updateTime() {
  TAG = "updateTime() ";
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
  timeTracker = micros();

  if (getLocalTime(&timeinfo)) {
    printTime = formatTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, ':');
    printDate = formatTime(timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year - 100, '.');
    uptimeString = convertSecToTimestamp<String>(millis() / ONETHOUSAND);

    printToOLED(printTime);
  }

  debugF(timeTracker);
  uTimeTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}




void getNTP() {
  TAG = "getNTP()  ";
  ntpTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
  // if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();

  while (WiFi.status() != WL_CONNECTED) {
    yield();
    delay(500);
    if (micros() - ntpTracker > (WiFiTimeout * ONETHOUSAND)) {
      // ESP_LOGE("NTP", "WiFi Connect Timeout. Check settings.");
      WiFi.reconnect();
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    configTzTime(time_zone, ntpServer1, ntpServer2, ntpServer3);  // gets the time from the NTP server
    updateTime();
    lastNTPtime = printTime + " " + printDate;
    WiFiIP = WiFi.localIP().toString();
  } else {
    ESP_LOGE("NTP", "Update failed.");
    lastNTPtimeFail = printTime + " " + printDate;
  }

  debugF(ntpTracker);
  ntpTracker = (micros() - ntpTracker) / double(ONETHOUSAND);
}





void pollIMU() {
  TAG = "pollIMU()   ";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  lis.read();
  X = lis.x;
  Y = lis.y;
  Z = lis.z;

  /*
  sensors_event_t event;
  lis.getEvent(&event);
  X = event.acceleration.x;
  Y = event.acceleration.y;
  Z = event.acceleration.z;
*/
  debugF(timeTracker);
  imuTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}





void pollINA2() {
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  // if (INA2.getConversionFlag()) {
  // BUS2_CNVR = true;
  TAG = "pollINA2()   ";
  timeTracker = micros();

  BUS2_BusVoltage = INA2.getBusVoltage_mV();
  BUS2_ShuntVoltage = INA2.getShuntVoltage_mV();
  BUS2_Current = INA2.getCurrent_mA();
  BUS2_Power = INA2.getPower_mW();
  // BUS2_OVF = INA2.getMathOverflowFlag();

  debugF(timeTracker);
  ina2Tracker = (micros() - timeTracker) / double(ONETHOUSAND);
  //  }
}





void pollBME() {
  TAG = "pollBME2()   ";
  timeTracker = micros();
  lastBMEpoll = printTime;
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  if (bme1.checkStatus() == BME68X_ERROR) {
    BME_ERROR = "[E] " + bme1.statusString();
    return;
  } else if (bme1.checkStatus() == BME68X_WARNING) {
    BME_ERROR = "[W] " + bme1.statusString();
  }

  bme1.setTPH(BME68X_OS_4X, BME68X_OS_8X, BME68X_OS_4X);
  bme1.setFilter(bmeFilter);
  bme1.setAmbientTemp(DTprobe[1].temperature);

  dewPoint = calcDewPoint(bme1_data.temperature, bme1_data.humidity);
  // Altitude = ((((((10 * log10((data.pressure / 100.0) / 1013.25)) / 5.2558797) - 1) / (-6.8755856 * pow(10, -6))) / ONETHOUSAND) * 0.30);  // approx, far from accurate

  repeater++;

  for (bmeProfile = 0; bmeProfile < numProfiles; bmeProfile++) {
    duration = durProf_1[bmeProfile];
    heaterTemp = heatProf_1[bmeProfile];

    delay(bmeProfilePause);
    bme1.setHeaterProf(heaterTemp, duration);
    bme1.setOpMode(BME68X_FORCED_MODE);

    while (!bme1.fetchData()) {
      taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
      delay(3);
    }
    bme1.getData(bme1_data);
    if (!conditioning_duration) bme_resistance[bmeProfile] += bme1_data.gas_resistance;
  }


  if (repeater == bmeSamples) {
    bme_gas_avg = ZERO;
    if (!conditioning_duration) {
      // offsetDelta = findSmallestValue(bme_resistance_avg);
      for (i = ZERO; i < numProfiles; ++i) {
        bme_resistance_avg[i] = (bme_resistance[i] / bmeSamples) - offsetDelta;
        bme_gas_avg += bme_resistance_avg[i];
      }
      bme_gas_avg = (bme_gas_avg / numProfiles);


      std::fill_n(bme_resistance, numProfiles, 0);  // empty resistance array
    }
    repeater = ZERO;
  }

  bme1.setOpMode(BME68X_SLEEP_MODE);

  debugF(timeTracker);
  bmeTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}






void pollSGP() {
  TAG = "pollSGP()    ";
  timeTracker = micros();
  lastSGPpoll = printTime;
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  auto compensationT = static_cast<uint16_t>((DTprobe[1].temperature + 45) * 65535 / 175);
  auto compensationRh = static_cast<uint16_t>(bme1_data.humidity * 65535 / 100);

  if (conditioning_duration) conditioning_duration--;

  if (!conditioning_duration) {
    sgp41.measureRawSignals(compensationRh, compensationT, srawVoc, srawNox);  // throw away conditioning
    yield();
    delay(142);
    error = sgp41.measureRawSignals(compensationRh, compensationT, srawVoc, srawNox);
    if (srawVoc) VOC = voc_algorithm.process(srawVoc);
    if (srawNox) NOX = nox_algorithm.process(srawNox);
    sgp41.turnHeaterOff();
  } else if (conditioning_duration > conditioning_duration / 3) {
    error = sgp41.executeConditioning(compensationRh, compensationT, srawVoc);  // defaultRh, defaultT
    voc_algorithm.process(srawVoc);
    nox_algorithm.process(srawNox = 16500);
  }

  if (error) errorToString(error, sgpErrorMsg, sizeof(sgpErrorMsg));
  debugF(timeTracker);
  sgpTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}


void configSGP() {
  // error = sgp41.turnHeaterOff();
  error = sgp41.executeSelfTest(sgpError);
  if (error) errorToString(error, sgpErrorMsg, sizeof(sgpErrorMsg));

  index_offset = 100;
  learning_time_offset_hours = 12;
  learning_time_gain_hours = 12;
  gating_max_duration_minutes = 180;
  std_initial = 50;
  gain_factor = 230;

  voc_algorithm.set_tuning_parameters(
    index_offset, learning_time_offset_hours, learning_time_gain_hours,
    gating_max_duration_minutes, std_initial, gain_factor);

  nox_algorithm.set_tuning_parameters(
    index_offset, learning_time_offset_hours, learning_time_gain_hours,
    gating_max_duration_minutes, std_initial, gain_factor);
}






void pollTemp() {
  TAG = "pollTemp()   ";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  if (tempSens.isConversionComplete()) {
    for (auto &probe : DTprobe) {
      probe.temperature = tempSens.getTempC(probe.address);
      yield();
    }
    tempSens.requestTemperatures();
  }

  if (DEBUG) oneWireSearch(GPIO_NUM_8);
  CPUTEMP = temperatureRead();

  debugF(timeTracker);
  tempTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}




void pollSCD30() {
  lastSCDpoll = printTime;
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  if (scd30.dataAvailable()) {
    scd30.readMeasurement();
    humidSCD = scd30.getHumidity();
    tempSCD = scd30.getTemperature();
    co2SCD = scd30.getCO2();

    scd30.setAltitudeCompensation(int(Altitude));               // meters
    scd30.setAmbientPressure(int(bme1_data.pressure / 100.0));  // mBar
    scdInterval = loggingInterval / ONETHOUSAND;
    scd30.setMeasurementInterval(scdInterval);

    double scdTempOffset = tempSCD - ((bme1_data.temperature + DTprobe[1].temperature) / 2);
    if (scdTempOffset) scd30.setTemperatureOffset(scdTempOffset);
  }

  debugF(timeTracker);
  scdTracker = (micros() - timeTracker) / double(ONETHOUSAND);
}


float calcDewPoint(double celsius, double humidity) {
  float temp = (Da * celsius) / (Db + celsius) + log(humidity * 0.01);
  return (Db * temp) / (Da - temp);
}




void debugF(uint32_t tracker) {

  if (DEBUG) {
    elapsedTime = (micros() - tracker) / double(ONETHOUSAND);
    ESP_LOGI(TAG, "%.3fms", elapsedTime);

    console[consoleLine][0] = printTime;
    console[consoleLine][1] = String(tracker / ONETHOUSAND) + "ms";
    console[consoleLine][2] = TAG;
    console[consoleLine][3] = String(elapsedTime) + "ms";

    for (int b = 4; b < numProfiles; b++) {  // Clear remaining console entries
      console[consoleLine][b] = String();
    }

    consoleLine++;
    if (consoleLine >= consoleRows) consoleLine = 0;
  }

  if (carousel == 4 && blockMenu) {
    taskManager.schedule(onceMicros(3), taskM);
  }
}



void pollMultiplexer() {
  PCABITS = io.read();
  for (int mp = ZERO; mp <= 17; i++) {
    P0[mp] = (PCABITS & (1 << mp)) != ZERO;
  }
}


inline bool isBitSet(uint16_t value, uint8_t mask) {
  return !(value & (1 << mask));
}



void printToOLED(String oledString) {
  if (OLEDon) {
    u8g2.clearBuffer();
    u8g2.drawStr(ZERO, 32, oledString.c_str());
    u8g2.sendBuffer();
  }
}

void toggleOLED() {

  preferences.begin("my - app", false);
  preferences.putBool("oled", OLEDon);
  preferences.end();

  io.write(PCA95x5::Port::P07, OLEDon ? PCA95x5::Level::H : PCA95x5::Level::L);
  if (OLEDon) {
    u8g2.begin();
    u8g2.setFontDirection(ZERO);
    u8g2.setFontMode(ZERO);
    u8g2.setFont(u8g2_font_logisoso28_tn);  // u8g2_font_u8glib_4_tf
  } else {
    u8g2.setPowerSave(1);
  }
}

void statusLED(bool LEDon) {
  io.write(PCA95x5::Port::P14, LEDon ? PCA95x5::Level::L : PCA95x5::Level::H);
  io.write(PCA95x5::Port::P15, LEDon ? PCA95x5::Level::L : PCA95x5::Level::H);
}


void addLOGmarker(String lead, String markerText) {
  LittleFS.begin();
  markerText = printTime + ", " + lead + " " + markerText + "\n";
  appendFile(LittleFS, logFilePath.c_str(), markerText.c_str());
  LittleFS.end();
}



uint8_t oneWireSearch(int pin) {
  Serial.begin(115200);
  OneWire ow(pin);

  uint8_t address[8];
  uint8_t count = ZERO;

  if (ow.search(address)) {
    Serial.print("DT pin: ");
    Serial.println(pin, DEC);
    do {
      count++;
      Serial.print("   { ");
      for (i = ZERO; i < 8; i++) {
        Serial.print("0x");
        if (address[i] < 0x10) Serial.print("0");
        Serial.print(address[i], HEX);
        if (i < 7) Serial.print(", ");
      }
      Serial.println(" }");
    } while (ow.search(address));

    Serial.print("Nr DT devices found: ");
    Serial.println(count);
  }
  Serial.end();
  return count;
}





String getResetReason() {
  esp_reset_reason_t resetReason = esp_reset_reason();

  switch (resetReason) {
    case ESP_RST_UNKNOWN:
      resetReasonString = "Unknown reset reason.";
      break;
    case ESP_RST_POWERON:
      resetReasonString = "Power-on reset.";
      break;
    case ESP_RST_EXT:
      resetReasonString = "External reset.";
      break;
    case ESP_RST_SW:
      resetReasonString = "Software reset.";
      break;
    case ESP_RST_PANIC:
      resetReasonString = "Exception/Panic reset.";
      break;
    case ESP_RST_INT_WDT:
      resetReasonString = "Watchdog timer reset.\n (soft or hardware)";
      break;
    case ESP_RST_TASK_WDT:
      resetReasonString = "Task watchdog timer reset.";
      break;
    case ESP_RST_WDT:
      resetReasonString = "Other watchdog reset.";
      break;
    case ESP_RST_DEEPSLEEP:
      resetReasonString = "Deep sleep reset.";
      break;
    case ESP_RST_BROWNOUT:
      resetReasonString = "Brownout reset. Check Power.";
      break;
    case ESP_RST_SDIO:
      resetReasonString = "SDIO reset.";
      break;
    default:
      resetReasonString = "";
      break;
  }
  return resetReasonString;
}

String print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wake_up_source;

  switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_EXT0: wakeupReasonString = "Wake-up from external signal with RTC_IO"; break;
    case ESP_SLEEP_WAKEUP_EXT1: wakeupReasonString = "Wake-up from external signal with RTC_CNTL"; break;
    case ESP_SLEEP_WAKEUP_TIMER: wakeupReasonString = "Wake up caused by a timer"; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: wakeupReasonString = "Wake up caused by a touchpad"; break;
    default: wakeupReasonString = /*String(wake_up_source) + " " + */ getResetReason(); break;
  }
  return wakeupReasonString;
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
  if (N <= ZERO) {
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
  if (Rows == ZERO || Columns == ZERO) {
    throw std::invalid_argument("Invalid array dimensions");
  }

  for (size_t row = ZERO; row < Rows; ++row) {
    for (size_t column = ZERO; column < Columns; ++column) {
      array[row][column] = T();  // Use default constructor if available
    }
  }
}


template<size_t Rows, size_t Columns>
void empty2DArray(String (&array)[Rows][Columns]) {
  if (Rows == ZERO || Columns == ZERO) {
    throw std::invalid_argument("Invalid array dimensions");
  }

  for (size_t row = ZERO; row < Rows; ++row) {
    for (size_t column = ZERO; column < Columns; ++column) {
      array[row][column] = String();  // Use String's default constructor
    }
  }
}
