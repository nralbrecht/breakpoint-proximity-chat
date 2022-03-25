#pragma once

#include "teamspeak/public_definitions.h"


typedef struct {
    float health;

	TS3_VECTOR mouthPosition;
	TS3_VECTOR mouthForward;
    TS3_VECTOR mouthUp;

    TS3_VECTOR earPosition;
	TS3_VECTOR earForward;
    TS3_VECTOR earUp;
} GameState;
