#pragma once

#include "stdafx.h"

#define gAPI instance::api

class instance {
public:
	static gw2al_core_vtable* api;
};
