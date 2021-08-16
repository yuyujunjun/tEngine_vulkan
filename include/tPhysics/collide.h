#pragma once
#include<functional>
#include<vector>
#include"numerical.h"
#include"AABB.h"
#include"ecs.h"
#include"fcl/geometry/collision_geometry.h"
#include"fcl/common/types.h"
#include"fcl/geometry/shape/box.h"
#include"fcl/geometry/shape/sphere.h"
#include"fcl/geometry/shape/plane.h"
#include"fcl/geometry/shape/convex.h"
#include<memory>
namespace tEngine {
	struct Mesh;
	class GameObject_;
	class Transform;
}


namespace tEngine {
	struct GJKSupportCollider {
		EntityID entity;
		fcl::CollisionGeometry<real>* geometry;
		fcl::Transform3<real> tf;
	};
	struct SphereCollider {
		SphereCollider(real radius=1.0);
		std::shared_ptr <fcl::Sphere<real>> fclData;
	};
	struct BoxCollider {
		BoxCollider(Vector3 boxSide=Vector3(1,1,1));
		std::shared_ptr < fcl::Box<real> >fclData;
	};
	struct MeshCollider {
		MeshCollider()=default;
		std::shared_ptr<fcl::Convex<real>> fclData=nullptr;
		void setMesh(const tEngine::Mesh& mesh);
	};
	struct PlaneCollider {
		PlaneCollider(const Vector3& normal=Vector3(0,1,0),real offset=0) ;
		std::shared_ptr<fcl::Plane<real>> fclData = nullptr;
	};
	/*class ColliderSystem;
	struct GJKSupportObj {
		ColliderSystem* system;
		EntityID id;
		bool operator==(const GJKSupportObj& obj2) {
			return id == obj2.id;
		}
	};*/
	//struct Collider {
	//	enum class ColliderType {
	//		Box,
	//		Sphere,
	//		Mesh
	//	};
	//public:
	//	ColliderType type;
	////	Vector3 worldCenter;
	//	AABB coarseAABB;
	//	/// <summary>
	//	/// if isTrigger is true, only detect if collide, otherwise detect the penetration at the same time
	//	/// </summary>
	//	bool isTrigger;
	//	Collider() :isTrigger(false),type(ColliderType::Box) {}
	//	const AABB* getAABB()const { return &coarseAABB; }	
	//};
	//class ColliderSystem:public System {
	//public:

	//	ColliderSystem()  {}
	//	virtual Vector3 SupportPoint(EntityID id,const Vector3& direction)const { return Vector3(0, 0, 0); };
	//	virtual void UpdateDerivedData(EntityID id) {};
	//};
	///// <summary>
	///// Center equals to origin, radius equals to 1
	///// </summary>
	//class SphereColliderSystem :public ColliderSystem {
	//
	//public:
	//	friend Vector3 SphereSupport(const void* obj, const Vector3& dir);
	//	SphereColliderSystem(){
	//		
	//	}
	//	Vector3 SupportPoint(EntityID id, const Vector3& direction)const override;
	//	void UpdateDerivedData(EntityID id)override;
	//	
	//};
	//
	//class BoxColliderSystem :public ColliderSystem {
	//	/// <summary>
	//	/// center and halfSize measured in local space
	//	/// </summary>
	//public:

	//	friend Vector3 BoxSupport(const void* obj, const Vector3& dir);
	//	void UpdateDerivedData(EntityID id)override;
	//	Vector3 SupportPoint(EntityID id, const Vector3& direction)const override;
	//};
	//class MeshColliderSystem :public ColliderSystem {
	//
	//public:


	//	void UpdateDerivedData(EntityID id)override;
	//	Vector3 SupportPoint(EntityID id,const Vector3& direction)const override;
	//};
}