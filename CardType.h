/*
 * CardType.h
 *
 *  Created on: Jul 14, 2019
 *      Author: AlexCamilo
 */

#pragma once
enum CardType {InvalidCard=0,Image,Menu,Animation,MenuItem,AnimationStep};
#include <WString.h>
String CardType2Str(CardType c);
CardType Str2CardType(String s);
