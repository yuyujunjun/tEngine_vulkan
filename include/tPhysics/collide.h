#pragma once
#include<functional>
#include<vector>
#include"numerical.h"
#include"Component.h"
#include"AABB.h"


namespace tEngine {
	struct Mesh;
	class GameObject_;
}


namespace tEngine {
	
	class Collider {
		//std::function<void(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* vec)> support;
	protected:
		AABB coarseAABB;
	public:
		Vector3 worldCenter;
		
		/// <summary>
		/// if isTrigger is true, only detect if collide, otherwise detect the penetration at the same time
		/// </summary>
		bool isTrigger;
		Collider() :isTrigger(false),worldCenter(0,0,0) {}
		const AABB* getAABB()const { return &coarseAABB; }
		/// <summary>
		/// Update worldCenter and calculate AABB
		/// </summary>
		virtual void UpdateDerivedData() {};
		std::function<Vector3(const void* obj, const Vector3 direction)> SupportPoint;
	//	virtual Vector3 SupportPoint(const Vector3& direction)const { return Vector3(0, 0, 0); };
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
		friend Vector3 SphereSupport(const void* obj, const Vector3& dir);
		SphereCollider() : center(0,0,0), scale(1){
			SupportPoint = SphereSupport;
		}
		void setScale(real s) { scale = s; }
		void setTranslate(const Vector3& v) { center = v; }
		void UpdateDerivedData()override;
		
	};
	
	class BoxCollider :public Collider {
		/// <summary>
		/// center and halfSize measured in local space
		/// </summary>
	public:
		Vector3 center;
		Vector3 halfSize;
		friend Vector3 BoxSupport(const void* obj, const Vector3& dir);
		BoxCollider(tEngine::GameObject_* gameObject) :Collider(gameObject), center(0, 0, 0), halfSize(1, 1, 1) {}
		void UpdateDerivedData()override;
		Vector3 SupportPoint(const Vector3& direction)const override {
			return BoxSupport(this, direction);
		}
	};
	class MeshCollider :public Collider {
		std::vector<Vector3> vertices;
		std::vector<Vector3> worldVertices;
	public:
		MeshCollider(tEngine::GameObject_* gameObject) :Collider(gameObject) {}
		void setMesh(const tEngine::Mesh& mesh);
		void UpdateDerivedData()override;
		Vector3 SupportPoint(const Vector3& direction)const override;
	};
}