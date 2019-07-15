/*
 * CardType.cpp
 *
 *  Created on: Jul 14, 2019
 *      Author: AlexCamilo
 */
#include "CardType.h"

String CardType2Str(CardType c){
	switch(c){
		case Image: return "image"; break;
		case Menu: return "menu"; break;
		case Animation: return "animation"; break;
		case MenuItem: return "menuitem"; break;
		case AnimationStep: return "animationstep"; break;
		case InvalidCard: return "invalid"; break;
	}
}
CardType Str2CardType(String s){
	if (s=="image") return Image;
	if (s=="menu") return Menu;
	if (s=="animation") return Animation;
	if (s=="menuitem") return MenuItem;
	if (s=="animationstep") return AnimationStep;
	return InvalidCard;
}


