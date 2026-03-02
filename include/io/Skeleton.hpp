#pragma once
#include <cstddef>
#include <vector>
#include <memory>
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace MDL { class Animation; }

namespace Rig {

class Bone {
    int16_t mParentIndex { -1 };
    std::shared_ptr<Bone> mParent { nullptr };
    glm::mat4 mModel { 1.0f }, mInverse { 1.0f }, mLocal { 1.0f }, mAnimationFrame { 1.0f };

public:
    void Model(glm::mat4 matrix) { mModel = matrix; }
    glm::mat4 Model() { return mModel; }

    void Inverse(glm::mat4 matrix) { mInverse = matrix; }
    glm::mat4 Inverse() { return mInverse; }

    glm::mat4 Local(){ return mLocal; }
    void Local(glm::mat4 matrix){ mLocal = matrix; }

    void Frame(glm::mat4 matrix) { mAnimationFrame = matrix; }
    glm::mat4 Frame() { return mAnimationFrame; }

    glm::mat4 Transform() { return mParentIndex != -1 ? mParent->Transform() * mLocal * mAnimationFrame : mLocal; }

    Bone();
    Bone(glm::mat4 matrix, glm::mat4 inverse);

    void SetParent(std::shared_ptr<Bone> bone){
        mParent = bone;
    }

    void SetParentIndex(int16_t index){
        mParentIndex = index;
    }

    std::shared_ptr<Bone> GetParent() {
        return mParent;
    }

    int16_t GetParentIndex(){
        return mParentIndex;
    }

    ~Bone();
};

class Skeleton {
    std::vector<glm::mat4> mPose;
    std::vector<std::shared_ptr<Bone>> mBones;
public:
    void AddBone(glm::mat4 matrix, glm::mat4 inverse);

    void WorldToLocal();
    void WorldToLocal(std::shared_ptr<Bone> bone);

    void RestPose();

    std::vector<glm::mat4>& GetPose(MDL::Animation* anim);

    glm::mat4 GetBoneTransform(int index);
    glm::mat4 GetBoneTransform(std::shared_ptr<Bone> bone);

    glm::mat4 GetWorld(std::size_t bone);

    std::shared_ptr<Bone> GetBone(int index) { return mBones[index]; }

    Skeleton();
    Skeleton(std::size_t boneCount);
    ~Skeleton();
};

}
