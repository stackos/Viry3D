/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#pragma once

#include "Component.h"
#include "GameObject.h"
#include "graphics/Material.h"
#include "graphics/MeshRenderer.h"

namespace Viry3D
{
	class BoneDrawer : public Component
	{
	public:
		Ref<Transform> root;
		Ref<Material> joint_material;
		Ref<Material> bone_material;
		Ref<Mesh> cone_mesh;

		void Init()
		{
			joint_material = RefMake<Material>(Shader::Find("Unlit/Texture"));
			joint_material->SetColor(MaterialProperty::COLOR, Color(248, 181, 0, 255) / 255.0f);

			bone_material = RefMake<Material>(Shader::Find("Unlit/Texture"));
			bone_material->SetColor(MaterialProperty::COLOR, Color(40, 187, 239, 255) / 255.0f);

			cone_mesh = CreateConeMesh();

			this->DrawBones(root);
		}

		void DrawBones(const Ref<Transform>& node)
		{
			Vector<Ref<Transform>> children;

			for (int i = 0; i < node->GetChildCount(); ++i)
			{
				children.Add(node->GetChild(i));
			}

			auto sphere = Resources::LoadMesh("Library/unity default resources.Sphere.mesh");
			auto joint = GameObject::Create("")->AddComponent<MeshRenderer>();
			joint->GetTransform()->SetParent(node);
			joint->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			joint->GetTransform()->SetLocalRotation(Quaternion::Identity());
			joint->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 0.05f);
			joint->SetMesh(sphere);
			joint->SetMaterial(joint_material);

			for (int i = 0; i < children.Size(); ++i)
			{
				const auto& child = children[i];

				auto bone_dir = child->GetPosition() - node->GetPosition();
				auto bone_len = bone_dir.Magnitude();
				auto bone_size = 0.04f;

				auto bone = GameObject::Create("")->AddComponent<MeshRenderer>();
				bone->GetTransform()->SetParent(node);
				bone->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
				bone->GetTransform()->SetRotation(Quaternion::FromToRotation(Vector3(0, 1, 0), bone_dir));
				bone->GetTransform()->SetLocalScale(Vector3(bone_size, bone_len, bone_size));
				bone->SetMesh(cone_mesh);
				bone->SetMaterial(bone_material);

				DrawBones(child);
			}
		}

		static Ref<Mesh> CreateConeMesh()
		{
			Vector<Mesh::Vertex> vertices;
			Vector<unsigned int> indices;
            Mesh::Vertex v;
            v.vertex = Vector3(0, 0, 0);
			vertices.Add(v);
            v.vertex = Vector3(0, 1, 0);
			vertices.Add(v);

			for (int i = 0; i < 360; ++i)
			{
				float x = 0.5f * cos(Mathf::Deg2Rad * i);
				float z = 0.5f * sin(Mathf::Deg2Rad * i);
                v.vertex = Vector3(x, 0, z);
				vertices.Add(v);

				if (i == 359)
				{
					indices.Add(0);
					indices.Add(2 + i);
					indices.Add(2);

					indices.Add(1);
					indices.Add(2);
					indices.Add(2 + i);
				}
				else
				{
					indices.Add(0);
					indices.Add(2 + i);
					indices.Add(2 + i + 1);

					indices.Add(1);
					indices.Add(2 + i + 1);
					indices.Add(2 + i);
				}
			}

			Ref<Mesh> mesh = RefMake<Mesh>(std::move(vertices), std::move(indices));
			return mesh;
		}
	};
}
