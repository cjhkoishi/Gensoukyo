#include "pch.h"
#include "BVH.h"
using namespace std;

bool cmpx(const BoundingBox& obj1, const BoundingBox& obj2) {
    return obj1.center.x < obj2.center.x;
}
bool cmpy(const BoundingBox& obj1, const BoundingBox& obj2) {
    return obj1.center.y < obj2.center.y;
}
bool cmpz(const BoundingBox& obj1, const BoundingBox& obj2) {
    return obj1.center.z < obj2.center.z;
}

BVHNode* constructBVH(std::vector<BoundingBox> objects, int l, int r, int n)
{
    if (l > r) return 0;

    BVHNode* node = new BVHNode();
    node->AA = glm::vec3(1145141919, 1145141919, 1145141919);
    node->BB = glm::vec3(-1145141919, -1145141919, -1145141919);

    // 计算 AABB
    for (int i = l; i <= r; i++) {
        // 最小点 AA
        node->AA.x = min(node->AA.x, objects[i].AA.x);
        node->AA.y = min(node->AA.y, objects[i].AA.y);
        node->AA.z = min(node->AA.z, objects[i].AA.z);
        // 最大点 BB
        node->BB.x = max(node->BB.x, objects[i].BB.x);
        node->BB.y = max(node->BB.y, objects[i].BB.y);
        node->BB.z = max(node->BB.z, objects[i].BB.z);
    }

    // 不多于 n 个三角形 返回叶子节点
    if ((r - l + 1) <= n) {
        node->n = r - l + 1;
        node->index = l;
        return node;
    }

    // 否则递归建树
    //float lenx = node->BB.x - node->AA.x;
    //float leny = node->BB.y - node->AA.y;
    //float lenz = node->BB.z - node->AA.z;
    //// 按 x 划分
    //if (lenx >= leny && lenx >= lenz)
    //    sort(objects.begin() + l, objects.begin() + r + 1, cmpx);
    //// 按 y 划分
    //if (leny >= lenx && leny >= lenz)
    //    sort(objects.begin() + l, objects.begin() + r + 1, cmpy);
    //// 按 z 划分
    //if (lenz >= lenx && lenz >= leny)
    //    sort(objects.begin() + l, objects.begin() + r + 1, cmpz);
 
    //// 递归
    //int mid = (l + r) / 2;
    //node->left = constructBVH(objects, l, mid, n);
    //node->right = constructBVH(objects, mid + 1, r, n);
    float Cost = INFINITY;
    int Axis = 0;
    int Split = (l + r) / 2;
    for (int axis = 0; axis < 3; axis++) {
        // 分别按 x，y，z 轴排序
        if (axis == 0) std::sort(&objects[0] + l, &objects[0] + r + 1, cmpx);
        if (axis == 1) std::sort(&objects[0] + l, &objects[0] + r + 1, cmpy);
        if (axis == 2) std::sort(&objects[0] + l, &objects[0] + r + 1, cmpz);

        // leftMax[i]: [l, i] 中最大的 xyz 值
        // leftMin[i]: [l, i] 中最小的 xyz 值
        std::vector<glm::vec3> leftMax(r - l + 1, glm::vec3(-INFINITY, -INFINITY, -INFINITY));
        std::vector<glm::vec3> leftMin(r - l + 1, glm::vec3(INFINITY, INFINITY, INFINITY));
        // 计算前缀 注意 i-l 以对齐到下标 0
        for (int i = l; i <= r; i++) {
            auto& t = objects[i];
            int bias = (i == l) ? 0 : 1;  // 第一个元素特殊处理

            leftMax[i - l].x = max(leftMax[i - l - bias].x, t.BB.x);
            leftMax[i - l].y = max(leftMax[i - l - bias].y, t.BB.y);
            leftMax[i - l].z = max(leftMax[i - l - bias].z, t.BB.z);

            leftMin[i - l].x = min(leftMin[i - l - bias].x, t.AA.x);
            leftMin[i - l].y = min(leftMin[i - l - bias].y, t.AA.y);
            leftMin[i - l].z = min(leftMin[i - l - bias].z, t.AA.z);
        }

        // rightMax[i]: [i, r] 中最大的 xyz 值
        // rightMin[i]: [i, r] 中最小的 xyz 值
        std::vector<glm::vec3> rightMax(r - l + 1, glm::vec3(-INFINITY, -INFINITY, -INFINITY));
        std::vector<glm::vec3> rightMin(r - l + 1, glm::vec3(INFINITY, INFINITY, INFINITY));
        // 计算后缀 注意 i-l 以对齐到下标 0
        for (int i = r; i >= l; i--) {
            auto& t = objects[i];
            int bias = (i == r) ? 0 : 1;  // 第一个元素特殊处理

            rightMax[i - l].x = max(rightMax[i - l + bias].x, t.BB.x);
            rightMax[i - l].y = max(rightMax[i - l + bias].y, t.BB.y);
            rightMax[i - l].z = max(rightMax[i - l + bias].z, t.BB.z);

            rightMin[i - l].x = min(rightMin[i - l + bias].x, t.AA.x);
            rightMin[i - l].y = min(rightMin[i - l + bias].y, t.AA.y);
            rightMin[i - l].z = min(rightMin[i - l + bias].z, t.AA.z);
        }

        // 遍历寻找分割
        float cost = INFINITY;
        int split = l;
        for (int i = l; i <= r - 1; i++) {
            float lenx, leny, lenz;
            // 左侧 [l, i]
            glm::vec3 leftAA = leftMin[i - l];
            glm::vec3 leftBB = leftMax[i - l];
            lenx = leftBB.x - leftAA.x;
            leny = leftBB.y - leftAA.y;
            lenz = leftBB.z - leftAA.z;
            float leftS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float leftCost = leftS * (i - l + 1);

            // 右侧 [i+1, r]
            glm::vec3 rightAA = rightMin[i + 1 - l];
            glm::vec3 rightBB = rightMax[i + 1 - l];
            lenx = rightBB.x - rightAA.x;
            leny = rightBB.y - rightAA.y;
            lenz = rightBB.z - rightAA.z;
            float rightS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float rightCost = rightS * (r - i);

            // 记录每个分割的最小答案
            float totalCost = leftCost + rightCost;
            if (totalCost < cost) {
                cost = totalCost;
                split = i;
            }
        }
        // 记录每个轴的最佳答案
        if (cost < Cost) {
            Cost = cost;
            Axis = axis;
            Split = split;
        }
    }

    // 按最佳轴分割
    if (Axis == 0) std::sort(&objects[0] + l, &objects[0] + r + 1, cmpx);
    if (Axis == 1) std::sort(&objects[0] + l, &objects[0] + r + 1, cmpy);
    if (Axis == 2) std::sort(&objects[0] + l, &objects[0] + r + 1, cmpz);

    // 递归
    node->left = constructBVH(objects, l, Split, n);
    node->right = constructBVH(objects, Split + 1, r, n);

    return node;
}
