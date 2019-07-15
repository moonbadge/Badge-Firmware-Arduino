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
		if (c->type == Image || c->type == Animation) {
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
					Serial.print("' key: '");Serial.print(key);Serial.print("' ");
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
	for (int i = 0; i < cards.size(); i++) {
		LunarCard *c = cards[i];

		if (c->type==Animation){
			Serial.print("Found Animation '");Serial.print(c->cardname);Serial.println("'");
			// Lookup card in JSON and iterate over animation steps
			int image_count = deck_json[c->index]["images"].size();
			Transition *en = new Transition();
			en->msDelay=1;
			en->type=Delay;
			en->refresh=None;
			en->timer=true;
			c->transitions.push_back(en);


			Transition *prev_t=NULL;
			LunarCard *firstSlide=NULL;
			for (int j = 0; j < image_count; j++) {
				String path = deck_json[c->index]["images"][j]["image"];
				unsigned int step_delay = deck_json[c->index]["images"][j]["delay"];
				// Create a slide.
				LunarCard *slide = new LunarCard();
				slide->type=AnimationStep;
				String animCardName = c->cardname+"@"+String(j);
				Serial.print("     Adding step '");Serial.print(animCardName);Serial.println("'");
				slide->cardname = animCardName;
				if (isAbsolute(path) == false) {
					path = deck_folder + path;
				}
				Serial.print("          Path: '");Serial.print(path);Serial.println("'");
				slide->image_path=path;
				// Add a transition to the next slide
				Transition *t = new Transition();
				slide->transitions.push_back(t);

				t->type = Delay;
				t->refresh=None;
				t->timer = true;
				t->msDelay = step_delay;
				Serial.print("          Delay ");Serial.print(step_delay);Serial.println("ms");
				// Copy parent's transitions into animation slides.

				for(int k=0; k<c->transitions.size(); k++){
					//Serial.print(k);Serial.print(":"); Serial.println(TransitionType2Str(c->transitions[k]->type));
					if (c->transitions[k]->type==Key){
						slide->transitions.push_back(c->transitions[k]);
						Serial.print("               Adding transition '");Serial.print(TransitionType2Str(c->transitions[k]->type));Serial.println("'");
					}
				}

				if(j==0){ //first
					Serial.println("          This is the first slide");
					// there isn't a previous slide. just save the transition
					prev_t = t;
					firstSlide = slide;
					en->target = firstSlide;
					c->image_path = firstSlide->image_path;
				} else if(j==image_count-1){ // last
					Serial.println("          This is the last slide");
					//set us as the previous slide's target

					prev_t->target = slide;
					// Set our target to the first slide
					t->target = firstSlide;
				} else { // middle
					// set us as the previous slide's target
					prev_t->target = slide;
					prev_t = t;
				}
				cards.push_back(slide);

			}


		}
	}

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
	if (currentCard==NULL) {
		Serial.println("Current Card is NULL");
		return;
	}
	currentCard->doEvents();
	TouchKey key = badge.getTouch();
	LunarCard *target;
	bool validEvent=false;
	if (currentCard->transitions.size() == 0){
		Serial.println("No Transitions");
		return;
	}
	Transition *t;
	for (int i = 0; i < currentCard->transitions.size(); i++) {
		t = currentCard->transitions[i];
		// Key Events?
		/*Serial.print(currentCard->cardname);
		Serial.print(":");
		Serial.print(i);
		Serial.print(":");
		Serial.println(TransitionType2Str(t->type));*/
		if (key != NoKey && t->type == Key && key == t->key) {
			Serial.print("Handling Key Event");
			Serial.print("     Key: '");
			Serial.print(key2str(key));
			Serial.println("' ");
			target = t->target;
			validEvent=true;
		}
		if (t->timer && t->type == Delay){
			unsigned long ct = millis();
			//Serial.print(t->msDelay+currentCard->msLoaded); Serial.print(":");Serial.print(millis()); Serial.print(":");Serial.println(t->msDelay+currentCard->msLoaded < ct);
			if ( (t->msDelay+currentCard->msLoaded) < ct){
				Serial.print("Handling Delay Event '");
				Serial.print("\tms: '");
				Serial.print(t->msDelay);
				Serial.print("' ");
				target = t->target;
				badge.refresh = t->refresh;
				validEvent=true;
			}
		}
	}
	if (validEvent) {
		//badge.refresh = target->refresh;
		Serial.print("     Target: '");
		Serial.print(target->cardname);
		Serial.println("'");
		Serial.print("     Refresh: '");
		Serial.print(RefreshType2Str(badge.refresh));
		Serial.println("'");
		showCard(target);

		if (t->type==Key){
			Serial.print("Waiting for release...");
			if (badge.waitForTouchRelease())
				Serial.println("Done!");
			else
				Serial.println("Timeout!");
			delay(800);
			resetTouch();
		}
	}

}

void LunarCardDeck::addCard(LunarCard * c) {
	cards.push_back(c);
}

