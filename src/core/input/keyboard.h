#pragma once

namespace Themp
{
	class Engine;
	namespace Input
	{
		constexpr int MAX_KEYS = 256;
		class Keyboard
		{
		public:
			enum class ButtonState : char { Up, Down, JustDown, JustUp };
			struct KeyboardState
			{
				ButtonState keys[MAX_KEYS];
			};
			const KeyboardState& GetState() const; 
		private:
			KeyboardState m_State;
			friend class Engine;
			friend class Manager;
		};
	}
}