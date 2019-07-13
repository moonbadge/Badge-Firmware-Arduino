
#include "LunarCardDeck.h"
#include "LunarCard.h"
#include "Transition.h"
#include "moonbadge.h"
#include <ArduinoJson.h>


extern MoonBadge badge;


bool LunarCardDeck::load(String path) {
  DynamicJsonDocument deck_json(10000);
  deck_folder = getPath(path);
  deck_filename = getFilename(path);
  deck_path = path;

  Serial.print("Loading Deck:\t'"); Serial.print(deck_path); Serial.println("'");
  File deck_file = badge.openFile( deck_path, "r");//SPIFFS.open( deck_path, "r");
  if (!deck_file) {
    Serial.println("Could not open file");
    //badge.print_text("Deck File\nMissing", 2, 16);
    return false;
  }
  DeserializationError err =  deserializeJson(deck_json, deck_file);
  switch (err.code()) {
    case DeserializationError::Ok:
      Serial.println(F("Deserialization succeeded"));
      break;
    case DeserializationError::InvalidInput:
      Serial.println(F("Invalid input!"));
      badge.print_text("Courrupt\nDeck", 2, 16);
      break;
    case DeserializationError::NoMemory:
      Serial.println(F("Not enough memory"));
      badge.print_text("Deck Too\nLarge", 2, 16);
      break;
    default:
      badge.print_text("Unknown\nFailure", 2, 16);
      break;
  }
  deck_file.close();

  //
  Serial.print("  Deck Size:\t"); Serial.println(deck_json.size());
  //badge.drawBitmapFromSpiffs("test.bmp", 0, 0, true);
  for (int i = 0; i < deck_json.size(); i++) {

    String cardtype = deck_json[i]["type"];
    LunarCard *c = NULL;
    if (cardtype=="image"){
      ImageCard *im = new ImageCard();
      String path = deck_json[i]["image"];
      if(isAbsolute(path)==false){
        path = deck_folder+path;
      }
    im->image_path=path;
    c = (LunarCard *) im;
    }
    if (cardtype=="menu"){
      
    }
    if (cardtype=="animation"){
      
    }
    String cardname = deck_json[i]["name"];
    c->cardname = cardname;
    for (int j = 0; j < deck_json[i]["jump"].size(); j++) {
      Transition *t = new Transition();
      String type = deck_json[i]["jump"][j]["type"];
      String key = deck_json[i]["jump"][j]["key"];
      String target_page = deck_json[i]["jump"][j]["destination"]["page"];
      t->type = type;
      t->key = key;
      t->target = target_page;
      c->addTransition(t);
    }
    Serial.println("Adding Card");
    Serial.println(c->toString());
    addCard(c);

  }
  showCard("entry");
}

bool LunarCardDeck::showCard(String display_card_name) {
  Serial.print("Showing Card '"); Serial.print(display_card_name); Serial.println("'");
  int card_index = -1;
  for (int i = 0; i < cards.size(); i++) {
    if (display_card_name == cards[i]->cardname) {
      Serial.print("\tIndex is: "); Serial.print(i); Serial.println("");
      card_index = i;
    }
  }
  if (card_index == -1) {
    String err = "Card Name \n'" + display_card_name + "' \nNot\nIn Deck";
    badge.print_text(err, 2, 16);
    return false;
  }
  current_card_index = card_index;
  return showCard(card_index);
}

bool LunarCardDeck::showCard(int card_index) {
  /*
  Serial.print("\tLoading Card #"); Serial.print(card_index); Serial.println("");
  Serial.print("\tImg Path: '"); Serial.print(cards[card_index].image); Serial.println("'");
  String path = cards[card_index].image;
  if(isAbsolute(path)==false){
    path = deck_folder+path;
  }
  badge.drawBitmapFromSpiffs(path, 0, 0, true);
  current_card_index = card_index;
*/
  cards[card_index]->show();
  return true;
}

void LunarCardDeck::doEvents() {
  LunarCard * c = cards[current_card_index];
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
  if (c->transitions.size() == 0) return;
  for (int i = 0; i < c->transitions.size(); i++) {
    Transition *t = c->transitions[i];
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
    showCard(target);
  }
  resetTouch();
}

void LunarCardDeck::addCard(LunarCard * c) {
  cards.push_back(c);
}
