#include "Enemy.h"
#include "Graphics/Graphics.h"
#include "EnemyManager.h"

void Enemy::DrawDebugPrimitive()
{
    DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();
    debugRenderer->DrawCylinder(position, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));
}
//破棄
void Enemy::Destroy()
{
    //敵マネージャーの削除に自分自身のアドレスを渡す
    EnemyManager::Instance().Remove(this);
}