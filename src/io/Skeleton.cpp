#include "io/Skeleton.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/matrix_interpolation.hpp"
#include "io/MdlIO.hpp"
#include <memory>

namespace Rig {

Bone::Bone(){}

Bone::~Bone(){}

Skeleton::Skeleton(){}

Skeleton::Skeleton(std::size_t boneCount){
    mBones.reserve(boneCount);
}

Skeleton::~Skeleton(){}

void Skeleton::Reset(){
    for(auto& bone : mBones){
        bone->mAnimationState = BoneAnimationState();
        bone->mAnimationState.mPosition = bone->mPosition;
        bone->mAnimationState.mRotation = bone->mRotation;
        bone->mAnimationState.mScale = bone->mScale;
    }

    Update();

    for(auto& bone : mBones){
        bone->mInverse = glm::inverse(bone->mTransform);
    }
}

void Skeleton::Update(){
    Updated = true;
    for(auto& bone : mBones){
        bone->mTransform = GetWorldMatrix(bone);
    }
}

glm::mat4 Skeleton::GetWorldMatrix(std::shared_ptr<Bone> bone){
    glm::mat4 transform(1.0f);
    transform = glm::scale(transform, bone->mAnimationState.mScale);
    transform *= glm::toMat4(bone->mAnimationState.mRotation);
    transform = glm::translate(transform, bone->mAnimationState.mPosition);

    if(bone->ParentIndex != -1){
        return transform * GetWorldMatrix(bone->mParent);
    } else {
        return transform;
    }
}

void Skeleton::ConvertWorldToLocalSpace(){
    for(auto& bone : mBones){
        ConvertWorldToLocalSpace(bone);
    }
    Reset();
}

void Skeleton::ConvertWorldToLocalSpace(std::shared_ptr<Bone> bone){
    if(bone->ParentIndex != -1){
        glm::mat4 mat = glm::inverse(GetBoneTransform(bone->mParent));
        bone->mPosition = glm::vec3(mat * glm::vec4(bone->mPosition, 1));
        bone->mRotation = glm::quat_cast(glm::extractMatrixRotation(mat)) * bone->mRotation;

    }
}


glm::mat4 Skeleton::GetBoneTransform(std::shared_ptr<Bone> bone){
    if(bone == nullptr){
        return glm::mat4(1.0f);
    }
    if(bone->ParentIndex == -1){
        return bone->GetTransform();
    } else {
        return bone->GetTransform() * GetBoneTransform(mBones[bone->ParentIndex]);
    }
}

}
