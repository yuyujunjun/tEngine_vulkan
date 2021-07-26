#pragma once
#include<functional>
#include"physicsCore.h"
#include"Component.h"
#include"AABB.h"
#include"libccd/support.h"

class tEngine::GameObject_;
namespace tPhysics {
	
	class Collider:public tEngine::Component {
		std::function<void(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* vec)> support;
	protected:
		AABB coarseAABB;
	public:
		
		
		/// <summary>
		/// if isTrigger is true, only detect if collide, otherwise detect the penetration at the same time
		/// </summary>
		bool isTrigger;
		Collider(tEngine::GameObject_* gameObject) :Component(gameObject),isTrigger(false) {}
		const AABB* getAABB()const { return &coarseAABB; }
		//store AABB for coarse phase
		virtual void UpdateDerivedData() {};
	};
	/// <summary>
	/// Center equals to origin, radius equals to 1
	/// </summary>
	class SphereCollider :public Collider {
		Vector3 center;
		real scale;
		//Vector3 worldCenter;
		//real worldScale;
	public:
		friend void SphereSupport(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* vec);
		SphereCollider(tEngine::GameObject_* gameObject) :Collider(gameObject), center(0,0,0), scale(1){}
		void setScale(real s) { scale = s; }
		void setTranslate(const Vector3& v) { center = v; }
		void UpdateDerivedData()override;
	};
	
	class BoxCollider :public Collider {
		/// <summary>
		/// center and halfSize measured in local space
		/// </summary>
		
		Vector3 worldCenter;
		
	public:
		Vector3 center;
		Vector3 halfSize;
		friend void BoxSupport(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* vec);
		BoxCollider(tEngine::GameObject_* gameObject) :Collider(gameObject), center(0, 0, 0), halfSize(1, 1, 1),worldCenter(0,0,0) {}
		void UpdateDerivedData()override;
	};
}