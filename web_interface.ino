
#include <pgmspace.h>


void handleNotFound() {
  String message = "Page Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}



#include <TimeLib.h>  // Include the TimeLib library for time functions

unsigned long convertTimestampToMillis(String timestamp) {
  int hours, minutes, seconds;
  sscanf(timestamp.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);

  // Ensure hours, minutes, and seconds are within valid ranges
  if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59) {
    return 10000000;  // Return 0 if the timestamp is invalid
  }

  return (hours * 3600UL + minutes * 60UL + seconds) * 1000UL;
}



void streamToServer(String filePath) {
  //  filePath = logFilePath;

  if (!LittleFS.begin()) {
    server.send(500, "text/plain", "<h2>Spiffs failed</h2>");
    return;
  }

  if (LittleFS.exists(filePath)) {
    File file = LittleFS.open(filePath, "r");
    Serial.println(file);
    String contentType = "application/octet-stream";  // Default to binary/octet-stream
    if (filePath.endsWith(".txt")) {
      contentType = "text/plain";
    } else if (filePath.endsWith(".log")) {
      contentType = "text/plain";
    } else if (filePath.endsWith(".jpg")) {
      contentType = "image/jpeg";
    } else if (filePath.endsWith(".csv")) {
      contentType = "text/csv";
    }

    server.sendHeader("Content-Type", contentType);
    server.sendHeader("Content-Disposition", "attachment; filename=" + String(file.name()));
    server.streamFile(file, contentType);
    file.close();
  } else {
    server.send(500, "text/plain", "File not found");
  }
  LittleFS.end();
}






void setupWebInterface() {  // in setup()

  if (!MDNS.begin("HEX")) {  //  http://HEX.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  server.on("/", HTTP_GET, []() {
    if (!server.authenticate(webHost.c_str(), webPw.c_str())) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", generateHomePage());
  });
  server.on("/sensors", HTTP_GET, []() {
    if (!server.authenticate(webHost.c_str(), webPw.c_str())) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", generateSensorsPage());
  });
  server.on("/utility", HTTP_GET, []() {
    if (!server.authenticate(webHost.c_str(), webPw.c_str())) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", generateUtilityPage());
  });
  server.on("/filesystem", HTTP_GET, []() {
    if (!server.authenticate(webHost.c_str(), webPw.c_str())) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", generateFSPage());
  });
  server.on("/download", HTTP_GET, []() {
    String downloadPath = server.arg("path");
    Serial.print("Download Path:");
    Serial.println(downloadPath);
    if (!server.authenticate(webHost.c_str(), webPw.c_str())) {
      return server.requestAuthentication();
    }
    streamToServer(downloadPath);
  });


  server.on("/upload", HTTP_POST, []() {
    File fsUploadFile;

    // Check authentication if needed
    if (!server.authenticate(webHost.c_str(), webPw.c_str())) {
      return server.requestAuthentication();
    }

    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      // Extract file extension
      String filename = upload.filename;
      int lastDotIndex = filename.lastIndexOf('.');
      if (lastDotIndex == -1) {
        server.send(400, "text/plain", "Invalid file format");
        return;
      }

      String fileExtension = filename.substring(lastDotIndex);

      // Check FS size before writing
      if (upload.totalSize > SPIFFS_free) {
        server.send(400, "text/plain", "File size exceeds available space");
        return;
      }

      LittleFS.begin();
      getSPIFFSsizes();
      // Open the file for writing
      fsUploadFile = LittleFS.open("/" + filename, "w");
      if (!fsUploadFile) {
        server.send(500, "text/plain", "Failed to open file for writing");
        return;
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // Write the next chunk of data to the file
      if (fsUploadFile) {
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      // File upload finished
      if (fsUploadFile) {
        fsUploadFile.close();
      }
    }
    LittleFS.end();
  });


  server.on("/toggleLED", []() {
    LEDon = !LEDon;
    statusLED(LEDon);
    server.send(200, "text/plain", "LED toggled");
  });
  server.on("/toggleOLED", []() {
    OLEDon = !OLEDon;
    preferences.begin("my - app", false);
    preferences.putBool("oled", OLEDon);
    preferences.end();
    io.write(PCA95x5::Port::P07, OLEDon ? PCA95x5::Level::H : PCA95x5::Level::L);
    if (OLEDon) {
      u8g2.clearBuffer();
      u8g2.sleepOn();
      u8g2.setFontDirection(0);
      u8g2.setFontMode(0);
      u8g2.setFont(u8g2_font_logisoso28_tn);  // u8g2_font_u8glib_4_tf
      u8g2.begin();
      delay(50);
      updateTime();
    }
    server.send(200, "text/plain", "OLED toggled");
  });
  server.on("/toggleFAN", []() {
    FANon = !FANon;
    FANon ? digitalWrite(GPIO_NUM_1, true) : digitalWrite(GPIO_NUM_1, false);
    server.send(200, "text/plain", "FAN toggled");
  });

  server.on("/deletePath", HTTP_GET, []() {
    String pathToDelete = "/" + server.arg("path");
    if (pathToDelete.length() > 0) {
      LittleFS.begin();
      removeDir(LittleFS, pathToDelete.c_str());
      deleteFile(LittleFS, pathToDelete.c_str());
      // createDir(LittleFS, pathToDelete.c_str());
      // LittleFS.remove(pathToDelete.c_str());
      LittleFS.end();

      server.send(200, "text/plain", "File or directory deleted: " + pathToDelete);
    } else {
      server.send(400, "text/plain", "Invalid or missing path parameter");
    }
  });
  server.on("/createFile", HTTP_GET, []() {
    String pathToCreate = "/" + server.arg("path");
    if (pathToCreate.length() > 0) {
      LittleFS.begin();
      writeFile(LittleFS, pathToCreate.c_str(), "");
      LittleFS.end();

      server.send(200, "text/plain", "File created: " + pathToCreate);
    } else {
      server.send(400, "text/plain", "Invalid or missing path parameter");
    }
  });
  server.on("/createDir", HTTP_GET, []() {
    String pathToCreate = "/" + server.arg("path");
    if (pathToCreate.length() > 0) {
      LittleFS.begin();
      createDir(LittleFS, pathToCreate.c_str());
      LittleFS.end();

      server.send(200, "text/plain", "Directory created: " + pathToCreate);
    } else {
      server.send(400, "text/plain", "Invalid or missing path parameter");
    }
  });

  server.on("/LOGMarker", HTTP_GET, []() {
    String markerText = server.arg("markerText");
    addLOGmarker("[M]", markerText);
    server.send(200, "text/plain", "LOG Marker set");
  });


  server.on("/updateLoggingInterval", HTTP_GET, []() {
    loggingInterval = server.arg("value").toInt();

    preferences.begin("my - app", false);
    preferences.putUInt("logItvl", loggingInterval);
    preferences.end();
    bmeInterval = loggingInterval;

    repeater = 0;
    consoleLine = 0;
    offsetDelta = 0;
    for (int i = 0; i < numProfiles; ++i) {  // empty resistance array
      bme_resistance[i] = 0;
    }
    launchUtility();
    server.send(200, "text/plain", "Logging interval updated");
  });

  server.on("/updateBMEsamples", HTTP_GET, []() {
    bmeSamples = server.arg("value").toInt();

    preferences.begin("my - app", false);
    preferences.putUInt("bmeSpls", bmeSamples);
    preferences.end();

    repeater = 0;
    consoleLine = 0;
    offsetDelta = 0;
    for (int i = 0; i < numProfiles; ++i) {  // empty resistance array
      bme_resistance[i] = 0;
    }
    launchUtility();
    server.send(200, "text/plain", "BME samples updated");
  });

  server.on("/updateBMEfilter", HTTP_GET, []() {
    bmeFilter = server.arg("value").toInt();
    preferences.begin("my - app", false);
    preferences.putUInt("bmeFilter", bmeFilter);
    preferences.end();

    server.send(200, "text/plain", "BME filter updated");
  });

  server.on("/updateBMEpause", HTTP_GET, []() {
    bmeProfilePause = server.arg("value").toInt();
    preferences.begin("my - app", false);
    preferences.putUInt("bmePause", bmeProfilePause);
    preferences.end();

    server.send(200, "text/plain", "BME pause updated");
  });
  server.on("/restart", []() {
    ESP.restart();
    server.send(200, "text/plain", "Restart");
  });
  server.on("/updateTFTbrightness", []() {
    String value = server.arg("value");
    TFTbrightness = value.toFloat();
    pwm.writeScaled(TFTbrightness);
    lastInputTime = micros();
    server.send(200, "text/plain", "Display Brightness");
  });
  server.on("/toggleLS", []() {
    SLEEPENABLE = !SLEEPENABLE;
    preferences.begin("my - app", false);
    preferences.putBool("sleep", SLEEPENABLE);
    preferences.end();
    server.send(200, "text/plain", "Sleep Enabled");
  });


  server.on("/toggleLOGGING", []() {
    LOGGING = !LOGGING;
    preferences.begin("my - app", false);
    preferences.putBool("logging", LOGGING);
    preferences.end();

    if (LOGGING) {
      launchUtility();
      server.send(200, "text/plain", "LOGGING enabled");
    } else {
      launchUtility();
      server.send(200, "text/plain", "LOGGING disabled");
    }
  });
  server.on("/toggleDEBUG", []() {
    DEBUG = !DEBUG;
    if (DEBUG) Serial.begin(115200);
    else Serial.end();
    consoleLine = 0;
    preferences.begin("my - app", false);
    preferences.putBool("debug", DEBUG);
    preferences.end();
    empty2DArray(console);
    server.send(200, "text/plain", "DEBUG mode enabled");
  });
  server.on("/toggleLOGBME", []() {
    serialPrintBME1 = !serialPrintBME1;
    consoleLine = 0;
    preferences.begin("my - app", false);
    preferences.putBool("bmelog", serialPrintBME1);
    preferences.end();
    empty2DArray(console);
    server.send(200, "text/plain", "BME LOG enabled");
  });
  server.on("/triggerUP", []() {
    UP = true;
    pwm.writeScaled(TFTbrightness = 1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "UP");
  });
  server.on("/triggerDOWN", []() {
    DOWN = true;
    pwm.writeScaled(TFTbrightness = 1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "DOWN");
  });
  server.on("/triggerLEFT", []() {
    LEFT = true;
    pwm.writeScaled(TFTbrightness = 1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "LEFT");
  });
  server.on("/triggerRIGHT", []() {
    RIGHT = true;
    pwm.writeScaled(TFTbrightness = 1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "RIGHT");
  });
  server.on("/triggerCTR", []() {
    BUTTON = true;
    pwm.writeScaled(TFTbrightness = 1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "CTR");
  });

  server.on("/updateNTP", []() {
    taskManager.schedule(onceMicros(15), getNTP);
    server.send(200, "text/plain", "NTP Time updated");
  });

  server.onNotFound(handleNotFound);
  server.begin();
}



String generateJavaScriptFunctions() {  // JavaScript functions "<script src='https://cdn.jsdelivr.net/npm/chart.js'>"
  return "<script>\n"
         "function updateLoggingInterval() { \n"
         "var intervalSelect = document.getElementById('loggingInterval');\n"
         "var selectedValue = intervalSelect.options[intervalSelect.selectedIndex].value;\n"
         "fetch('/updateLoggingInterval?value=' + selectedValue);\n"
         "}\n"
         "function updateBMEsamples() { "
         "var sampleSelect = document.getElementById('bmeSamples');"
         "var selectedValue = sampleSelect.options[sampleSelect.selectedIndex].value;"
         "fetch('/updateBMEsamples?value=' + selectedValue);"
         "}"
         "function updateBMEfilter() { "
         "var filter = document.getElementById('bmeFilter');"
         "var selectedValue = filter.options[filter.selectedIndex].value;"
         "fetch('/updateBMEfilter?value=' + selectedValue);"
         "}"
         "function updateBMEpause() { "
         "var pauseMs = document.getElementById('bmePause');"
         "var selectedValue = pauseMs.options[pauseMs.selectedIndex].value;"
         "fetch('/updateBMEpause?value=' + selectedValue);"
         "}"
         "function toggleLED() { fetch('/toggleLED'); }"
         "function toggleLightSleep() { fetch('/toggleLS'); }"
         "function toggleLOGGING() { fetch('/toggleLOGGING'); }"
         "function deletePath() { "
         "  var pathToDelete = prompt('Please enter the path to delete:');"
         "  if (pathToDelete !== null) {"
         "    fetch('/deletePath?path=' + encodeURIComponent(pathToDelete));"
         "  }"
         "}"
         "function createFile() { "
         "  var pathToCreate = prompt('Please enter filename (path):');"
         "  if (pathToCreate !== null) {"
         "    fetch('/createFile?path=' + encodeURIComponent(pathToCreate));"
         "  }"
         "}"
         "function createDir() { "
         "  var pathToCreate = prompt('Please enter folder name (path):');"
         "  if (pathToCreate !== null) {"
         "    fetch('/createDir?path=' + encodeURIComponent(pathToCreate));"
         "  }"
         "}"
         "function LOGMarker() { "
         "  var note = prompt('LOG Marker. Please enter marker Note:');"
         "  if (note !== null) {"
         "    fetch('/LOGMarker?markerText=' + encodeURIComponent(note));"
         "  }"
         "}"
         "function download() {"
         "  var pathToDownload = prompt('Please enter path to download:');"
         "  if (pathToDownload !== null) {"
         "     window.location.href = '/download?path=' + encodeURIComponent(pathToDownload);"
         "  }"
         "}"
         "function uploadFile() { "
         "  var input = document.getElementById('fileInput');"
         "  var file = input.files[0];"
         "  if (file) {"
         "    var allowedFormats = ['.csv', '.txt'];"
         "    var fileFormat = file.name.substring(file.name.lastIndexOf('.')).toLowerCase();"
         "    if (allowedFormats.includes(fileFormat)) {"
         "      var formData = new FormData();"
         "      formData.append('upload', file);"
         "      fetch('/upload', { method: 'POST', body: formData });"
         "    } else {"
         "      alert('Invalid file format. Please upload a file with one of the following formats: .csv, .txt');"
         "    }"
         "  }"
         "}"

         "function toggleDEBUG() { fetch('/toggleDEBUG'); }"
         "function toggleLOGBME() { fetch('/toggleLOGBME'); }"
         "function toggleOLED() { fetch('/toggleOLED'); }"
         "function triggerUP() { fetch('/triggerUP'); }"
         "function triggerDOWN() { fetch('/triggerDOWN'); }"
         "function triggerLEFT() { fetch('/triggerLEFT'); }"
         "function triggerRIGHT() { fetch('/triggerRIGHT'); }"
         "function triggerCTR() { fetch('/triggerCTR'); }"
         "function toggleFAN() { fetch('/toggleFAN'); }"
         "function restartESP() { fetch('/restart'); }"
         "function updateNTP() { fetch('/updateNTP'); }"

         "function updateTFTbrightness(value) {\n"
         "  document.getElementById('TFTslider').value = value;\n"
         "  var xhr = new XMLHttpRequest();\n"
         "  xhr.open('GET', '/updateTFTbrightness?value=' + value, true);\n"
         "  xhr.send();\n"
         "}\n"
         "document.addEventListener('keydown', function(event) {\n"
         "  switch(event.key) {\n"
         "    case 'ArrowUp':\n"
         "      fetch('/triggerUP');\n"
         "      break;\n"
         "    case 'ArrowDown':\n"
         "      fetch('/triggerDOWN');\n"
         "      break;\n"
         "    case 'ArrowLeft':\n"
         "      fetch('/triggerLEFT');\n"
         "      break;\n"
         "    case 'ArrowRight':\n"
         "      fetch('/triggerRIGHT');\n"
         "      break;\n"
         "    case 'Enter':\n"
         "      fetch('/triggerCTR');\n"
         "      break;\n"
         "  }\n"
         "});\n"
         "function calculateMaxValue(data) {"
         "  var max = 0;"
         "  for (var i = 0; i < data.getNumberOfRows(); i++) {"
         "    for (var j = 1; j < data.getNumberOfColumns(); j++) {"
         "      var value = data.getValue(i, j);"
         "      if (value > max) {"
         "        max = value;"
         "      }"
         "    }"
         "  }"
         "  return max;"
         "}\n"
         "</script>\n";
}


String generateCSSstyles() {
  return "<style>"
         "@import url('https://fonts.cdnfonts.com/css/din-alternate');"
         "@import url('https://fonts.cdnfonts.com/css/demon-cubic-block-nkp');"  // font-family: 'DemonCubicBlock NKP Black', sans-serif;    font-family: 'DemonCubicBlock NKP Dark', sans-serif;    font-family: 'DemonCubicBlock NKP Shade', sans-serif;    font-family: 'DemonCubicBlock NKP Tile', sans-serif;
         "@import url('https://fonts.cdnfonts.com/css/barcode-font');"           // font-family: 'barcode font', sans-serif;
         "body { font-family: 'Helvetica Neue', sans-serif; background-color: #303030; display: block; margin-left: auto; margin-right: auto; }"
         "table { width: 710px; margin: 15px; padding: 20px; background-color: #D8D8D8; border-radius: 20px; display: block; table-layout: fixed; box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);}"
         "th, td { white-space: nowrap; border-radius: 10px; padding-top: 1px;  padding-bottom: 1px; color: #050505; text-align: left;}"
         "h2, h3 { font-family: 'DIN Alternate', sans-serif; color: #303030; text-align: left; margin-bottom: 5px; }"
         "p { font-family: 'DIN Alternate', sans-serif; margin: 3px 5px; } "
         "a.button { font-family: 'DIN Alternate', sans-serif; display: inline-block; margin: 4px; padding: 8px; text-decoration: none; border: none; color: white; background-color: #008080; border: solid 1px #009090; border-radius: 8px; font-size: 17px; cursor: pointer; box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);}"
         "button { font-family: 'DIN Alternate', sans-serif; display: inline-block; margin: 2px; padding: 5px 10px; text-decoration: none; border: 1px #505050; color: white; border-radius: 8px; font-size: 11px; cursor: pointer; box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2); }"
         "div { color: white; }"
         "#navbar { background-color: transparent; text-align: center; justify-content: center; }"
         "#sidebar { height: 850px; background-color: #353535; display: inline-block; justify-content: center; border: solid 1px #505050; border-radius: 20px; padding: 2px; margin: 15px; padding-top: 20px; text-align:center; box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);}"
         "#blocksFont { font-family: 'DemonCubicBlock NKP Shade', sans-serif; }"
         "#barcodeFont { font-family: 'barcode font', sans-serif; }"
         "</style>";
}


String generateCommonPageStructure(String content) {
  String pageC = "<!DOCTYPE html>";
  pageC += "<html lang='en'>";
  pageC += "<head>";
  pageC += "<meta charset='UTF-8' http-equiv='refresh' content='" + String(loggingInterval / ONETHOUSAND) + "' >";
  pageC += "<title>[HEX]POD Center</title>";
  pageC += "<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>";
  pageC += generateCSSstyles();
  pageC += "</head>";
  pageC += "<body>";
  pageC += "<div style='display: flex; padding: 15px; '>";
  pageC += "<div id='sidebar'>";
  pageC += generateNavBar();
  pageC += "</div>";
  pageC += "<div style='display: block; justify-content:center;'>";
  pageC += content;
  pageC += "</div>";
  pageC += "</div>";
  pageC += generateJavaScriptFunctions();
  pageC += "</body></html>";
  return pageC;
}



String generateNavBar() {
  // HTML for the navigation bar
  // String page = "<div style='text-align:center; margin-bottom: 5px; margin-top: 10px;  '>";

  String page = "<img src='https://i.ibb.co/RDjzjYV/Hex-Logo-transp-2-copy.png' onclick='download()' alt='Hex-Logo' border='0' style='width: 75px; height: auto;'></img>";

  page += "<p id='blocksFont' style='color:#C0C0C0; font-size:40px; text-align: center; '>loy</p><br>";
  // page += "<p>" + String(codeRevision) + "<br><br></p>";
  page += "<hr style='border: 2px solid #303030; padding: 0px; margin: 0px;'><br>";

  page += "<div style='text-align:center; '>";
  page += "<a class='button' href='/'>MAIN</a> <br>";
  page += "<a class='button' href='/sensors'>AIR</a> <br>";
  page += "<a class='button' href='/utility'>SYS</a> <br>";
  page += "<a class='button' href='/filesystem'>LOG</a></div><br>";

  page += "<div style='text-align:center; color: #808080; font-size:11px; '>";
  page += printTime + "<br>";
  page += String(restarts) + " Restarts<br>";
  page += "Uptime " + uptimeString + "<br>";
  page += WiFiIP + "</div>";

  // Controls
  page += "<hr style='border: 2px solid #303030; padding: 0px; margin 0px;'>";
  page += "<div style='text-align:center; '>";
  page += "<table style='max-width: 160px; max-height:160px; text-align:center; justify-content: center; display: flex; background-color: transparent; box-shadow: none;'>";
  page += "<tr><td></td><td><button onclick='triggerUP()' style='background-color:#008080;'>&#8593;</button></td><td></td></tr>";
  page += "<tr><td><button onclick='triggerLEFT()' style='background-color:#008080;' >&#8592;</button></td><td>&nbsp;</td><td><button onclick='triggerRIGHT()' style='background-color:#008080' ;>&#8594;</button></td></tr>";
  page += "<tr><td><button onclick='restartESP()' style='background-color:#008080;' >&#8634;</button></td><td><button onclick='triggerDOWN()' style='background-color:#008080' ;>&#8595;</button></td><td></td></tr>";
  page += "</table></div>";
  page += "<hr style='border: 2px solid #303030; padding: 0px; margin 0px;'>";

  // Toggles
  page += "<div style='text-align:center; max-width: 150px; padding: 10px; margin: 10px; background-color: transparent;'>";
  page += "<button onclick='toggleLOGGING()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(LOGGING ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>LOGGING</button>";
  page += "<button onclick='toggleDEBUG()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(DEBUG ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>DEBUG</button>";
  page += "<p id='barcodeFont' style='font-size:30px; color:#b0b0b0; '><br></br>HEX-POD " + String(codeRevision) + "<br></p>";
  page += "</div>";

  return page;
}




String generateConsole() {
  String consoleOutput;
  // Update circular buffer with the current sensor data
  // Display the last 55 entries in reverse order
  for (int line = consoleLine; line < consoleRows; line++) {
    // int index = (consoleLine + line) % 55;  // Calculate the circular index

    for (i = 0; i < consoleColumns; i++) {

      // console[line][i] = SDarray[SDIndex][i];
      consoleOutput += console[line][i];

      if (i < consoleColumns - 1) {
        if (line == consoleRows) line = 0;
        consoleOutput += "\t";  // Add a tab between values, except for the last one
      } else {
        consoleOutput += "\n";
      }
    }
  }
  return consoleOutput;
}



String generateHomePage() {

  String page = "<table min-width: 70%; '>";

  page += "<tr><td><h2>[HEX]POD " + String(codeRevision) + "</h2></td></tr>";
  page += "<tr><td>TFT Brightness: </td><td><input style='cursor:pointer;' type='range' id='TFTslider' min='0' max='1' step='0.05' value='" + String(TFTbrightness) + "' oninput='updateTFTbrightness(this.value)'></td></tr>";
  page += "<tr><td><b>Log Period</td>";
  page += "<td><label for='loggingInterval'>Interval</label></td>";
  page += "<td><select id='loggingInterval' onchange='updateLoggingInterval()'>";
  page += generateTimeOptions(loggingInterval);
  page += "</select></td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";
  page += "<tr><td><button onclick='toggleLOGBME()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(serialPrintBME1 ? "#008080; border: none;" : "#505050; border: solid 1px #505050;") + "'>BME Web Console</button></td></tr>";

  page += "</table>";

  page += "<table style='min-height: 300px; min-width: 50%; padding: 40px;'>";
  page += "<tr><td><h2>Web Console</h2></td></tr>";
  page += "<tr><td><pre>" + generateConsole() + "</pre>";
  page += "<td></tr></table>";


  return generateCommonPageStructure(page);
}


String generateSensorsPage() {
  String page = "<div style='display: flex;'>";  // Use flex container to make tables side by side
  page += "<table>";
  // Sensor Settings
  page += "<tr><td colspan='5'><h2> Sensor Settings </h2></td></tr>";

  page += "<tr><td><label for='loggingInterval'><b> Interval</b></label></td>";
  page += "<td><select id='loggingInterval' onchange='updateLoggingInterval()'>";
  page += generateTimeOptions(loggingInterval);
  page += "</select></td>";
  page += "</tr>";

  page += "<tr><td><label for='bmeSamples'><b>Samples</b></label></td>";
  page += "<td><select id='bmeSamples' onchange='updateBMEsamples()'>";
  page += valueOptions(bmeSamples);
  page += "</select></td>";
  page += "</tr>";

  page += "<tr><td><label for='bmeFilter'><b>Filter</b></label></td>";
  page += "<td><select id='bmeFilter' onchange='updateBMEfilter()'>";
  page += valueOptions(bmeFilter);
  page += "</select></td>";
  page += "</tr>";

  page += "<tr><td><label for='bmePause'><b> Pause</b>[ms]</label></td>";
  page += "<td><select id='bmePause' onchange='updateBMEpause()'>";
  page += valueOptions(bmeProfilePause);
  page += "</select></td>";
  page += "</tr>";

  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr><td><b> Conditioning </td><td>" + String(conditioning_duration) + "s</td></tr>";
  page += "<tr><td><b> Offst Delta </td><td>" + String(offsetDelta) + "</td></tr>";
  page += "<tr><td><b> Smpl Delta </td><td>" + String(samplingDelta) + "</td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row

  page += "<tr style='font-size: 12px;'><td><b> Profile Heater </td>";
  for (i = 0; i < numProfiles; i += 1) {
    page += "<td><b>H" + String(i + 1) + "</b></br>" + String(heatProf_1[i]) + "&deg;</td>";
  }
  page += "</tr>";

  page += "<tr style='font-size: 12px;'><td><b> Profile Duration </td>";
  for (i = 0; i < numProfiles; i += 1) {
    page += "<td><b>D" + String(i + 1) + "</b></br>" + String(durProf_1[i]) + "ms</td>";
  }
  page += "</tr>";
  page += "</table>";


  // Temp Humidity Press Chart
  page += "<table style='padding:0px;'><tr><td style='padding: 0px; margin: 0px; '>";
  page += "<div id='thp_chart' style='margin:0px; padding: 0px; height: 370px; width: 750px; '>";
  page += generateTHPchart();
  page += "</div>";
  page += "</td></tr></table> ";


  page += "</div>";
  page += "<div style='display: flex;'>";  // Use flex container to make tables side by side


  // BME1 Table
  page += "<table>";
  page += "<tr><td colspan='5'><h2> BME[1] </h2></td></tr>";
  page += "<tr><td>" + String(BME_ERROR) + "</td></tr>";
  page += "<tr style='background-color: #707070;'>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> Duration </b><br>" + String(bmeTracker) + "ms</div></td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> Last </b><br> " + String(lastBMEpoll) + "</div></td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> Poll Pd </b><br>" + String((bmeInterval / double(ONETHOUSAND)) / bmeSamples) + "s</div></td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> GAS_AVG </b><br>" + String(bme_gas_avg) + "</td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> Temp </b><br> " + String(data.temperature) + "&deg;C</td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> Humid </b><br> " + String(data.humidity) + "%</td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> Press </b><br> " + String(data.pressure / ONETHOUSAND) + "mBar</td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b> Alt </b><br> " + String(Altitude) + "m</td></tr>";
  page += "</tr>";

  page += "<tr><td>&nbsp;</td></tr>";  // empty Row

  page += "<tr style='font-size: 14px;'></td><td>";
  for (i = 0; i < numProfiles; i += 1) {
    if (i == 7) page += "</tr><tr style='font-size: 14px;'><td></td>";
    page += "<td><b>BME_" + String(i) + "</b></br>" + String(bme_resistance_avg[i]) + "</td>";
  }
  page += "</tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "</table>";

  // CHART
  page += "<table style='padding:0px;'><tr><td style='padding: 0px; margin: 0px; '>";
  page += "<div id='chart_div' style='margin:0px; padding: 0px; height: 370px; width: 750px; '>";
  page += generateBMEchart();
  page += "</div>";
  page += "</td></tr></table> ";


  page += "</div>";
  /*
  page += "<div style='display: flex;'>";  // Use flex container to make tables side by side


  //BME2 table
  page += "<table>";
  page += "<tr><td colspan='5'><h2> BME[2] </h2></td></tr>";
  page += "<tr><td>" + String(BME_ERROR) + "</td></tr>";
  page += "<tr style='background-color: #707070;'>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b>Duration</b><br> " + String() + "ms</div></td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b>Last</b><br> " + String() + "</div></td>";
  page += "<td><div style='padding: 7px; color: #FFFFFF;'><b>Poll Pd</b><br> " + String() + "s</div></td>";
  page += "</tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "</table>";

  // CHART 2
  page += "<table style='padding:0px;'><tr><td style='padding: 0px; margin: 0px; '>";
  page += "<div id='chart_div' style='margin:0px; padding: 0px; height: 370px; width: 750px; '>";

  page += "</div>";
  page += "</td></tr></table> ";


  page += "</div>"; */
  page += "<div style='display: flex;'>";  // Use flex container to make tables side by side


  // SGP41 Table
  page += "<table>";
  page += "<tr><td colspan='5'><h2> SGP41 </h2></td></tr>";
  page += "<tr><td>" + String(sgpErrorMsg) + "</td></tr>";
  page += "<tr style='background-color: #707070;'>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Duration</b><br>" + String(sgpTracker) + "ms</div></td>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Last</b><br> " + String(lastSGPpoll) + "</div></td>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Poll Pd</b><br>" + String(sgpInterval / double(ONETHOUSAND)) + "s</div></td>";
  page += "</tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr style='font-size: 14px;'></td><td><td><b> VOC </td><td><b> NOx </td><td><b> rawVOC </td><td><b> rawNOx </td></tr>";
  page += "<tr style='font-size: 14px;'></td><td><td>" + String(VOC) + "</td><td>" + String(NOX) + "</td><td>" + String(srawVoc) + "</td><td>" + String(srawNox) + "</td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr style='font-size: 14px;'><td></td><td><b> Index Offs </td><td><b> Learn Time_H </td><td><b> learn Time Gain_H </td><td><b> Gate Max Dur_M </td><td><b> Std Initial </td><td><b> Gain Factor </td></tr>";

  page += "<tr style='font-size: 14px;'>";
  voc_algorithm.get_tuning_parameters(
    index_offset, learning_time_offset_hours, learning_time_gain_hours,
    gating_max_duration_minutes, std_initial, gain_factor);
  page += "<td><b>VOC</td>";
  page += "<td>" + String(index_offset) + "</td>";
  page += "<td>" + String(learning_time_offset_hours) + "</td>";
  page += "<td>" + String(learning_time_gain_hours) + "</td>";
  page += "<td>" + String(gating_max_duration_minutes) + "</td>";
  page += "<td>" + String(std_initial) + "</td>";
  page += "<td>" + String(gain_factor) + "</td>";
  page += "</tr>";
  page += "<tr style='font-size: 14px;'>";
  nox_algorithm.get_tuning_parameters(
    index_offset, learning_time_offset_hours, learning_time_gain_hours,
    gating_max_duration_minutes, std_initial, gain_factor);
  page += "<td><b>NOX</td>";
  page += "<td>" + String(index_offset) + "</td>";
  page += "<td>" + String(learning_time_offset_hours) + "</td>";
  page += "<td>" + String(learning_time_gain_hours) + "</td>";
  page += "<td>" + String(gating_max_duration_minutes) + "</td>";
  page += "<td>" + String(std_initial) + "</td>";
  page += "<td>" + String(gain_factor) + "</td>";
  page += "</tr>";
  page += "</table>";

  // SCD41 log chart
  page += "<table style='padding:0px;'><tr><td style='padding: 0px; margin: 0px; '>";
  page += "<div id='sgp_chart' style='margin:0px; padding: 0px; height: 370px; width: 750px; '>";
  page += generateSGPchart();
  page += "</div>";
  page += "</td></tr></table> ";

  page += "</div>";

  return generateCommonPageStructure(page);
}



String generateUtilityPage() {

  getDeviceInfo();

  String page = "<div style='display: flex;'>";
  // Device Controls
  page += generateDeviceControlsTable();
  page += "</div>";


  page += "<div style='display: flex;'>";
  // Device Stats
  page += generateDeviceStatsTable();
  // System Sensors
  page += generateSystemSensorsTable();
  page += "</div>";


  page += "<div style='display: flex;'>";
  // Task Manager
  page += generateTaskManagerTable();
  // File System
  if (LittleFS.begin()) {
    getSPIFFSsizes();
    page += generateFSTable();
    LittleFS.end();
  }
  page += "</div>";

  return generateCommonPageStructure(page);
}




String generateFSPage() {
  String page;

  if (LittleFS.begin()) {

    getSPIFFSsizes();

    page = "<div style='display: flex; '>";
    page += generateFSTable();
    page += "</div>";

    page += "<div style='display: flex;'>";
    page += generateLogFileContent();
    page += "</div>";

    LittleFS.end();
  } else {
    page = "<table>";
    page += "<tr><td><h3>LittleFS / SPIFFS couldn't be mounted</h3></td></tr>";
    page += "</table>";
  }

  return generateCommonPageStructure(page);
}




String generateTimeOptions(int selectedValue) {
  String pageS = "";

  // Seconds
  for (int i = 2; i <= 59; i += 5) {
    if (i == 7) i -= 2;
    int value = i * 1000;
    pageS += "<option value='" + String(value) + "s'";
    if (value == selectedValue) {
      pageS += " selected";
    }
    pageS += ">" + String(i) + "s</option>";
  }
  // Minutes
  for (int i = 1; i <= 59; i += 5) {
    if (i > 4 && i < 7) i -= 1;
    if (i > 30) i += 10;
    int value = i * MINUTES_IN_HOUR * 1000;
    pageS += "<option value='" + String(value) + "m'";
    if (value == selectedValue) {
      pageS += " selected";
    }
    pageS += ">" + String(i) + "m</option>";
  }
  // Hours
  for (int i = 1; i <= 24; i++) {
    int value = i * HOURS_IN_DAY * 1000;
    pageS += "<option value='" + String(value) + "h'";
    if (value == selectedValue) {
      pageS += " selected";
    }
    pageS += ">" + String(i) + "h</option>";
  }

  return pageS;
}


String valueOptions(int selectedValue) {
  String pageS = "";
  int i;

  for (i = 0; i <= 100; i += 1) {
    int value = i;
    pageS += "<option value='" + String(value) + "'";
    if (value == selectedValue) {
      pageS += " selected";
    }
    pageS += ">" + String(i) + "</option>";
  }

  return pageS;
}
