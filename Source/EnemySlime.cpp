#include "EnemySlime.h"

EnemySlime::EnemySlime()
{
    model = new Model("Data/Model/Slime/Slime.mdl");

    scale.x = scale.y = scale.z = 0.01f;

    radius = 0.5f;
    height = 1.0f;
}

EnemySlime::~EnemySlime()
{
    delete model;
}

void EnemySlime::Update(float elapsedTime)
{
    UpdateVelocity(elapsedTime);
    //無敵時間更新
    UpdateInvincibleTimer(elapsedTime);
    UpdateTransform();

    model->UpdateTransform(transform);
}

void EnemySlime::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    shader->Draw(dc, model);
}

void EnemySlime::OnDead()
{
    Destroy();
}
//#include "Graphics/Graphics.h"
//#include "SceneGame.h"
//#include "Camera.h"
//#include "EnemyManager.h"
//#include "EnemySlime.h"
//
//// 初期化
//void SceneGame::Initialize()
//{
//	//ステージ初期化
//	stage = new Stage();
//
//	player = new Player();
//
//	Graphics& graphics = Graphics::Instance();
//	Camera& camera = Camera::Instance();
//	camera.SetLookAt(
//		DirectX::XMFLOAT3(0, 10, -10),
//		DirectX::XMFLOAT3(0, 0, 0),
//		DirectX::XMFLOAT3(0, 1, 0)
//	);
//	camera.SetPerspectiveFov(
//		DirectX::XMConvertToRadians(45),
//		graphics.GetScreenWidth() / graphics.GetScreenHeight(),
//		0.1f,
//		1000.0f
//	);
//	//カメラコントローラー初期化
//	cameraController = new CameraController();
//
//	EnemyManager& enemyManager = EnemyManager::Instance();
//	EnemySlime* slime = new EnemySlime();
//	slime->SetPosition(DirectX::XMFLOAT3(0, 0, 5));
//	enemyManager.Register(slime);
//}
////5のやつ
////PositionA=XMLoadFloat3(position)
////Positionb=XMLoadFloat3(position)
////vec=subsutract(positionA,positionB)
////LengthSq=length(vec)
////
////range=radiusA+radiusB
////if(range*range)
////
////Vec=DirectX::normaize(vec)
////Vec=DirectX::scale(vec)
////positionB=DirectX::add(positionA,vec)
////storefloat3(outpositionB,positionB)
//// 
//// 
//// エネミー同士の衝突処理
////敵の数を取得
////敵の数分回す
////敵Bを取得
////押し出し後の位置
////衝突処理(if文)
////敵Aの位置
////敵Aの半径
////敵Bの半径
////敵Bの半径
////押し出し後の位置
////敵Bに押し出し
//
//// 終了化
//void SceneGame::Finalize()
//{
//	//カメラコントローラー終了化
//	if (cameraController != nullptr)
//	{
//		delete cameraController;
//		cameraController = nullptr;
//	}
//	//ステージ終了化
//	if (stage != nullptr)
//	{
//		delete stage;
//		stage = nullptr;
//	}
//
//	if (player != nullptr)
//	{
//		delete player;
//		player = nullptr;
//	}
//	EnemyManager::Instance().Clear();
//}
//
//// 更新処理
//void SceneGame::Update(float elapsedTime)
//{
//	DirectX::XMFLOAT3 target = player->GetPosition();
//	target.y += 0.5f;
//	cameraController->SetTarget(target);
//	cameraController->Update(elapsedTime);
//	//ステージ更新処理
//	stage->Update(elapsedTime);
//
//	player->Update(elapsedTime);
//
//	EnemyManager::Instance().Update(elapsedTime);
//}
//
//// 描画処理
//void SceneGame::Render()
//{
//	Graphics& graphics = Graphics::Instance();
//	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
//	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
//	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();
//
//	// 画面クリア＆レンダーターゲット設定
//	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)
//	dc->ClearRenderTargetView(rtv, color);
//	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
//	dc->OMSetRenderTargets(1, &rtv, dsv);
//
//	// 描画処理
//	RenderContext rc;
//	rc.lightDirection = { 0.0f, -1.0f, 0.0f, 0.0f };	// ライト方向（下方向）
//
//	Camera& camera = Camera::Instance();
//	rc.view = camera.GetView();
//	rc.projection = camera.GetProjection();
//
//	// 3Dモデル描画
//	{
//		Shader* shader = graphics.GetShader();
//		shader->Begin(dc, rc);
//		//ステージ描画
//		stage->Render(dc, shader);
//
//		player->Render(dc, shader);
//
//		EnemyManager::Instance().Render(dc, shader);
//		shader->End(dc);
//	}
//
//	// 3Dデバッグ描画
//	{
//
//		//プレイヤーデバッグプリミティブ描画
//		player->DrawDebugPrimitive();
//		// ラインレンダラ描画実行
//		graphics.GetLineRenderer()->Render(dc, rc.view, rc.projection);
//
//		// デバッグレンダラ描画実行
//		graphics.GetDebugRenderer()->Render(dc, rc.view, rc.projection);
//	}
//
//	// 2Dスプライト描画
//	{
//
//	}
//
//	// 2DデバッグGUI描画
//	{
//		player->DrawDwbugGUI();
//	}
//}
