#pragma once
#include "pch.h"
struct BVHNode
{
    BVHNode* left = NULL;
    BVHNode* right = NULL;
    int n, index;          
    glm::vec3 AA, BB;    
};

struct BoundingBox {
    glm::vec3 AA, BB;
    glm::vec3 center;
    int index;
};

BVHNode* constructBVH(std::vector<BoundingBox>& objects, int l, int r, int n);