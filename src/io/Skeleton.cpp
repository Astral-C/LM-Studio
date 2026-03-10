#include "io/Skeleton.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/matrix_interpolation.hpp"
#include "glm/matrix.hpp"
#include "io/MdlIO.hpp"
#include "glm/gtx/string_cast.hpp"
#include <format>
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
        bone->mAnimation = glm::mat4(1.0f);
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
    if(bone->ParentIndex != -1){
        return GetWorldMatrix(bone->mParent) * bone->mLocal * bone->mAnimation;
    } else {
        return bone->mLocal * bone->mAnimation;
    }
}

void Skeleton::ConvertWorldToLocalSpace(){
    for(auto& bone : mBones){
        ConvertWorldToLocalSpace(bone);
    }
    for(int i = 0; i < mBones.size(); i++){
        std::cout << std::format("======Bone {}======\n{}\n", i, glm::to_string(mBones[i]->mLocal)) << std::endl;
    }
    Reset();
}

void Skeleton::ConvertWorldToLocalSpace(std::shared_ptr<Bone> bone){
    if(bone->ParentIndex != -1){
        bone->mLocal = glm::inverse(bone->mParent->mTransform) * bone->mTransform;
    } else {
        bone->mLocal = bone->mTransform;
    }
}

}
