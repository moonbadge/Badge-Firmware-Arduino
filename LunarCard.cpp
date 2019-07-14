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
  //String path = image;
  //if(isAbsolute(path)==false){
  //  path = path;//deck.deck_folder+path;
  //}
  badge.drawBitmapFromSpiffs(image_path, 0, 0, true);
  
}

void ImageCard::doEvents(){
	int touch_num = badge.getTouch();
	  String touch_key;
	  if (touch_num != 0) {
	    switch (touch_num) {
	      case 0: break; // no touch
	      case 1: touch_key = "button-up"; break;
	      case 2: touch_key = "button-down"; break;
	      case 3: touch_key = "button-left"; break;
	      case 4: touch_key = "button-right"; break;
	    }
	  }

	  String target = "";
	  bool haveTarget = false;
	  if (transitions.size() == 0) return;
	  for (int i = 0; i < transitions.size(); i++) {
	    Transition *t = transitions[i];
	    badge.refresh = t->refresh;
	    if (t->type == "keypress" && touch_num != 0) {
	      if (touch_key == t->key) {
	        target = t->target;
	        Serial.println("Handling Touch Transition!");
	        Serial.print("Waiting for release...");
	        if (badge.waitForTouchRelease()) Serial.println("Done!");
	        else Serial.println("Timeout!");
	        delay(500);
	        Serial.print("\tKey: '");    Serial.print(touch_key); Serial.println("'");
	        haveTarget = true;
	        break;
	      }
	    }
	  }
	  if (haveTarget) {
	    Serial.print("\tTarget: '"); Serial.print(target); Serial.println("'");
	    show();
	  }
	  resetTouch();
}

void MenuCard::doEvents(){

}

bool MenuCard::show(){

}

void MenuCard::addItem(MenuEntry *me){
	items.push_back(me);
}
