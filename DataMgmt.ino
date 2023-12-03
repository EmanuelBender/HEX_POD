#include <pgmspace.h>

String path = "/_LOG/deviceLOG.txt";
const char *logfilePath = path.c_str();

void getSPIFFSsizes() {
  SPIFFS_size = LittleFS.totalBytes();
  SPIFFS_used = LittleFS.usedBytes();
  SPIFFS_free = SPIFFS_size - SPIFFS_used;
  percentUsedLFS = (SPIFFS_used * 100.0) / SPIFFS_size;
  percentLeftLFS = 100.0 - percentUsedLFS;
}

void getProgramInfo() {
  program_size = ESP.getFreeSketchSpace();
  program_used = ESP.getSketchSize();
  program_free = program_size - program_used;
  program_UsedP = (program_used * 100.0) / program_size;
  program_LeftP = 100.0 - program_UsedP;
}

void getDeviceInfo() {

  LittleFS.begin();
  getSPIFFSsizes();
  LittleFS.end();

  esp_flash_get_size(NULL, &out_size);
  esp_flash_get_physical_size(NULL, &flash_size);
  esp_chip_info(&chip_info);

  WiFiIP = WiFi.SSID();

  getProgramInfo();

  flash_speed = ESP.getFlashChipSpeed();
  flash_size = double(ESP.getFlashChipSize());
  free_flash_size = flash_size - program_used - SPIFFS_used;

  cpu_freq_mhz = getCpuFrequencyMhz();
  cpu_xtal_mhz = getXtalFrequencyMhz();
  cpu_abp_hz = getApbFrequency();


  if (psramFound()) {                                   // PSRAM
    heap_caps_get_info(&deviceInfo, MALLOC_CAP_SPIRAM); /*
    size_t total_free_bytes;      ///<  Total free bytes in the heap. Equivalent to multi_free_heap_size().
    size_t total_allocated_bytes; ///<  Total bytes allocated to data in the heap.
    size_t largest_free_block;    ///<  Size of largest free block in the heap. This is the largest malloc-able size.
    size_t minimum_free_bytes;    ///<  Lifetime minimum free heap size. Equivalent to multi_minimum_free_heap_size().
    size_t allocated_blocks;      ///<  Number of (variable size) blocks allocated in the heap.
    size_t free_blocks;           ///<  Number of (variable size) free blocks in the heap.
    size_t total_blocks;          ///<  Total number of (variable size) blocks in the heap. */
  }
  // RAM
  // esp_get_free_heap_size();
  // esp_get_minimum_free_heap_size();
  // esp_get_free_internal_heap_size();
}


void logging() {  // Assign values to the array at the current index
  TAG = "logging()    ";
  loggingTracker = micros();
  timeTracker = loggingTracker;
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  getDeviceInfo();

  if (conditioning_duration == 0) {
    String airLog;
    airLog = printTime + ", ";
    /* 
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
  SDarray[SDIndex][12] = "PSRAM Total: " + String(ESP.getPsramSize() / 1024) + "KB  Free: " + String(ESP.getFreePsram() / 1024) + "KB";
  SDarray[SDIndex][13] = "SPIFFS Free: " + String(file_system_size / ONEMILLION) + "MB  Used: " + String(file_system_used / ONEMILLION) + "MB";
  SDarray[SDIndex][14] = String(esp_get_free_heap_size() / 1024.0) + "KB";
  SDarray[SDIndex][15] = String(esp_get_minimum_free_heap_size() / 1024.0) + "KB";
  SDarray[SDIndex][16] = String(esp_get_free_internal_heap_size() / 1024.0) + "KB";
  SDarray[SDIndex][17] = String(program_size / 1024.0) + "KB";
  SDarray[SDIndex][18] = CPUTEMP;
  SDarray[SDIndex][19] = tempValues[0];
  SDarray[SDIndex][20] = BUS2_BusVoltage;
  SDarray[SDIndex][21] = BUS2_Current;
  SDarray[SDIndex][22] = "X" + String(X) + " Y" + String(Y) + " Z" + String(Z);
  SDarray[SDIndex][23] = data.temperature;
  SDarray[SDIndex][24] = data.humidity;
  SDarray[SDIndex][25] = data.pressure;
*/
    for (int i = 0; i < numProfiles; ++i) {
      // SDarray[SDIndex][26 + i] = bme_resistance_avg[i];
      airLog += String(bme_resistance_avg[i]) + ", ";
    }
    airLog += String(data.temperature, 3) + ", ";
    airLog += String(data.humidity, 2) + ", ";
    airLog += String(data.pressure, 3) + ", ";
    airLog += String(VOC) + ", ";
    airLog += String(NOX) + ", ";
    airLog += String(srawVoc) + ", ";
    airLog += String(srawNox);
    airLog += "\r\n";

    /* SDarray[SDIndex][numProfiles + 26] = VOC;
  SDarray[SDIndex][numProfiles + 27] = NOX;
  SDarray[SDIndex][numProfiles + 28] = srawVoc;
  SDarray[SDIndex][numProfiles + 29] = srawNox; */


    // if (serialPrintLOG) {
    // String logLine = "";
    // Serial.print("Raw Data: ");
    /*
    for (int j = 0; j < consoleColumns; ++j) {
      if (!SDarray[SDIndex][j].isEmpty()) {
        // Serial.print(SDarray[SDIndex][j]);
        // consoleLine++;
        // console[consoleLine][i] = String(SDarray[SDIndex][j]);
        // logLine += SDarray[SDIndex][j] + ", ";
      }
    } */
    // logLine += "\r\n";
    // Serial.println();

    LittleFS.begin();
    // const char *logLineCStr = logLine.c_str();
    const char *airLogcstr = airLog.c_str();
    appendFile(LittleFS, logfilePath, airLogcstr);
    LittleFS.end();
    airLog.clear();
    // }
  }

  // SDIndex++;  // Move to the next row for the next measurements
  // if (SDIndex >= consoleRows) SDIndex = 0;

  debugF(loggingTracker);
  loggingTracker = (micros() - loggingTracker) / double(ONETHOUSAND);
}



void mountSD() {
  TAG = "initSD()   ";
  // timeTracker = micros();
  SDinserted = !digitalRead(GPIO_NUM_47);

  if (SDinserted) {

    if (!SD.begin(SD_CS, sdSPI, 40000000, "/", 50)) {
      // tft.drawString("SD: Card mount failed.", 60, 100, 3);
      return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      // tft.drawString("SD: No SD Card found.", 60, 100, 3);
      return;
    }

    // tft.drawString("Card Size:   " + String(SD.cardSize() / (1024 * 1024)) + "MB", 20, 10, 2);
    // tft.drawString("Total Space: " + String(SD.totalBytes() / (1024 * 1024)) + "MB", 20, 20, 2);
    // tft.drawString("Used Space:  " + String(SD.usedBytes() / (1024 * 1024)) + "MB", 20, 30, 2);

    if (cardType == CARD_MMC) {
      // tft.drawString("Type: MMC", 20, 40, 2);
    } else if (cardType == CARD_SD) {
      // tft.drawString("Type: SDSC", 20, 40, 2);
    } else if (cardType == CARD_SDHC) {
      // tft.drawString("Type: SDHC", 20, 40, 2);
    } else {
      // tft.drawString("Type: UNKNOWN", 20, 40, 2);
    }


    // listDir(SD, "/", 0);
    // createDir(SD, "/testFolder");
    // listDir(SD, "/", 0);
    // removeDir(SD, "/testFolder");
    // delay(10000);
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
  filesCount = 0;

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

  getSPIFFSsizes();
  file.close();
}




void appendFile(fs::FS &fs, const char *path, const char *message) {
  // Serial.printf("Appending to file: %s\n", path);

  if (percentLeftLFS <= 0.05) {
    Serial.println("SPIFFS full.");
    return;
  }

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    // Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }

  getSPIFFSsizes();
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
