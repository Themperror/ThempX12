#include "input\manager.h"
#include "input\keyboard.h"

using namespace Themp::Input;




template<typename T>
void Manager::AddInputDevice(int userIndex)
{
	static_assert(sizeof(T) == 0, "AddInputDevice<T> not implemented!");
}

template<>
void Manager::AddInputDevice<Keyboard>(int userIndex)
{
	m_Keyboards.resize(userIndex+1);
}


template<typename T>
T& Manager::GetDevice(int userIndex)
{
	static_assert(sizeof(T) == 0, "GetDevice<T> not implemented!");
	return std::move({});
}

template<>
Keyboard& Manager::GetDevice<Keyboard>(int userIndex)
{
	return m_Keyboards[userIndex];
}

template<typename T>
bool Manager::HasUser(int userIndex)
{
	static_assert(sizeof(T) == 0, "GetDevice<T> not implemented!");
	return false;
}

template<>
bool Manager::HasUser<Keyboard>(int userIndex)
{
	return m_Keyboards.size() > userIndex;
}

void Manager::Update()
{
	int maxUsers = GetMaxUserIndex();
	for (int user = 0; user < maxUsers; user++)
	{
		if (HasUser<Input::Keyboard>(user))
		{
			Input::Keyboard& keyboard = GetDevice<Input::Keyboard>(user);
			for (int i = 0; i < MAX_KEYS; i++)
			{
				Input::Keyboard::ButtonState currentKey = keyboard.m_State.keys[i];
				if (currentKey == Input::Keyboard::ButtonState::JustDown)//pressed
				{
					keyboard.m_State.keys[i] = Input::Keyboard::ButtonState::Down;
				}
				else if (currentKey == Input::Keyboard::ButtonState::JustUp)//unpressed
				{
					keyboard.m_State.keys[i] = Input::Keyboard::ButtonState::Up;
				}
			}
		}
	}
}

int Manager::GetMaxUserIndex()
{
	return static_cast<int>(m_Keyboards.size()) - 1;
}