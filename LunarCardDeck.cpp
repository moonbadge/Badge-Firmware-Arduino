
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
    	MenuCard *m = new MenuCard();
    	String menu_default_position = deck_json[i]["menu-default-position"];
    	String jump_back = deck_json[i]["jump-back"]["page"];
    	int menucount=deck_json[i]["options"].size();
    	for (int j=0; j<menucount; j++){
    		MenuEntry *me = new MenuEntry();
    		String path = deck_json[i]["options"][j]["image"];
    	      if(isAbsolute(path)==false){
    	        path = deck_folder+path;
    	      }
    		String target = deck_json[i]["options"][j]["destination"]["page"];
    		me->target=target;
    	}
      

    	c = (LunarCard *) m;
    }
    if (cardtype=="animation"){
      
    }
    String cardname = deck_json[i]["name"];
    c->cardname = cardname;
    for (int j = 0; j < deck_json[i]["event"].size(); j++) {
      Transition *t = new Transition();
      String type = deck_json[i]["event"][j]["type"];
      String key = deck_json[i]["event"][j]["key"];
      String target_page = deck_json[i]["event"][j]["destination"]["page"];
      String refresh = deck_json[i]["event"][j]["refresh"];
      t->type = type;
      t->key = key;
      t->target = target_page;
      if (refresh=="none") t->refresh = None;
      else if (refresh=="short") t->refresh = Short;
      else if (refresh=="full") t->refresh = Full;
      else t->refresh = Full;
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
  cards[card_index]->show();
  return true;
}

void LunarCardDeck::doEvents() {
  LunarCard * c = cards[current_card_index];
  c->doEvents();
}

void LunarCardDeck::addCard(LunarCard * c) {
  cards.push_back(c);
}

bool AnimationCard::show(){

}

void AnimationCard::doEvents(){

}
