#pragma once

#include <vector>
#include <set>
#include "Enemy.h"

class EnemyManager
{
private:
    EnemyManager() {}
    ~EnemyManager() {}

public:
    static EnemyManager& Instance()
    {
        static EnemyManager instance;
        return instance;
    }

    void Update(float elapsedTime);

    void Render(ID3D11DeviceContext* context, Shader* shader);

    void DrawDebugPrimitive();

    void Clear();

    void Register(Enemy* enemy);
    //エネミー削除
    void Remove(Enemy* enemy);

    int GetEnemyCount() const { return static_cast<int>(enemies.size()); }

    Enemy* GetEnemy(int index) { return enemies.at(index); }
private:
    std::vector<Enemy*> enemies;
    std::set<Enemy*> removes;

    void CollisionEnemyVsEnemies();
};