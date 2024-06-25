#include "ProjectileHoming.h"

ProjectileHoming::ProjectileHoming(ProjectileManager* manager):Projectile(manager)
{
    model = new Model("Data/Model/Sword/Sword.mdl");

    scale.x = scale.y = scale.z = 3.0f;
}
ProjectileHoming::~ProjectileHoming()
{
    delete model;
}

void ProjectileHoming::Update(float elapsedTime)
{
#if true
    //寿命処理
    lifeTimer -= elapsedTime;
    if (lifeTimer <= 0)
    {
        Destroy();
    }
    //移動
    {
        float moveSpeed = this->moveSpeed * elapsedTime;
        //位置+=前方向*速さ
        position.x += direction.x * moveSpeed;
        position.y += direction.y * moveSpeed;
        position.z += direction.z * moveSpeed;
    }
    //旋回
    {
        float turnSpeed = this->turnSpeed * elapsedTime;

        //ターゲットまでのベクトルを算出
        DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&position);
        DirectX::XMVECTOR Target = DirectX::XMLoadFloat3(&target);
        DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Target,Position);

        DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
        float lengthSq;
        DirectX::XMStoreFloat(&lengthSq, LengthSq);
        if (lengthSq > 0.00001f)
        {
            //ターゲットまでのベクトルを単位ベクトル化(正規化) ベクトル化=正規化
            Vec = DirectX::XMVector3Normalize(Vec);
            //向いている方向(前方方向)ベクトルをXMVECTORに変換
            DirectX::XMVECTOR Direction = DirectX::XMLoadFloat3(&direction);
            //前方方向ベクトルとターゲットまでのベクトルの内積(角度)を算出
            DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Direction,Vec);

            float dot;
            DirectX::XMStoreFloat(&dot, Dot);

            //2つの単位ベクトルの角度が小さいほど
            //1.0に近づくという性質を利用して回転速度を調整する
            float rot = 1.0f - dot;
            if (rot > turnSpeed) rot = turnSpeed;
            //回転角度があるなら回転処理をする
            //回転角度は絶対値で0より上かどうかをチェックする
            if (fabsf(rot)>0.0001f)
            {
                //回転軸を計算(前方方向ベクトルとターゲットまでのベクトルの外積)
                DirectX::XMVECTOR Axis = DirectX::XMVector3Cross(Direction, Vec);
                Axis = DirectX::XMVector3Normalize(Axis);
                //回転軸と回転量からDirectX::XMMatriRotationAxisを使って回転行列を算出
                DirectX::XMMATRIX Rotation =DirectX::XMMatrixRotationAxis(Axis,rot) ;
                //現在の行列を回転させる
                    DirectX::XMMATRIX Transform = DirectX::XMLoadFloat4x4(&transform);
                    //回転後の姿勢=現在の姿勢*回転行列
                    Transform = DirectX::XMMatrixMultiply(Transform,Rotation);
                    //回転後の前方方向を取り出し,単位ベクトル化する
                    Direction = DirectX::XMVector3Normalize(Transform.r[2]);
                    DirectX::XMStoreFloat3(&direction, Direction);
            }
        }
    }
    UpdateTransform();

    model->UpdateTransform(transform);
#endif
}

void ProjectileHoming::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    shader->Draw(dc, model);
}

void ProjectileHoming::Launch(const DirectX::XMFLOAT3& direction,
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& target)
{
    this->direction = direction;
    this->position = position;
    this->target = target;

    UpdateTransform();
}