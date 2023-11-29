#include <pgmspace.h>

int8_t menuIndex = 0;
int8_t lastMenuIndex = 0;
// int menuSprites = 5;
int datumX = TFT_WIDTH / 2;
int datumY = TFT_HEIGHT / 2;
int scrollX = 0;
// TFT_eSprite sprite1 = TFT_eSprite(&tft);

void FLOW_MENU() {
  timeTracker = micros();
  tft.fillScreen(TFT_BLACK);
  taskManager.reset();

  SECID = taskManager.schedule(repeatMillis(1000), updateTime);
  MENUID = taskManager.schedule(repeatMillis(150), pollMENU);
  BTNID = taskManager.schedule(onceMillis(1), pollBUTTONS);
  NTPID = taskManager.schedule(repeatSeconds(60), getNTP);
  TEMPID = taskManager.schedule(repeatMillis(1500), pollTemp);

  // sprite1.createSprite(50, 50);
  if (DEBUG) {
    timeTracker = (micros() - timeTracker) / 1000;
    ESP_LOGI(TAG, "%.3lfms", timeTracker);
  }
}

void pollBUTTONS() {
  timeTracker = micros();

  if (INT_TRGR) {
    if (millis() - lastInputTime > 140) {
      INT_TRGR = false;
      lastInputTime = millis();

      UP = !io.read(PCA95x5::Port::P13);     // BTN Up
      DOWN = !io.read(PCA95x5::Port::P10);   // BTN Down
      LEFT = !io.read(PCA95x5::Port::P11);   // BTN Left
      RIGHT = !io.read(PCA95x5::Port::P12);  // BTN Right
      BUTTON = !digitalRead(GPIO_NUM_0);
    }
  }
  if (DEBUG) {
    timeTracker = (micros() - timeTracker) / 1000;
    ESP_LOGI(TAG, "%.3lfms", timeTracker);
  }
}

void pollMENU() {
  timeTracker = micros();
  tft.drawNumber(menuIndex, datumX, datumY, 2);

  if (UP || DOWN || LEFT || RIGHT || BUTTON) {
    if (DOWN) {
      DOWN = false;
      lastMenuIndex = menuIndex;
      menuIndex--;
    }
    if (UP) {
      UP = false;
      menuIndex++;
    }

    if (lastMenuIndex - menuIndex > 0) {
      for (scrollX = 0; scrollX < 200; scrollX++) {
        datumX++;
        /*sprite1.drawSmoothCircle(datumX, datumY, 80, TFT_CYAN, TFT_BLACK);
        sprite1.drawNumber(menuIndex, datumX, datumY, 2);
        sprite1.pushSprite(datumX, datumY);*/
      }
    }

    if (lastMenuIndex - menuIndex < 0) {
      for (scrollX = 0; scrollX < 200; scrollX++) {
        datumX--;
        /*sprite1.drawSmoothCircle(datumX, datumY, 80, TFT_CYAN, TFT_BLACK);
        sprite1.drawNumber(menuIndex, datumX, datumY, 2);
        sprite1.pushSprite(datumX, datumY);*/
      }
    }

    switch (menuIndex) {
      case 0:
        break;
      case 1: break;
      case 2: break;
      case 3: break;
      case 4: break;
    }
  }
  if (DEBUG) {
    timeTracker = (micros() - timeTracker) / 1000;
    ESP_LOGI(TAG, "%.3lfms", timeTracker);
  }
}
