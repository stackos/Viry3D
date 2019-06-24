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
#include "json/json.h"
#include "io/File.h"
#include "container/Map.h"

namespace Viry3D
{
	class BoneMapper : public Component
	{
	public:
		struct BoneState
		{
			Vector3 pos;
			Quaternion rot;
			Vector3 sca;

			WeakRef<BoneState> parent;
			Quaternion rot_world;

			const Quaternion& RotWorld()
			{
				if (parent.expired())
				{
					return rot;
				}
				else
				{
					rot_world = parent.lock()->RotWorld() * rot;
					return rot_world;
				}
			}
		};

		void Init()
		{
			// body
			avatar_bones.Add("Hips");
			avatar_bones.Add("Spine");
			avatar_bones.Add("Chest"); // optional
			avatar_bones.Add("UpperChest"); // optional
			// head
			avatar_bones.Add("Neck"); // optional
			avatar_bones.Add("Head");
			avatar_bones.Add("LeftEye"); // optional
			avatar_bones.Add("RightEye"); // optional
			avatar_bones.Add("Jaw"); // optional
			// left arm
			avatar_bones.Add("LeftShoulder"); // optional
			avatar_bones.Add("LeftUpperArm");
			avatar_bones.Add("LeftLowerArm");
			avatar_bones.Add("LeftHand");
			// right arm
			avatar_bones.Add("RightShoulder"); // optional
			avatar_bones.Add("RightUpperArm");
			avatar_bones.Add("RightLowerArm");
			avatar_bones.Add("RightHand");
			// left leg
			avatar_bones.Add("LeftUpperLeg");
			avatar_bones.Add("LeftLowerLeg");
			avatar_bones.Add("LeftFoot");
			avatar_bones.Add("LeftToes"); // optional
			// right leg
			avatar_bones.Add("RightUpperLeg");
			avatar_bones.Add("RightLowerLeg");
			avatar_bones.Add("RightFoot");
			avatar_bones.Add("RightToes"); // optional

			if (root_src)
			{
				this->FindBones(root_src, src_bones);
			}

			if (root_dst)
			{
				this->FindBones(root_dst, dst_bones);
			}

			Json::Value src_base_pose_root;
			if (this->LoadJson(src_base_pose_path, src_base_pose_root))
			{
				this->LoadBoneState(src_base_pose_root, src_base_pose, Ref<BoneState>());
			}

			Json::Value dst_base_pose_root;
			if (this->LoadJson(dst_base_pose_path, dst_base_pose_root))
			{
				this->LoadBoneState(dst_base_pose_root, dst_base_pose, Ref<BoneState>());
			}

			this->LoadJson(bone_map_path, bone_map);
		}

		void FindBones(const Ref<Transform>& t, Vector<Ref<Transform>>& bones)
		{
			bones.Add(t);

			for (int i = 0; i < t->GetChildCount(); ++i)
			{
				const Ref<Transform>& child = t->GetChild(i);
				FindBones(child, bones);
			}
		}

		bool LoadJson(const String& path, Json::Value& root)
		{
			String full_path = Engine::Instance()->GetDataPath() + "/" + path;
			if (File::Exist(full_path))
			{
				String json = File::ReadAllText(full_path);

				auto reader = Ref<Json::CharReader>(Json::CharReaderBuilder().newCharReader());
				const char* begin = json.CString();
				const char* end = begin + json.Size();
				if (reader->parse(begin, end, &root, nullptr))
				{
					return true;
				}
			}
			return false;
		}

		void LoadBoneState(const Json::Value& bone, Map<String, Ref<BoneState>>& states, const Ref<BoneState>& parent)
		{
			const char* name = bone["name"].asCString();
			if (!states.Contains(name))
			{
				auto state = RefMake<BoneState>();
				const Json::Value& pos = bone["localPosition"];
				state->pos = Vector3(pos["x"].asFloat(), pos["y"].asFloat(), pos["z"].asFloat());
				const Json::Value& rot = bone["localRotation"];
				state->rot = Quaternion(rot["x"].asFloat(), rot["y"].asFloat(), rot["z"].asFloat(), rot["w"].asFloat());
				const Json::Value& sca = bone["localScale"];
				state->sca = Vector3(sca["x"].asFloat(), sca["y"].asFloat(), sca["z"].asFloat());
				state->parent = parent;
				state->rot_world = state->RotWorld();

				states.Add(name, state);

				const Json::Value& children = bone["children"];
				for (uint32_t i = 0; i < children.size(); ++i)
				{
					const Json::Value& child = children[i];
					LoadBoneState(child, states, state);
				}
			}
		}

		virtual void Update()
		{
			if (src_bones.Size() == 0 ||
				dst_bones.Size() == 0 ||
				src_base_pose.Size() == 0 ||
				dst_base_pose.Size() == 0)
			{
				return;
			}

			for (int i = 0; i < avatar_bones.Size(); ++i)
			{
				const Json::Value& pair = bone_map[avatar_bones[i].CString()];
				if (pair)
				{
					const char* src = pair["src"].asCString();
					const char* dst = pair["dst"].asCString();

					if (strlen(src) > 0 && strlen(dst) > 0)
					{
						auto find_bone = [](const Vector<Ref<Transform>>& bones, const char* name) {
							for (int j = 0; j < bones.Size(); ++j)
							{
								if (bones[j]->GetName() == name)
								{
									return bones[j];
								}
							}
							return Ref<Transform>();
						};

						Ref<Transform> src_bone = find_bone(src_bones, src);
						Ref<Transform> dst_bone = find_bone(dst_bones, dst);

						if (src_bone && dst_bone)
						{
							Ref<BoneState> src_base_state = src_base_pose[src];
							Ref<BoneState> dst_base_state = dst_base_pose[dst];

							Quaternion rot = Quaternion::Inverse(src_base_state->rot) * src_bone->GetLocalRotation();
							dst_bone->SetLocalRotation(dst_base_state->rot * rot);

							float scale = root_dst->GetLocalPosition().y / root_src->GetLocalPosition().y;
							Vector3 pos = src_bone->GetLocalPosition() - src_base_state->pos;
							dst_bone->SetLocalPosition(dst_base_state->pos + pos * scale);
						}
					}
				}
			}
		}

		Vector<String> avatar_bones;
		Ref<Transform> root_src;
		Ref<Transform> root_dst;
		String src_base_pose_path;
		String dst_base_pose_path;
		String bone_map_path;
		Vector<Ref<Transform>> src_bones;
		Vector<Ref<Transform>> dst_bones;
		Map<String, Ref<BoneState>> src_base_pose;
		Map<String, Ref<BoneState>> dst_base_pose;
		Json::Value bone_map;
	};
}
