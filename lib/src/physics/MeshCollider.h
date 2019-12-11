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

#include "Collider.h"
#include "math/Vector3.h"

namespace Viry3D
{
	class Mesh;

	class MeshCollider: public Collider
	{
	public:
        MeshCollider();
		virtual ~MeshCollider();
		virtual void SetIsRigidbody(bool value);
		void SetMesh(const Ref<Mesh>& mesh) { m_mesh = mesh; }

	protected:
        virtual void Start();
		virtual void OnTransformDirty();

	private:
		Ref<Mesh> m_mesh;
		void* m_collider_data;
		unsigned short* m_indices;
		Vector3* m_vertices;
	};
}
