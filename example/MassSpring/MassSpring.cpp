#include"MassSpring.h"
#include"Noise.h"
using namespace tEngine;
namespace SpringSolver {
	void MSpring::Start(tEngine::Device* device)  {
		fluid.setZero();
		resolution[0] = resolution[1] = 45;
		BuildMesh();
		mesh.createVertexBuffer(device, nullptr, tEngine::BufferDomain::Host);
		mesh.createIdxBuffer(device, nullptr);
	

		tEngine::Transform transform;
		/*transform.scale = glm::vec3(0.2, 0.2, 0.2);
		transform.position = glm::vec3(0, 0.4, -1.4);
		for (auto& p:mesh.getMesh().vertices) {
			glm::vec4 v = glm::vec4(p.Position.x,p.Position.y,p.Position.z,1);
			v=transform.Matrix()* v;
			p.Position = glm::vec3(v.x,v.y,v.z);
		}*/
		auto meshAsset = LoadMesh("sphere.obj");
		collide.setMeshUpload(meshAsset->mesh,device);
	
		BuildSpring();
		BuildMatrix();
	}
	void MSpring::BuildMesh() {
		auto& mesh = this->mesh.getMesh();
		double step_x = 1.0 / resolution[0];
		double step_y = 1.0 / resolution[1];
		auto Idx = [this](int i, int j) {
			return i * resolution[0] + j;
		};
		auto coordinate = [step_x, step_y, this](int i, int j) {
			return glm::vec3(j * step_x - 0.5, 1 - i * step_y, -1);
		};
		mesh.vertices.resize(resolution[0] * resolution[1]);
		mesh.indices.clear();
		for (int j = 0; j < resolution[0] - 1; ++j) {
			for (int i = 0; i < resolution[1] - 1; ++i) {
				mesh.indices.push_back(Idx(i, j));
				mesh.indices.push_back(Idx(i + 1, j));
				mesh.indices.push_back(Idx(i + 1, j + 1));
				mesh.indices.push_back(Idx(i, j));
				mesh.indices.push_back(Idx(i + 1, j + 1));
				mesh.indices.push_back(Idx(i, j + 1));

			}
		}
		for (int j = 0; j < resolution[0]; ++j) {
			for (int i = 0; i < resolution[1]; ++i) {
				mesh.vertices[Idx(i, j)].Position = coordinate(i, j);

			}
		}
		mesh.UpdateNormal();
		//FlipFace(mesh);
		
	}
	void MSpring::BuildSpring() {
		constraint.clear();
		auto& mesh = this->mesh.getMesh();
		p = Eigen::VectorXd(3 * mesh.vertices.size());
		v = Eigen::VectorXd(p.size());
		v.setZero();
		{
			int idx = 0;
			for (auto& v : mesh.vertices) {
				p[3 * idx] = v.Position.x;
				p[3 * idx + 1] = v.Position.y;
				p[3 * idx + 2] = v.Position.z;
				++idx;
			}
		}
		auto Idx = [this](int i, int j) {
			return i * resolution[0] + j;
		};

		std::vector<std::pair<size_t, size_t> >Unique;
		const auto& vs = mesh.vertices;
		auto AddEdge = [&Unique, this, &vs](int i, int j) {
			for (auto u : Unique) {
				if (u.first == i && u.second == j)return;
				if (u.first == j && u.second == i)return;
			}
			Unique.push_back({ i,j });
			constraint.push_back(std::make_shared<Spring>(std::array<int, 2>({(int) i,(int)j }), p,k));
		};
		//Fill Spring
		for (int i = 0; i < resolution[1] - 1; ++i) {
			for (int j = 0; j < resolution[0] - 1; ++j) {
				AddEdge(Idx(i, j), Idx(i + 1, j));
				AddEdge(Idx(i, j), Idx(i, j + 1));
				AddEdge(Idx(i, j), Idx(i + 1, j + 1));
				AddEdge(Idx(i, j + 1), Idx(i + 1, j + 1));
				AddEdge(Idx(i, j + 1), Idx(i + 1, j));
				AddEdge(Idx(i + 1, j), Idx(i + 1, j + 1));
			}
		}
		
		max_edgeLength = 0;
		for (int i = 0; i < constraint.size(); ++i) {
			max_edgeLength= constraint[i]->Length() > max_edgeLength?constraint[i]->Length():max_edgeLength;
		}
		//max_edgeLength *= sqrt(2);
		for (int i = 0; i < resolution[1] - 2; ++i) {
			for (int j = 0; j < resolution[0] - 2; ++j) {
						AddEdge(Idx(i, j), Idx(i + 2, j));
						AddEdge(Idx(i, j), Idx(i, j+2));
			}
		}
		constraint.size();

		//Fill Attachment
		constraint.push_back((std::make_shared< StaticPoint>(p.block<3, 1>(0, 0), 0,k)));
		constraint.push_back((std::make_shared< StaticPoint>(p.block<3, 1>(Idx(0, resolution[1]-6) * 3, 0), Idx(0, resolution[1] - 1),k)));
		d = Eigen::VectorXd(3 * constraint.size());
	}
	void MSpring::BuildMatrix() {

		int pointNum = p.size() / 3;
		force = Eigen::VectorXd(p.size());
		L = Eigen::SparseMatrix<double>(p.size(), p.size());

		std::vector<T> tripletList;
		for (int i = 0; i < constraint.size(); i++) {
			auto& s = constraint[i];
			s->Build_L(i, tripletList);
		}
		L.setFromTriplets(tripletList.begin(), tripletList.end());
		tripletList.clear();
		J = Eigen::SparseMatrix<double>(p.size(), 3 * constraint.size());
		for (int i = 0; i < constraint.size(); i++) {
			auto& s = constraint[i];
			s->Build_J(i, tripletList);
		}
		J.setFromTriplets(tripletList.begin(), tripletList.end());
		tripletList.clear();

		Eigen::SparseMatrix<double> M(p.size(), p.size());
		Eigen::SparseMatrix<double> Identity(p.size(), p.size());
		Identity.setIdentity();

		for (int i = 0; i < pointNum; i++) {
			//Push(x_id, x_id, mass);
			tripletList.push_back(T(3 * i, 3 * i, mass));
			tripletList.push_back(T(3 * i + 1, 3 * i + 1, mass));
			tripletList.push_back(T(3 * i + 2, 3 * i + 2, mass));
			//M.coeffRef(x_id, x_id) = mass;
		}
		M.setFromTriplets(tripletList.begin(), tripletList.end());
		M_h2L = M + h * h * L;

		Solver.analyzePattern(M_h2L);
		Solver.factorize(M_h2L);
		double Regularization = 0.00001;
		//bool success = true;
		while (Solver.info() != Eigen::Success)
		{
			Regularization *= 10;
			M_h2L = M_h2L + Regularization * Identity;
			Solver.factorize(M_h2L);
			//success = false;
		}


	}
	void MSpring::Update(float dt) {
		auto noise = tEngine::SimplexNoise2D(glm::vec2(time,time+0.2)*0.02f);
		time += dt;
		constant = vk::su::clamp(noise,-.1f,.1f);
		fluid = constant * Eigen::Vector3d(0.1,0.1,0.3);
	//	fluid = fluid.normalized();
	//	fluid = Eigen::Vector3d(tEngine::PerlinNoise3D());
		//std::cout << x << std::endl;
		int substeps = dt / h;
		substeps = std::min(int(1.0 / 30 / h), substeps);
		substeps = std::max(1, substeps);
		Eigen::VectorXd penetration(p.size());
		for (int i = 0; i < substeps; i++) {
			Setb();
			x = y;
			for (int i = 0; i < 100; i++) {
				auto LastX = x;
				for (int i = 0; i < constraint.size(); i++) {

					d.block<3, 1>(3 * i, 0) = constraint[i]->SolveD(x);

				}
				Eigen::VectorXd rightHand = h * h * (J * d + force) + y * mass;
				x = Solver.solve(rightHand);

				double relativeError = (x - LastX).squaredNorm() / (x - p).squaredNorm();
				//std::cout << "error " << relativeError << std::endl;
				if (relativeError < 1e-4) {
					break;
				}

			}
			v = (x - p) / h;
		//	IntersectSelf();//Update v
			x = p + h * v;
			//Collide
			penetration.setZero();
			Eigen::Vector3d center(0, 0.4, -1.4);
			double radius = 0.2;
			for (int i = 0; i < p.size() / 3; i++) {
				auto position = x.block<3, 1>(3 * i, 0);
				auto diff = position - center;
				auto dist = diff.norm() - radius - 0.01;
				if (dist<0) {
					
					penetration.block<3, 1>(3 * i, 0) =  diff.normalized()*dist;// +(polygon.t + 0.01) * path.normalized();
				}
			
			}

			x -= penetration;
			p = x;


			UpdateMesh();
		}
	}
	
	
	void MSpring::Setb() {
		auto& mesh = this->mesh.getMesh();
		//inertia y
		y = p + h * v;
		//force
	//	auto& mesh = gb->meshFilter->mesh;
		force.setZero();
		auto toEigen = [](glm::vec3 n) {
			return Eigen::Vector3d(n[0], n[1], n[2]);
		};
		for (int i = 0; i < p.size() / 3; i++) {
			force[3 * i + 1] = -9.8 * mass;
			force.block<3, 1>(3 * i, 0) += -v.block<3, 1>(3 * i, 0) * m_damping_coefficient*mass;
			Eigen::Vector3d velocity = v.block<3, 1>(3 * i, 0);
			Eigen::Vector3d normal = toEigen(mesh.vertices[i].Normal);
			
			force.block<3, 1>(3 * i, 0) += viscousCof*normal.dot(fluid - velocity)* normal;
		}
		//force[1] = 0;

	}
	void MSpring::UpdateMesh() {
		auto& mesh = this->mesh.getMesh();
	//	auto& mesh = gb->meshFilter->mesh;
		for (int i = 0; i < mesh.vertices.size(); i++) {
			mesh.vertices[i].Position = glm::vec3(p[3 * i], p[3 * i + 1], p[3 * i + 2]);
		}
		mesh.UpdateNormal();
		
	}
	void MSpring::DampVelocity() {
		//double m_damping_coefficient = 1e-2;
		auto& mesh = this->mesh.getMesh();

		// post-processing damping
		Eigen::Vector3d pos_mc(0.0, 0.0, 0.0), vel_mc(0.0, 0.0, 0.0);
		unsigned int i, size;
		double denominator(0.0);// , mass(0.0);
		//auto& mesh = gb->meshFilter->mesh;
		size =mesh.vertices.size();
		for (i = 0; i < size; ++i)
		{
			pos_mc += mass * p.block<3,1>(i*3,0);
			vel_mc += mass * v.block<3, 1>(i * 3, 0);
			denominator += mass;
		}
	//	std::cout << mass << std::endl;
		assert(denominator != 0.0);
		pos_mc /= denominator;
		vel_mc /= denominator;

		Eigen::Vector3d  angular_momentum(0.0, 0.0, 0.0), r(0.0, 0.0, 0.0);
		Eigen::Matrix3d  inertia, r_mat;
		inertia.setZero(); r_mat.setZero();

		for (i = 0; i < size; ++i)
		{
			//mass = m_mesh->m_mass_matrix.coeff(x_id * 3, x_id * 3);

			r = p.block<3, 1>(i * 3, 0) - pos_mc;
			angular_momentum += r.cross(mass * v.block<3, 1>(i * 3, 0));

			//r_mat = EigenMatrix3(0.0,  r.z, -r.y,
			//                    -r.z, 0.0,  r.x,
			//                    r.y, -r.x, 0.0);

			r_mat.coeffRef(0, 1) = r[2];
			r_mat.coeffRef(0, 2) = -r[1];
			r_mat.coeffRef(1, 0) = -r[2];
			r_mat.coeffRef(1, 2) = r[0];
			r_mat.coeffRef(2, 0) = r[1];
			r_mat.coeffRef(2, 1) = -r[0];

			inertia += r_mat * r_mat.transpose() * mass;
		}
		Eigen::Vector3d angular_vel = inertia.inverse()*( angular_momentum);
		
		Eigen::Vector3d delta_v(0.0, 0.0, 0.0);
		for (i = 0; i < size; ++i)
		{
			r = p.block<3, 1>(i * 3, 0) - pos_mc;
		
			delta_v = vel_mc + angular_vel.cross(r) - v.block<3, 1>(i * 3, 0);
			v.block<3, 1>(i * 3, 0) += m_damping_coefficient * delta_v;
			//std::cout << ver.block<3,1>(x_id*3,0) << std::endl;
		}
	}
	MSpring::~MSpring() {
		
	}
}