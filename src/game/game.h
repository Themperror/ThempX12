#pragma once
#include "core/renderer/types.h"
#include "core/components/sceneobject.h"

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
		std::vector<Themp::SceneObject> m_3DObjects;
	};
}