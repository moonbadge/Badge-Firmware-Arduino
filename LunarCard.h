#ifndef LUNAR_CARD_H
#define LUNAR_CARD_H

#include "LunarCardDeck.h"
#include "Transition.h"
#include <vector>
#include <String.h>


using namespace std;
class LunarCard {
  public:
    String cardname;
    vector<Transition *> transitions;
    String toString();
    void addTransition(Transition *t);
    virtual bool show()=0;
    

};

class ImageCard : public LunarCard {
  public:
    String image_path;
    bool show();
};
#endif
