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
  
  SPIFFS.begin();

  Serial.println("SPIFFS started");
  badge.init();
  deck.load("/main.json");
  //deck.showCard("wrong");

  

}

void loop() {
  // put your main code here, to run repeatedly:
  deck.doEvents();
  delay(100);
}
