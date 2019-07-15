#include "LunarCardDeck.h"
#include "LunarCard.h"
#include "Transition.h"
#include "CardType.h"
#include "moonbadge.h"
#include "RefreshType.h"
#include <ArduinoJson.h>

extern MoonBadge badge;

bool LunarCardDeck::load(String path) {
	currentCard==NULL;
	DynamicJsonDocument deck_json(10000);
	deck_folder = getPath(path);
	deck_filename = getFilename(path);
	deck_path = path;

	Serial.print("Loading Deck:\t'");
	Serial.print(deck_path);
	Serial.println("'");
	File deck_file = badge.openFile(deck_path, "r"); //SPIFFS.open( deck_path, "r");
	if (!deck_file) {
		Serial.println("Could not open file");
		//badge.print_text("Deck File\nMissing", 2, 16);
		return false;
	}
	DeserializationError err = deserializeJson(deck_json, deck_file);
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

	// Pass one, add cards to array.
	Serial.println("  Adding Cards...");
	Serial.print("  Deck Size:\t");
	Serial.println(deck_json.size());
	for (int i = 0; i < deck_json.size(); i++) {
		LunarCard *c = new LunarCard();
		String cardtype = deck_json[i]["type"];
		String cardname = deck_json[i]["name"];
		Serial.print("Adding Card '");Serial.print(cardname);Serial.print("' ...");
		CardType ctype = Str2CardType(cardtype);
		if (ctype != Invalid) {
			c->type = ctype;
			c->cardname = cardname;
			c->index = i;
			cards.push_back(c);
			Serial.println("ok!");
		} else {
			Serial.println("failed!");
			delete c;
		}
	}

	// Pass Two, add transitions
	for (int i = 0; i < cards.size(); i++) {
		LunarCard *c = cards[i];
		// Do images for now
		if (c->type == Image) {
			String path = deck_json[i]["image"];
			if (isAbsolute(path) == false) {
				path = deck_folder + path;
			}
			c->image_path = path;
			// Add Events
			for (int j = 0; j < deck_json[c->index]["event"].size(); j++) {
				Transition *t = new Transition();
				String type = deck_json[c->index]["event"][j]["type"];
				if (type=="keypress") t->type=Key;
				else if (type=="delay") t->type=Delay;
				else t->type=Invalid;



				String target = deck_json[c->index]["event"][j]["destination"]["page"];
				Serial.print("Adding '");Serial.print(TransitionType2Str(t->type));
				t->target = findCardByName(target);

				String refresh = deck_json[i]["event"][j]["refresh"];
				t->refresh = Str2RefreshType(refresh);

				if (t->type == Key){
					String key = deck_json[i]["event"][j]["key"];
					Serial.print(" key: '");Serial.print(key);Serial.print("' ");
					t->key = str2key(key);
				}
				if (t->type == Delay){
					unsigned int delay = deck_json[i]["event"][j]["delay"];
					Serial.print(" delay: '");Serial.print(delay);Serial.print("' ");
					t->msDelay = delay;
					t->timer = true;
					// todo
				}
				Serial.print("' transition from '");Serial.print(c->cardname);Serial.print("' to '");Serial.print(target);Serial.print("' ");
				if (t->target){
					c->transitions.push_back(t);
				}
				else {
					Serial.println("Could not find card!");
					delete t;
				}
				Serial.println();
			}
		}
	}

	// Pass three, expand menus and animations


	showCard("entry");
}

bool LunarCardDeck::showCard(String display_card_name) {
	int card_index = -1;
	for (int i = 0; i < cards.size(); i++) {
		if (display_card_name == cards[i]->cardname) {
			card_index=i;
		}
	}
	if (card_index == -1) {
		String err = "Card Name \n'" + display_card_name + "' \nNot\nIn Deck";
		badge.print_text(err, 2, 16);
		return false;
	}
	return showCard(card_index);
}

LunarCard *LunarCardDeck::findCardByName(String n){
	for (int i=0; i<cards.size(); i++){
		LunarCard *c = cards[i];
		if (c->cardname==n) return c;
	}
	return NULL;
}

bool LunarCardDeck::showCard(int card_index) {
	showCard(cards[card_index]);
	return true;
}

bool LunarCardDeck::showCard(LunarCard *c){
	currentCard = c;
	Serial.print("Showing Card '");
	Serial.print(currentCard->cardname);
	Serial.println("'");
	currentCard->show();
}

void LunarCardDeck::doEvents() {
	if (currentCard==NULL) return;
	currentCard->doEvents();
	TouchKey key = badge.getTouch();
	LunarCard *target;
	bool validEvent=false;
	if (currentCard->transitions.size() == 0){
		return;
	}
	for (int i = 0; i < currentCard->transitions.size(); i++) {
		Transition *t = currentCard->transitions[i];
		badge.refresh = t->refresh;
		// Key Events?
		if (key != NoKey && t->type == Key && key == t->key) {
			Serial.print("Handling Key Event '");
			Serial.print("\tKey: '");
			Serial.print(key2str(key));
			Serial.print("' ");
			target = t->target;
			validEvent=true;
		}
		if (t->timer && t->type == Delay){
//			//Serial.print(t->msDelay+currentCard->msLoaded); Serial.print(":");Serial.print(millis()); Serial.print(":");Serial.println(t->msDelay+currentCard->msLoaded > millis());
			if (t->msDelay+currentCard->msLoaded > millis()){
				Serial.print("Handling Delay Event '");
				Serial.print("\tms: '");
				Serial.print(t->msDelay);
				Serial.print("' ");
				target = t->target;
				validEvent=true;
			}
		}
	}
	if (validEvent) {
		Serial.print("\t Target: '");
		//Serial.print(target->cardname);
		Serial.println("'");
		showCard(target);
		Serial.print("Waiting for release...");
		if (badge.waitForTouchRelease())
			Serial.println("Done!");
		else
			Serial.println("Timeout!");
		delay(800);
	}
	resetTouch();
}

void LunarCardDeck::addCard(LunarCard * c) {
	cards.push_back(c);
}

