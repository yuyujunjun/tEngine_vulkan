#pragma once
#include"EngineContext.h"
#include"Eigen/Sparse"
#include"Eigen/Dense"
#include"GameObject.h"
#include"tTransform.h"
#include"Camera.h"
//Different constraint
namespace SpringSolver {
	using T = Eigen::Triplet<double>;
	struct ConstraintImpl {
		void Push(std::vector<T> &tripletList,int i, int j, double k) {
			tripletList.push_back(T(3 * i, 3 * j, k));
			tripletList.push_back(T(3 * i + 1, 3 * j + 1, k));
			tripletList.push_back(T(3 * i + 2, 3 * j + 2, k));
		};
		virtual Eigen::Vector3d SolveD(const Eigen::VectorXd& x) { return Eigen::Vector3d(); }
		virtual void Build_J(size_t index, std::vector<T>& tripletlist) {}
		virtual void Build_L(size_t index, std::vector<T>& tripletlist) {}
		virtual double Length() { return 0; };
	};
	struct Spring:public ConstraintImpl {
		Spring(std::array<int,2>pos_id,const Eigen::VectorXd& p,double k):pos_id(pos_id),k(k) {
			restLength=(p.block<3, 1>(3 * pos_id[0], 0) - p.block<3, 1>(3 * pos_id[1], 0)).norm();
		}
		//using T;
		Eigen::Vector3d SolveD(const Eigen::VectorXd& x)override {
			Eigen::Vector3d direction = x.block<3, 1>(3 * pos_id[0], 0) - x.block<3, 1>(3 * pos_id[1], 0);
			direction = direction.normalized() * restLength;
			return direction;
		}
		void Build_J(size_t index, std::vector<T>& tripletlist)override {
			Push(tripletlist,pos_id[0], index, k);
			Push(tripletlist,pos_id[1], index, -k);
		}
		void Build_L(size_t index, std::vector<T>& tripletlist)override {
			auto& s = pos_id;
			Push(tripletlist,s[0], s[0], k);
			Push(tripletlist,s[1], s[1], k);
			Push(tripletlist,s[0], s[1], -k);
			Push(tripletlist,s[1], s[0], -k);
		
		}
		int operator[](int id) {
			return pos_id[id];
		}
		double Length() { return restLength; }
	private:
		double k=1e2;
		double restLength;
		std::array<int, 2> pos_id;
	};
	struct StaticPoint :public ConstraintImpl {
		StaticPoint(Eigen::Vector3d pos,int pos_id,double k):k(k),pos(pos),pos_id(pos_id) {

		}
		//using T;
		//pos-d=0
		Eigen::Vector3d SolveD(const Eigen::VectorXd& x)override {
			return pos;
		}
		void Build_J(size_t index, std::vector<T>& tripletlist)override {
			Push(tripletlist, pos_id, index, k);
		}
		void Build_L(size_t index, std::vector<T>& tripletlist)override {
			auto& s = pos_id;
			Push(tripletlist, s, s, k);
		}
		double Length() { return 0; }
	private:
		double k=1e5;
		Eigen::Vector3d pos;
		int pos_id;
	};
	struct MSpring  {
		MSpring() = default;
		
		void Start(tEngine::Device* device);
		void BuildSpring();
		void BuildMatrix();
		void Update(float dt);
		void BuildMesh();
		void Setb();
	//	void ConstructCell(const Eigen::VectorXd& x, const double length);
	//	void IntersectSelf();
		void UpdateMesh();
		void DampVelocity();
		~MSpring();
		std::vector<std::shared_ptr<ConstraintImpl>> constraint;
		int resolution[2];
		double mass = .1;
		double h=1.0/120;
		double k = 3e5;
		double m_damping_coefficient=3;
		float viscousCof = 0.5;
		double max_edgeLength;
		Eigen::Vector3d fluid;
		double constant=1;
		tEngine::Mesh mesh;
		//tEngine::MeshBuffer mesh;
		//tEngine::MeshBuffer collide;
		double time = 0;
	private:
	
		//Calculated stroe number
		Eigen::SparseMatrix<double> L;
		Eigen::SparseMatrix<double> M_h2L;
		Eigen::SparseMatrix<double> J;
		Eigen::VectorXd y;
		Eigen::VectorXd d;
		Eigen::VectorXd force;
		Eigen::SimplicialLLT<Eigen::SparseMatrix<double>, Eigen::Upper> Solver;

		Eigen::VectorXd x;//iterated position
		Eigen::VectorXd v;//current velocity
		Eigen::VectorXd p;//current position

	};
	
}
