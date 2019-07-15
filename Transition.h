#pragma once
#include "RefreshType.h"
//#include "LunarCard.h"
class LunarCard;
#include "TouchKey.h"

using namespace std;

enum TransitionType {Invalid=0,Key,Delay};
class Transition {
  public:
    unsigned int msDelay=0;
    bool timer=false;
     TouchKey key;
     TransitionType type;
    LunarCard *target;
    RefreshType refresh;
};

String TransitionType2Str(TransitionType t);

