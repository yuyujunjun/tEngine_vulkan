#include<list>
#include"tCCD.h"
#include"collide.h"
#include"support.h"
#include"Log.h"
#include"ecs.h"
/// <summary>
/// Some Basic function comes from libccd
/// </summary>
void __tccdSupport(const void* _obj1, const void* _obj2, const ccd_t& ccd,const ccd_vec3_t* dir, ccd_support_t* supp) {
    using namespace tEngine;


   auto sup1=obj1->SupportPoint(tEngine::Vector3(dir->v[0], dir->v[1], dir->v[2]));
   supp->v1.v[0] = sup1[0]; supp->v1.v[1] = sup1[1]; supp->v1.v[2] = sup1[2];
   auto sup2=obj2->SupportPoint(tEngine::Vector3(-dir->v[0], -dir->v[1], -dir->v[2]));
   supp->v2.v[0] = sup2[0]; supp->v2.v[1] = sup2[1]; supp->v2.v[2] = sup2[2];
   ccdVec3Sub2(&supp->v, &supp->v1, &supp->v2);
}
namespace tEngine {

    int GJKPenetration(const Collider* obj1, const Collider* obj2, ContactInfo* info){
        ccd_t ccd;
        CCD_INIT(&ccd); // initialize ccd_t struct
        ccd_real_t depth;
        ccd_vec3_t dir;
        ccd_vec3_t pos;
        int ret = ccdGJKPenetration(obj1, obj2, &ccd, &depth, &dir, &pos);
        if (ret == 0) {
            info->depth = depth;
            info->dir = Vector3(dir.v[0],dir.v[1],dir.v[2]);
            info->pos = Vector3(pos.v[0], pos.v[1], pos.v[2]);
        }
        else if (ret == -2) {
            tEngine::LOGD(tEngine::LogLevel::Debug, "CCD memory allocation failed");
        }
        return ret;
    }
    bool GJKIntersect(const Collider* obj1, const Collider* obj2) {
        ccd_t ccd;
        CCD_INIT(&ccd); // initialize ccd_t struct
        return ccdGJKIntersect(obj1,obj2,&ccd);
      
    }
}