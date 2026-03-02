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
    glm::mat4 mTransform { 1.0f }, mLocal { 1.0f };

public:
    void Transform(glm::mat4 matrix) { mTransform = matrix; }
    glm::mat4 Transform() { return mTransform; }

    void Local(glm::mat4 matrix) { mLocal = matrix; }
    glm::mat4 Local() { return mLocal; }

    Bone();
    Bone(glm::mat4 matrix);

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
    std::vector<std::shared_ptr<Bone>> mBones;
public:
    void AddBone(glm::mat4 matrix);

    void ToLocal();
    void ToLocal(std::shared_ptr<Bone> bone);
    void RestPose(std::vector<glm::mat4>& poseOut);
    std::vector<glm::mat4> GetPose(MDL::Animation* anim);

    glm::mat4 GetWorld(std::size_t bone);
    glm::mat4 GetWorldAnimated(std::size_t bone, MDL::Animation* anim);

    std::shared_ptr<Bone> GetBone(int index) { return mBones[index]; }

    Skeleton();
    Skeleton(std::size_t boneCount);
    ~Skeleton();
};

}
