#include "LunarCard.h"
#include "moonbadge.h"
#include "Transition.h"
#include <Arduino.h>
#include <ArduinoJson.h>
extern MoonBadge badge;

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
