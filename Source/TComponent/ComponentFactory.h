#pragma once
#include"Component.h"
namespace tEngine {
	struct requireCamera {};
	struct Renderer {};
	static ComponentManager<Renderer> componentRenderer;
	static ComponentManager<requireCamera> componentReCamera;
	void f() {
		componentReCamera.createComponent();
	}

}