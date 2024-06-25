#include "EnemyManager.h"
#include "Collision.h"

void EnemyManager::Update(float elapsedTime)
{
    for (Enemy* enemy : enemies)
    {
        enemy->Update(elapsedTime);
    }
    CollisionEnemyVsEnemies();
    //”jŠüˆ—
    for (Enemy* enemy : removes)
    {
        //std::vector‚©‚ç—v‘f‚ğíœ‚·‚éê‡‚ÍƒCƒeƒŒ[ƒ^[‚Åíœ‚µ‚È‚¯‚ê‚Î‚È‚ç‚È‚¢
        std::vector<Enemy*>::iterator it = std::find(enemies.begin(), enemies.end(), enemy);

        if (it != enemies.end())
        {
            enemies.erase(it);
        }
        //’eŠÛ‚Ì”jŠüˆ—
        delete enemy;
    }
    //”jŠüƒŠƒXƒg‚ğƒNƒŠƒA
    removes.clear();
    //“G“¯m‚ÌÕ“Ëˆ—
    CollisionEnemyVsEnemies();
}

void EnemyManager::Render(ID3D11DeviceContext* context, Shader* shader)
{
    for (Enemy* enemy : enemies)
    {
        enemy->Render(context, shader);
    }
}
void EnemyManager::Register(Enemy* enemy)
{
    enemies.emplace_back(enemy);
}

void EnemyManager::Clear()
{
    for (Enemy* enemy : enemies)
    {
        delete enemy;
    }
    enemies.clear();
}
//ƒGƒlƒ~[“¯m‚ÌÕ“Ëˆ—
void EnemyManager::CollisionEnemyVsEnemies()
{
    size_t enemyCount = enemies.size();
    //i”Ô–Ú‚Ì“GˆÈ~‚Ì“G‚ğ1‘Ì‚¸‚ÂÕ“Ë‚µ‚Ä‚¢‚é‚©Šm”F‚µ‚Ä‚¢‚é
    for (int i = 0; i < enemyCount; ++i)
    {
        Enemy* enemyA = enemies.at(i);
        for (int j = i + 1; j < enemyCount; ++j)
        {
            Enemy* enemyB = enemies.at(j);
            DirectX::XMFLOAT3 outPosition;
            if (Collision::IntersectCylinderVsCylinder(
                enemyA->GetPosition(),
                enemyA->GetRadius(),
                enemyA->GetHeight(),
                enemyB->GetPosition(),
                enemyB->GetRadius(),
                enemyB->GetHeight(),
                outPosition))
            {
                enemyB->SetPosition(outPosition);
            }
        }
    }
}
void EnemyManager::DrawDebugPrimitive()
{
    for (Enemy* enemy : enemies)
    {
        enemy->DrawDebugPrimitive();
    }
}

void EnemyManager::Remove(Enemy* enemy)
{
    removes.insert(enemy);
}