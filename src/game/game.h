#pragma once
#include "core/renderer/types.h"
#include "core/components/sceneobject.h"

#include "core/camera.h"

#include <vector>
#include <memory>

namespace Themp
{
	class Camera;
}
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
		std::unique_ptr<Themp::Camera> m_Camera;
	};
}