#include <imgui.h>
#include "Player.h"
#include "Input/Input.h"
#include "Graphics/Graphics.h"
#include "Camera.h"
#include "EnemyManager.h"
#include "Collision.h"
#include "ProjectileStraight.h"
#include "ProjectileHoming.h"

Player::Player()
{
    model = new Model("Data/Model/Mr.Incredible/Mr.Incredible.mdl");

    scale.x = scale.y = scale.z = 0.01f;

    hitEffect = new Effect("Data/Effect/Hit.efk");
}

Player::~Player()
{
    delete hitEffect;
    delete model;
}

void Player::Update(float elapsedTime)
{
    DirectX::XMFLOAT3 moveVec = GetMoveVec();

    float moveSpeed = this->moveSpeed * elapsedTime;
    position.x += moveVec.x * moveSpeed;
    position.z += moveVec.z * moveSpeed;

    InputMove(elapsedTime);

    InputJump();

    InputProjectile();
    
    ColllisionPlayerVsEnemies();
    
    CollisionProjectilesVsEnemies();

    //オブジェクト行列を更新
    UpdateTransform();

    UpdateVelocity(elapsedTime);

    projectileManager.Update(elapsedTime);
    //モデル行列更新
    model->UpdateTransform(transform);
}

void Player::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    shader->Draw(dc, model);
    projectileManager.Render(dc, shader);
}

void Player::DrawDwbugGUI()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputFloat3("Position", &position.x);
            DirectX::XMFLOAT3 a;
            a.x = DirectX::XMConvertToDegrees(angle.x);
            a.y = DirectX::XMConvertToDegrees(angle.y);
            a.z = DirectX::XMConvertToDegrees(angle.z);
            ImGui::InputFloat3("Angle", &a.x);
            angle.x = DirectX::XMConvertToRadians(a.x);
            angle.y = DirectX::XMConvertToRadians(a.y);
            angle.z = DirectX::XMConvertToRadians(a.z);

            ImGui::InputFloat3("Scale", &scale.x);
        }
    }
    ImGui::End();
}

DirectX::XMFLOAT3 Player::GetMoveVec() const
{
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();

    Camera& camera = Camera::Instance();
    const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;

    float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    DirectX::XMFLOAT3 vec;
    vec.x = (cameraRightX * ax) + (cameraFrontX * ay);
    vec.z = (cameraRightZ * ax) + (cameraFrontZ * ay);
    vec.y = 0.0f;
    return vec;
}

void Player::DrawDebugPrimitive()
{
    DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

    //衝突判定用用のデバッグ球を描画
    debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));
    projectileManager.DrawDebugPrimitive();
}

void Player::InputMove(float elapsedTime)
{
    DirectX::XMFLOAT3 moveVec = GetMoveVec();

    Move(moveVec.x,moveVec.z,moveSpeed);

    Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);
}


void Player::InputJump()
{
    GamePad& gamePad = Input::Instance().GetGamePad();
    if (position.y <= 0) jumpCount = 0;
    if (gamePad.GetButtonDown() & GamePad::BTN_A)
    {
        if (jumpCount < jumpLimit)
        {
            jumpCount++;
            Jump(jumpSpeed);
        }
    }
}

void Player::InputProjectile()
{
#if true
    GamePad& gamePad = Input::Instance().GetGamePad();

    //直進弾丸発射
    if (gamePad.GetButtonDown() & GamePad::BTN_X)
    {
        //前方向
        DirectX::XMFLOAT3 dir;
        dir.x = sinf(angle.y);
        dir.y = 0.0f;
        dir.z = cosf(angle.y);
            //発射位置(プレイヤーの腰あたり)
        DirectX::XMFLOAT3 pos;
        pos.x = position.x;
        pos.y = position.y + height / 2;
        pos.z = position.z;
        //発射
        ProjectileStraight* projectile = new ProjectileStraight(&projectileManager);
        projectile->Launch(dir, pos);
    }
    if (gamePad.GetButtonDown() & GamePad::BTN_Y)
    {
        //前方向
        DirectX::XMFLOAT3 dir;
        dir.x = sinf(angle.y);
        dir.y = 0.0f;
        dir.z = cosf(angle.y);
        //発射位置(プレイヤーの腰あたり)
        DirectX::XMFLOAT3 pos;
        pos.x = position.x;
        pos.y = position.y + height / 2;
        pos.z = position.z;
        //ターゲット
        DirectX::XMFLOAT3 target;
        target.x = pos.x + dir.x * 1000.0f;
        target.y = pos.y + dir.y * 1000.0f;
        target.z = pos.z + dir.z * 1000.0f;
        //一番近くの敵をターゲットにする
        //一番遠い場所を設定しておく
        float dist = FLT_MAX;
        //敵マネージャーを取得
        EnemyManager& enemyManager = EnemyManager::Instance();
        int enemyCount = enemyManager.GetEnemyCount();
        // 敵の数を取得
        // 一番近い敵敵の数だけチェックする
        for (int i=0;i<enemyCount;++i)
        {
            //敵との距離判定
            //1番目の敵を取得
            Enemy* enemy = EnemyManager::Instance().GetEnemy(i);
            //プレイヤー(自分)の位置をXMVECTORに変換
            DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&position);
            DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
            DirectX::XMVECTOR V = DirectX::XMVectorSubtract(E,P);
            //敵の位置をXMVECTORに変換
            //敵方向へのベクトルを計算
            //そのベクトルの長さを計算し
            DirectX::XMVECTOR D = DirectX::XMVector3Length(V);
            
            //単一の値にする
            float d;
            DirectX::XMStoreFloat(&d, D);
            //今まで計算した距離の中で一番小さい距離なら
            if (d < dist)
            {
                dist = d;
                //targetの位置を更新(敵の位置を設定)
                target = enemy->GetPosition();
                //targetのyだけ腰の高さにしておく
                target.y += enemy->GetHeight() * 0.5f;
            }
        }
        //発射
        ProjectileHoming* projectile = new ProjectileHoming(&projectileManager);
        projectile->Launch(dir, pos, target);
    }
#endif
}

void Player::OnLanding()
{
    jumpCount = 0;
}

void Player::ColllisionPlayerVsEnemies()
{
    EnemyManager& enemyManager = EnemyManager::Instance();

    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < enemyCount; ++i)
    {
        Enemy* enemy = enemyManager.GetEnemy(i);

        DirectX::XMFLOAT3 outPosition;
        if (Collision::IntersectCylinderVsCylinder(position,radius,height,enemy->GetPosition(),enemy->GetRadius(),enemy->GetHeight(),outPosition))
        {
            //enemy->SetPosition(outPosition);
            
            DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&position);
            DirectX::XMVECTOR E = DirectX::XMLoadFloat3(&enemy->GetPosition());
            DirectX::XMVECTOR V = DirectX::XMVectorSubtract(P,E);
            DirectX::XMVECTOR N = DirectX::XMVector3Normalize(V);
            DirectX::XMFLOAT3 normal;
            DirectX::XMStoreFloat3(&normal, N);
            //enemy->ApplyDamage(1,0.5f);
            if (normal.y > 0.8f)
            {
                Jump(jumpSpeed * 0.5f);
            }
            else
            {
                enemy->SetPosition(outPosition);
            }
        }
    }
    //敵の真上付近に当たったかをチェック
}

void Player::CollisionProjectilesVsEnemies()
{
    EnemyManager& enemyManager = EnemyManager::Instance();
//全ての弾丸と全ての敵を総当たりで衝突処理
    int projectileCount = projectileManager.GetProjectileCount();
    int enemyCount = enemyManager.GetEnemyCount();
    for (int i = 0; i < projectileCount; ++i)
    {
        Projectile* projectile = projectileManager.GetProlectile(i);

        for (int j = 0; j < enemyCount; ++j)
        {
            Enemy* enemy = enemyManager.GetEnemy(j);

            //衝突処理
            DirectX::XMFLOAT3 outPosition;
            if (Collision::IntersectSphereVsCylinder(
                projectile->GetPosition(),
                projectile->GetRadius(),
                enemy->GetPosition(),
                enemy->GetRadius(),
                enemy->GetHeight(),
                outPosition))
            {
                //ダメージを与える
                if (enemy->ApplyDamage(1, 0.5f))
                {
                    //吹き飛ばす
                    {
                        DirectX::XMFLOAT3 impulse;
                        const float power = 10.0f; //吹き飛ばす力
                        //敵の位置を取得
                        const DirectX::XMFLOAT3& e = enemy->GetPosition();
                        //敵の位置を取得
                        const DirectX::XMFLOAT3& p = projectile->GetPosition();
                        //弾から敵の位置へのベクトルを取得
                        float vx = e.x - p.x;
                        float vz = e.z - p.z;
                        //そのベクトルを正規化(長さを計算し、長さで割る)
                        float lengthXZ = sqrtf(vx*vx+vz*vz);
                        vx /= lengthXZ;
                        vz /= lengthXZ;
                        //衝撃の値を設定(xzは正規化したベクトルをpower分スケーリング)
                        impulse.x = vx*power;
                        impulse.y = power * 0.5f; //yはちょっと浮かせる
                        impulse.z = vz*power;
                        enemy->AddImpulse(impulse);
                    }
                    //ヒットエフェクト再生
                    {
                        DirectX::XMFLOAT3 e = enemy->GetPosition();
                        e.y += enemy->GetHeight() * 0.5f;
                        hitEffect->Play(e);
                    }
                    //弾丸破棄
                    projectile->Destroy();
                }
            }
        }
    }
}
