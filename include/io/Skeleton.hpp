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

class Bone : public std::enable_shared_from_this<Bone> {
public:
    glm::mat4 mTransform, mInverse, mLocal, mAnimation;

    std::shared_ptr<Bone> mParent;
    std::vector<std::shared_ptr<Bone>> mChildren;

    int ParentIndex;

    Bone();
    ~Bone();
};

class Skeleton {
    std::vector<glm::mat4> mPose;
    bool Updated { false };

    void ConvertWorldToLocalSpace(std::shared_ptr<Bone> bone);
public:
    glm::mat4 GetWorldMatrix(std::shared_ptr<Bone> bone);
    std::vector<std::shared_ptr<Bone>> mBones;

    void Reset();
    void Update();

    void ConvertWorldToLocalSpace();

    Skeleton();
    Skeleton(std::size_t boneCount);
    ~Skeleton();
};

}
