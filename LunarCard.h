#pragma once

#include "LunarCardDeck.h"
#include "Transition.h"
#include "CardType.h"
#include <vector>
#include <String.h>

class LunarCard {
  public:
    String cardname;
    int index;
    unsigned long msLoaded=0;
    vector<Transition *> transitions;
    CardType type;
    String toString();
    void addTransition(Transition *t);
    String image_path;
    bool show();
    void doEvents();
    

};
