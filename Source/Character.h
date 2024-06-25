#pragma once

#include <DirectXMath.h>

class Character
{
public:
    Character() {}
    virtual ~Character(){}

    void UpdateTransform();

    const DirectX::XMFLOAT3& GetPosition() const { return position; };

    void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; };

    const DirectX::XMFLOAT3 GetAngle() const { return angle; };

    void SetAngle(const DirectX::XMFLOAT3& angle) { this->angle = angle; };

    const DirectX::XMFLOAT3& GetScale() const { return scale; }

    void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }
    //半径取得
    float GetRadius() const { return radius; }
    //衝撃を与える
    void AddImpulse(const DirectX::XMFLOAT3& impulse);

    bool IsGround() const { return isGround; };

    float GetHeight() const { return height; }

    bool ApplyDamage(int damage, float invincibleTime);

    int GetHealth() const { return health; }

    int GetMaxHealth() const { return maxHealth; }
protected:
    void Jump(float speed);

    void UpdateVelocity(float elapssedTime);
    //無敵時間更新
    void UpdateInvincibleTimer(float elapsedTime);
     void Move(float vx, float vz, float speed);
     void Turn(float elapsedTime, float vx, float vz, float speed);
     virtual void OnLanding() {}

     virtual void onDamaged() {}

     virtual void OnDead() {}
    //継承先でもアクセスできるようにする
protected:
    DirectX::XMFLOAT3 position = { 0,0,0 };
    DirectX::XMFLOAT3 angle = { 0,0,0 };
    DirectX::XMFLOAT3 scale = { 1,1,1 };
    DirectX::XMFLOAT4X4 transform = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };

    float radius = 0.5f;
    float gravity = -1.0f;
    DirectX::XMFLOAT3 velocity = { 0,0,0 };
    bool isGround = false;
    float height = 2.0f;
    int health = 5;
    int maxHealth = 5;
    float invincibleTimer = 1.0f;
    float friction = 0.5f;
    float acceleration = 1.0f;
    float maxMoveSpeed = 5.0f;
    float moveVecX = 0.0f;
    float moveVecZ = 0.0f;
    float airControl = 0.04f;
    float stepOffset = 1.0f;
    float slopeRate = 1.0f;
private:
    //垂直速力更新処理
    void UpdateVerticalVelocity(float elapsedFrame);
    //垂直移動更新処理
    void UpdateVerticalMove(float elapsedTime);
    //水平速力更新処理
    void UpdateHorizontalVelocity(float elapsedFrame);
    //水平移動更新処理
    void UpdateHorizontalMove(float elapsedTime);
};