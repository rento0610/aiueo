#pragma once

#include <vector>
#include <set>
#include "Projectile.h"

class ProjectileManager
{
public:
    ProjectileManager();
    ~ProjectileManager();

    void Update(float elapsedTime);

    void Render(ID3D11DeviceContext* dc, Shader* shader);

    void DrawDebugPrimitive();

    void Register(Projectile* projectile);

    void Clear();

    int GetProjectileCount() const { return static_cast<int>(projectiles.size()); }

    Projectile* GetProlectile(int index) { return projectiles.at(index); }

    void Remove(Projectile* projectile);

private:
    std::vector<Projectile*> projectiles;
    std::set<Projectile*> removes;
};