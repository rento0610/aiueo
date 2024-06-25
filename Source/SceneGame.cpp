#include "Graphics/Graphics.h"
#include "SceneGame.h"
#include "Camera.h"
#include "EnemyManager.h"
#include "EnemySlime.h"
#include "EffectManager.h"
#include <DirectXMath.h>
#include "Input/Input.h"

// 初期化
void SceneGame::Initialize()
{
	//ステージ初期化
	stage = new Stage();

	player = new Player();

	Graphics& graphics = Graphics::Instance();
	Camera& camera = Camera::Instance();
	camera.SetLookAt(
		DirectX::XMFLOAT3(0, 10, -10),
		DirectX::XMFLOAT3(0, 0, 0),
		DirectX::XMFLOAT3(0, 1, 0)
	);
	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),
		0.1f,
		1000.0f
	);
	//カメラコントローラー初期化
	cameraController = new CameraController();
	for (int i = 0; i < 2; ++i)
	{
		EnemyManager& enemyManager = EnemyManager::Instance();
		EnemySlime* slime = new EnemySlime();
		slime->SetPosition(DirectX::XMFLOAT3(i * 2.0f, 0, 5));
		enemyManager.Register(slime);
	}
	//ゲージスプライト
	gauge = new Sprite();
}

// 終了化
void SceneGame::Finalize()
{
	EnemyManager::Instance().Clear();
	//ゲージスプライト終了化
	if (gauge != nullptr)
	{
		delete gauge;
		gauge = nullptr;
	}
	//カメラコントローラー終了化
	if (cameraController != nullptr)
	{
		delete cameraController;
		cameraController = nullptr;
	}
	//ステージ終了化
	if (stage != nullptr)
	{
		delete stage;
		stage = nullptr;
	}

	if (player != nullptr)
	{
		delete player;
		player = nullptr;
	}
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	DirectX::XMFLOAT3 target = player->GetPosition();
	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);
	//ステージ更新処理
	stage->Update(elapsedTime);

	player->Update(elapsedTime);

	EnemyManager::Instance().Update(elapsedTime);

	EffectManager::Instance().Update(elapsedTime);
}

// 描画処理
void SceneGame::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// 画面クリア＆レンダーターゲット設定
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	// 描画処理
	RenderContext rc;
	rc.lightDirection = { 0.0f, -1.0f, 0.0f, 0.0f };	// ライト方向（下方向）

	Camera& camera = Camera::Instance();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	// 3Dモデル描画
	{
		Shader* shader = graphics.GetShader();
		shader->Begin(dc, rc);
		//ステージ描画
		stage->Render(dc, shader);

		player->Render(dc, shader);

		EnemyManager::Instance().Render(dc, shader);
		shader->End(dc);
	}

	//3Dエフェクト描画
	{
		EffectManager::Instance().Render(rc.view, rc.projection);
	}
	// 3Dデバッグ描画
	{
		//プレイヤーデバッグプリミティブ描画
		player->DrawDebugPrimitive();
		//エネミーデバッグプリミティブ描画
		EnemyManager::Instance().DrawDebugPrimitive();
		// ラインレンダラ描画実行
		graphics.GetLineRenderer()->Render(dc, rc.view, rc.projection);

		// デバッグレンダラ描画実行
		graphics.GetDebugRenderer()->Render(dc, rc.view, rc.projection);
	}

	// 2Dスプライト描画
	{
		RenderEnemyGauge(dc, rc.view, rc.projection);
	}

	// 2DデバッグGUI描画
	{
		player->DrawDwbugGUI();
	}
}

//エネミーHPゲージ描画
void SceneGame::RenderEnemyGauge(
	ID3D11DeviceContext* dc,
	const DirectX::XMFLOAT4X4& view,
	const DirectX::XMFLOAT4X4& projection)
{
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//変換行列
	DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&view);
	DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	//全ての敵に頭上にHPゲージを表示
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		//エネミーの頭上のワールド座標(敵の位置)
		DirectX::XMFLOAT3 worldPosition = enemy->GetPosition();
		//yの位置だけ高さ分足す
		worldPosition.y += enemy->GetHeight();
		
		//敵の位置をXMVECTORに変換
		DirectX::XMVECTOR Worldposition = DirectX::XMLoadFloat3(&worldPosition);
		//ワールド座標(3D)からスクリーン座標(2D)へ変換
		DirectX::XMVECTOR ScreenPosition = DirectX::XMVector3Project(
			Worldposition,
			0.0f, 0.0f,
			viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth,
			Projection, View, World);

		//スクリーン座標(3Dから2DにしたXMVECTORをXMFLOAT2に変換)
		DirectX::XMFLOAT2 screenPosition;
		DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

		//ゲージの長さ
		const float gaugeWidth = 30.0f;
		const float gaugeHeight = 5.0f;

		//ゲージの長さを計算(現在健康状態/最大健康状態)
		float healthRate = enemy->GetHealth() / static_cast<float>(enemy->GetMaxHealth());
		//ゲージ描画
		gauge->Render(dc,
			screenPosition.x - gaugeWidth / 2,//spriteの左位置(敵のx位置-ゲージ幅の半分)
			screenPosition.y - gaugeHeight,//Spriteの上位置(敵のy座標-ゲージ高さ)
			gaugeWidth * healthRate,//Spriteの幅(ゲージの幅*健康状態比率)
			gaugeHeight,//Spriteの高さ(ゲージの高さ)
			0.0f, 0.0f,//Tetureの切り取り位置XY
			gaugeWidth,//Tetureの切り取り幅
			gaugeHeight,//Tetureの切り取り高さ
			0.0f,//Spriteの角度
			1.0f, 0.0f, 0.0f, 1.0f//Spriteno色(r,g,b,a)
		);
	}

	//エネミー配置処理
	Mouse& mouse = Input::Instance().GetMouse();
	if (mouse.GetButtonDown() & Mouse::BTN_LEFT)
	{
		//マウスカーソル座標を取得
		DirectX::XMFLOAT3 screenPosition;
		screenPosition.x = static_cast<float>(mouse.GetPositionX());
		screenPosition.y = static_cast<float>(mouse.GetPositionY());

		DirectX::XMVECTOR ScreenPosition, WorldPosition;
		//レイの始点を計算
		screenPosition.z = 0.0f;
		ScreenPosition = DirectX::XMLoadFloat3(&screenPosition);
		//始点を2Dから3Dに変換
		WorldPosition = DirectX::XMVector3Unproject(ScreenPosition,
			0.0f, 0.0f,
			viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth,
			Projection,
			View, World);
		//ワールド空間上でのレイの始点(WorldPositionをXMFLOAT3に変換)
		DirectX::XMFLOAT3 rayStart;
		DirectX::XMStoreFloat3(&rayStart, WorldPosition);
		//レイの終点を算出
		screenPosition.z = 1.0f;	//ビューポートの深度を1.0に設定
		ScreenPosition = DirectX::XMLoadFloat3(&screenPosition);//screenPositionをXMVECTORに変換
		//始点を2Dから3Dに変換
		DirectX::XMVECTOR worldPositionEnd = DirectX::XMVector3Unproject(ScreenPosition,
			0.0f, 0.0f,
			viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth,
			Projection,
			View, World);
		//ワールド空間上でのレイの終点(WorldPositionをXMFLOAT3に変換)
		DirectX::XMFLOAT3 rayEnd;
		DirectX::XMStoreFloat3(&rayEnd, worldPositionEnd);

		//レイキャスト(上で作ったレイとステージとの衝突)
		Stage& stage = Stage::Instance();
		HitResult hit;
		if (stage.RayCast(rayStart, rayEnd, hit))
		{
			//敵を配置(敵を生成)
			EnemySlime* slime = new EnemySlime();
			//敵の位置をレイとヒットした位置に設定
			slime->SetPosition(hit.position);
			//敵管理に登録
			EnemyManager::Instance().Register(slime);
		}
	}
}
