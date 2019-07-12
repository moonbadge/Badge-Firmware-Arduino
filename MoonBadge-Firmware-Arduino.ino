using namespace std;

#include "moonbadge.h"
#include "LunarCard.h"

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
  if(deck.load("SD:/main.json")==false){
    Serial.println("Failed to load SDMMC, Falling back to SPIFFS");
    deck.load("SPI:/main.json");
  }

  

}

void loop() {
  // put your main code here, to run repeatedly:
  deck.doEvents();
  delay(100);
}
