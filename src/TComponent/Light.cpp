#include "Light.h"
namespace tEngine {
	const glm::mat4& Light::world_to_lightMatrix() {
		return transform.Matrix();
	}
}
