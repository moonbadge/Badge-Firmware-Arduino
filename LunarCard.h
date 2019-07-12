#ifndef lunarcard-h
#define lunarcard-h

using namespace std;
#include <FS.h>
#include "SPIFFS.h"
#include "moonbadge.h"

#include <vector>



class Transition {
  public:
    String type;
    String key;
    String target;
};

class LunarCard {
  public:
    String cardname;
    String image;
    vector<Transition> transitions;
    String toString();
    void addTransition(Transition t);

};

class LunarCardDeck {
    int current_card_index = 0;

    vector<LunarCard> cards;
  public:
    String deck_folder;
    String deck_filename;
    String deck_path;
    bool load(String deck_path);
    void addCard(LunarCard c);
    bool showCard(String display_card_name);
    bool showCard(int index);

    void doEvents(void);



};



#endif
