#pragma once
#include "renderer/types.h"
namespace Game
{
	class Game
	{
	public:
		void Start();
		void Stop();
		void Update(double delta);

	private:

		Themp::D3D::MainPassHandle m_MainPass = Themp::D3D::InvalidHandle;
	};
}