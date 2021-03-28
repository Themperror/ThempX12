#include "keyboard.h"
using namespace Themp::Input;

const Keyboard::KeyboardState& Keyboard::GetState() const
{
	return m_State;
}
