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

#pragma once

#include "memory/Ref.h"
#include "math/Vector3.h"
#include "container/Vector.h"

namespace Viry3D
{
	class Collider;

	struct RaycastHit
	{
		Vector3 point;
		Vector3 normal;
		WeakRef<Collider> collider;
	};

	class Physics
	{
	public:
		static void Init();
		static void Deinit();
		static void Update();
		static void AddRigidBody(void* body);
		static void RemoveRigidBody(void* body);
		static void AddCharacter(void* character);
		static void RemoveCharacter(void* character);
		static bool Raycast(RaycastHit& hit, const Vector3& from, const Vector3& dir, float length, int layer_mask = -1);
		static Vector<RaycastHit> RaycastAll(const Vector3& from, const Vector3& dir, float length, int layer_mask = -1);
	};
}
