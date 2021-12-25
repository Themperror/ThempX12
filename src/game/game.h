#pragma once
#include "core/renderer/types.h"
#include "core/renderer/object3d.h"

#include <vector>

namespace Game
{
	class Game
	{
	public:
		void Start();
		void Stop();
		void Update(double delta);

	private:
		std::vector<Themp::D3D::Object3D> m_3DObjects;
	};
}