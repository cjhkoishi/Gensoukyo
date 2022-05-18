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

    // ���� AABB
    for (int i = l; i <= r; i++) {
        // ��С�� AA
        node->AA.x = min(node->AA.x, objects[i].AA.x);
        node->AA.y = min(node->AA.y, objects[i].AA.y);
        node->AA.z = min(node->AA.z, objects[i].AA.z);
        // ���� BB
        node->BB.x = max(node->BB.x, objects[i].BB.x);
        node->BB.y = max(node->BB.y, objects[i].BB.y);
        node->BB.z = max(node->BB.z, objects[i].BB.z);
    }

    // ������ n �������� ����Ҷ�ӽڵ�
    if ((r - l + 1) <= n) {
        node->n = r - l + 1;
        node->index = l;
        return node;
    }

    // ����ݹ齨��
    float lenx = node->BB.x - node->AA.x;
    float leny = node->BB.y - node->AA.y;
    float lenz = node->BB.z - node->AA.z;
    // �� x ����
    if (lenx >= leny && lenx >= lenz)
        sort(objects.begin() + l, objects.begin() + r + 1, cmpx);
    // �� y ����
    if (leny >= lenx && leny >= lenz)
        sort(objects.begin() + l, objects.begin() + r + 1, cmpy);
    // �� z ����
    if (lenz >= lenx && lenz >= leny)
        sort(objects.begin() + l, objects.begin() + r + 1, cmpz);
 
    // �ݹ�
    int mid = (l + r) / 2;
    node->left = constructBVH(objects, l, mid, n);
    node->right = constructBVH(objects, mid + 1, r, n);

    return node;
}
