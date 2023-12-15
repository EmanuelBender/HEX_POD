#include <pgmspace.h>



void reloadMenu() {  // one-shot
  TAG = "reloadMenu()";
  timeTracker = micros();

  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM);

  lastHighlightIndex = lastCarousel * 38;
  highlightIndex = carousel * 38;

  tft.fillRoundRect(15, 30 + lastHighlightIndex, TFT_WIDTH - 30, 30, radius, TFT_BLACK);
  tft.fillRoundRect(16, 30 + highlightIndex, TFT_WIDTH - 32, 30, radius, TFT_DARKCYAN);
  tft.drawNumber(carousel, 25, 32 + highlightIndex, 4);

  for (int i = 1; i < menuItems + 1; i++) {
    tft.drawRoundRect(15, 29 + (i * 38), TFT_WIDTH - 30, 32, radius, TFT_LIGHTGREY);
  }

  for (int i = 1; i < menuItems + 1; i++) {
    tft.drawString(menuOptions[i - 1], 45, 33 + (i * 38), 4);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  taskManager.schedule(onceMicros(3), statusBar);
}


void statusBar() {
  statBaTracker = micros();

  if (currentPowerState == IDLE || currentPowerState == NORMAL) {
    tft.drawFastHLine(5, 10, 240, TFT_WHITE);
    tft.setTextPadding(20);
    taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
    // tft.drawRoundRect(0, 0, TFT_WIDTH, TFT_HEIGHT, 41, TFT_WHITE); // Screen Border 41px rounded radius

    tft.setTextDatum(TL_DATUM);
    tft.drawString(uptimeString, 33, 0, 1);

    tft.drawString(printTime, 95, 0, 1);

    tft.setTextDatum(TR_DATUM);
    tft.drawString(String(restarts) + " rst", TFT_WIDTH - 37, 0, 1);
  }

  debugF(statBaTracker);
  statBaTracker = (micros() - statBaTracker) / ONETHOUSAND;
}




void homePage() {
  TAG = "homePage";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
  taskManager.schedule(onceMicros(20), statusBar);

  tft.setTextDatum(TC_DATUM);
  tft.setTextPadding(180);
  tft.drawString(printTime, TFT_WIDTH / 2, 90, 6);
  tft.drawString(printDate, TFT_WIDTH / 2, 135, 4);
}




void sensorPage() {
  TAG = "sensorPage()";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(60);

  tft.drawString("X " + String(X), 20, (26 * 2), 2);
  tft.drawString("Y " + String(Y), 85, (26 * 2), 2);
  tft.drawString("Z " + String(Z), 150, (26 * 2), 2);
  tft.drawString("CPU " + String(temperatureRead()), 20, (26 * 3), 2);
  tft.drawString("ESP " + String(tempValues[0]), 110, (26 * 3), 2);
  tft.setTextPadding(90);
  tft.drawString("BUS " + String(BUS2_BusVoltage / double(ONETHOUSAND)) + "V", 20, (26 * 4), 2);
  tft.drawString("AMP " + String(BUS2_Current) + "mA", 115, (26 * 4), 2);
  tft.drawString("SHU " + String(BUS2_ShuntVoltage) + "mV", 20, (26 * 5), 2);
  tft.drawString("PWR " + String(BUS2_Power) + "mW", 115, (26 * 5), 2);

  tft.setTextPadding(45);
  tft.drawString("BME:" + String(data.meas_index), 20, (26 * 6), 2);  // data1.temperature , data1.humidity, data1.pressure
  tft.drawString("T" + String(data.temperature), 75, (26 * 6), 2);
  tft.drawString("H" + String(data.humidity), 128, (26 * 6), 2);
  double pressure = (data.pressure / 10000.0);
  tft.drawString("P" + String(pressure), 182, (26 * 6), 2);
  tft.setTextPadding(70);
  tft.drawString("A" + String(bme_resistance_avg[0]), 20, (26 * 7), 2);  // bme1_resistance, gas_index, meas_index, bme1_idac
  tft.drawString("B" + String(bme_resistance_avg[1]), 90, (26 * 7), 2);
  tft.drawString("C" + String(bme_resistance_avg[2]), 160, (26 * 7), 2);
  tft.drawString("D" + String(bme_resistance_avg[3]), 20, (26 * 8), 2);
  tft.drawString("E" + String(bme_resistance_avg[4]), 90, (26 * 8), 2);
  tft.drawString("D" + String(bme_resistance_avg[5]), 160, (26 * 8), 2);
  tft.setTextPadding(90);
  tft.drawString("VOC " + String(VOC), 20, (26 * 9), 2);  // sgpHumidity, sgpTemperature
  tft.drawString("NOX " + String(NOX), 115, (26 * 9), 2);
}


void utilPage() {
  TAG = "utilPage()  ";
  timeTracker = micros();
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(150);

  if (menuTrigger && blockMenu) {
    menuTrigger = false;
    // taskManager.schedule(onceMicros(30), statusBar);

    if (DOWN || CLICK_DOWN) {
      DOWN = false;
      utilIndex = (utilIndex % 9) + 1;
      // UTILID = taskManager.schedule(onceMicros(10), utilPage);
      // return;
    }
    if (UP) {
      utilIndex = ((utilIndex - 2 + 9) % 9) + 1;
      UP = false;
      // UTILID = taskManager.schedule(onceMicros(10), utilPage);
      // return;
    }

    if (LEFT) {
      LEFT = false;
      blockMenu = false;
      // launchUtility();
      // UTILID = taskManager.schedule(onceMicros(10), utilPage);
      taskManager.schedule(onceMicros(2), reloadMenu);
      return;
    }

    if (BUTTON || RIGHT || CLICK) {
      taskManager.cancelTask(UTILID);
      BUTTON = false;
      RIGHT = false;
      switch (utilIndex) {
        case 1:
          debugF(timeTracker);
          taskManager.reset();
          lis.setDataRate(LIS3DH_DATARATE_LOWPOWER_5KHZ);
          initializeCube();
          IMUID = taskManager.schedule(repeatMicros(630), pollIMU);  // 625us * 50 = 31.25ms, 550 * 55 * 30.35ms
          // taskManager.setTaskEnabled(IMUID, true);
          taskManager.schedule(repeatMicros(630), calcCube);
          WEB = taskManager.schedule(repeatMillis(webServerPollMs), pollServer);
          tft.fillScreen(TFT_BLACK);
          break;
        case 2:
          WIFIID = taskManager.schedule(onceMillis(1), WiFiScan);
          break;
        case 3:
          debugF(timeTracker);
          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF);
          if (!BLE.begin()) {
            if (DEBUG) {
              ESP_LOGE(TAG, "Bluetooth® Low Energy module failed!");
            }
          }
          taskManager.reset();
          taskManager.schedule(repeatMillis(1000), BLEscan);
          break;
        case 4:
          debugF(timeTracker);
          taskManager.reset();
          WEB = taskManager.schedule(repeatMillis(webServerPollMs), pollServer);

          LittleFS.begin();
          deleteFile(LittleFS, logFilePath.c_str());
          writeFile(LittleFS, logFilePath.c_str(), logHeader.c_str());
          LittleFS.end();

          break;
        case 5:
          LEDon = !LEDon;
          statusLED(LEDon);
          break;
        case 6:
          OLEDon = !OLEDon;
          io.write(PCA95x5::Port::P07, OLEDon ? PCA95x5::Level::H : PCA95x5::Level::L);
          if (OLEDon) {
            u8g2.begin();
            delay(50);
            u8g2.setFontDirection(0);
            u8g2.setFontMode(0);
            u8g2.setFont(u8g2_font_logisoso28_tn);  // u8g2_font_u8glib_4_tf
            updateTime();
          }
          break;
        case 7:
          FANon = !FANon;
          if (FANon) {
            digitalWrite(GPIO_NUM_1, true);
          } else {
            digitalWrite(GPIO_NUM_1, false);
          }
          break;
        case 8:
          debugF(timeTracker);
          taskManager.reset();
          WEB = taskManager.schedule(repeatMillis(webServerPollMs), pollServer);
          taskManager.schedule(onceMillis(50), colorTest);
          // tft.fillScreen(TFT_BLACK);
          break;

        case 9:
          debugF(timeTracker);
          ESP.restart();
          break;

        default: break;
      }
    }

    if (utilIndex != 1 || utilIndex != 6) {  // draw Menu
      tft.setTextColor(TFT_WHITE);

      for (int i = 1; i <= 9; i++) {
        tft.fillRoundRect(17, menuRowM + (i * menuRowM), TFT_WIDTH - 34, 22, radius + 1, TFT_BLACK);
      }
      tft.fillRoundRect(17, menuRowM + (utilIndex * menuRowM), TFT_WIDTH - 34, 22, radius + 1, TFT_ORANGE);

      tft.drawString("3D Cube Demo", 30, 55, 2);
      tft.drawString("WiFi Network Scanner", 30, 55 + menuRowM, 2);
      tft.drawString("BLE Scanner", 30, 55 + (menuRowM * 2), 2);
      tft.drawString("Delete LOGfile", 30, 55 + (menuRowM * 3), 2);
      if (LEDon) {
        tft.drawString("LED On", 30, 55 + (menuRowM * 4), 2);
      } else {
        tft.drawString("LED Off", 30, 55 + (menuRowM * 4), 2);
      }
      if (OLEDon) {
        tft.drawString("OLED On", 30, 55 + (menuRowM * 5), 2);
      } else {
        tft.drawString("OLED Off", 30, 55 + (menuRowM * 5), 2);
      }
      if (FANon) {
        tft.drawString("Fan On", 30, 55 + (menuRowM * 6), 2);
      } else {
        tft.drawString("Fan Off", 30, 55 + (menuRowM * 6), 2);
      }
      tft.drawString("Color Wheel", 30, 55 + (menuRowM * 7), 2);
      tft.drawString("ESP32 Reset", 30, 55 + (menuRowM * 8), 2);
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
}





void colorTest() {
  tft.fillScreen(TFT_BLACK);

  uint16_t colors[12];
  uint16_t Delta = (TFT_WIDTH - 1) / 12;
  bool smooth;

  for (int i = 0; i < 2; i++) {  // locate color wheel colors
    /*colors[i + 22] = tft.alphaBlend(128 + i * 127, TFT_RED, TFT_MAGENTA);
    colors[i + 20] = tft.alphaBlend(128 + i * 127, TFT_MAGENTA, TFT_CYAN);
    colors[i + 18] = tft.alphaBlend(128 + i * 127, TFT_CYAN, TFT_GREEN);
    colors[i + 16] = tft.alphaBlend(128 + i * 127, TFT_GREEN, TFT_YELLOW);
    colors[i + 14] = tft.alphaBlend(128 + i * 127, TFT_YELLOW, TFT_ORANGE);
    colors[i + 12] = tft.alphaBlend(128 + i * 127, TFT_ORANGE, TFT_RED);*/
    colors[i + 10] = tft.alphaBlend(128 + i * 127, TFT_RED, TFT_MAGENTA);
    colors[i + 8] = tft.alphaBlend(128 + i * 127, TFT_MAGENTA, TFT_CYAN);
    colors[i + 6] = tft.alphaBlend(128 + i * 127, TFT_CYAN, TFT_GREEN);
    colors[i + 4] = tft.alphaBlend(128 + i * 127, TFT_GREEN, TFT_YELLOW);
    colors[i + 2] = tft.alphaBlend(128 + i * 127, TFT_YELLOW, TFT_ORANGE);
    colors[i + 0] = tft.alphaBlend(128 + i * 127, TFT_ORANGE, TFT_RED);
  }

  for (int i = 0; i < 7; i++) {
    for (uint16_t angle = 0; angle <= 345; angle += 15) {
      if (angle < 180) {
        smooth = false;
      } else {
        smooth = true;
      }
      uint16_t colorRadius = i * Delta;
      uint16_t wheelColor = tft.alphaBlend((i * 255.0) / 7.0, colors[angle / 15], TFT_WHITE);
      tft.drawArc(120, 140, colorRadius, colorRadius - Delta, angle, angle + 15, wheelColor, TFT_BLACK, smooth);
    }
    smooth = false;  // Only outer ring is smooth
  }

  Delta = (TFT_WIDTH - 1) / 38;
  for (int i = 0; i < 7; i++) {
    for (uint16_t angle = 0; angle <= 330; angle += 30) {
      uint16_t colorRadius = i * Delta;
      uint16_t wheelColor = tft.alphaBlend((i * 255.0) / 7.0, colors[angle / 30], TFT_WHITE);
      tft.drawArc(200, 40, colorRadius, colorRadius - Delta, angle, angle + 30, wheelColor, TFT_BLACK, smooth);
      tft.drawArc(200, 240, colorRadius, colorRadius - Delta, angle, angle + 30, wheelColor, TFT_BLACK, smooth);
    }
    smooth = false;  // Only outer ring is smooth
  }
  tft.drawFastVLine(TFT_WIDTH / 2, 0, TFT_HEIGHT, TFT_BLACK);
}




String assembleTaskData() {
  String result;

  for (const auto& task : tasks) {
    if (taskFreeSlots[*task.taskId] != char('F') /* && *task.tracker > 0.0  && *task.taskId != 0*/) {
      result += "[" + String(*task.taskId) + "]" + String(taskFreeSlots[*task.taskId]) + " " + task.taskName + " " + String(*task.tracker) + "ms\n";
    }
  }

  return result;
}



void taskM() {
  timeTracker = micros();

  if (carousel == 4 && blockMenu) {
    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(180);

    // updateTaskArray();

    String taskData = assembleTaskData();
    std::istringstream taskStream(taskData.c_str());

    for (i = 0; i < slotsSize; i++) {
      std::string line;  // Change the type to std::string
      getline(taskStream, line);

      int yPos = (16 * i) + 50;

      if (!line.empty()) {
        tft.drawString(line.c_str(), 30, yPos, 2);
      } else {
        tft.drawString("                ", 30, yPos, 2);
      }
    }

    // taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

    if (DEBUG) {
      timeTracker = (micros() - timeTracker) / ONETHOUSAND;
      ESP_LOGI("taskM()", "%.3lfms", timeTracker);
    }
  }
}






void systemPage() {
  TAG = "systemPage()";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);
  tft.setTextDatum(TL_DATUM);

  getDeviceInfo();

  if (DOWN) {
    sysIndex -= 1;
    DOWN = false;
  }
  if (UP && sysIndex <= -1) {
    sysIndex += 1;
    UP = false;
  }

  if (menuTrigger) {
    menuTrigger = false;
    if (sysIndex <= 0) {
      tft.setTextPadding(220);

      // unsigned major_rev = chip_info.revision;
      // unsigned minor_rev = chip_info.revision % 100;

      for (int i = 2; i <= 40; i++) {
        if (sysIndex + i >= 2 && sysIndex + i <= 40) {

          switch (i) {
            case 2:
              label = String(CONFIG_IDF_TARGET) /* + " " + String(major_rev)*/;
              if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
                value = "failed";
                ESP_LOGE(TAG, "Get flash size failed");
              } else {
                value = String(flash_size / ONEMILLIONB) + "MB " + String((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external") + " flash";
              }
              break;
            case 3:
              label = "CPU:       ";
              value = String(cpu_freq_mhz) + " MHZ" + " x" + String(chip_info.cores);
              break;
            case 4:
              label = "Features:  ";
              value = String((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi, " : "") + String((chip_info.features & CHIP_FEATURE_BT) ? "BT, " : "") + String((chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "");
              break;
            case 5:
              label = "PSRAM:    ";
              // value = String((chip_info.features & CHIP_FEATURE_EMB_PSRAM ? "YES " : "-- "));
              value = "T: " + String(deviceInfo.total_allocated_bytes / KILOBYTE) + "Kb,  F:" + String(deviceInfo.total_free_bytes / KILOBYTE) + "Kb";
              break;
            case 6:
              label = "SPIFFS:    ";
              value = "F: " + String(SPIFFS_size / ONEMILLIONB, 3) + "Mb,  U: " + String(SPIFFS_used / ONEMILLIONB, 3) + "Mb";
              break;
            case 7:
              label = "XTAL:      ";
              value = String(cpu_xtal_mhz) + " MHZ";
              break;
            case 8:
              label = "ABP:       ";
              value = String(cpu_abp_hz / ONEMILLION) + " MHZ";
              break;
            case 9:
              label = "I2C:        ";
              value = String(I2C_SPEED / 1000) + " KHZ";
              break;
            case 10:
              label = "SPI:        ";
              value = String(SPI_FREQUENCY / ONEMILLION) + " MHZ";
              break;
            case 11:
              label = "SPI_READ: ";
              value = String(SPI_READ_FREQUENCY / ONEMILLION) + " MHZ";
              break;
            case 12:
              label = "SD:        ";
              value = SDinserted ? "TRUE" : "FALSE";
              break;
            case 13:
              label = "OS Version ";
              value = String(codeRevision);
              break;
            case 14:
              label = print_wakeup_reason();
              value = "";
              break;
            case 15:
              label = "Free_heap:      ";
              value = String(esp_get_free_heap_size() / KILOBYTE) + "KB";
              break;
            case 16:
              label = "Minimum_heap:   ";
              value = String(esp_get_minimum_free_heap_size() / KILOBYTE) + "KB";
              break;
            case 17:
              label = "Internal_heap:   ";
              value = String(esp_get_free_internal_heap_size() / KILOBYTE) + "KB";
              break;
            case 18:
              label = "Sketch Used:     ";
              value = String(program_UsedP) + "%";
              break;
            case 19:
              label = "WiFi IP:         ";
              value = String(WiFiIP);
              break;
            case 20:
              label = "WiFi Status:      ";
              value = String(WiFi.status());
              break;
            case 21:
              label = "Last NTP OK:     ";
              value = lastNTPtime;
              break;
            case 22:
              label = "Last NTP fail:   ";
              value = lastNTPtimeFail;
              break;
            case 23:
              label = "CPU Temp:       ";
              value = String(CPUTEMP);
              break;
            case 24:
              label = "INA219:         ";
              value = INA2.isConnected() ? "OK" : "NaN" + String(INA2_iscalibrated ? "Cal" : "NoCal");
              break;
            case 25:
              label = "BME688:         ";
              value = BME_ERROR;
              break;
            case 26:
              label = "SGP41:          ";
              value = String(sgpErrorMsg);
              break;

            default:
              label = "-: ";
              value = " ";
          }

          tft.drawString(label + " " + value, 20, (17 * (i + sysIndex)) + 9, 2);
        }
      }
    }
    debugF(timeTracker);
  }
}



void WiFiScan() {
  TAG = "WiFiScan()";
  timeTracker = micros();
  tft.fillScreen(TFT_BLACK);
  String encType;

  tft.setTextDatum(TC_DATUM);
  tft.drawString("Scanning...", TFT_WIDTH / 2, TFT_HEIGHT / 2, 4);


  tft.setTextDatum(TL_DATUM);
  int n = WiFi.scanNetworks();
  tft.fillScreen(TFT_BLACK);
  if (DEBUG) {
    ESP_LOGI(TAG, "Scan done.");
  }
  if (n == 0) {

    if (DEBUG) ESP_LOGI(TAG, "no networks found");

    tft.drawString("no networks found", TFT_WIDTH / 2, TFT_HEIGHT / 2, 4);
  } else {
    if (n < 0) {

      if (DEBUG) ESP_LOGI(TAG, "Wi-Fi scan error.");

      tft.drawString("Wi-Fi scan error.", TFT_WIDTH / 2, TFT_HEIGHT / 2, 4);
      return;
    }

    if (DEBUG) {
      ESP_LOGI(TAG, "%d networks found", n);
      ESP_LOGI(TAG, "Nr | SSID                             | RSSI | CH | Encryption");
    }
    tft.drawString(String(n) + " Networks Found", 15, 13, 2);

    for (int x = 0; x < n; ++x) {  // Print SSID and RSSI for each network found
      encType = "";

      switch (WiFi.encryptionType(x)) {
        case WIFI_AUTH_OPEN:
          encType = "open";
          break;
        case WIFI_AUTH_WEP:
          encType = "WEP";
          break;
        case WIFI_AUTH_WPA_PSK:
          encType = "WPA";
          break;
        case WIFI_AUTH_WPA2_PSK:
          encType = "WPA2";
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          encType = "WPA+WPA2";
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          encType = "WPA2-EAP";
          break;
        case WIFI_AUTH_WPA3_PSK:
          encType = "WPA3";
          break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
          encType = "WPA2+WPA3";
          break;
        case WIFI_AUTH_WAPI_PSK:
          encType = "WAPI";
          break;
        default:
          encType = "NaN";
      }

      if (DEBUG) ESP_LOGI(TAG, "%2d | %-32.32s | %4d | %2d | %s", (x + 1), WiFi.SSID(x).c_str(), WiFi.RSSI(x), WiFi.channel(x), encType);

      // tft.drawFastHLine(0, (11 * (x + 2)) + 20, 240, TFT_DARKGREY);
      tft.drawString(String((x + 1)) + " " + WiFi.SSID(x) + "   | " + WiFi.RSSI(x) + "db CH:" + WiFi.channel(x) + " " + encType, 5, (15 * (x + 2)) + 11, 2);
    }
  }

  WiFi.scanDelete();
}



void BLEscan() {
  TAG = "BLEscan()";
  timeTracker = micros();
  tft.setTextPadding(200);
  tft.setTextDatum(TL_DATUM);
  BLE.scan();
  int W = 0;
  scanData[30];

  for (i = 0; i < BLEresults; i++) {
    peripheral = BLE.available();

    if (peripheral) {
      if (peripheral.hasLocalName()) {
        scanData[i].result = String(peripheral.localName());
        scanData[i].rssi = peripheral.rssi();
      } else {
        scanData[i].result = "";
      }
    }
  }
  // Bubble Sort implementation for sorting based on RSSI values
  for (W = 0; i < BLEresults; W++) {
    for (i = 0; i < BLEresults - 1; i++) {
      for (int j = 0; j < BLEresults - i - 1; j++) {
        if (scanData[j].rssi < scanData[j + 1].rssi) {
          // swap the elements
          BLEData temp = scanData[j];
          scanData[j] = scanData[j + 1];
          scanData[j + 1] = temp;
        }
      }
    }
  }


  tft.fillScreen(TFT_BLACK);

  W = 0;
  for (i = 0; i < BLEresults; i++) {
    if (scanData[i].result != "") {
      W++;
      tft.drawString(scanData[i].result + " | " + String(scanData[i].rssi) + "db", 15, 20 + (17 * W), 2);
    }
  }
  taskManager.schedule(onceMicros(3), statusBar);
}
