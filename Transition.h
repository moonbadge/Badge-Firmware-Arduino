#ifndef TRANSITION_H
#define TRANSITION_H
#include <String.h>
#include <WString.h>

using namespace std;

enum RefreshType { None = 0, Short, Full };
class Transition {
  public:
    String type;
    String key;
    String target;
    RefreshType refresh;
};
#endif
