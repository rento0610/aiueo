#include "Character.h"
#include "Stage.h"

void Character::UpdateTransform()
{
    //スケール行列を作成
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x,scale.y,scale.z);
    //回転行列を作成
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x,angle.y,angle.z);
    //位置行列を作成
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x,position.y,position.z);
    //3つの行列を組み合わせ,ワールド行列を作成
    DirectX::XMMATRIX W = S * R * T;
    //計算したワールド行列を取り出す
    DirectX::XMStoreFloat4x4(&transform, W);
}
void Character::Jump(float speed)
{
    velocity.y = speed;
}

void Character::UpdateVelocity(float elapsedTime)
{
    //経過フレーム
    float elapsedFrame = 60.0f * elapsedTime;

    UpdateVerticalVelocity(elapsedFrame);

    UpdateHorizontalVelocity(elapsedFrame);

    UpdateVerticalMove(elapsedTime);

    UpdateHorizontalMove(elapsedTime);
}

void Character::Turn(float elapsedTime, float vx, float vz, float speed)
{
    speed *= elapsedTime;

    float length = sqrtf(vx * vx + vz * vz);
    if (length < 0.001f) return;

    vx /= length;
    vz /= length;

    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    float dot = (frontX * vx) + (frontZ * vz);

    float rot = 1.0f - dot;
    if (rot > speed) rot = speed;
    float cross = (frontZ * vx) - (frontX * vz);

    if (cross < 0.0f)
    {
        angle.y -= rot;
    }
    else
    {
        angle.y += rot;
    }
}

void Character::Move(float vx, float vz, float speed)
{
    //移動方向ベクトルを設定
    moveVecX = vx;
    moveVecZ = vz;
    //最大速度設定
    maxMoveSpeed = speed;
}
//ダメージを与える
bool Character::ApplyDamage(int damage, float invincibleTime)
{
    //ダメージが0の場合は健康状態を変更する必要がない
    if (damage == 0) return false;
    //死亡している場合は健康状態を変更しない
    if (health <= 0) return false;
    //無敵時間中はダメージを与えない
    if (invincibleTimer > 0.0f) return false;
    //無敵時間設定
    invincibleTimer = invincibleTime;
    //ダメージ処理
    health -= damage;
    //死亡通知
    if (health <= 0)
    {
        OnDead();
    }
    //ダメージ通知
    else
    {
        onDamaged();
    }
    //健康状態が変更した場合はtrueを返す
    return true;
}

void Character::UpdateInvincibleTimer(float elapsedTime)
{
    if (invincibleTimer > 0.0f)
    {
        invincibleTimer -= elapsedTime;
    }
}

void Character::UpdateVerticalVelocity(float elapsedFrame)
{
    //重力処理
    velocity.y += gravity * elapsedFrame;
}

void Character::UpdateVerticalMove(float elapsedTime)
{
    //垂直方向の移動量
    float my = velocity.y * elapsedTime;
    slopeRate = 0.0f;

    //落下中
    if (my < 0.0f)
    {
        //レイの開始位置は足元より少し上
        DirectX::XMFLOAT3 start = { position.x,position.y + stepOffset,position.z };
        //レイの終点位置は移動後の位置
        DirectX::XMFLOAT3 end = { position.x,position.y + my,position.z };
        //レイキャストよる地面判定
        HitResult hit;
        if (Stage::Instance().RayCast(start, end, hit))
        {
            //地面に接地している
            position.y = hit.position.y;
            //傾斜率の計算(ポリゴンがどれくらい斜めなのかを計算)
            float normalizeLengthXZ = sqrtf(hit.normal.x * hit.normal.x + hit.normal.z * hit.normal.z);
            slopeRate = 1.0f - (hit.normal.y / (normalizeLengthXZ + hit.normal.y));
            //着地した
            if (!isGround)
            {
                OnLanding();
            }
            isGround = true;
            velocity.y = 0.0f;
        }
        else
        {
            //空中に浮いている
            position.y += my;
            isGround = false;
        }
    }
    //上昇中
    else if (my > 0.0f)
    {
        position.y += my;
        isGround = false;
    }
}

void Character::AddImpulse(const DirectX::XMFLOAT3& impulse)
{
    //速力に力を与える
    velocity.x += impulse.x;
    velocity.y += impulse.y;
    velocity.z += impulse.z;
}

void Character::UpdateHorizontalVelocity(float elapsedFrame)
{
    //XZ平面の速力を減速する
    //三平方の定理
    float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);//三角形の斜辺を求めている
    if (length > 0.0f)
    {
        //摩擦力
        float friction = this->friction * elapsedFrame;
        //空中にいるときは摩擦力を減らす
        if (!isGround) friction *= airControl;
        //速力を単位ベクトル化(XMVector2Normalize)
        float vx = velocity.x / length;
        float vz = velocity.z / length;
          
        //摩擦による横方向の減速処理
        if (length > friction)
        {
            //単位ベクトル化した速力を摩擦力分スケーリングした値を速力から引く
            velocity.x -= vx * friction;
            velocity.z -= vz * friction;
        }
        //横方向の速力が摩擦力以下になったので速力を無効化
        else
        {
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
    }
    //XZ平面の速力を加速する
    if (length <= maxMoveSpeed)
    {
        //移動ベクトルがゼロベクトルでないなら加速する
        float moveVecLength = sqrtf(moveVecX*moveVecX+moveVecZ*moveVecZ);
        if (moveVecLength > 0.0f)
        {
            //加速力
            float acceleration = this->acceleration * elapsedFrame;
            //空中にいるときは加速力を減らす
            if (!isGround) acceleration *= airControl;
            //移動ベクトルによる加速処理
            velocity.x += moveVecX * acceleration;
            velocity.z += moveVecZ * acceleration;
            //最大速度制限
            float length = sqrtf(velocity.x * velocity.x * velocity.z * velocity.z);
            if (length > maxMoveSpeed)
            {
                //速度ベクトルを正規化
                float vx = velocity.x / length;
                float vz = velocity.z / length;
                //最大速さ分スケーリングした値を速度ベクトルに代入
                velocity.x = vx + maxMoveSpeed;
                velocity.z = vz + maxMoveSpeed;
            }
            //下り坂でガタガタしないようにする
            if (isGround && slopeRate > 0.0f)
            {
                velocity.y -= length * slopeRate * elapsedFrame;
            }
        }
    }
    //移動ベクトルをリセット
    moveVecX = 0;
    moveVecZ = 0;
}

void Character::UpdateHorizontalMove(float elapsedTime)
{
    ////移動処理
    //position.x += velocity.x * elapsedTime;
    //position.z += velocity.z * elapsedTime;

    //水平速力計算
    float velocityLengthXZ = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
    if (velocityLengthXZ > 0.0f)
    {
        //水平速力量計算
        float mx = velocity.x * elapsedTime;
        float mz = velocity.z * elapsedTime;

        //レイの開始位置と終点位置
        DirectX::XMFLOAT3 start = { position.x,position.y + stepOffset,position.z };
        DirectX::XMFLOAT3 end = { position.x + mx,position.y + stepOffset,position.z + mz };

        //レイキャストによる壁判定
        HitResult hit;
        if (Stage::Instance().RayCast(start, end, hit))
        {
            //壁までのベクトル
            DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
            DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
            DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Start, End);

            //壁の法線
            DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&hit.normal);

            //入射ベクトルを法線に射影
            DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Vec, Normal);

            //補正位置の計算
            //法線ベクトルにDot分スケーリングする
            DirectX::XMVECTOR CollectPosition = DirectX::XMVectorMultiply(Normal, Dot);
            //DirectX::XMVECTOR CollectPosition = DirectX::XMVectorScale(Normal, DirectX::XMVectorGetX(Dot));
            //CollectPositionにEndの位置を足した位置が最終的な位置
            DirectX::XMFLOAT3 collectPosition;
            DirectX::XMStoreFloat3(&collectPosition, DirectX::XMVectorAdd(End, CollectPosition));

            //hitPositionを開始位置とし、collectPositionを終点位置としてさらにレイキャスト
            HitResult hit2;
            if (!Stage::Instance().RayCast(hit.position, collectPosition, hit2))
            {
                //当たってなかったら

                //xとyの成分のみ
                position.x = collectPosition.x;
                position.z = collectPosition.z;
            }
            else
            {
                //当たっていたらhit2.positionを最終的な位置として反映
                position.x = hit2.position.x;
                position.z = hit2.position.z;
            }
        }
        else
        {
            //移動
            position.x += mx;
            position.z += mz;
        }
    }
}