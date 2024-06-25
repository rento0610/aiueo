#include "Collision.h"

//���͐^�񒆂���_
//�~���͉�����_
bool Collision::IntersectSphereVsSphere(
    const DirectX::XMFLOAT3& positionA,
    float radiusA,
    const DirectX::XMFLOAT3& positionB,
    float radiusB,
    DirectX::XMFLOAT3& outPositionB)
{
    //A��B�̒P�ʃx�N�g�����Z�o
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB= DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq);

    //��������
    float range = radiusA + radiusB;
    if (range*range<lengthSq)
    {
        return false;
    }

    //A��B�������o��
    Vec = DirectX::XMVector3Normalize(Vec);
    Vec = DirectX::XMVectorScale(Vec, range);
    PositionB = DirectX::XMVectorAdd(PositionA, Vec);
    DirectX::XMStoreFloat3(&outPositionB, PositionB);

    return true;
}

bool Collision::IntersectCylinderVsCylinder(
    const DirectX::XMFLOAT3& positionA,
    float radiusA,
    float heightA,
    const DirectX::XMFLOAT3& positionB,
    float radiusB,
    float heightB,
    DirectX::XMFLOAT3& outPositionB
)
{
    //XZ���ʂł͈̔̓`�F�b�N
    if (positionA.y > positionB.y + heightB)
    {
        return false;
    }
    if (positionA.y + heightA < positionB.y)
    {
        return false;
    }
    //X���m������
    float vx = positionB.x - positionA.x;
    //Z���m������
    float vz = positionB.z - positionA.z;
    //XZ�̒������v�Z����
    float distXZ = sqrtf(vx * vx + vz * vz);
    //���aA�Ɣ��aB�̒������v�Z����
    float range = radiusA + radiusB;
    if (distXZ>range)
    {
        return false;
    }
    vx /= distXZ;
    vz /= distXZ;
    outPositionB.x = positionA.x + vx * range;
    outPositionB.y = positionB.y;
    outPositionB.z = positionA.z + vz * range;

    return true;
}

bool Collision::IntersectSphereVsCylinder(
    const DirectX::XMFLOAT3& spherePosition,
    float sphereRadius,
    const DirectX::XMFLOAT3& cylinderPosition,
    float cylinderRadius,
    float cylinderHeight,
    DirectX::XMFLOAT3& outCylinderPosition
)
{
    //���̑������~���̓�����Ȃ瓖�����Ă��Ȃ�
    if (spherePosition.y - sphereRadius > cylinderPosition.y + cylinderHeight) return false;
    // ���̓����~���̑�����艺�Ȃ瓖�����Ă��Ȃ�
    if (spherePosition.y + sphereRadius < cylinderPosition.y) return false;
    //XZ���ʂł͈̔̓`�F�b�N
    if (spherePosition.y > spherePosition.y + cylinderHeight)
    {
        return false;
    }
    if (spherePosition.y + sphereRadius < cylinderPosition.y)
    {
        return false;
    }
    //X���m������
    float vx = cylinderPosition.x - spherePosition.x;
    //Z���m������
    float vz = cylinderPosition.z - spherePosition.z;
    //XZ�̒������v�Z����
    float distXZ = sqrtf(vx * vx + vz * vz);
    //���aA�Ɣ��aB�̒������v�Z����
    float range = sphereRadius + cylinderRadius;
    //XZ�̒��������aA�Ɣ��aB�̒������傫�������瓖�����Ă��Ȃ�
    if (distXZ > range)
    {
        return false;
    }
    vx /= distXZ;
    vz /= distXZ;
    //A��B�������o��
    //vx��vz�𐳋K��
    outCylinderPosition.x = spherePosition.x + vx * range;
    outCylinderPosition.y = cylinderPosition.y;
    outCylinderPosition.z = spherePosition.z + vz * range;

    return true;
}

bool Collision::IntersectRayVsModel(
    const DirectX::XMFLOAT3& start,
    const DirectX::XMFLOAT3& end,
    const Model* model,
    HitResult& result
)
{
   /* if (end.y < 0.0f)
    {
        result.position.x = end.x;
        result.position.y = 0.0f;
        result.position.z = end.z;
        result.normal.x = 0.0f;
        result.normal.y = 1.0f;
        result.normal.z = 0.0f;
        return true;
    }
    return false;*/
    DirectX::XMVECTOR WorldStart = DirectX::XMLoadFloat3(&start);
    DirectX::XMVECTOR WorldEnd = DirectX::XMLoadFloat3(&end);
    DirectX::XMVECTOR WorldRayVec = DirectX::XMVectorSubtract(WorldEnd, WorldStart);
    DirectX::XMVECTOR WorldRayLength = DirectX::XMVector3Length(WorldRayVec);
    //���[���h��Ԃ̃��C�̒���
    DirectX::XMStoreFloat(&result.distance, WorldRayLength);
    
    bool hit = false;
    //�S�Ẵ��b�V���Ƃ̏Փ˔�����s��(mesh=fbx(mdl)�̒��̈�̃|���S���̉�)
    const ModelResource* resource = model->GetResource();
    for (const ModelResource::Mesh& mesh : resource->GetMeshes())
    {
        //���b�V���m�[�h�擾
        const Model::Node& node = model->GetNodes().at(mesh.nodeIndex);
        //���C�����[���h��Ԃ��烍�[�J�����W�֕ϊ�
        DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&node.worldTransform);
        DirectX::XMMATRIX InverseWorldTransform = DirectX::XMMatrixInverse(nullptr, WorldTransform);

        DirectX::XMVECTOR S = DirectX::XMVector3TransformCoord(WorldStart, InverseWorldTransform);
        DirectX::XMVECTOR E = DirectX::XMVector3TransformCoord(WorldEnd, InverseWorldTransform);
        DirectX::XMVECTOR SE = DirectX::XMVectorSubtract(E, S);
        DirectX::XMVECTOR V = DirectX::XMVector2Normalize(SE);
        DirectX::XMVECTOR Length = DirectX::XMVector3Length(SE);

        //���C�̒���
        float neart;
        DirectX::XMStoreFloat(&neart, Length);

        //�O�p�`(��)�Ƃ̌�������
        const std::vector<ModelResource::Vertex>& vertices = mesh.vertices;
        const std::vector<UINT>indices = mesh.indices;

        int materialIndex = -1;
        DirectX::XMVECTOR HitPosition;
        DirectX::XMVECTOR HitNormal;
        for (const ModelResource::Subset& subset : mesh.subsets)
        {
            for (UINT i = 0; i < subset.indexCount; i += 3)
            {
                UINT index = subset.startIndex + i;
                //vertices�͒��_�̔z��Aindices�̓C���f�b�N�X�̔z��ŁA�ǂ̒��_���ǂ̏����ŎO�p�`���\�����邩�����Ă���
                //�O�p�`�̒��_�𒊏o
                const ModelResource::Vertex& a = vertices.at(indices.at(index));//���_�����擾���Ă���
                const ModelResource::Vertex& b = vertices.at(indices.at(index+1));
                const ModelResource::Vertex& c = vertices.at(indices.at(index+2));

                DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&a.position);
                DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&b.position);
                DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&c.position);

                //�O�p�`�̎O�Ӄx�N�g�����Z�o
                DirectX::XMVECTOR AB = DirectX::XMVectorSubtract(B,A);
                DirectX::XMVECTOR BC = DirectX::XMVectorSubtract(C, B);
                DirectX::XMVECTOR CA = DirectX::XMVectorSubtract(A, C);

                //�O�p�`�̖@���x�N�g�����Z�o(�@���x�N�g���͎O�p�`�̓�̕ӂ̊O�ς���邱�Ƃŋ��߂���)
                DirectX::XMVECTOR N = DirectX::XMVector3Cross(AB, BC);
                
                //���ς̌��ʂ��v���X�Ȃ�Η�����
                DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(V, N);
                float d;
                DirectX::XMStoreFloat(&d, Dot);
                if (d >= 0) continue;
                
                //���C�ƕ��ʂ̌�_���Z�o
                //���C�̎n�_����O�p�`�̈�_�ւ̃x�N�g�����v�Z
                DirectX::XMVECTOR SA = DirectX::XMVectorSubtract(A, S);
                //�@���Ƃ��̃x�N�g���Ƃ̎ˉe���v�Z
                DirectX::XMVECTOR X = DirectX::XMVectorDivide(DirectX::XMVector3Dot(N, SA), Dot);
                float x;
                DirectX::XMStoreFloat(&x, X);
                if (x<.0f || x>neart) continue;//��_�܂ł̋��������܂łɌv�Z�����ŋߋ�����葽���Ƃ��̓X�L�b�v
                //�x�_�܂ł̈ʒu���v�Z
                DirectX::XMVECTOR P = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(V, X), S);
                //DirectX::XMVECTOR P = DirectX::XMVectorAdd(DirectX::XMVectorScale(V, x), S);//����ł��ł���

                    //��_���O�p�`�̓����ɂ��邩����
                    //1��
                    DirectX::XMVECTOR PA = DirectX::XMVectorSubtract(A,P);
                    DirectX::XMVECTOR Cross1 = DirectX::XMVector3Cross(PA,AB);
                    DirectX::XMVECTOR Dot1 = DirectX::XMVector3Dot(Cross1,N);
                    float dot1;
                    DirectX::XMStoreFloat(&dot1, Dot1);
                    //��󂪋t����(�}�C�i�X)��������O�p�`�̊O��
                    if (dot1<0.0f) continue;
                    //2��
                    DirectX::XMVECTOR PB = DirectX::XMVectorSubtract(B, P);
                    DirectX::XMVECTOR Cross2 = DirectX::XMVector3Cross(PB, BC);
                    DirectX::XMVECTOR Dot2 = DirectX::XMVector3Dot(Cross2, N);
                    float dot2;
                    DirectX::XMStoreFloat(&dot2, Dot2);

                    if (dot2 < 0.0f) continue;
                    //3��
                    DirectX::XMVECTOR PC = DirectX::XMVectorSubtract(C, P);
                    DirectX::XMVECTOR Cross3 = DirectX::XMVector3Cross(PC, CA);
                    DirectX::XMVECTOR Dot3 = DirectX::XMVector3Dot(Cross3, N);
                    float dot3;
                    DirectX::XMStoreFloat(&dot3, Dot3);

                    if (dot3 < 0.0f) continue;

                    //�ŋߋ������X�V
                    neart = x;
                    //��_�Ɩ@�����X�V
                    HitPosition = P;
                    HitNormal = N;
                    materialIndex = subset.materialIndex;
            }
        }
        if (materialIndex >= 0)
        {
            //���[�J����Ԃ��烏�[���h��Ԃ֕ϊ�
            DirectX::XMVECTOR WorldPosition = DirectX::XMVector3TransformCoord(HitPosition,WorldTransform);
            DirectX::XMVECTOR WorldCrossVec = DirectX::XMVectorSubtract(WorldPosition, WorldStart);
            DirectX::XMVECTOR WorldCrossLength = DirectX::XMVector3Length(WorldCrossVec);
            float distance;
            DirectX::XMStoreFloat(&distance, WorldCrossLength);

            //�q�b�g���ۑ�
            if (result.distance > distance)
            {
                DirectX::XMVECTOR WorldNormal = DirectX::XMVector3TransformNormal(HitNormal, WorldTransform);

                result.distance = distance;
                result.materialIndex = materialIndex;
                DirectX::XMStoreFloat3(&result.position, WorldPosition);
                DirectX::XMStoreFloat3(&result.normal, DirectX::XMVector3Normalize(WorldNormal));
                hit = true;
            }
        }
    }
    return hit;
}