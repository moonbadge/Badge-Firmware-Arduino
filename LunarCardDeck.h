#pragma once

#include <String.h>
#include <vector>
#include "LunarCard.h"
#include "Transition.h"



using namespace std;
class LunarCardDeck {
    LunarCard *currentCard;

    vector<LunarCard *> cards;
  public:
    String deck_folder;
    String deck_filename;
    String deck_path;
    bool load(String deck_path);
    void addCard(LunarCard * c);
    bool showCard(String display_card_name);
    bool showCard(LunarCard *c);
    bool showCard(int index);

    LunarCard *findCardByName(String n);
    void doEvents(void);



};

