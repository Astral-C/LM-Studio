#include "io/Skeleton.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "io/MdlIO.hpp"
#include <memory>

namespace Rig {

Bone::Bone(){}

Bone::Bone(glm::mat4 matrix, glm::mat4 inverse){
    mModel = matrix;
    mInverse = inverse;
}

Bone::~Bone(){}

Skeleton::Skeleton(){}

Skeleton::Skeleton(std::size_t boneCount){
    mBones.reserve(boneCount);
    mPose.resize(boneCount);
}

Skeleton::~Skeleton(){}

void Skeleton::AddBone(glm::mat4 matrix, glm::mat4 inverse){
    mBones.push_back(std::make_shared<Bone>(matrix, inverse));
}

void Skeleton::RestPose(){
    for(std::size_t i = 0; i < mBones.size(); i++){
        if(mBones[i]->GetParentIndex() != -1){
            mBones[i]->Local(glm::inverse(mBones[i]->GetParent()->Model()) * mBones[i]->Model());
        } else {
            mBones[i]->Local(glm::mat4(1.0f));
        }
    }
}

std::vector<glm::mat4>& Skeleton::GetPose(MDL::Animation* anim){
    if(anim != nullptr){
        for(int i = 0; i < mBones.size(); i++){
            mBones[i]->Frame(anim->GetJoint(i));
        }
    }

    for(std::size_t i = 0; i < mBones.size(); i++){
        mPose[i] = mBones[i]->Inverse() * mBones[i]->Transform();
    }

    return mPose;

}

}
