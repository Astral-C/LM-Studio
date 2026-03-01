#pragma once
#include "glm/glm.hpp"

class Bone {
    glm::mat4 mTransform;

public:
    glm::mat4 GetWorld();

};

class Skeleton {
    std::vector<Bone> mBones;

public:
    void Update();

};
