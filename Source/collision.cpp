#include "Collision.h"

//球は真ん中が基準点
//円柱は下が基準点
bool Collision::IntersectSphereVsSphere(
    const DirectX::XMFLOAT3& positionA,
    float radiusA,
    const DirectX::XMFLOAT3& positionB,
    float radiusB,
    DirectX::XMFLOAT3& outPositionB)
{
    //A→Bの単位ベクトルを算出
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB= DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq);

    //距離測定
    float range = radiusA + radiusB;
    if (range*range<lengthSq)
    {
        return false;
    }

    //AがBを押し出す
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
    //XZ平面での範囲チェック
    if (positionA.y > positionB.y + heightB)
    {
        return false;
    }
    if (positionA.y + heightA < positionB.y)
    {
        return false;
    }
    //X同士を引く
    float vx = positionB.x - positionA.x;
    //Z同士を引く
    float vz = positionB.z - positionA.z;
    //XZの長さを計算する
    float distXZ = sqrtf(vx * vx + vz * vz);
    //半径Aと半径Bの長さを計算する
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
    //球の足元が円柱の頭より上なら当たっていない
    if (spherePosition.y - sphereRadius > cylinderPosition.y + cylinderHeight) return false;
    // 球の頭が円柱の足元より下なら当たっていない
    if (spherePosition.y + sphereRadius < cylinderPosition.y) return false;
    //XZ平面での範囲チェック
    if (spherePosition.y > spherePosition.y + cylinderHeight)
    {
        return false;
    }
    if (spherePosition.y + sphereRadius < cylinderPosition.y)
    {
        return false;
    }
    //X同士を引く
    float vx = cylinderPosition.x - spherePosition.x;
    //Z同士を引く
    float vz = cylinderPosition.z - spherePosition.z;
    //XZの長さを計算する
    float distXZ = sqrtf(vx * vx + vz * vz);
    //半径Aと半径Bの長さを計算する
    float range = sphereRadius + cylinderRadius;
    //XZの長さが半径Aと半径Bの長さより大きかったら当たっていない
    if (distXZ > range)
    {
        return false;
    }
    vx /= distXZ;
    vz /= distXZ;
    //AがBを押し出す
    //vxとvzを正規化
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
    //ワールド空間のレイの長さ
    DirectX::XMStoreFloat(&result.distance, WorldRayLength);
    
    bool hit = false;
    //全てのメッシュとの衝突判定を行う(mesh=fbx(mdl)の中の一つのポリゴンの塊)
    const ModelResource* resource = model->GetResource();
    for (const ModelResource::Mesh& mesh : resource->GetMeshes())
    {
        //メッシュノード取得
        const Model::Node& node = model->GetNodes().at(mesh.nodeIndex);
        //レイをワールド空間からローカル座標へ変換
        DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&node.worldTransform);
        DirectX::XMMATRIX InverseWorldTransform = DirectX::XMMatrixInverse(nullptr, WorldTransform);

        DirectX::XMVECTOR S = DirectX::XMVector3TransformCoord(WorldStart, InverseWorldTransform);
        DirectX::XMVECTOR E = DirectX::XMVector3TransformCoord(WorldEnd, InverseWorldTransform);
        DirectX::XMVECTOR SE = DirectX::XMVectorSubtract(E, S);
        DirectX::XMVECTOR V = DirectX::XMVector2Normalize(SE);
        DirectX::XMVECTOR Length = DirectX::XMVector3Length(SE);

        //レイの長さ
        float neart;
        DirectX::XMStoreFloat(&neart, Length);

        //三角形(面)との交差判定
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
                //verticesは頂点の配列、indicesはインデックスの配列で、どの頂点がどの順序で三角形を構成するか示している
                //三角形の頂点を抽出
                const ModelResource::Vertex& a = vertices.at(indices.at(index));//頂点情報を取得している
                const ModelResource::Vertex& b = vertices.at(indices.at(index+1));
                const ModelResource::Vertex& c = vertices.at(indices.at(index+2));

                DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&a.position);
                DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&b.position);
                DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&c.position);

                //三角形の三辺ベクトルを算出
                DirectX::XMVECTOR AB = DirectX::XMVectorSubtract(B,A);
                DirectX::XMVECTOR BC = DirectX::XMVectorSubtract(C, B);
                DirectX::XMVECTOR CA = DirectX::XMVectorSubtract(A, C);

                //三角形の法線ベクトルを算出(法線ベクトルは三角形の二つの辺の外積を取ることで求められる)
                DirectX::XMVECTOR N = DirectX::XMVector3Cross(AB, BC);
                
                //内積の結果がプラスならば裏向き
                DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(V, N);
                float d;
                DirectX::XMStoreFloat(&d, Dot);
                if (d >= 0) continue;
                
                //レイと平面の交点を算出
                //レイの始点から三角形の一点へのベクトルを計算
                DirectX::XMVECTOR SA = DirectX::XMVectorSubtract(A, S);
                //法線とそのベクトルとの射影を計算
                DirectX::XMVECTOR X = DirectX::XMVectorDivide(DirectX::XMVector3Dot(N, SA), Dot);
                float x;
                DirectX::XMStoreFloat(&x, X);
                if (x<.0f || x>neart) continue;//交点までの距離が今までに計算した最近距離より多きときはスキップ
                //支点までの位置を計算
                DirectX::XMVECTOR P = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(V, X), S);
                //DirectX::XMVECTOR P = DirectX::XMVectorAdd(DirectX::XMVectorScale(V, x), S);//これでもできる

                    //交点が三角形の内側にあるか判定
                    //1つめ
                    DirectX::XMVECTOR PA = DirectX::XMVectorSubtract(A,P);
                    DirectX::XMVECTOR Cross1 = DirectX::XMVector3Cross(PA,AB);
                    DirectX::XMVECTOR Dot1 = DirectX::XMVector3Dot(Cross1,N);
                    float dot1;
                    DirectX::XMStoreFloat(&dot1, Dot1);
                    //矢印が逆方向(マイナス)だったら三角形の外積
                    if (dot1<0.0f) continue;
                    //2つめ
                    DirectX::XMVECTOR PB = DirectX::XMVectorSubtract(B, P);
                    DirectX::XMVECTOR Cross2 = DirectX::XMVector3Cross(PB, BC);
                    DirectX::XMVECTOR Dot2 = DirectX::XMVector3Dot(Cross2, N);
                    float dot2;
                    DirectX::XMStoreFloat(&dot2, Dot2);

                    if (dot2 < 0.0f) continue;
                    //3つめ
                    DirectX::XMVECTOR PC = DirectX::XMVectorSubtract(C, P);
                    DirectX::XMVECTOR Cross3 = DirectX::XMVector3Cross(PC, CA);
                    DirectX::XMVECTOR Dot3 = DirectX::XMVector3Dot(Cross3, N);
                    float dot3;
                    DirectX::XMStoreFloat(&dot3, Dot3);

                    if (dot3 < 0.0f) continue;

                    //最近距離を更新
                    neart = x;
                    //交点と法線を更新
                    HitPosition = P;
                    HitNormal = N;
                    materialIndex = subset.materialIndex;
            }
        }
        if (materialIndex >= 0)
        {
            //ローカル空間からワールド空間へ変換
            DirectX::XMVECTOR WorldPosition = DirectX::XMVector3TransformCoord(HitPosition,WorldTransform);
            DirectX::XMVECTOR WorldCrossVec = DirectX::XMVectorSubtract(WorldPosition, WorldStart);
            DirectX::XMVECTOR WorldCrossLength = DirectX::XMVector3Length(WorldCrossVec);
            float distance;
            DirectX::XMStoreFloat(&distance, WorldCrossLength);

            //ヒット情報保存
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