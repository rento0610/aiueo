#pragma once

#include "Graphics/Sprite.h"
#include "Scene.h"

class SceneTitle :public Scene
{
public:
    SceneTitle() {}
    ~SceneTitle() override {}
    void Initialize() override;
    void Finalize() override;
    void Update(float elapsedTime) override;
    void Render() override;

private:
    Sprite* sprite = nullptr;
};