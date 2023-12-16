
#include <pgmspace.h>





String generateTHPchart() {
  LittleFS.begin();
  File file = LittleFS.open(logFilePath.c_str(), "r");
  if (!file) {
    LittleFS.end();
    return "console.error('Error opening file');";
  }
  String chartData = "<script type='text/javascript'>";
  chartData += "function drawTPHchart() {\n"
               "  var data_thp = google.visualization.arrayToDataTable([\n"
               "    [";

  chartData += "'" + logColumns[0] + "', ";  // time
  chartData += "'" + logColumns[log_idx_bme1_temp] + "', ";
  chartData += "'" + logColumns[log_idx_bme1_humid] + "', ";
  chartData += "'" + logColumns[log_idx_bme1_press] + "'";
  chartData += "],\n";

  String line;
  size_t lineCount = 0;

  while (file.available() && lineCount < chart_max_data) {
    line = file.readStringUntil('\n');

    int commaIndex = line.indexOf(',');
    String timestampChar = line.substring(0, commaIndex);
    String values = line.substring(commaIndex + 1);

    String valueArray[4];
    int i = 0;
    size_t lastCommaIndex = 0;

    valueArray[0] = convertLogTimestampForChart(timestampChar);

    while (i <= log_idx_bme1_press) {
      size_t currentCommaIndex = values.indexOf(',', lastCommaIndex);

      if (currentCommaIndex == std::string::npos) {
        currentCommaIndex = values.length();
      }

      String value = values.substring(lastCommaIndex, currentCommaIndex);
      if (value.length() == 0) {
        break;
      } else {
        if (i == log_idx_bme1_temp - 1 || i == log_idx_bme1_humid - 1 || i == log_idx_bme1_press - 1) valueArray[i - (log_idx_bme1_temp - 2)] = value;
      }
      lastCommaIndex = currentCommaIndex + 1;  // Move to the next character after the comma
      i++;
    }

    lineCount++;
    if (i < log_idx_bme1_temp) {  // skip the line if empty
      continue;
    }

    chartData += "["
                 + valueArray[0] + ", "
                 + valueArray[1] + ", "
                 + valueArray[2] + ", "
                 + valueArray[3]
                 + "],\n";
  }

  chartData += "  ]);\n\n"
               "  var maxDataValue = calculateMaxValue(data_thp);\n"
               "  var options_tph = {\n"
               "    title: 'Temp, Humid, Press',\n"
               "    curveType: 'function',\n"
               "    legend: { position: 'bottom' },\n"
               "    backgroundColor: 'transparent',\n"
               "    series: {\n"
               // "      16: {targetAxisIndex: 1},\n"
               // "      2: {targetAxisIndex: 1},\n"
               "      2: {targetAxisIndex: 1}\n"
               "    },\n"
               "    vAxes: {\n"
               "      0: { viewWindow: { min: -5, max: 80 } },  // Left axis\n"
               "      1: { viewWindow: { min: maxDataValue - 3000, max: maxDataValue + 3000 } }   // Right axis\n"
               "    }\n"
               "  };\n\n"
               "  var chart = new google.visualization.LineChart(document.getElementById('thp_chart'));\n"
               "  chart.draw(data_thp, options_tph);\n"
               "}\n";
  chartData += "google.charts.load('current', {'packages':['corechart']});"
               "google.charts.setOnLoadCallback(drawTPHchart);"
               "</script>";

  file.close();
  LittleFS.end();
  return chartData;
}



String generateBMEchart() {
  LittleFS.begin();
  File file = LittleFS.open(logFilePath.c_str(), "r");
  if (!file) {
    LittleFS.end();
    return "console.error('Error opening file');";
  }
  // size_t fileSize = file.size();
  // size_t startPosition = fileSize > 8000 ? fileSize - 8000 : 0;
  // file.seek(startPosition);

  String chartData = "<script type='text/javascript'>";
  chartData += "function drawBMEchart() {\n"
               "  var data = google.visualization.arrayToDataTable([\n"
               "    [";


  for (int i = 0; i < 15; ++i) {
    chartData += "'" + logColumns[i] + "'";
    if (i < 14) chartData += ", ";
  }

  chartData += "],\n";
  String line;
  size_t lineCount = 0;

  while (file.available() && lineCount < chart_max_data) {
    line = file.readStringUntil('\n');

    // Split the line into timestamp and values
    int commaIndex = line.indexOf(',');
    String timestampChar = line.substring(0, commaIndex);
    String values = line.substring(commaIndex + 1);

    String valueArray[15];
    int i = 1;
    size_t lastCommaIndex = 0;

    valueArray[0] = convertLogTimestampForChart(timestampChar);

    // Split the values using commas
    while (i < 15) {
      size_t currentCommaIndex = values.indexOf(',', lastCommaIndex);

      if (currentCommaIndex == std::string::npos) {
        currentCommaIndex = values.length();
      }

      String value = values.substring(lastCommaIndex, currentCommaIndex);

      if (value.length() == 0) {
        break;
      } else {
        valueArray[i] = value;
      }

      lastCommaIndex = currentCommaIndex + 1;  // Move to the next character after the comma
      i++;
    }

    lineCount++;

    if (i < 14) {
      continue;
    }

    chartData += "["
                 + valueArray[0] + ", "
                 + valueArray[1] + ", " + valueArray[2] + ", "
                 + valueArray[3] + ", " + valueArray[4] + ", "
                 + valueArray[5] + ", " + valueArray[6] + ", "
                 + valueArray[7] + ", " + valueArray[8] + ", "
                 + valueArray[9] + ", " + valueArray[10] + ", "
                 + valueArray[11] + ", " + valueArray[12] + ", "
                 + valueArray[13] + ", " + valueArray[14] + ", "
                 //  + valueArray[15] + ", " + valueArray[16] + ", "
                 //  + valueArray[17] + ", " + valueArray[18] + ", "
                 //  + valueArray[19] + ", " + valueArray[20] + ", " + valueArray[21]
                 + "],\n";
  }



  chartData += "  ]);\n\n"
               "  var maxDataValue = calculateMaxValue(data);\n"
               "  var options = {\n"
               "    title: 'BME[1] log',\n"
               "    curveType: 'function',\n"
               "    legend: { position: 'bottom' },\n"
               "    backgroundColor: 'transparent',\n"
               // "    width: '100%',\n"   // Set width to 100% for responsiveness
               // "    height: '100%',\n"  // Set height to 100% for responsiveness
               // "    series: {\n"
               // "      16: {targetAxisIndex: 1},\n"
               // "      19: {targetAxisIndex: 1},\n"
               // "      20: {targetAxisIndex: 1},\n"
               // "    },\n"
               "    vAxes: {\n"
               "      0: { viewWindow: { min: 0, max: maxDataValue } },  // Left axis\n"
               "      1: { viewWindow: { min: 0, max: maxDataValue } }   // Right axis\n"
               "    }\n"
               "  };\n\n"
               "  var chart = new google.visualization.LineChart(document.getElementById('chart_div'));\n"
               "  chart.draw(data, options);\n"
               "}\n";
  chartData += "google.charts.load('current', {'packages':['corechart']});"
               "google.charts.setOnLoadCallback(drawBMEchart);"
               "</script>";

  file.close();
  LittleFS.end();
  return chartData;
}




String generateSGPchart() {
  LittleFS.begin();
  File file = LittleFS.open(logFilePath.c_str(), "r");
  if (!file) {
    LittleFS.end();
    return "console.error('Error opening file');";
  }
  String chartData = "<script type='text/javascript'>";
  chartData += "function drawSGPchart() {\n"
               "  var data_sgp = google.visualization.arrayToDataTable([\n"
               "    [";

  chartData += "'" + logColumns[0] + "', ";  // time
  chartData += "'" + logColumns[log_idx_sgp_voc] + "', ";
  chartData += "'" + logColumns[log_idx_sgp_nox] + "'";
  chartData += "],\n";

  String line;
  size_t lineCount = 0;

  while (file.available() && lineCount < chart_max_data) {
    line = file.readStringUntil('\n');

    int commaIndex = line.indexOf(',');
    String timestampChar = line.substring(0, commaIndex);
    String values = line.substring(commaIndex + 1);

    String valueArray[3];
    int i = 0;
    size_t lastCommaIndex = 0;

    valueArray[0] = convertLogTimestampForChart(timestampChar);


    while (i <= log_idx_sgp_nox) {
      size_t currentCommaIndex = values.indexOf(',', lastCommaIndex);

      if (currentCommaIndex == std::string::npos) {
        currentCommaIndex = values.length();
      }

      String value = values.substring(lastCommaIndex, currentCommaIndex);
      if (value.length() == 0) {
        break;
      } else {
        if (i == log_idx_sgp_voc - 1 || i == log_idx_sgp_nox - 1) valueArray[i - (log_idx_sgp_voc - 2)] = value;
      }
      lastCommaIndex = currentCommaIndex + 1;  // Move to the next character after the comma
      i++;
    }

    lineCount++;
    if (i < log_idx_sgp_voc) {  // skip the line if empty
      continue;
    }

    chartData += "["
                 + valueArray[0] + ", "
                 + valueArray[1] + ", "
                 + valueArray[2]
                 + "],\n";
  }

  chartData += "  ]);\n\n"
               "  var maxDataValue = calculateMaxValue(data_sgp);\n"
               "  var options_sgp = {\n"
               "    title: 'VOC, NOx',\n"
               "    curveType: 'function',\n"
               "    legend: { position: 'bottom' },\n"
               "    backgroundColor: 'transparent',\n"
               "    series: {\n"
               "      1: {targetAxisIndex: 1}\n"
               "    },\n"
               "    vAxes: {\n"
               "      0: { viewWindow: { min: 0, max: maxDataValue + 20 } },  // Left axis\n"
               "      1: { viewWindow: { min: 0, max: maxDataValue + 20 } }   // Right axis\n"
               "    }\n"
               "  };\n\n"
               "  var chart = new google.visualization.LineChart(document.getElementById('sgp_chart'));\n"
               "  chart.draw(data_sgp, options_sgp);\n"
               "}\n";
  chartData += "google.charts.load('current', {'packages':['corechart']});"
               "google.charts.setOnLoadCallback(drawSGPchart);"
               "</script>";

  file.close();
  LittleFS.end();
  return chartData;
}


String generateSensorSettingsTable() {

  // Sensor Settings Table
  String output = "<table>";
  output += "<tr><td colspan='5'><h2> Sensor Settings </h2></td></tr>";

  output += "<tr><td><label for='loggingInterval'><b> Interval</b></label></td>";
  output += "<td><select id='loggingInterval' onchange='updateLoggingInterval()'>";
  output += generateTimeOptions(loggingInterval);
  output += "</select></td>";
  output += "</tr>";

  output += "<tr><td><label for='bmeSamples'><b>Samples</b></label></td>";
  output += "<td><select id='bmeSamples' onchange='updateBMEsamples()'>";
  output += valueOptions(bmeSamples);
  output += "</select></td>";
  output += "</tr>";

  output += "<tr><td><label for='bmeFilter'><b>Filter</b></label></td>";
  output += "<td><select id='bmeFilter' onchange='updateBMEfilter()'>";
  output += valueOptions(bmeFilter);
  output += "</select></td>";
  output += "</tr>";

  output += "<tr><td><label for='bmePause'><b> Pause</b>[ms]</label></td>";
  output += "<td><select id='bmePause' onchange='updateBMEpause()'>";
  output += valueOptions(bmeProfilePause);
  output += "</select></td>";
  output += "</tr>";

  output += "<tr><td>&nbsp;</td></tr>";  // empty Row
  output += "<tr><td><b> Conditioning </td><td>" + String(conditioning_duration) + "s</td></tr>";
  output += "<tr><td><b> Offst Delta </td><td>" + String(offsetDelta) + "</td></tr>";
  output += "<tr><td><b> Smpl Delta </td><td>" + String(samplingDelta) + "</td></tr>";
  output += "<tr><td>&nbsp;</td></tr>";  // empty Row

  output += "<tr style='font-size: 12px;'><td><b> Profile Heater </td>";
  for (i = 0; i < numProfiles; i++) {
    output += "<td><b>H" + String(i) + "</b></br>" + String(heatProf_1[i]) + "&deg;</td>";
  }
  output += "</tr>";

  output += "<tr style='font-size: 12px;'><td><b> Profile Duration </td>";
  for (i = 0; i < numProfiles; i++) {
    output += "<td><b>D" + String(i) + "</b></br>" + String(durProf_1[i]) + "ms</td>";
  }
  output += "</tr>";
  output += "</table>";

  return output;
}


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
  output += "<tr><th colspan='2'><h2> Device Stats </h2></th></tr>";
  output += "<tr><td style='font-size:11px;'>" + String(CONFIG_IDF_TARGET) + "<br> Model " + String(chip_info.model) + "<br> Rev " + String(chip_info.full_revision) + "." + String(chip_info.revision) + "</td>";
  output += "<td id='subhead' colspan='2'> " + String(powerStateNames[currentPowerState]) + "</td>";
  output += "<td id='subhead' colspan='2'>" + resetReasonString + "</td></tr>";
  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td id='subhead'><b> CPU </td><td>" + String(cpu_freq_mhz) + "MHz</td><td>" + String(flash_speed / ONEMILLION) + "MHz flash</td><td>" + String(chip_info.cores) + "Core</td>";
  output += "<td>" + String((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi | " : "") + String((chip_info.features & CHIP_FEATURE_BT) ? "BT " : "") + String((chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "") + "</td></tr>";
  output += "<tr><td id='subhead'><b>Flash " + String((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embed" : "ext") + "</td><td> T: " + String(flash_size / ONEMILLIONB) + "Mb</td><td> F: " + String(free_flash_size / ONEMILLIONB) + "Mb</td><td>U: " + String(flash_used / ONEMILLIONB) + "Mb</td><td>" + String(flash_UsedP) + "%</td></tr>";
  if (program_size > 0) output += "<tr><td id='subhead'><b>Program</td><td> T: " + String(program_size / ONEMILLIONB, 2) + "Mb</td><td> F: " + String(program_free / ONEMILLIONB, 2) + "Mb</td><td> U: " + String(program_used / ONEMILLIONB, 2) + "Mb</td><td>" + String(program_UsedP) + "%</td></tr>";
  if (deviceInfo.total_allocated_bytes > 0) output += "<tr><td id='subhead'><b>PSRAM</td><td> T: " + String(deviceInfo.total_allocated_bytes / KILOBYTE) + "Kb</td><td>  F: " + String(deviceInfo.total_free_bytes / KILOBYTE) + /*"Mb</td><td>  T Blocks: " + String(deviceInfo.total_blocks) + "</td><td>  F Blocks: " + String(deviceInfo.free_blocks) + */ "</td></tr>";
  if (SPIFFS_size > 0) output += "<tr><td id='subhead'><b>SPIFFS</td><td> T: " + String(SPIFFS_size / ONEMILLIONB) + "Mb</td><td>  F: " + String(SPIFFS_free / ONEMILLIONB) + "Mb</td><td>  U: " + String(SPIFFS_used / ONEMILLION) + "Mb</td><td>" + String(percentUsedLFS) + "%</td></tr>";

  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td id='subhead'><b> RAM </td><td><b> Free Heap </td><td><b> Free Int Heap </td><td><b> Min Free Heap </td><td><b> </td><td><b>  </td>";
  output += "<tr><td></td><td>" + String(esp_get_free_heap_size() / KILOBYTE) + "Kb</td>";
  output += "<td>" + String(esp_get_free_internal_heap_size() / KILOBYTE) + "Kb</td>";
  output += "<td>" + String(esp_get_minimum_free_heap_size() / KILOBYTE) + "Kb</td> ";
  output += "</tr>";
  output += "<tr><td colspan='6'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td id='subhead'><b> Comms </td><td><b> WiFi SSID </td><td><b> Local IP </td><td><b> RSSI </td><td><b> Channel </td>";
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
  output += "<tr><th colspan='5'><h2>System Sensors</h2></th></tr>";
  output += "<tr><td id='subhead'><b>INA2</b></br> " + String(INA2.isConnected() ? "&check;" : "&cross;") + " " + String(INA2_iscalibrated ? "&check;" : "&cross;") + "</br>" + String(BUS2_OVF ? "OverflowMath!" : "") + "</td>";
  // output += "<tr><td colspan='4'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<td id='subhead'><b>Volt </b></br> " + String(BUS2_BusVoltage / (ONETHOUSAND), 2) + "V</td>"
            + "<td id='subhead'><b>Amp</b></br> " + String(BUS2_Current, 2) + "mA</td><td id='subhead'><b>Shunt</b></br> " + String(BUS2_ShuntVoltage, 2) + "mV</td><td id='subhead'><b>Power</b></br> " + String(BUS2_Power, 2) + "mW</td></tr>";
  // output += "<tr><td>" + String(BUS2_BusVoltage / (ONETHOUSAND)) + "V</td><td>" + String(BUS2_Current) + "mA</td><td>" + String(BUS2_ShuntVoltage) + "mV</td><td>" + String(BUS2_Power) + "mW</td></tr>";
  output += "<tr><td>&nbsp;</td></tr>";  // empty Row DTdevice
  output += "<tr><td colspan='5'><hr style='border: 1px solid #808080;'></td></tr>";

  for (i = 0; i < sizeof(DTprobe) / sizeof(DTprobe[0]); i++) {
    output += "<td id='subhead' style='width:90px;'><b>" + String(DTprobe[i].name) + "</b></br>";
    output += String(DTprobe[i].temperature) + "&deg;C</td>";
  }
  output += "</tr>";

  // output += "<tr><td><b>CPU</td><td><b>BME[1]</td></tr>";
  // output += "<tr><td>" + String(CPUTEMP) + "&deg;C</td><td>" + String(data.temperature) + "&deg;C</td></tr>";

  output += "<tr><td id='subhead'><b>CPU</b></br>" + String(CPUTEMP) + "&deg;C</td>";
  output += "<td id='subhead'><b>BME[1]</b></br>" + String(data.temperature) + "&deg;C</td>";
  output += "</tr>";

  output += "<tr><td colspan='5'><hr style='border: 1px solid #808080;'></td></tr>";

  output += "<tr><td><b>Light Sensors</td></tr>";

  output += "</table>";

  return output;
}



String generateTaskManagerTable() {
  String table = "<table style='display: flex;'>";
  table += "<tr><th colspan='2'><h2>Task Manager</h2></th></tr>";
  // table += "<tr><td><b>ID\tState\tName\tLast Dur\tLast</td></tr>";

  for (const auto& task : tasks) {
    if (taskFreeSlots[*task.taskId] != char('F') && *task.tracker > 0.00 || *task.taskId != 0) {  // don't add free, unscheduled task slots
      table += "<tr><td id='subhead' style='padding: 5px 15px; text-align: left; width: 120px; border-top-left-radius: 10px; border-bottom-left-radius: 10px; border-top-right-radius: 0px; border-bottom-right-radius: 10x;'>[" + String(*task.taskId) + "]<b> " + String(task.taskName) + "</b></td>"
               + "<td id='subhead' style='padding: 5px; text-align: left; width: 15px; border-radius: 0px; '>[" + String(taskFreeSlots[*task.taskId]) + "]</td>"
               + "<td id='subhead' style='padding: 5px 15px; text-align: left; min-width: 120px; border-top-left-radius: 0px; border-bottom-left-radius: 0px; border-top-right-radius: 10px; border-bottom-right-radius: 10px;''>" + String(*task.tracker) + "ms</td>"
               + "</tr>";
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
  table_fs += "<tr><td colspan='5'><hr style='border: 1px solid #808080;'></td></tr>";

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
