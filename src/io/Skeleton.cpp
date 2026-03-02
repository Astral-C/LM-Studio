#include "io/Skeleton.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "io/MdlIO.hpp"
#include <memory>

namespace Rig {

Bone::Bone(){}

Bone::Bone(glm::mat4 matrix){
    mTransform = matrix;
}

glm::mat4 Skeleton::GetWorld(std::size_t index){
    if(mBones[index]->GetParentIndex() != -1){
        return GetWorld(mBones[index]->GetParentIndex()) * mBones[index]->Local();
    } else {
        return mBones[index]->Local();
    }
}

glm::mat4 Skeleton::GetWorldAnimated(std::size_t index, MDL::Animation* anim){
    if(mBones[index]->GetParentIndex() != -1){
        return GetWorldAnimated(mBones[index]->GetParentIndex(), anim) * mBones[index]->Local() * anim->GetJoint(index);
    } else {
        return mBones[index]->Local() * anim->GetJoint(index);
    }
}

void Skeleton::ToLocal(std::shared_ptr<Bone> bone){
    if(bone->GetParent() != nullptr){
        glm::mat4 parentTransform = glm::inverse(bone->GetParent()->Transform());

        bone->Local(parentTransform * bone->Transform());
    }
}

Bone::~Bone(){}

Skeleton::Skeleton(){}

Skeleton::Skeleton(std::size_t boneCount){
    mBones.reserve(boneCount);
}

Skeleton::~Skeleton(){}

void Skeleton::AddBone(glm::mat4 matrix){
    mBones.push_back(std::make_shared<Bone>(matrix));
}

void Skeleton::ToLocal(){
    for(auto bone : mBones){
        ToLocal(bone);
    }
}

std::vector<glm::mat4> Skeleton::GetPose(MDL::Animation* anim){
    std::vector<glm::mat4> bones;
    bones.reserve(mBones.size());

    if(anim == nullptr){
        for(int i = 0; i < mBones.size(); i++){
            bones.push_back(GetWorld(i));
        }
    } else {
        for(int i = 0; i < mBones.size(); i++){
            bones.push_back(GetWorldAnimated(i, anim));
        }
    }

    return bones;
}

}
