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

glm::mat4 Bone::Transform(){
    glm::mat4 transform(1.0f);
    std::shared_ptr<Bone> bone = shared_from_this();

    while(bone != nullptr){
        transform = bone->Local() * transform;
        bone = bone->mParent;
    }

    return transform;
}

Skeleton::Skeleton(){}

Skeleton::Skeleton(std::size_t boneCount){
    mBones.reserve(boneCount);
    mPose.resize(boneCount);
}

Skeleton::~Skeleton(){}

void Skeleton::AddBone(glm::mat4 matrix, glm::mat4 inverse){
    mBones.push_back(std::make_shared<Bone>(matrix, inverse));
}

void Skeleton::WorldToLocal(){
    for(std::size_t i = 0; i < mBones.size(); i++){
        if(mBones[i]->GetParentIndex() != -1){
            mBones[i]->Local(glm::inverse(mBones[i]->GetParent()->Matrix()) * mBones[i]->Matrix());
        } else {
            mBones[i]->Local(mBones[i]->Matrix());
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
        mPose[i] = (mBones[i]->Matrix() * mBones[i]->Frame()) * mBones[i]->Inverse();
    }

    return mPose;

}

}
