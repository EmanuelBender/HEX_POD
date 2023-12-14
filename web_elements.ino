
#include <pgmspace.h>





String generateDeviceControlsTable() {

  String output = "<table>";
  output += "<tr><th colspan='6'><h2>Device Controls</h2></th></tr>";

  output += "<tr>";
  output += "<td><button onclick='toggleLED()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(LEDon ? "#008080; border: none;" : "#505050; border: solid 1px #808080;") + "'>LED</button></td>";
  output += "<td><button onclick='toggleFAN()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(FANon ? "#008080; border: none;" : "#505050; border: solid 1px #808080;") + "'>FAN</button></td>";
  output += "<td><button onclick='toggleOLED()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(OLEDon ? "#008080; border: none;" : "#505050; border: solid 1px #808080;") + "'>OLED</button></td>";
  output += "<td><button onclick='toggleLightSleep()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(SLEEPENABLE ? "#008080; border: none;" : "#505050; border: solid 1px #808080;") + "'>SLEEP</button></td>";
  output += "<td><button onclick='updateNTP()' style='padding: 10px 15px; font-size: 14px; background-color:#505050; border: solid 1px #505050;'>Update Time</button></td>";
  output += "</tr>";
  output += "</table>";

  return output;
}


String generateDeviceStatsTable() {
  String output = "<table>";
  output += "<tr><th colspan='2'><h2>Device Stats</h2></th></tr>";
  output += "<tr><td>" + String(CONFIG_IDF_TARGET) + "<br> Model " + String(chip_info.model) + "<br> Rev " + String(chip_info.full_revision) + "." + String(chip_info.revision) + "</td>";
  output += "<td><b>Power State</td><td> " + String(powerStateNames[currentPowerState]) + "</td>";
  output += "<td><b>Reset </td><td>" + resetReasonString + "</td></tr>";
  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td><b>CPU:</td><td>" + String(cpu_freq_mhz) + "MHZ</td><td>" + String(CPUTEMP) + "&deg;C</td><td>" + String(chip_info.cores) + "Core</td>";
  output += "<td>" + String((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi | " : "") + String((chip_info.features & CHIP_FEATURE_BT) ? "BT " : "") + String((chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "") + "</td></tr>";
  output += "<tr><td><b>Flash " + String((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embed" : "ext") + "</td><td> Total: " + String(flash_size / ONEMILLIONB) + "Mb</td><td> Free: " + String(free_flash_size / ONEMILLIONB) + "Mb</td><td>" + String(flash_speed / ONEMILLION) + "MHz</td><td>" + String(flash_UsedP) + "%</td></tr>";
  if (program_size > 0) output += "<tr><td><b>Program</td><td> Total: " + String(program_size / ONEMILLIONB, 2) + "Mb</td><td> Free: " + String(program_free / ONEMILLIONB, 2) + "Mb</td><td> Used: " + String(program_used / ONEMILLIONB, 2) + "Mb</td><td>" + String(program_UsedP) + "%</td></tr>";
  if (deviceInfo.total_allocated_bytes > 0) output += "<tr><td><b>PSRAM</td><td> Total: " + String(deviceInfo.total_allocated_bytes / KILOBYTE) + "Kb</td><td>  Free: " + String(deviceInfo.total_free_bytes / KILOBYTE) + /*"Mb</td><td>  T Blocks: " + String(deviceInfo.total_blocks) + "</td><td>  F Blocks: " + String(deviceInfo.free_blocks) + */ "</td></tr>";
  if (SPIFFS_size > 0) output += "<tr><td><b>SPIFFS</td><td> Total: " + String(SPIFFS_size / ONEMILLIONB) + "Mb</td><td>  Free: " + String(SPIFFS_free / ONEMILLIONB) + "Mb</td><td>  Used: " + String(SPIFFS_used / ONEMILLION) + "Mb</td><td>" + String(percentUsedLFS) + "%</td></tr>";

  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td><b>RAM</td><td><b> Free Heap </td><td><b> Min Free Heap  </td><td><b> Free Int Heap </td><td><b> </td><td><b>  </td>";
  output += "<tr><td></td><td>" + String(esp_get_free_heap_size() / KILOBYTE) + "Kb</td>";
  output += "<td>" + String(esp_get_minimum_free_heap_size() / KILOBYTE) + "Kb</td> ";
  output += "<td>" + String(esp_get_free_internal_heap_size() / KILOBYTE) + "Kb</td>";
  output += "</tr>";
  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td><b> Comms </td><td><b> WiFi SSID </td><td><b> Local IP </td><td><b> RSSI </td><td><b> Channel </td>";
  output += "<tr><td>&nbsp;</td>";
  output += "<td>" + String(WiFi.SSID()) + "</td>";
  output += "<td>" + String(WiFiIP) + "</td>";  // wifiStatusChar[WiFi.status()]
  output += "<td>" + String(WiFi.RSSI()) + "db</td>";
  output += "<td>" + String(WiFi.channel()) + "</td>";
  output += "</tr>";
  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td>&nbsp;</td><td><b> SD Card </td><td><b> </td><td><b> Last NTP </td><td><b> Last Reset </td><td></td></tr>";
  output += "<tr><td>&nbsp;</td><td>";
  output += SDinserted ? "Present" : "None";
  output += "</td>";
  output += "<td>&nbsp;</td>";
  output += "<td>" + String(lastNTPtime) + "</td>";
  output += "<td>" + lastRestart + "</td>";
  output += "</tr>";
  output += "</table>";

  return output;
}

String generateSystemSensorsTable() {

  String output = "<table>";
  output += "<tr><th colspan='4'><h2>System Sensors</h2></th></tr>";
  output += "<tr><td><b>INA2</td><td> " + String(INA2.isConnected() ? "Connected" : "") + "</td><td>" + String(INA2_iscalibrated ? "Calibrated" : "") + "</td><td>" + String(BUS2_OVF ? "OverflowMath!" : "") + "</tr></td>";
  output += "<tr><td colspan='4'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td><b>Volt</td><td><b>Amp</td><td><b>Shunt</td><td><b>Power</b></td></tr>";
  output += "<tr><td>" + String(BUS2_BusVoltage / (ONETHOUSAND)) + "V</td><td>" + String(BUS2_Current) + "mA</td><td>" + String(BUS2_ShuntVoltage) + "mV</td><td>" + String(BUS2_Power) + "mW</td></tr>";
  output += "<tr><td>&nbsp;</td></tr>";  // empty Row DTdevice
  output += "<tr><td colspan='4'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td colspan='4'><b>" + String(DTdevice) + " DS18B20</td></tr>";
  output += "<tr>";

  for (int i = 0; i < DTdevice; i++) {
    output += "<td><b>" + String(tempID[i]) + "</td>";
  }
  output += "</tr><tr>";

  for (int i = 0; i < DTdevice; i++) {
    if (tempValues[i] > 0.0) output += "<td>" + String(tempValues[i]) + "&deg;C</td>";
  }
  output += "</tr>";

  output += "<tr><td><b>CPU</td></tr>";
  output += "<tr><td>" + String(CPUTEMP) + "&deg;C</td></tr>";

  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td><b>Light Sensors</td></tr>";

  output += "</table>";

  return output;
}



String generateTaskManagerTable() {
  String table = "<table>";
  table += "<tr><th colspan='5'><h2>Task Manager</h2></th></tr>";
  table += "<tr><td><b>ID</td><td><b>State</td><td><b>Name</td><td><b>Last Dur</td><td><b>Last</td></tr>";

  for (const auto& task : tasks) {
    if (taskArray[*task.taskId] != "" /*&& *task.tracker > 0.0 */ && *task.taskId != 0) {  // don't add free, unscheduled task slots
      table += "<tr><td>[" + String(*task.taskId) + "]</td><td>" + String(taskArray[*task.taskId]) + "</td><td>" + task.taskName + " </td><td>" + " " + String(*task.tracker) + "ms</td></tr>";
    }
  }

  table += "</table>";
  return table;
}



String generateFSTable() {

  filesCount = 0;
  directoryCount = 0;

  String table_fs = "<table>";
  table_fs += "<tr><th><h2>FS</h2></th></tr>";

  table_fs += "<tr><td><b>Size</td><td>" + String(SPIFFS_size / ONEMILLIONB, 3) + "Mb</td></tr>";
  // table_fs += "<tr><td><b>Used</td><td>" + String(SPIFFS_used / ONEMILLIONB, 3) + "Mb</td></tr>";
  table_fs += "<tr><td><b>Sp Left</td><td>" + String(percentLeftLFS, 2) + "% </td></tr>";
  table_fs += "<tr><td><b>Log Path </td><td>" + String(logFilePath) + "</td></tr>";
  table_fs += "<tr><td colspan='4'><hr style='border: 1px solid #808080;'></td></tr>";

  table_fs += "<tr>";
  table_fs += "<td><button onclick='createFile()' style='padding: 10px 15px; font-size: 14px; background-color: #505050; border: solid 1px #808080;')>New File</button></td>";
  table_fs += "<td><button onclick='createDir()' style='padding: 10px 15px; font-size: 14px; background-color: #505050; border: solid 1px #808080;')>New Folder</button></td>";
  table_fs += "<td><button onclick='deletePath()' style='padding: 10px 15px; font-size: 14px; background-color: #505050; border: solid 1px #808080;')>Delete</button></td>";
  table_fs += "<td><button onclick='download()' style='padding: 10px 15px; font-size: 14px; background-color: #505050; border: solid 1px #808080;')>Download</button></td>";
  table_fs += "<td><input type='file' id='fileInput' accept='.csv, .txt' onchange='uploadFile()' style='display: none;'><button onclick='document.getElementById(\"fileInput\").click();' style='padding: 10px 15px; font-size: 14px; background-color: #505050; border: solid 1px #808080; '>Upload</button></td>";
  table_fs += "</tr>";

  table_fs += "<tr><td>&nbsp;</td></tr>";

  table_fs += "<tr><th><b>/</th></tr>";
  table_fs += "<pre>" + listDirWeb(LittleFS, "/", 4) + "</pre>";

  table_fs += "<tr><td>&nbsp;</td></tr>";
  table_fs += "<tr><td><button onclick='LOGMarker()' style='padding: 10px 15px; font-size: 14px; background-color: #505050; border: solid 1px #808080;')>LOG Marker</button></td></tr>";
  table_fs += "<tr><td><b>" + String(directoryCount) + " Folders</td></tr>";
  table_fs += "<tr><td><b>" + String(filesCount) + " Files</td></tr>";

  table_fs += "</table>";

  return table_fs;
}



String generateLogFileContent() {

  File file = LittleFS.open(logFilePath);
  String output;

  if (file) {
    server.sendHeader("Content-Type", "text/html");
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Content-Disposition", "inline; filename=" + String(file.name()));

    float fileSizeMB = float(file.size()) / ONEMILLIONB;

    output += "<table style='width:auto;'>";
    output += "<tr><th><h2>Log File Content</h2></th></tr>";
    output += "<tr><td><b>" + String(file.name()) + "</td></tr>";
    output += "<tr><td>[" + String(fileSizeMB, 4) + "mb]</td></tr><tr><td><pre>";

    while (file.available()) {
      uint8_t buffer[1024];
      size_t bytesRead = file.read(buffer, sizeof(buffer));
      if (bytesRead > 0) {
        output += String(reinterpret_cast<const char*>(buffer), bytesRead);
        yield();  // Allow the server to handle other tasks
        // taskManager.delayMicroseconds(500);
      }
    }

    output += "</pre></td></tr></table>";
    file.close();
    server.client().stop();
  } else {
    output += "<table>";
    output += "<tr><td><h3>File does not exist. </h3></td></tr>";
    output += "</table>";
  }

  return output;
}
