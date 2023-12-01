#include <pgmspace.h>


void logging() {  // Assign values to the array at the current index
  TAG = "logging()    ";
  timeTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  cpu_freq_mhz = getCpuFrequencyMhz();
  cpu_xtal_mhz = getXtalFrequencyMhz();
  cpu_abp_hz = getApbFrequency();

  SDarray[SDIndex][0] = printTime;
  SDarray[SDIndex][1] = printDate;
  SDarray[SDIndex][2] = String(CONFIG_IDF_TARGET);
  SDarray[SDIndex][3] = codeRevision;
  SDarray[SDIndex][4] = print_wakeup_reason();
  SDarray[SDIndex][5] = currentPowerState;
  SDarray[SDIndex][6] = SLEEPENABLE;
  SDarray[SDIndex][7] = WiFi.status();
  SDarray[SDIndex][8] = lastNTPtime;
  SDarray[SDIndex][9] = String(cpu_freq_mhz) + "MHZ" + " " + String(chip_info.cores) + "Core";
  SDarray[SDIndex][10] = String((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi " : "") + String((chip_info.features & CHIP_FEATURE_BT) ? "BT " : "") + String((chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "");
  SDarray[SDIndex][11] = String(flash_size / ONEMILLION) + "Mb " + String((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external") + " flash";
  SDarray[SDIndex][12] = "PSRAM Total: " + String(ESP.getPsramSize() / 1000) + "KB  Free: " + String(ESP.getFreePsram() / 1000) + "KB";
  SDarray[SDIndex][13] = "SPIFFS Free: " + String(file_system_size / ONEMILLION) + "MB  Total: " + String(free_size / ONEMILLION) + "MB";
  SDarray[SDIndex][14] = String(esp_get_free_heap_size() / 1000.0) + "KB";
  SDarray[SDIndex][15] = String(esp_get_minimum_free_heap_size() / 1000.0) + "KB";
  SDarray[SDIndex][16] = String(esp_get_free_internal_heap_size() / 1000.0) + "KB";
  SDarray[SDIndex][17] = String(program_size / 1000.0) + "KB";
  SDarray[SDIndex][18] = temperatureRead();
  SDarray[SDIndex][19] = temp1;
  SDarray[SDIndex][20] = BUS2_BusVoltage;
  SDarray[SDIndex][21] = BUS2_Current;
  SDarray[SDIndex][22] = "X" + String(X) + " Y" + String(Y) + " Z" + String(Z);
  SDarray[SDIndex][23] = data.temperature;
  SDarray[SDIndex][24] = data.humidity;
  SDarray[SDIndex][25] = data.pressure;
  for (int i = 0; i < numProfiles; ++i) {
    SDarray[SDIndex][26 + i] = bme_resistance_avg[i];
  }
  SDarray[SDIndex][numProfiles + 26] = VOC;
  SDarray[SDIndex][numProfiles + 27] = NOX;
  SDarray[SDIndex][numProfiles + 28] = srawVoc;
  SDarray[SDIndex][numProfiles + 29] = srawNox;


  SDIndex++;  // Move to the next row for the next measurements
  if (SDIndex >= consoleRows) SDIndex = 0;

  // if (serialPrintLOG) {
  String logLine;
  for (int j = 0; j < consoleColumns; ++j) {
    if (SDarray[SDIndex][j] != "") {
      // Serial.print(SDarray[SDIndex][j]);

      // consoleLine++;
      // console[consoleLine][i] = String(SDarray[SDIndex][j]);
      // Serial.print('\t');
      logLine += SDarray[SDIndex][j] + ", ";
    }
  }
  Serial.println();

  LittleFS.begin();
  // const char *logLineCStr = logLine.c_str();
  const char *logLineCStr = SDarray[SDIndex][0].c_str();

  appendFile(LittleFS, "/_LOG/deviceLOG.txt", logLineCStr);
  // appendFile(LittleFS, "/_LOG/test.txt", logLineCStr);

  LittleFS.end();
  Serial.print("Data: ");
  Serial.println(logLine);

  // }

  debugF(timeTracker);
  loggingTracker = (micros() - timeTracker) / 1000.0;
}



void mountSD() {
  TAG = "mountSD()";
  // timeTracker = micros();
  SDinserted = !digitalRead(GPIO_NUM_47);

  if (SDinserted) {

    sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);  // MOSI 11, SCK 12, MISO 13, CS 10
    sdSPI.setFrequency(40000000);                   // Try different frequencies
    sdSPI.setDataMode(SPI_MODE0);                   // Try different modes

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(60);

    if (!SD.begin(SD_CS, sdSPI, 40000000, "/", 50)) {
      tft.drawString("SD: Card mount failed.", 60, 100, 3);
      return;
    }


    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      tft.drawString("SD: No SD Card found.", 60, 100, 3);
      return;
    }

    tft.drawString("Card Size:   " + String(SD.cardSize() / (1024 * 1024)) + "MB", 20, 10, 2);
    tft.drawString("Total Space: " + String(SD.totalBytes() / (1024 * 1024)) + "MB", 20, 20, 2);
    tft.drawString("Used Space:  " + String(SD.usedBytes() / (1024 * 1024)) + "MB", 20, 30, 2);

    if (cardType == CARD_MMC) {
      tft.drawString("Type: MMC", 20, 40, 2);
    } else if (cardType == CARD_SD) {
      tft.drawString("Type: SDSC", 20, 40, 2);
    } else if (cardType == CARD_SDHC) {
      tft.drawString("Type: SDHC", 20, 40, 2);
    } else {
      tft.drawString("Type: UNKNOWN", 20, 40, 2);
    }


    // listDir(SD, "/", 0);
    // createDir(SD, "/testFolder");
    // listDir(SD, "/", 0);
    // removeDir(SD, "/testFolder");
    delay(10000);
  } else {
    tft.drawString("No SD card inserted.", 60, 100, 3);
  }
}


void handleSPIFFS() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(60);

  if (!LittleFS.begin(false, "/littlefs", 20, "spiffs")) {
    tft.drawString("LITTLEFS/SPIFFS couldn't be Mounted.", 60, 100, 3);
    return;
  }

  file_system_size = LittleFS.totalBytes();
  file_system_used = LittleFS.usedBytes();

  // createDir(LITTLEFS, "/_LOG/test.txt");

  // listDir(LittleFS, "/", 3);
  createDir(LittleFS, "/_LOG");
  writeFile(LittleFS, "/_LOG/test.txt", "test");
  // listDir(LittleFS, "/", 3);

  LittleFS.end();
}




String listWebDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  String fsString = "";
  File root = fs.open(dirname);

  if (!root) {
    return "<tr><td>Failed to open directory</td></tr>";
  }

  if (!root.isDirectory()) {
    return "<tr><td>Not a Directory</td></tr>";
  }

  File file = root.openNextFile();
  int fileId = 0;
  while (file) {
    String fileIdStr = String(fileId);
    if (file.isDirectory()) {
      fsString += "<tr>";
      fsString += "<div style='color:black;' class='dir' id='dir_" + fileIdStr + "'><td>&nbsp;</td><td>&rarr; " + String(file.name()) + "</td></div>";
      fsString += "</tr>";
      if (levels) {
        fsString += listWebDir(fs, file.path(), levels - 1);
      }
    } else {
      filesCount++;
      fsString += "<tr>";
      fsString += "<td><div style='color:black;' class='file' id='fil_" + fileIdStr + "'><td>&nbsp;</td><td>" + "&rarr;</td>";
      double fileSizeMB = double(file.size()) / 1024.0 / 1024.0;
      fsString += "<td>" + String(file.name()) + "</td>";
      fsString += "<td>" + String(fileSizeMB, 3) + "MB</td>";
      fsString += "</div></tr>";
    }
    file = root.openNextFile();
    fileId++;
  }


  return fsString;
}




void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  // Serial.printf("Listing directory: %s\n", dirname);
  uint8_t filesCount = 0;

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    tft.drawString("Not a directory", 60, 100, 3);
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      // tft.drawString(String(file.name()));
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      filesCount++;
      // tft.drawString(String(file.name())), 20, 40 + (12 * filesCount), 2); //  + " Size: " + String(file.size()
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}




void createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}
