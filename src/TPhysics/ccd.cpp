#include"ccd.h"
#include"collide.h"
/// <summary>
/// Some Basic function comes from libccd
/// </summary>
namespace tPhysics {
     /// <summary>
     /// return distance2 from P to x0-x1
     /// </summary>
     /// <param name="P"></param>
     /// <param name="x0"></param>
     /// <param name="x1"></param>
     /// <param name="witness"></param>
     /// <returns></returns>
     real pointSegmentDist2(const Vector3& P,const Vector3& x0, const Vector3& x1, Vector3* witness){
        // The computation comes from solving equation of segment:
        //      S(t) = x0 + t.d
        //          where - x0 is initial point of segment
        //                - d is direction of segment from x0 (|d| > 0)
        //                - t belongs to <0, 1> interval
        // 
        // Than, distance from a segment to some point P can be expressed:
        //      D(t) = |x0 + t.d - P|^2
        //          which is distance from any point on segment. Minimization
        //          of this function brings distance from P to segment.
        // Minimization of D(t) leads to simple quadratic equation that's
        // solving is straightforward.
        //
        // Bonus of this method is witness point for free.
        Vector3 tmp;
        real dist, t;

        // direction of segment
        Vector3 d = x1 - x0;
       
        // precompute vector from P to x0
        Vector3 a = x0 - P;

        t = -glm::dot(a, d);
        t /= glm::dot(d,d);

        if (t < 0.f || isZero(t)) {
            tmp = x0 - P;
            dist = glm::dot(tmp,tmp);
            if (witness)
                *witness = x0;
        }
        else if (t > 1 || ccdEqual(t, 1.f)) {
            tmp = x1 - P;
            dist = glm::dot(tmp, tmp);
            if (witness)
                *witness=x1;
        }
        else {
            if (witness) {
                *witness = d;
                *witness *= t;
                *witness += x0;
                tmp = *witness - P;
                dist = glm::dot(tmp,tmp);
            }
            else {
                // recycling variables
                d *= t;
                d += a;
                dist = glm::dot(d, d);
            }
        }

        return dist;
    }

 
    /// <summary>
    /// Return distance2 from P to ABC
    /// </summary>
    /// <param name="P"></param>
    /// <param name="x0"></param>
    /// <param name="B"></param>
    /// <param name="C"></param>
    /// <param name="witness"></param>
    /// <returns></returns>
    real PointTriDist2(const Vector3& P, const Vector3& A, const Vector3& B,const Vector3& C, Vector3* witness) {
        // Computation comes from analytic expression for triangle (x0, B, C)
        //      T(s, t) = x0 + s.d1 + t.d2, where d1 = B - x0 and d2 = C - x0 and
        // Then equation for distance is:
        //      D(s, t) = | T(s, t) - P |^2
        // This leads to minimization of quadratic function of two variables.
        // The solution from is taken only if s is between 0 and 1, t is
        // between 0 and 1 and t + s < 1, otherwise distance from segment is
        // computed.

        Vector3 d1, d2, a;
        real u, v, w, p, q, r, d;
        real s, t, dist, dist2;
        Vector3 witness2;
       // Vector3& witness = *witness_;
        d1 = B - A;
        d2 = C - A;
        a = A - P;

        u = glm::dot(a, a);
        v = glm::dot(d1, d1);
        w = glm::dot(d2, d2);
        p = glm::dot(a, d1);
        q = glm::dot(a, d2);
        r = glm::dot(d1, d2);

        d = w * v - r * r;
        if (isZero(d)) {
            // To avoid division by zero for zero (or near zero) area triangles
            s = t = -1.;
        }
        else {
            s = (q * r - w * p) / d;
            t = (-s * r - q) / w;
        }

        if ((isZero(s) || s > (real)0)
            && (ccdEqual(s, (real)1) || s < (real)1)
            && (isZero(t) || t > (real)0)
            && (ccdEqual(t, (real)1) || t < (real)1)
            && (ccdEqual(t + s, (real)1) || t + s < (real)1)) {

            if (witness) {
                d1 *= s;
                d2 *= t;
                *witness= A;
                *witness += d1;
                *witness+=d2;
                Vector3 tmp = *witness - P;
                dist = glm::dot(tmp, tmp);
            }
            else {
                dist = s * s * v;
                dist += t * t * w;
                dist += real(2.) * s * t * r;
                dist += real(2.) * s * p;
                dist += real(2.) * t * q;
                dist += u;
            }
        }
        else {
            dist = pointSegmentDist2(P, A, B, witness);

            dist2 = pointSegmentDist2(P, A, C, &witness2);
            if (dist2 < dist) {
                dist = dist2;
                if (witness)
                    *witness = witness2;
            }

            dist2 = pointSegmentDist2(P, B, C, &witness2);
            if (dist2 < dist) {
                dist = dist2;
                if (witness)
                    *witness = witness2;
            }
        }

        return dist;
    }


	void ccdSupport(const Collider* obj1, const Collider* obj2, const Vector3& dir, Support* sup) {
		sup->v1 = obj1->SupportPoint(dir);
		sup->v2 = obj2->SupportPoint(-dir);
		sup->v = sup->v1 - sup->v2;
	}
    int doSimplex2(Simplex* simplex, Vector3* dir)
    {
        const Support* A, * B;
        Vector3 AB, AO, tmp;
        real dot;

        // get last added as A
        A = &simplex->pt[simplex->last_id];//last_id=1 for sure
        // get the other point
        B = &simplex->pt[0];
        // compute AB oriented segment
        AB = B->v - A->v;
        // compute AO vector
        AO = -A->v;
        // dot product AB . AO
        dot = glm::dot(AB, AO);

        // check if origin doesn't lie on AB segment
        tmp=glm::cross(AB, AO);
   
        if (glm::dot(tmp,tmp)<CCD_EPS && dot > 0.) {//origin point on the line AB
            return 1;
        }

        // check if origin is in area where AB segment is
        if (isZero(dot) || dot < 0.) { //dot < 0 means we havn't reach the origin
            simplex->pt[0] = *A;
            simplex->last_id = 0;
            *dir = AO;
      
        }
        else {
            // origin is in area where AB segment is

            // keep simplex untouched and set direction to
            // AB x AO x AB
            *dir = glm::cross(glm::cross(AB, AO),AB);
     
        }

        return 0;
    }

    int doSimplex3(Simplex* simplex, Vector3* dir)
    {
        const Support* A, * B, * C;
        Vector3 AO, AB, AC, ABC, tmp;
        real dot, dist;

        // get last added as A
        A = &simplex->pt[simplex->last_id];
        // get the other points
        B = &simplex->pt[1];
        C = &simplex->pt[0];

        // check touching contact
        dist = PointTriDist2(Vector3(0,0,0), A->v, B->v, C->v, NULL);
        if (isZero(dist)) {
            return 1;
        }
       
        // check if triangle is really triangle (has area > 0)
        // if not simplex can't be expanded and thus no itersection is found
        if (ccdEqual(A->v, B->v) || ccdEqual(A->v, C->v)) {
            return -1;
        }

        // compute AO vector
        AO = -A->v;

        // compute AB and AC segments and ABC vector (perpendircular to triangle)
        AB = B->v - A->v;
        AC = C -> v - A->v;
        ABC = glm::cross(AB, AC);
        tmp = glm::cross(ABC, AC);
        dot = glm::dot(tmp, AO);
        if (isZero(dot) || dot > (real)0) {
            dot = glm::dot(AC, AO);
            if (isZero(dot) || dot > (real)0) {
                // C is already in place
                simplex->pt[1] = *A;
                simplex->last_id = 1;
                *dir = glm::cross(glm::cross(AC, AO), AC);
            }
            else {
            ccd_do_simplex3_45:
                dot = glm::dot(AB, AO);
                if (isZero(dot) || dot > (real)0) {
                    simplex->pt[0] = *B;
                    simplex->pt[1] = *A;
                    simplex->last_id = 1;
                    *dir = glm::cross(glm::cross(AB, AO), AB);
                }
                else {
                    simplex->pt[0] = *A;
                    simplex->last_id = 0;
                    *dir = AO;
                }
            }
        }
        else {
            tmp =glm::cross( AB, ABC);
            dot = glm::dot(tmp, AO);
            if (isZero(dot) || dot > (real)0) {
                goto ccd_do_simplex3_45;
            }
            else {
                dot = glm::dot(ABC, AO);
                if (isZero(dot) || dot > (real)0) {
                    *dir = ABC;
                }
                else {
                    Support Ctmp=*C;
                    simplex->pt[0] = *B;
                    simplex->pt[1] = Ctmp;
                    *dir = -ABC;
                }
            }
        }

        return 0;
    }

    int doSimplex4(Simplex* simplex, Vector3* dir)
    {
        const Support* A, * B, * C, * D;
        Vector3 AO, AB, AC, AD, ABC, ACD, ADB;
        int B_on_ACD, C_on_ADB, D_on_ABC;
        int AB_O, AC_O, AD_O;
        real dist;

        // get last added as A
        A = &simplex->pt[simplex->last_id];
        // get the other points
        B = &simplex->pt[2];
        C = &simplex->pt[1];
        D = &simplex->pt[0];

        // check if tetrahedron is really tetrahedron (has volume > 0)
        // if it is not simplex can't be expanded and thus no intersection is
        // found
        dist = PointTriDist2(A->v, B->v, C->v, D->v, NULL);
        if (isZero(dist)) {
            return -1;
        }
        constexpr Vector3 origin(0, 0, 0);
        // check if origin lies on some of tetrahedron's face - if so objects
        // intersect
        dist = PointTriDist2(origin, A->v,B->v, C->v, NULL);
        if (isZero(dist))
            return 1;
        dist = PointTriDist2(origin, A->v, C->v, D->v, NULL);
        if (isZero(dist))
            return 1;
        dist = PointTriDist2(origin, A->v, B->v, D->v, NULL);
        if (isZero(dist))
            return 1;
        dist = PointTriDist2(origin, B->v, C->v, D->v, NULL);
        if (isZero(dist))
            return 1;

        // compute AO, AB, AC, AD segments and ABC, ACD, ADB normal vectors
        AO = -A->v;
        AB = B->v - A->v;
        AC = C->v - A->v;
        AD = D->v - A->v;
        ABC = AB - AC;
        ACD = AC - AD;
        ADB = AD - AB;


        // side (positive or negative) of B, C, D relative to planes ACD, ADB
        // and ABC respectively
        B_on_ACD = sign(glm::dot(ACD, AB));
        C_on_ADB = sign(glm::dot(ADB, AC));
        D_on_ABC = sign(glm::dot(ABC, AD));

        // whether origin is on same side of ACD, ADB, ABC as B, C, D
        // respectively
        AB_O = sign(glm::dot(ACD, AO)) == B_on_ACD;
        AC_O = sign(glm::dot(ADB, AO)) == C_on_ADB;
        AD_O = sign(glm::dot(ABC, AO)) == D_on_ABC;

        if (AB_O && AC_O && AD_O) {
            // origin is in tetrahedron
            return 1;

            // rearrange simplex to triangle and call doSimplex3()
        }
        else if (!AB_O) {
            // B is farthest from the origin among all of the tetrahedron's
            // points, so remove it from the list and go on with the triangle
            // case

            // D and C are in place
            simplex->pt[2] = *A;
            simplex->last_id = 2;
        }
        else if (!AC_O) {
            // C is farthest
            simplex->pt[1] = *D;
            simplex->pt[0] = *B;
            simplex->pt[2] = *A;
            simplex->last_id = 2;

        }
        else { // (!AD_O)
            simplex->pt[0] = *C;
            simplex->pt[1] = *B;
            simplex->pt[2] = *A;
            simplex->last_id = 2;
        }

        return doSimplex3(simplex, dir);
    }

    int doSimplex(Simplex* simplex, Vector3* dir) {
        if (simplex->last_id == 1)return doSimplex2(simplex, dir);
        else if (simplex->last_id == 2)return doSimplex3(simplex, dir);
        else return doSimplex4(simplex, dir);
    }

	int GJK(const Collider* obj1, const Collider* obj2, unsigned maxIterations) {
		Simplex simplex;
		Support last_support;
        
		Vector3 dir = (obj1->worldCenter - obj2->worldCenter);
        dir = isZero(dir) ? Vector3(1, 0, 0) : glm::normalize(dir);

		ccdSupport(obj1, obj2, dir, &last_support);
		simplex.last_id=0;
		simplex.pt[simplex.last_id] = last_support;
       
		//find direction from support direction to origin
		dir = -last_support.v;
       
		for (unsigned iteration = 0; iteration < maxIterations; ++iteration) {
            dir = glm::normalize(dir);
			ccdSupport(obj1, obj2, dir, &last_support);
            
			if (glm::dot(last_support.v, dir) < 0) {
				return -1;//no interection
			}
            simplex.last_id++;
            simplex.pt[simplex.last_id] = last_support;
            int do_simplex_res = doSimplex(&simplex, &dir);
            if (do_simplex_res == 1)return 0;//intersection found
            else if (do_simplex_res == -1)return -1;//intersection not found

            if (isZero(glm::dot(dir, dir))) {
                return -1;//intersection not found
            }
            
		}
        return -1;
	}
}