#pragma once
#include <vector>
namespace Themp
{
	namespace Input
	{
		class Keyboard;
		class Manager
		{
		public:
			//Add Device

			template<typename T>
			void AddInputDevice(int userIndex);

			//Get Device
			template<typename T>
			T& GetDevice(int userIndex);

			//Has User
			template<typename T>
			bool HasUser(int userIndex);


			void Update();
			int GetMaxUserIndex();
		private:
			std::vector<Keyboard> m_Keyboards;
		};
	}
}