
#include <pgmspace.h>


void handleRoot() {
  if (!server.authenticate("hexpod", "noguess8142")) {
    return server.requestAuthentication();
  }
  server.send(200, "text/html", generateHomePage());
}

void handleSensors() {
  if (!server.authenticate("hexpod", "noguess8142")) {
    return server.requestAuthentication();
  }
  server.send(200, "text/html", generateSensorsPage());
}

void handleUtility() {
  if (!server.authenticate("hexpod", "noguess8142")) {
    return server.requestAuthentication();
  }
  server.send(200, "text/html", generateUtilityPage());
}


void setupWebInterface() {

  server.on("/", HTTP_GET, handleRoot);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/utility", HTTP_GET, handleUtility);

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
  server.on("/updateLoggingInterval", HTTP_GET, []() {
    loggingInterval = server.arg("value").toInt();
    bmeInterval = loggingInterval;
    taskManager.cancelTask(BMEID);
    taskManager.cancelTask(SGPID);
    taskManager.cancelTask(LOG);
    taskManager.cancelTask(TEMPID);

    preferences.begin("my - app", false);
    preferences.putUInt("logItvl", loggingInterval);
    preferences.end();

    repeater = bmeSamples - 1;
    consoleLine = 0;

    TEMPID = taskManager.schedule(repeatMillis(loggingInterval), pollTemp);
    BMEID = taskManager.schedule(repeatMillis(bmeInterval / bmeSamples), pollBME);
    SGPID = taskManager.schedule(repeatMillis(sgpInterval), pollSGP);
    LOG = taskManager.schedule(repeatMillis(loggingInterval / bmeSamples), logging);
    server.send(200, "text/plain", "Logging interval updated");
  });

  server.on("/updateBMEsamples", HTTP_GET, []() {
    bmeSamples = server.arg("value").toInt();
    taskManager.cancelTask(BMEID);
    taskManager.cancelTask(SGPID);
    taskManager.cancelTask(LOG);

    preferences.begin("my - app", false);
    preferences.putUInt("bmeSpls", bmeSamples);
    preferences.end();

    repeater = bmeSamples - 1;
    consoleLine = 0;

    BMEID = taskManager.schedule(repeatMillis(bmeInterval / bmeSamples), pollBME);
    SGPID = taskManager.schedule(repeatMillis(sgpInterval), pollSGP);
    LOG = taskManager.schedule(repeatMillis(loggingInterval / bmeSamples), logging);
    server.send(200, "text/plain", "BME samples updated");
  });

  server.on("/updateBMEpause", HTTP_GET, []() {
    bmeProfilePause = server.arg("value").toInt();
    preferences.begin("my - app", false);
    preferences.putUInt("bmePause", bmeProfilePause);
    preferences.end();

    server.send(200, "text/plain", "BME pause updated");
  });

  server.on("/updateTFTbrightness", []() {
    String value = server.arg("value");
    TFTbrightness = value.toFloat();
    pwm.writeScaled(TFTbrightness);
    lastInputTime = micros();
    server.send(200, "text/plain", "Display Brightness");
  });

  server.on("/restart", []() {
    ESP.restart();
    server.send(200, "text/plain", "Restart");
  });
  server.on("/toggleLS", []() {
    SLEEPENABLE = !SLEEPENABLE;
    preferences.begin("my - app", false);
    preferences.putBool("sleep", SLEEPENABLE);
    preferences.end();
    server.send(200, "text/plain", "Sleep Enabled");
  });
  server.on("/toggleDEBUG", []() {
    DEBUG = !DEBUG;
    consoleLine = 0;
    preferences.begin("my - app", false);
    preferences.putBool("debug", DEBUG);
    preferences.end();
    emptyLogArray();
    server.send(200, "text/plain", "DEBUG mode enabled");
  });
  server.on("/toggleLOGBME", []() {
    serialPrintBME1 = !serialPrintBME1;
    consoleLine = 0;
    preferences.begin("my - app", false);
    preferences.putBool("bmelog", serialPrintBME1);
    preferences.end();
    emptyLogArray();
    server.send(200, "text/plain", "BME LOG enabled");
  });
  server.on("/triggerUP", []() {
    UP = true;
    pwm.writeScaled(1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "UP");
  });
  server.on("/triggerDOWN", []() {
    DOWN = true;
    pwm.writeScaled(1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "DOWN");
  });
  server.on("/triggerLEFT", []() {
    LEFT = true;
    pwm.writeScaled(1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "LEFT");
  });
  server.on("/triggerRIGHT", []() {
    RIGHT = true;
    pwm.writeScaled(1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "RIGHT");
  });
  server.on("/triggerCTR", []() {
    BUTTON = true;
    pwm.writeScaled(1.0);
    taskManager.schedule(onceMicros(10), pollButtons);
    server.send(200, "text/plain", "CTR");
  });
  server.on("/toggleFAN", []() {
    FANon = !FANon;
    FANon ? digitalWrite(GPIO_NUM_1, true) : digitalWrite(GPIO_NUM_1, false);
    server.send(200, "text/plain", "FAN toggled");
  });
  server.on("/updateNTP", []() {
    taskManager.schedule(onceMicros(15), getNTP);
    server.send(200, "text/plain", "NTP Time updated");
  });


  server.onNotFound(handleNotFound);
  server.begin();
}



String generateJavaScriptFunctions() {  // JavaScript functions
                                        // "<script src='https://cdn.jsdelivr.net/npm/chart.js'>"
  return "<script>"
         "function updateLoggingInterval() { "
         "var intervalSelect = document.getElementById('loggingInterval');"
         "var selectedValue = intervalSelect.options[intervalSelect.selectedIndex].value;"
         "fetch('/updateLoggingInterval?value=' + selectedValue);"
         "}"
         "function updateBMEsamples() { "
         "var sampleSelect = document.getElementById('bmeSamples');"
         "var selectedValue = sampleSelect.options[sampleSelect.selectedIndex].value;"
         "fetch('/updateBMEsamples?value=' + selectedValue);"
         "}"
         "function updateBMEpause() { "
         "var pauseMs = document.getElementById('bmePause');"
         "var selectedValue = pauseMs.options[pauseMs.selectedIndex].value;"
         "fetch('/updateBMEpause?value=' + selectedValue);"
         "}"
         "function toggleLED() { fetch('/toggleLED'); }"
         "function toggleLightSleep() { fetch('/toggleLS'); }"
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


         "function updateTFTbrightness(value) {"
         "  document.getElementById('TFTslider').value = value;"
         "  var xhr = new XMLHttpRequest();"
         "  xhr.open('GET', '/updateTFTbrightness?value=' + value, true);"
         "  xhr.send();"
         "}"
         /*
         "var chartData = {"
         "labels: Array.from({ length: bme_resistance_avg[0].length }, (_, i) => i + 1),"
         "datasets: bme_resistance_avg.map((data, index) => ({"
         "label: 'Dataset ' + (index + 1),"
         "data: data,"
         "fill: false,"
         "borderColor: 'rgba(140, 140, 140, 1)',"  // Adjust color as needed
         "})),"
         "};"

         "var ctx = document.getElementById('myChart').getContext('2d');"
         "var myChart = new Chart(ctx, {"
         "type: 'line',"
         "data: chartData,"
         "options: {"
         "responsive: true,"
         "maintainAspectRatio: false,"
         "scales: {"
         "x: {"
         "type: 'linear',"
         "position: 'bottom',"
         "},"
         "},"
         "},"
         "});"
*/
         "document.addEventListener('keydown', function(event) {"
         "  switch(event.key) {"
         "    case 'ArrowUp':"
         "      fetch('/triggerUP');"
         "      break;"
         "    case 'ArrowDown':"
         "      fetch('/triggerDOWN');"
         "      break;"
         "    case 'ArrowLeft':"
         "      fetch('/triggerLEFT');"
         "      break;"
         "    case 'ArrowRight':"
         "      fetch('/triggerRIGHT');"
         "      break;"
         "    case 'Enter':"
         "      fetch('/triggerCTR');"
         "      break;"
         "  }"
         "});"
         "</script>";
}


String generateCSSstyles() {  // flex-grow: 1;
  return "<style>"
         "body { font-family: 'Helvetica Neue', sans-serif; background-color: #303030; margin: 0; text-align: center; display: block; margin-left: auto; margin-right: auto; }"
         "table { width: 700px; margin-left: auto; margin-right: auto; margin-top: 5px; margin-bottom: 5px; background-color: #D8D8D8; border-radius: 10px; padding: 5px; display: block; }"
         "th, td { border-spacing: 3px 5px; padding: 0px 5px; color: #050505;  margin: 10px; text-align: left; }"
         "h2 { color: #303030; text-align: left; margin-bottom: 5px; }"
         "h3 { color: #303030; text-align: left; margin-bottom: 5px; }"
         "#navbar { background-color: #303030; text-align: center; }"
         "a.button { display: inline-block; margin: 5px; padding: 10px 10px; text-decoration: none; border: none; color: white; background-color: #008080; border-radius: 8px; font-size: 16px; cursor: pointer; }"
         "button {  display: inline-block; margin: 2px; padding: 5px 10px; text-decoration: none; border: 1px #505050; color: white; background-color: #008080; border-radius: 8px; font-size: 11px; cursor: pointer; }"
         "div { color: white; text-align: center; border-radius: 8px; }"
         "#sidebar { width: 200px; background-color: #303030; padding: 10px;}"
         "</style>";
}


String generateCommonPageStructure(String content) {

  String pageC = "<html lang='en'>";
  pageC += "<head>";
  pageC += "<meta http-equiv='refresh' content='" + String(loggingInterval / 1000) + "' >";
  pageC += "<title>[HEX]POD Center</title>";
  pageC += generateCSSstyles();
  pageC += "</head>";
  pageC += "<body>";
  pageC += "<div style='display: flex; padding: 15px; '>";
  pageC += "<div id='sidebar' style=' text-align:center; display: inline-block; border: solid 1px #505050; border-radius: 15px; ' >";
  pageC += generateNavBar();  // Include your existing navbar in the sidebar
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
  String pageN = "<div style='text-align:center; '><a class='button' href='/'>MAIN</a> <br> <a class='button' href='/sensors'>AIR</a> <br> <a class='button' href='/utility'>SYS</a></div><br>";
  pageN += "<div style='text-align:center; font-size:11px; '>" + server.uri() + "<br>" + printTime + "<br>" + printDate + "</div>";
  pageN += "<div style='text-align:center; font-size:11px; '>" + String(restarts) + " Restarts<br>";
  pageN += "Uptime " + uptimeString;
  pageN += "</div>";
  pageN += "<table style='border: solid 1px #505050; border-radius: 15px; max-width: 150px; max-height:150px; text-align:center; display: inline-block; background-color: transparent;'>";
  pageN += "<tr><td></td><td><button onclick='triggerUP()'>&#8593;</button></td><td></td>";
  pageN += "<tr><td><button onclick='triggerLEFT()'>&#8592;</button></td><td><button onclick='triggerCTR()'>&#9678;</button></td><td><button onclick='triggerRIGHT()'>&#8594;</button></td>";
  pageN += "<tr><td><button onclick='restartESP()'>&#8634;</button></td><td><button onclick='triggerDOWN()'>&#8595;</button></td><td></td></tr>";
  pageN += "</table>";
  // Buttons
  pageN += "<div style='text-align:center; max-width: 150px; padding: 10px; margin: 10px; border: solid 1px #505050; border-radius: 15px; background-color: transparent;'>";
  pageN += "<tr><td><button onclick='toggleLED()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(LEDon ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>LED</button></td></tr>";
  pageN += "<tr><td><button onclick='toggleFAN()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(FANon ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>FAN</button></td></tr>";
  pageN += "<tr><td><button onclick='toggleOLED()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(OLEDon ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>OLED</button></td></tr>";
  pageN += "<tr><td><button onclick='toggleLightSleep()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(SLEEPENABLE ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>SLEEP</button></td></tr>";
  pageN += "<tr><td><button onclick='updateNTP()' style='padding: 10px 15px; font-size: 14px; background-color:transparent; border: solid 1px #505050;'>NTPTime</button></td></tr>";
  pageN += "<tr><td><button onclick='toggleDEBUG()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(DEBUG ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>DEBUG</button></td></tr>";
  pageN += "<tr><td><button onclick='toggleLOGBME()' style='padding: 10px 15px; font-size: 14px; background-color: " + String(serialPrintBME1 ? "#008080; border: none;" : "transparent; border: solid 1px #505050;") + "'>BME LOG</button></td></tr>";
  // pageN += "</table>";
  pageN += "</div>";
  return pageN;
}



String generateHomePage() {
  String page = "<table min-width: 1000px;>";
  page += "<tr><td><h2>[HEX]POD " + String(codeRevision) + "</h2></td></tr>";
  page += "<tr><td>TFT Brightness: </td><td><input style='cursor:pointer;' type='range' id='TFTslider' min='0' max='1' step='0.05' value='" + String(TFTbrightness) + "' oninput='updateTFTbrightness(this.value)'></td></tr>";
  page += "<tr><td><b>Log Period</td>";
  page += "<td><label for='loggingInterval'>Interval " + String(loggingInterval / 1000) + "</label></td>";
  page += "<td><select id='loggingInterval' onchange='updateLoggingInterval()'>";
  page += generateTimeOptions(loggingInterval);
  page += "</select></td></tr>";
  page += "</table>";


  // page += "<div style='text-align:left; margin-top: 15px;'>";
  // page += "<h2>Sensor Data Console</h2>";
  page += "<table style='min-height: 300px; min-width: 1000px; padding: 50px; '>";
  page += "<tr><td><pre>" + generateConsole() + "</pre>";
  page += "<td></tr></table>";

  return generateCommonPageStructure(page);
}




String generateConsole() {
  String consoleOutput;

  // Update circular buffer with the current sensor data

  // Display the last 55 entries in reverse order
  for (int line = 0; line < 55; line++) {
    // int index = (consoleLine + line) % 55;  // Calculate the circular index

    for (int i = 0; i < 14; i++) {
      if (serialPrintLOG) {
        console[line][i] = SDarray[SDIndex][i];
      } else {
        consoleOutput += console[line][i];
      }
      if (i < 13) {
        consoleOutput += "\t";  // Add a tab between values, except for the last one
      } else {
        consoleOutput += "\n";
      }
    }
  }

  return consoleOutput;
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
  for (int i = 1; i <= 59; i += 15) {
    if (i > 5 && i < 20) i -= 1;
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

  for (i = 0; i <= 30; i += 1) {
    int value = i;
    pageS += "<option value='" + String(value) + "'";
    if (value == selectedValue) {
      pageS += " selected";
    }
    pageS += ">" + String(i) + "</option>";
  }

  return pageS;
}


String generateSensorsPage() {
  String page = "<div style='display: flex;'>";  // Use flex container to make tables side by side
  page += "<table style=' margin: 20px; padding: 20px; '>";

  page += "<tr><td><h2> Sensor Settings </h2></td></tr>";

  page += "<tr><td><label for='loggingInterval'><b> Interval</b>[s]</label></td>";
  page += "<td><select id='loggingInterval' onchange='updateLoggingInterval()'>";
  page += generateTimeOptions(loggingInterval);
  page += "</select></td>";
  page += "</tr>";

  // page += "<tr><td><b>Samples</b></td>";
  page += "<tr><td><label for='bmeSamples'><b>Samples</b></label></td>";
  page += "<td><select id='bmeSamples' onchange='updateBMEsamples()'>";
  page += valueOptions(bmeSamples);
  page += "</select></td>";
  page += "</tr>";

  //page += "<tr><td><b> Pause</b>[ms]</td>";
  page += "<tr><td><label for='bmeSamples'><b> Pause</b>[ms]</label></td>";
  page += "<td><select id='bmePause' onchange='updateBMEpause()'>";
  page += valueOptions(bmeProfilePause);
  page += "</select></td>";
  page += "</tr>";

  page += "<tr><td><b> Conditioning </td><td>" + String(conditioning_duration) + "s</td></tr>";
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

  // Empty Table
  page += "<table style=' margin: 20px; padding: 20px; '>";
  // page += "<tr><td><canvas id='myChart' width='400' height='200'></canvas></td></tr>";
  page += "</table>";

  page += "</div>";                        // Close the flex container
  page += "<div style='display: flex;'>";  // Use flex container to make tables side by side

  // BME1 Table
  page += "<table style=' margin: 20px; padding: 15px; '>";
  page += "<tr><td><h2> BME_1 </h2></td></tr>";
  page += "<tr><td>" + String(BME_ERROR) + "</td></tr>";
  page += "<tr style='background-color: #707070;'>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Duration</b><br>" + String(bmeTracker) + "ms</div></td>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Last</b><br> " + String(lastBMEpoll) + "</div></td>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Poll Pd</b><br>" + String(bmeInterval / 1000) + "s</div></td>";
  page += "<td></td><td></td><td></td><td></td><td></td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr style='font-size: 14px;'><td></td><td><b> GAS_AVG </td><td><b> Temp </td><td><b >Humid </td><td><b> Press </td><td><b> Alt </td></tr>";
  page += "<tr style='font-size: 14px;'><td></td><td>" + String(bme_gas_avg) + "</td><td>" + String(data.temperature) + "&deg;C</td><td>" + String(data.humidity) + "%</td><td>" + String(data.pressure / 1000) + "mBar</td><td>" + String(Altitude) + "m</td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row

  page += "<tr style='font-size: 14px;'></td><td>";
  for (i = 0; i < numProfiles; i += 1) {
    if (i == 7) page += "</tr><tr style='font-size: 14px;'><td></td>";
    page += "<td><b>BME_" + String(i + 1) + "</b></br>" + String(bme_resistance_avg[i]) + "</td>";
  }
  page += "</tr>";

  page += "</table>";

  //BME2 table
  page += "<table style='margin: 20px; padding: 15px; '>";
  page += "<tr><td><h2>BME_2</h2></td></tr>";
  page += "</table>";

  page += "</div>";                        // Close the flex container
  page += "<div style='display: flex;'>";  // Use flex container to make tables side by side

  // SGP41 Table
  page += "<table style=' margin: 20px; padding: 15px; '>";
  page += "<tr><td><h2> SGP41 </h2></td></tr>";
  page += "<tr><td>" + String(sgpErrorMsg) + "</td></tr>";
  page += "<tr style='background-color: #707070;'>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Duration</b><br>" + String(sgpTracker) + "ms</div></td>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Last</b><br> " + String(lastSGPpoll) + "</div></td>";
  page += "<td><div style='padding: 5px; color: #FFFFFF;'><b>Poll Pd</b><br>" + String(sgpInterval / 1000) + "s</div></td>";
  page += "<td></td><td></td><td></td><td></td></tr>";
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
  page += "</table>";  // button execute conditioning with conditioning_s = 10  or  sgp41.executeConditioning(data.humidity * 65535 / 100, (data.temperature + 45) * 65535 / 175, srawVoc);  // defaultRh, defaultT

  // SCD41 table
  page += "<table style='margin: 20px; padding: 15px; '>";
  page += "<tr><td><h2>SCD41</h2></td></tr>";
  page += "</table>";

  page += "</div>";  // Close the flex container

  return generateCommonPageStructure(page);
}



String generateUtilityPage() {

  SDinserted = digitalRead(GPIO_NUM_47);

  String page = "<div style='display: flex;'>";  // Use flex container to make tables side by side

  // System Info Table
  page += "<table style=' margin: 20px; padding: 15px 15px;'>";
  page += "<tr><td><h2>Device Stats</h2></tr>";
  page += "<tr><td><b>" + String(CONFIG_IDF_TARGET) + "</td>";
  page += "<td><b>Power State</b></td><td> " + String(powerStateNames[currentPowerState]) + "</td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr><td><b>CPU:</td><td>" + String(cpu_freq_mhz) + "MHZ" + "</td><td>" + String(temperatureRead()) + "&deg;C</td><td>" + String(chip_info.cores) + "Core</td>";
  page += "<td>" + String((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi | " : "") + String((chip_info.features & CHIP_FEATURE_BT) ? "BT " : "") + String((chip_info.features & CHIP_FEATURE_BLE) ? "BLE " : "") + "</td></tr>";
  page += "<tr><td><b>Flash</td><td>" + String(flash_size / ONEMILLION) + "MB " + String((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embed" : "ext") + "</td></tr>";
  page += "<tr><td><b>PSRAM</td><td> Total: " + String(ESP.getPsramSize() / 1000) + "KB</td><td>  Free: " + String(ESP.getFreePsram() / 1000) + "KB</td></tr>";
  page += "<tr><td><b>SPIFFS</td><td> Used: " + String(file_system_size / ONEMILLION) + "MB</td><td>  Free: " + String(free_size / ONEMILLION) + "MB</td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr><td><b> Free Heap </td><td><b> Min Free Heap  </td><td><b> Free Int Heap </td><td><b> Sketch Size </td>";
  page += "<tr><td>" + String(esp_get_free_heap_size() / 1000.0) + "KB</td>";
  page += "<td>" + String(esp_get_minimum_free_heap_size() / 1000.0) + "KB</td> ";
  page += "<td>" + String(esp_get_free_internal_heap_size() / 1000.0) + "KB</td>";
  page += "<td>" + String(program_size / 1000.0) + "KB</td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr><td><b> WiFi SSID </td><td><b> Status </td><td><b> RSSI </td><td><b> Channel </td><td><b> Last NTP </td>";
  page += "<tr>";
  page += "<td>" + String(WiFi.SSID()) + "</td>";
  page += "<td>" + String(wifiStatusChar[WiFi.status()]) + "</td>";
  page += "<td>" + String(WiFi.RSSI()) + "db</td>";
  page += "<td>" + String(WiFi.channel()) + "</td>";
  page += "<td>" + String(lastNTPtime) + "</td>";
  // page += "<td>" + String(lastNTPtimeFail) + "</td>";
  page += "</tr>";
  page += "<tr><td><b> SD Card </td><td><b>  </td><td><b>  </td><td><b>  </td></tr>";
  page += "<tr><td>";
  page += SDinserted ? "Present" : "Empty";
  page += "</td></tr>";
  page += "</table>";

  // System Sensors
  page += "<table style=' margin: 20px; padding: 15px 15px;'>";
  page += "<tr><td><h2>System Sensors</h2></td></tr>";
  page += "<tr><td><b>Volt</b></td><td><b>Amp</b></td><td><b>Shunt</b></td><td><b>Power</b></td></tr>";
  page += "<tr><td>" + String(BUS2_BusVoltage) + "V</td><td>" + String(BUS2_Current) + "mA</td><td>" + String(BUS2_ShuntVoltage) + "mV</td><td>" + String(BUS2_Power) + "mW</td></tr>";
  page += "<tr><td>&nbsp;</td></tr>";  // empty Row
  page += "<tr><td><b>Temp CPU</b></td><td><b>ESP Temp</b></td><td><b>Temp 3</b></td><td><b>Temp 4</b></td><td><b>Temp 5</b></td><td><b>Temp 6</b></td></tr>";
  page += "<tr><td>" + String(temperatureRead()) + "&deg;C</td><td>" + String(temp1) + "&deg;C</td><td> - </td><td> - </td><td> - </td><td> - </td></tr>";
  page += "</table>";

  page += "</div>";  // Close the flex container

  page += "<div style='display: flex;'>";  // Use flex container to make tables side by side
    // Task Manager
  page += "<table style=' margin: 20px; padding: 20px 15px;'>";
  page += "<tr><td><h2>Task Manager</h2></td></tr>";
  if (statBaTracker > 0.0) page += "<tr><td>[" + String(STATID) + "] updateStat()</td><td>" + String(taskFreeSlots[STATID]) + " " + String(statBaTracker) + "ms<td></tr>";
  if (uTimeTracker > 0.0) page += "<tr><td>[" + String(SECID) + "] updateTime()</td><td>" + String(taskFreeSlots[SECID]) + " " + String(uTimeTracker) + "ms<td></tr>";
  if (ntpTracker > 0.0) page += "<tr><td>[" + String(NTPID) + "] getNTP()</td><td>" + String(taskFreeSlots[NTPID]) + " " + String(ntpTracker) + "ms<td></tr>";
  if (tempTracker > 0.0) page += "<tr><td>[" + String(TEMPID) + "] pollTemp()</td><td>" + String(taskFreeSlots[TEMPID]) + " " + String(tempTracker) + "ms<td></tr>";
  if (ina2Tracker > 0.0) page += "<tr><td>[" + String(INA2ID) + "] pollINA()</td><td>" + String(taskFreeSlots[INA2ID]) + " " + String(ina2Tracker) + "ms<td></tr>";
  if (bmeTracker > 0.0) page += "<tr><td>[" + String(BMEID) + "] pollBME()</td><td>" + String(taskFreeSlots[BMEID]) + " " + String(bmeTracker) + "ms<td></tr>";
  if (sgpTracker > 0.0) page += "<tr><td>[" + String(SGPID) + "] pollSGP()</td><td>" + String(taskFreeSlots[SGPID]) + " " + String(sgpTracker) + "ms<td></tr>";
  if (loggingTracker > 0.0) page += "<tr><td>[" + String(LOG) + "] logging()</td><td>" + String(taskFreeSlots[LOG]) + " " + String(loggingTracker) + "ms<td></tr>";
  if (powerStTracker > 0.0) page += "<tr><td>[" + String(ST1) + "] powerStates()</td><td>" + String(taskFreeSlots[ST1]) + " " + String(powerStTracker) + "ms<td></tr>";
  if (clientTracker > 0.0) page += "<tr><td>[" + String(WEB) + "] pollServer()</td><td>" + String(taskFreeSlots[WEB]) + " " + String(clientTracker) + "ms<td></tr>";
  if (taskFreeSlots[IMUID] == 'r' || taskFreeSlots[IMUID] == 'R' && imuTracker > 0.0) page += "<tr><td>[" + String(IMUID) + "] pollIMU()</td><td>" + String(taskFreeSlots[IMUID]) + " " + String(imuTracker) + "ms<td></tr>";

  page += "</table>";

  page += "</div>";  // Close the flex container

  return generateCommonPageStructure(page);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
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