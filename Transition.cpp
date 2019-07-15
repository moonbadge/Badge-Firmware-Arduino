#include "Transition.h"

String TransitionType2Str(TransitionType t){
	switch(t){
	case Key: return "Key";
	case Delay: return "Delay";
	case Invalid: return "Invalid";
	}
}
