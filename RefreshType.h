/*
 * RefreshType.h
 *
 *  Created on: Jul 13, 2019
 *      Author: AlexCamilo
 */
#pragma once
#include <wString.h>

enum RefreshType { None = 0, Short, Full };


String RefreshType2Str(RefreshType c);
RefreshType Str2RefreshType(String s);

#pragma once
