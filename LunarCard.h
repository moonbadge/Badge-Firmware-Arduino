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


class MenuCardEntry {
public:
	String image_path;
	String target;
	bool show();
};
class MenuCard : public LunarCard{

};



class AnimationSlide{
public:
	int duration;
	RefreshType refresh;
	String image_path;
};

class AnimationCard : public LunarCard {
  public:
    vector<AnimationSlide *> slides;
    bool show();
    void animate();
};


#endif
