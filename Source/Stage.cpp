#include "Stage.h"

static Stage* instance = nullptr;

Stage& Stage::Instance()
{
    return *instance;
}

Stage::Stage()
{
    instance = this;
    //�X�e�[�W���f����ǂݍ���
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
    //�V�F�[�_�[�Ƀ��f����`�悵�Ă��炤
    shader->Draw(dc, model);
}

bool Stage::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
    return Collision::IntersectRayVsModel(start, end, model, hit);
}