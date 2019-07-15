using namespace std;

#include "moonbadge.h"
#include "LunarCardDeck.h"
#include "LunarCard.h"
#include <Adafruit_GFX.h>
#include <GxEPD2.h>
#include <ArduinoJson.h>


using namespace std;
MoonBadge badge;
LunarCardDeck deck;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println("GxEPD2_Spiffs_Example");

  //pathtest();

  badge.init();
  // Try and load from SD, fallback to flash.
  if(deck.load("SD:/slide.json")==false){
    Serial.println("Failed to load SDMMC, Falling back to SPIFFS");
    deck.load("SPI:/main.json");
  }

  

}

void loop() {
  // put your main code here, to run repeatedly:
  deck.doEvents();
  delay(100);
}
