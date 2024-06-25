#pragma once

#include "Graphics/Shader.h"
#include "Character.h"

class Enemy :public Character
{
public:
    Enemy(){}
    ~Enemy()override{}

    virtual void Update(float elapsedTime) = 0;

    virtual void Render(ID3D11DeviceContext* dc, Shader* shader) = 0;

    void Destroy();

    virtual void DrawDebugPrimitive();
};