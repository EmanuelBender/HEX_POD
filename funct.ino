
#include <pgmspace.h>

void pollServer() {
  TAG = "pollWeb()      ";
  timeTracker = micros();
  server.handleClient();
  debugF(timeTracker);
  clientTracker = (micros() - timeTracker) / 1000.0;
}

void IRAM_ATTR CTR_ISR() {
  BUTTON = true;
  BTNID = taskManager.schedule(onceMicros(5), pollButtons);
}
void IRAM_ATTR UDLR_ISR() {
  INT_TRGR = true;
  BTNID = taskManager.schedule(onceMicros(1), pollButtons);
}
void IRAM_ATTR SD_ISR() {
  SDinserted = !digitalRead(GPIO_NUM_47);
  // ESP_LOGI("SD", "%s", SDinserted ? "Inserted" : "Removed");
}


void launchUtility() {
  TAG = "launchUtility()";
  timeTracker = micros();
  taskManager.reset();
  tft.fillScreen(TFT_BLACK);
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  debugF(timeTracker);

  // CLKID = taskManager.schedule(repeatMillis(30), accClick);
  STATID = taskManager.schedule(repeatMillis(995), statusBar);
  SECID = taskManager.schedule(repeatMillis(997), updateTime);
  NTPID = taskManager.schedule(repeatSeconds(getNTPInterval), getNTP);
  TEMPID = taskManager.schedule(repeatMillis(980), pollTemp);
  INA2ID = taskManager.schedule(repeatMillis(500), pollINA2);
  BMEID = taskManager.schedule(repeatMillis(bmeInterval / bmeSamples), pollBME);
  SGPID = taskManager.schedule(repeatMillis(sgpInterval), pollSGP);
  LOG = taskManager.schedule(repeatMillis(loggingInterval), logging);
  ST1 = taskManager.schedule(repeatSeconds(1), PowerStates);
  WEB = taskManager.schedule(repeatMillis(webServerPollMs), pollServer);
  IMUID = taskManager.schedule(repeatMicros(imuInterval), pollIMU);
  taskManager.setTaskEnabled(IMUID, false);

  if (!blockMenu) taskManager.schedule(onceMicros(1), reloadMenu);
}


void PowerStates() {
  TAG = "PowerStates()";
  powerStTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  /* if (powerStTracker - lastInputTime > lightsleepDelay) {
    currentPowerState = LIGHT_SLEEP;
  } else */
  if (powerStTracker - lastInputTime > powersaveDelay) {
    currentPowerState = POWER_SAVE;
  } else if (powerStTracker - lastInputTime > idleDelay) {
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
      taskManager.cancelTask(STATID);
      // taskManager.setTaskEnabled(STATID, false);

      while (TFTbrightness > 0.0) {
        TFTbrightness -= 0.01;
        pwm.writeScaled(TFTbrightness);
        taskManager.yieldForMicros(500);
      }

    } else if (currentPowerState == IDLE) {
      // WEB = taskManager.schedule(repeatMillis(webServerPollMs * 2), handleWebClient);
      setCpuFrequencyMhz(160);
    } else if (currentPowerState == NORMAL) {
      // taskManager.setTaskEnabled(STATID, true);
      setCpuFrequencyMhz(240);
    }
  }

  previousPowerState = currentPowerState;

  debugF(powerStTracker);
  powerStTracker = (micros() - powerStTracker) / 1000.0;
}





void pollButtons() {

  pollMultiplexer();
  TAG = "pollButtons()";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  if (micros() - lastInputTime <= 250000) {
    return;
  }

  if (INT_TRGR) {
    UP = P0[11];
    DOWN = P0[8];
    LEFT = P0[9];
    RIGHT = P0[10];
    INT_TRGR = false;
    while (TFTbrightness < 1.0) {  // here as long as no brightness slider in TFT ui
      TFTbrightness += 0.01;
      pwm.writeScaled(TFTbrightness);
    }
  }

  if (UP || DOWN || LEFT || RIGHT || BUTTON || CLICK) {
    menuTrigger = true;
    lastInputTime = micros();
    taskManager.schedule(onceMicros(500), statusBar);

    if (LEFT) {
      LEFT = false;
      CLICK_LEFT = false;
      blockMenu = false;
      lis.setDataRate(LIS3DH_DATARATE_POWERDOWN);
      launchUtility();
      taskManager.schedule(onceMicros(2), reloadMenu);
      tft.fillScreen(TFT_BLACK);
    }

    if (!blockMenu) {
      if (DOWN || CLICK_DOWN) {
        DOWN = false;
        CLICK_DOWN = false;
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
        RIGHT = false;
        BUTTON = false;
        CLICK = false;
        blockMenu = true;
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(TL_DATUM);

        switch (carousel) {
          case 1:
            HOMEID = taskManager.schedule(repeatMillis(900), homePage);
            break;
          case 2:
            for (i = 1; i < 9; i++) {
              tft.drawRoundRect(5, 22 + (i * menuRowM), TFT_WIDTH - 14, 24, radius, TFT_DARKGREY);
            }
            tft.drawString("Sensors", 15, 15, 4);
            lis.setDataRate(LIS3DH_DATARATE_LOWPOWER_5KHZ);
            taskManager.setTaskEnabled(IMUID, true);
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
            taskManager.schedule(onceMicros(10), taskM);
            break;
          case 5:
            tft.drawString("System Info", 15, 15, 4);
            SYSID = taskManager.schedule(repeatMillis(50), systemPage);
            break;
          default:
            break;
        }
      }
    } else {
      if (carousel == 3) UTILID = taskManager.schedule(onceMicros(10), utilPage);
    }
    taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
  }
  debugF(timeTracker);
}




void pollMultiplexer() {
  PCABITS = io.read();

  for (int i = 0; i <= 17; i++) {
    P0[i] = isBitSet(PCABITS, i);  // Now you can access individual values using P0[0], P0[1], ..., P0[17]
  }
}


inline bool isBitSet(uint16_t value, uint8_t mask) {
  return !(value & (1 << mask));
}

void statusLED(bool LEDon) {
  io.write(PCA95x5::Port::P14, LEDon ? PCA95x5::Level::L : PCA95x5::Level::H);
  io.write(PCA95x5::Port::P15, LEDon ? PCA95x5::Level::L : PCA95x5::Level::H);
}





void updateTime() {
  TAG = "updateTime() ";
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
  uTimeTracker = micros();

  if (getLocalTime(&timeinfo)) {
    printTime = formatTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, ':');
    printDate = formatTime(timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year - 100, '.');
    uptimeString = convertSecToTimestamp<String>(millis() / 1000);
  } else {
    // getNTP();
  }

  // if (timeinfo.tm_year < 23 || timeTracker < 10000) { getNTP(); }

  if (OLEDon) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 32, printTime.c_str());
    u8g2.sendBuffer();
  }

  debugF(uTimeTracker);
  uTimeTracker = (micros() - uTimeTracker) / 1000.0;
}




void getNTP() {
  TAG = "getNTP()  ";
  ntpTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  //if (WiFi.status() != WL_CONNECTED) {
  // WiFi.reconnect();
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (millis() - ntpTracker > WiFiTimeout) {
      ESP_LOGE("NTP", "WiFi Timeout. None found.");
      break;
    }
  }
  // }

  if (DEBUG) {
    ESP_LOGI("NTP", "Getting time..");
  }

  if (WiFi.status() == WL_CONNECTED) {
    // taskManagerLock.lock();
    configTzTime(time_zone, ntpServer1, ntpServer2);  // gets the time from the NTP server
    delay(20);
    updateTime();
    lastNTPtime = printTime + " " + printDate;
    WiFiIP = WiFi.localIP().toString();

    // setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    //tzset();
    // taskManagerLock.unlock();

  } else {
    ESP_LOGE("NTP", "Update failed.");
    lastNTPtimeFail = printTime + " " + printDate;
  }

  debugF(ntpTracker);
  ntpTracker = (micros() - ntpTracker) / 1000.0;
}




void pollTemp() {
  TAG = "pollTemp()   ";
  tempTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  temp1 = tempSens.getTempC(tempProbe1);
  tempSens.setWaitForConversion(false);
  tempSens.requestTemperatures();

  if (temp1 == DEVICE_DISCONNECTED_C) {
    ESP_LOGE(TAG, "temp1 disconnected");
    temp1 = NAN;
  }

  debugF(tempTracker);
  tempTracker = (micros() - tempTracker) / 1000.0;
}




void pollIMU() {
  TAG = "pollIMU()   ";
  imuTracker = micros();
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
  debugF(imuTracker);
  imuTracker = (micros() - imuTracker) / 1000.0;
}



void pollINA2() {

  BUS2_CNVR = INA2.getConversionFlag();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  if (BUS2_CNVR) {
    TAG = "pollINA2()   ";
    ina2Tracker = micros();

    BUS2_BusVoltage = INA2.getBusVoltage();
    BUS2_ShuntVoltage = INA2.getShuntVoltage_mV();
    BUS2_Current = INA2.getCurrent_mA();
    BUS2_Power = INA2.getPower_mW();
    BUS2_OVF = INA2.getMathOverflowFlag();

    debugF(ina2Tracker);
    ina2Tracker = (micros() - ina2Tracker) / 1000.0;
  }
}



void pollBME() {
  TAG = "pollBME2()   ";
  bmeTracker = micros();
  lastBMEpoll = convertSecToTimestamp<String>(bmeTracker / 1000 / 1000);
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  int i;

  if (bme.checkStatus()) {
    if (bme.checkStatus() == BME68X_ERROR) {
      BME_ERROR = "[E] " + bme.statusString();
      return;
    } else if (bme.checkStatus() == BME68X_WARNING) {
      BME_ERROR = "[W] " + bme.statusString();
    }
  }

  Altitude = ((((((10 * log10((data.pressure / 100.0) / 1013.25)) / 5.2558797) - 1) / (-6.8755856 * pow(10, -6))) / 1000) * 0.30);


  bme.setTPH(BME68X_OS_4X, BME68X_OS_8X, BME68X_OS_4X);
  bme.setFilter(bmeFilter);
  bme.fetchData();
  bme.getData(data);
  bme.setAmbientTemp(data.temperature);

  repeater++;

  for (bmeProfile = 0; bmeProfile < numProfiles; bmeProfile++) {
    duration = durProf_1[bmeProfile];
    heaterTemp = heatProf_1[bmeProfile];

    delay(bmeProfilePause);
    bme.setAmbientTemp(data.temperature);
    bme.setHeaterProf(heaterTemp, duration);
    bme.setOpMode(BME68X_FORCED_MODE);
    // delayMicroseconds(bme.getMeasDur());

    while (!bme.fetchData()) {
      delay(5);
    }
    bme.getData(data);
    if (!conditioning_duration) bme_resistance[bmeProfile] += data.gas_resistance;
  }

  if (repeater == bmeSamples) {
    bme_gas_avg = 0;

    if (!conditioning_duration) {

      for (i = 0; i < numProfiles; ++i) {  // calc Average Samples
        bme_resistance[i] /= bmeSamples;
      }

      // samplingDelta = ((bmeInterval / 1000.0) / bmeSamples) / 60.0 /*+ (durProf_1[0])*/;  // work in progress, compensation for saturation of sensor, when too many measurements make the sensor go hot..?

      for (i = 0; i < numProfiles; ++i) {  // apply sampl delta & average
        // bme_resistance[i] *= samplingDelta; work in progress
        bme_gas_avg += bme_resistance[i];
      }
      bme_gas_avg /= numProfiles;
      smallestValue = findSmallestValue(bme_resistance);
      offsetDelta = smallestValue;

      // Serial.printf("Curr Meas: %d", repeater);
      // Serial.println();
      // Serial.printf("Offset Delta:%d", offsetDelta);
      // Serial.println();
      // Serial.printf("Sampling Delta: %f", samplingDelta);
      // Serial.println();

      for (i = 0; i < numProfiles; ++i) {
        bme_resistance_avg[i] = (bme_resistance[i] - offsetDelta);  // work in progress

        if (serialPrintBME1) {
          console[consoleLine][i] = String(bme_resistance_avg[i]);
          Serial.print("BME" + String(i) + ":" + String(bme_resistance_avg[i]) + "\t");
        }
      }

      if (serialPrintBME1) {
        consoleLine++;
        if (consoleLine >= 55) consoleLine = 0;
        Serial.println();
      }

      for (i = 0; i < numProfiles; ++i) {  // empty resistance array
        bme_resistance[i] = 0;
      }
    }
    repeater = 0;
  }
  bme.setOpMode(BME68X_SLEEP_MODE);



  debugF(bmeTracker);
  bmeTracker = (micros() - bmeTracker) / 1000.0;
}





void pollSGP() {
  TAG = "pollSGP()    ";
  timeTracker = micros();
  lastSGPpoll = convertSecToTimestamp<String>(timeTracker / 1000 / 1000);
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  auto compensationT = static_cast<uint16_t>((data.temperature + 45) * 65535 / 175);
  auto compensationRh = static_cast<uint16_t>(data.humidity * 65535 / 100);

  if (conditioning_duration > 0) {
    error = sgp41.executeConditioning(compensationRh, compensationT, srawVoc);  // defaultRh, defaultT
    if (error) errorToString(error, sgpErrorMsg, sizeof(sgpErrorMsg));
    conditioning_duration--;
  } else {
    error = sgp41.measureRawSignals(compensationRh, compensationT, srawVoc, srawNox);
    delay(140);
    error = sgp41.measureRawSignals(compensationRh, compensationT, srawVoc, srawNox);
    VOC = voc_algorithm.process(srawVoc);
    NOX = nox_algorithm.process(srawNox);
    error = sgp41.turnHeaterOff();
    if (error) errorToString(error, sgpErrorMsg, sizeof(sgpErrorMsg));
  }

  debugF(timeTracker);
  sgpTracker = (micros() - timeTracker) / 1000.0;
}


void configSGP() {
  // error = sgp41.turnHeaterOff();
  error = sgp41.executeSelfTest(sgpError);
  if (error) errorToString(error, sgpErrorMsg, sizeof(sgpErrorMsg));


  voc_algorithm.set_tuning_parameters(
    100, 12, 12,    // 100, 12, 12
    180, 50, 230);  // 180, 50!, 230

  voc_algorithm.get_tuning_parameters(
    index_offset, learning_time_offset_hours, learning_time_gain_hours,
    gating_max_duration_minutes, std_initial, gain_factor);


  nox_algorithm.set_tuning_parameters(
    100, 12, 12,    // 100, 12, 12
    180, 50, 230);  // 180, 50!, 230

  nox_algorithm.get_tuning_parameters(
    index_offset, learning_time_offset_hours, learning_time_gain_hours,
    gating_max_duration_minutes, std_initial, gain_factor);
}




void debugF(uint32_t tracker) {
  if (DEBUG) {
    float elapsedTime = (micros() - tracker) / 1000.0;
    ESP_LOGI(TAG, "%.3lfms", elapsedTime);

    console[consoleLine][0] = printTime;
    console[consoleLine][1] = String(tracker / 1000) + "ms";
    console[consoleLine][2] = TAG;
    console[consoleLine][3] = String(elapsedTime) + "ms";

    // Clear remaining console entries
    for (int b = 4; b < numProfiles; b++) {
      console[consoleLine][b] = String();
    }

    consoleLine++;
    if (consoleLine >= 55) consoleLine = 0;
  }

  if (carousel == 4 && blockMenu) {
    taskManager.schedule(onceMicros(5), taskM);
  }
}






String getResetReason() {
  resetReason = esp_reset_reason();
  switch (resetReason) {
    case ESP_RST_UNKNOWN:
      resetReasonString = "Unknown reset reason";
      break;
    case ESP_RST_POWERON:
      resetReasonString = "Power-on reset";
      break;
    case ESP_RST_EXT:
      resetReasonString = "External reset";
      break;
    case ESP_RST_SW:
      resetReasonString = "Software reset";
      break;
    case ESP_RST_PANIC:
      resetReasonString = "Exception/Panic reset";
      break;
    case ESP_RST_INT_WDT:
      resetReasonString = "Watchdog timer reset (software or hardware)";
      break;
    case ESP_RST_TASK_WDT:
      resetReasonString = "Task watchdog timer reset";
      break;
    case ESP_RST_WDT:
      resetReasonString = "Other watchdog reset";
      break;
    case ESP_RST_DEEPSLEEP:
      resetReasonString = "Deep sleep reset";
      break;
    case ESP_RST_BROWNOUT:
      resetReasonString = "Brownout reset";
      break;
    case ESP_RST_SDIO:
      resetReasonString = "SDIO reset";
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
    default: wakeupReasonString = String(wake_up_source) + " " + getResetReason(); break;
  }
  return wakeupReasonString;
}
