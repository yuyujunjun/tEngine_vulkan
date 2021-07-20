#pragma once
namespace tEngine {
	class GameObject_;
	class System {
		friend class tWorld;
	public:
		virtual void ExecuteAllComponents(float dt) = 0;
	};
	class Component {
	public:
		Component() = default;
		Component(GameObject_* gameObject) :gameObject(gameObject) {}
		GameObject_* gameObject = 0;
		virtual ~Component() {}

	};
}