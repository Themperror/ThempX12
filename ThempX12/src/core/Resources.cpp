#include "Engine.h"
#include "Resources.h"
#include "renderer/control.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace Themp
{
	std::unique_ptr<Resources> Resources::instance;
}