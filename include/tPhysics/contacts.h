#pragma once
#include"numerical.h"
#include"tCCD.h"
namespace tEngine {
	class ContactResolver;
	class Collider;
	class RigidBody;
	class RigidBodySystem;
/// <summary>
/// Obtain Orthonormal Basis given contact normal(contact normal as y)
/// </summary>
/// <param name="x">const</param>
/// <param name="y"></param>
/// <param name="z"></param>
	inline const Mat3& calculateContactsBasis(const glm::vec3& normal) {
		glm::vec3 y, z;
		if (abs(normal.x) > abs(normal.z)) {
			const real s = (real)1.0f /sqrt(normal.z * normal.z + normal.x * normal.x);
			// The new X axis is at right angles to the world Y axis.
			y.x = normal.z * s;
			y.y = 0;
			y.z = -normal.x * s;
			// The new Y axis is at right angles to the new X and Z axes.
			z.x = normal.y * y.x;
			z.y = normal.z * y.x - normal.x * y.z;
			z.z = -normal.y * y.x;
		}
		else {
			const real s = (real)1.0 /sqrt(normal.z * normal.z +normal.y * normal.y);
			// The new X axis is at right angles to the world X axis.
			y.x = 0;
			y.y = -normal.z * s;
			y.z = normal.y * s;
			// The new Y axis is at right angles to the new X and Z axes.
			z.x = normal.y * y.z - normal.z * y.y;
			z.y = -normal.x * y.z;
			z.z = normal.x * y.y;
		}
		return glm::mat3(-y, normal, z);
	}
	class Contact {
		friend class ContactResolver;
	public:
		//Collider* collider[2];
		Transform* transform[2];
		RigidBody* rigidBody[2];
		real velocityPerUnitImpulse[2];//沿法线方向的单位冲量引起的角速度变化导致的的延法线速度变化
		real friction;
		real restitution;
		Vector3 contactPoint;
		Vector3 contactNormal;
		real penetration;
		void SetData(RigidBody* obj1,Transform* t1, RigidBody* obj2,Transform* t2, const ContactInfo& info);
	protected:
		Mat3 contact2World;
		Vector3 contactVelocity;
		real desiredDeltaVelocity;
		Vector3 relativeContactPosition[2];
	protected:
		void calculateInternals(real duration);
		/// <summary>
		/// Calculate the velocity of the contact point on the given collider
		/// </summary>
		/// <param name="bodyIdx"></param>
		/// <param name="duration"></param>
		/// <returns></returns>
		Vector3 calculateLocalVelocity(unsigned bodyIdx, real duration);
		void calculateDesiredDeltaVelocity(real duration);
		
		void calculateAngularInertiaPerUnit();
		//void ApplyImpulse(const Vector3& impulse, RigidBody* body, Vector3* velocityChange, Vector3* rotationChange);
		void ApplyVelocityChange(Vector3 linearChange[2],Vector3 angularChange[2]);
		void ApplyPositionChange(Vector3 linearChange[2],Vector3 angularChange[2],real penetration);
		void MatchAwakeState();//if one object is awake, awake another
		Vector3 calculateFrictionlessImpulse();
		Vector3 calculateFrictionImpulse();
	};
	class ContactResolver {
	protected:
		unsigned velocityIterations;//number of iterations to perform when resolving velocity
		unsigned positionIterations;
		real velocityEpsilon;//if velocity less than epsilon, consider it to be zero to avoid instability
		real positionEpsilon;//penetrations epsilon
	public:
		unsigned velocityIterationsUsed;
		unsigned positionIterationsUsed;
	private:
		bool validSettings;
	public:
		ContactResolver(unsigned iterations, real velocityEpsilon = (real)0.01, real positionEpsilon = (real)0.01);
		ContactResolver(unsigned velocityIterations,unsigned positionIterations,real velocityEpsilon = (real)0.01,real positionEpsilon = (real)0.01);
		/**
		* Returns true if the resolver has valid settings and is ready to go.
		*/
		bool isValid()
		{
			return (velocityIterations > 0) &&
				(positionIterations > 0) &&
				(velocityEpsilon >= 0.0f) &&
				(positionEpsilon >= 0.0f);
		}
		void resolveContacts(Contact* contactArray, unsigned numContacts, real duration);
	protected:
		void prepareContacts(Contact* contactArray, unsigned numContacts, real duration);
		/**
	   * Resolves the velocity issues with the given array of constraints,
	   * using the given number of iterations.
	   */
		void adjustVelocities(Contact* contactArray,unsigned numContacts,real duration);

		/**
		 * Resolves the positional issues with the given array of constraints,
		 * using the given number of iterations.
		 */
		void adjustPositions(Contact* contactArray,unsigned numContacts,real duration);
	};
	class ContactGenerator {
	public:
		virtual unsigned addContact(Contact* contact, unsigned limit)const=0;
	};
}