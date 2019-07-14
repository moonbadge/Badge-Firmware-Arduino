#ifndef TRANSITION_H
#define TRANSITION_H
#include <String.h>
#include <WString.h>
#include "RefreshType.h"
#include "TouchKey.h"

using namespace std;


class Transition {
  public:
    String type;
    TouchKey key;
    String target;
    RefreshType refresh;
};
#endif
