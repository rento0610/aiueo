#pragma once

#include "Graphics/Sprite.h"
#include "Scene.h"
#include <thread>

class SceneLoading :public Scene
{
public:
    SceneLoading(Scene* nextScene):nextScene(nextScene) {}
    SceneLoading() {}
    ~SceneLoading() override {}
    void Initialize() override;
    void Finalize() override;
    void Update(float elapsedTime) override;
    void Render() override;
private:
    //ローディングスレッド
    static void LoadingThread(SceneLoading* scene);
private:
    Sprite* sprite = nullptr;
    float angle = 0.0f;
    Scene* nextScene = nullptr;
    std::thread* thread = nullptr;
};