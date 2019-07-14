#include "LunarCard.h"
#include "moonbadge.h"
#include "Transition.h"
#include <Arduino.h>
#include <ArduinoJson.h>
extern MoonBadge badge;

using namespace std;
void LunarCard::addTransition(Transition * t) {
  transitions.push_back(t);
}

String LunarCard::toString() {
  String response = "\tName:\t'" + cardname + "'\n";
  for (int i = 0; i < transitions.size(); i++) {
    Transition *t = transitions[i];
    response = response + "\t\tTransition\n\t\t\t" + "Type: " + t->type + "\n\t\t\tTarget: '" + t->target + "'" + "\n\t\t\tKey: '" + t->key + "'\n";
  }
  return response;
}

bool ImageCard::show(){
  Serial.print("\tImg Path: '"); Serial.print(image_path); Serial.println("'");
  badge.drawBitmapFromSpiffs(image_path, 0, 0, true);
  
}

void ImageCard::doEvents(){

}

void MenuCard::doEvents(){
	TouchKey key = badge.getTouch();
	if (key==NoKey) return;
	if (key==Left)

}

bool MenuCard::show(){
	MenuEntry *me = items[selected_item];
	badge.drawBitmapFromSpiffs(me->image_path, 0, 0, true);
}

void MenuCard::addItem(MenuEntry *me){
	items.push_back(me);
}
