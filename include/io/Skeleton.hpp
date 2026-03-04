#pragma once
#include <cstddef>
#include <vector>
#include <memory>
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"

namespace MDL { class Animation; }

namespace Rig {

struct BoneAnimationState {
    glm::vec3 mPosition { 0.0f }, mScale { 1.0f };
    glm::quat mRotation { glm::identity<glm::quat>() };
};

class Bone : public std::enable_shared_from_this<Bone> {
public:
    glm::mat4 mTransform, mInverse;

    glm::vec3 mPosition { 0.0f }, mScale { 1.0f };
    glm::quat mRotation { glm::identity<glm::quat>() };

    std::shared_ptr<Bone> mParent;
    std::vector<std::shared_ptr<Bone>> mChildren;

    int ParentIndex;

    BoneAnimationState mAnimationState;


    glm::mat4 GetTransform() {
        glm::mat4 transform(1.0f);
        transform = glm::scale(transform, mScale);
        transform *= glm::toMat4(mRotation);
        transform = glm::translate(transform, mPosition);
        return transform;
    }

    void EulerRotation(glm::vec3 v) { mRotation = glm::toQuat(glm::orientate4(v)); }
    glm::vec3 EulerRotation() { return glm::eulerAngles(mRotation); }

    Bone();
    ~Bone();
};

class Skeleton {
    std::vector<glm::mat4> mPose;
    bool Updated { false };

    glm::mat4 GetWorldMatrix(std::shared_ptr<Bone> bone);
    void ConvertWorldToLocalSpace(std::shared_ptr<Bone> bone);
public:
    std::vector<std::shared_ptr<Bone>> mBones;

    void Reset();
    void Update();

    void ConvertWorldToLocalSpace();

    glm::mat4 GetBoneTransform(std::shared_ptr<Bone> bone);
    glm::mat4 GetBoneTransform(int index) { return GetBoneTransform(mBones[index]); }

    Skeleton();
    Skeleton(std::size_t boneCount);
    ~Skeleton();
};

}
