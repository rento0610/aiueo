#include "Stage.h"

static Stage* instance = nullptr;

Stage& Stage::Instance()
{
    return *instance;
}

Stage::Stage()
{
    instance = this;
    //ステージモデルを読み込み
    model = new Model("Data/Model/ExampleStage/ExampleStage.mdl");
}

Stage::~Stage()
{
    delete model;
}

void Stage::Update(float elapsedTime)
{

}

void Stage::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    //シェーダーにモデルを描画してもらう
    shader->Draw(dc, model);
}

bool Stage::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
    return Collision::IntersectRayVsModel(start, end, model, hit);
}