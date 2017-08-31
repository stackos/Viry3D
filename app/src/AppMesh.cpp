/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "Main.h"
#include "Application.h"
#include "GameObject.h"
#include "Resource.h"
#include "graphics/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/Material.h"
#include "graphics/Texture2D.h"
#include "renderer/MeshRenderer.h"
#include "ui/UILabel.h"
#include "time/Time.h"
#include "io/MemoryStream.h"

using namespace Viry3D;

class AppMesh : public Application
{
public:
	AppMesh();
	virtual void Start();
	virtual void Update();

	WeakRef<GameObject> m_cube;
	float m_rotate_deg;
};

#if 1
VR_MAIN(AppMesh);
#endif

AppMesh::AppMesh()
{
	this->SetName("Viry3D::AppMesh");
	this->SetInitSize(800, 600);
}

void AppMesh::Start()
{
	this->CreateFPSUI(20, 1, 1);

	auto camera = GameObject::Create("camera")->AddComponent<Camera>();
	camera->GetTransform()->SetPosition(Vector3(0, 6, -10));
	camera->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));
	camera->SetCullingMask(1 << 0);

	auto mesh = Mesh::Create();

	auto buffer = ByteBuffer((sizeof(Vector3) + sizeof(Vector2)) * 8);
	MemoryStream ms(buffer);
	ms.Write<Vector3>(Vector3(-1, 1, -1));
	ms.Write<Vector2>(Vector2(0, 0));
	ms.Write<Vector3>(Vector3(-1, -1, -1));
	ms.Write<Vector2>(Vector2(0, 1));
	ms.Write<Vector3>(Vector3(1, -1, -1));
	ms.Write<Vector2>(Vector2(1, 1));
	ms.Write<Vector3>(Vector3(1, 1, -1));
	ms.Write<Vector2>(Vector2(1, 0));
	ms.Write<Vector3>(Vector3(-1, 1, 1));
	ms.Write<Vector2>(Vector2(1, 0));
	ms.Write<Vector3>(Vector3(-1, -1, 1));
	ms.Write<Vector2>(Vector2(1, 1));
	ms.Write<Vector3>(Vector3(1, -1, 1));
	ms.Write<Vector2>(Vector2(0, 1));
	ms.Write<Vector3>(Vector3(1, 1, 1));
	ms.Write<Vector2>(Vector2(0, 0));
	mesh->SetVertexCount(8);
	mesh->SetVertexBufferData(buffer);
	mesh->AddVertexAttributeOffset({ VertexAttributeType::Vertex, 0 });
	mesh->AddVertexAttributeOffset({ VertexAttributeType::Texcoord, sizeof(Vector3) });

	unsigned short triangles[] = {
		0, 1, 2, 0, 2, 3,
		3, 2, 6, 3, 6, 7,
		7, 6, 5, 7, 5, 4,
		4, 5, 1, 4, 1, 0,
		4, 0, 3, 4, 3, 7,
		1, 5, 6, 1, 6, 2
	};
	mesh->triangles.AddRange(triangles, 36);

	auto mat = Material::Create("Diffuse");
	mesh->Update();

	auto obj = GameObject::Create("mesh");
	auto renderer = obj->AddComponent<MeshRenderer>();
	renderer->SetSharedMesh(mesh);
	renderer->SetSharedMaterial(mat);

	Resource::LoadTextureAsync("Assets/AppMesh/wow.png.tex",
		[=](Ref<Object> obj) {
		auto tex = RefCast<Texture>(obj);
		mat->SetMainTexture(tex);
	});

	m_cube = obj;
	m_rotate_deg = 0;

	Resource::LoadGameObjectAsync("Assets/AppMesh/plane.prefab");
}

void AppMesh::Update()
{
	m_cube.lock()->GetTransform()->SetLocalRotation(Quaternion::Euler(0, m_rotate_deg, 0));
	m_rotate_deg += 0.1f;
}