#include "RefreshType.h"


String RefreshType2Str(RefreshType c){
	switch(c){
		case None: return "none"; break;
		case Short: return "short"; break;
		case Full: return "full"; break;
	}
	return "";
}
RefreshType Str2RefreshType(String s){
	if (s=="none") return None;
	if (s=="short") return Short;
	if (s=="full") return Full;
	return Full;
}

