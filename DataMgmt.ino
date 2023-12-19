#include <pgmspace.h>




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

void getFlashInfo() {
  flash_size = ESP.getFlashChipSize();
  flash_speed = ESP.getFlashChipSpeed();
  free_flash_size = flash_size - program_used - SPIFFS_used;
  flash_used = flash_size - free_flash_size;
  flash_UsedP = (flash_used * 100.0) / flash_size;
  flash_LeftP = 100.0 - flash_UsedP;
}




void getDeviceInfo() {

  SDinserted = !digitalRead(GPIO_NUM_47);
  resetReasonString = print_wakeup_reason();
  if (WiFi.status() == WL_CONNECTED) WiFiIP = WiFi.localIP().toString();

  LittleFS.begin();
  getSPIFFSsizes();
  LittleFS.end();

  getProgramInfo();

  getFlashInfo();

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




void logging() {
  TAG = "logging()    ";
  loggingTracker = micros();
  taskManager.checkAvailableSlots(taskFreeSlots, slotsSize);

  if (!conditioning_duration) {
    logFilePath = rootHexPath + "/LOG_" + printDate + ".csv";

    std::ostringstream airLog;
    airLog << printTime.c_str() << ", ";

    for (const auto &resistance : bme_resistance_avg) {
      airLog << resistance << ", ";
    }

    airLog.precision(2);                               // Set precision
    airLog << std::fixed << bme1_data.temperature << ", ";  // set precision for temperature
    airLog << std::fixed << bme1_data.humidity << ", ";
    airLog << std::fixed << bme1_data.pressure << ", ";
    airLog << VOC << ", ";
    airLog << NOX << ", ";
    airLog << srawVoc << ", ";
    airLog << srawNox << ", ";
    airLog << co2SCD << ", ";
    airLog << std::fixed << tempSCD << ", ";
    airLog << std::fixed << humidSCD << "\n";

    if (!LittleFS.begin()) return;  // Ensure file system initialization succeeds
    appendFile(LittleFS, logFilePath.c_str(), airLog.str().c_str());
    LittleFS.end();
  }

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
  }
}



String listDirWeb(fs::FS &fs, const char *dirname, uint8_t levels) {
  String fsString = "";
  File root = fs.open(dirname);

  fileId = 0;

  if (!root || !root.isDirectory()) {
    Serial.println("Root: " + String(root));
    return fsString = "<tr><td>[Error] root is not a directory, or root doesn't exist. !</td></tr>";
  }

  File file = root.openNextFile();

  while (file) {
    String fileIdStr = String(fileId);
    if (file.isDirectory()) {
      directoryCount++;

      fsString += "<tr style='border-bottom: 1px solid #808080;'>";

      if (levels == 3) fsString += "<br><td>&nbsp;</td>";
      if (levels == 2) fsString += "<td>&nbsp;</td><td>&nbsp;</td>";
      if (levels == 1) fsString += "<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>";

      fsString += "<div style='color:black;' class='dir' id='dir_" + fileIdStr + "'><td><b>/" + String(file.name()) + "</td></div></tr>";
      if (levels) {
        fsString += listDirWeb(fs, file.path(), levels - 1);
      }
    } else {
      filesCount++;
      fsString += "<tr style='border-bottom: 1px solid #808080;'>";

      if (levels == 3) fsString += "<td>&nbsp;</td>";
      if (levels == 2) fsString += "<td>&nbsp;</td><td>&nbsp;</td>";
      if (levels == 1) fsString += "<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>";

      fsString += "<div style='color:black;' class='file' id='fil_" + fileIdStr + "'>";
      double fileSizeMB = double(file.size()) / KILOBYTE / KILOBYTE;
      fsString += "<td>/" + String(file.name()) + " [";
      fsString += String(fileSizeMB, 3) + "Mb]</td></div></tr>";
    }
    file = root.openNextFile();
    fileId++;
  }

  return fsString;
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
  if (!fs.exists(path)) {
    createDir(fs, rootHexPath.c_str());  // create system root Dir for safety
    if (LOGGING) {
      writeFile(fs, logFilePath.c_str(), logHeader.c_str());
      if (!fs.open(path, FILE_APPEND)) {
        file = fs.open(path, FILE_APPEND);
      }
      // file.print(logHeader.c_str());
    }
  }

  // writeFile(fs, path, message);

  if (!file.print(message)) {
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
