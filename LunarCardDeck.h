#ifndef lunarcard-h
#define lunarcard-h



class LunarCard;
#include <String.h>
#include <vector>
#include "LunarCard.h"
#include "Transition.h"



using namespace std;
class LunarCardDeck {
    int current_card_index = 0;

    vector<LunarCard *> cards;
  public:
    String deck_folder;
    String deck_filename;
    String deck_path;
    bool load(String deck_path);
    void addCard(LunarCard * c);
    bool showCard(String display_card_name);
    bool showCard(int index);

    void doEvents(void);



};



#endif
