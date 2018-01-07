/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
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
	class BoxCollider: public Collider
	{
		DECLARE_COM_CLASS(BoxCollider, Collider);

	public:
		BoxCollider():
			m_center(0, 0, 0),
			m_size(1, 1, 1)
		{
		}
		const Vector3& GetCenter() const { return m_center; }
		void SetCenter(const Vector3& center);
		const Vector3& GetSize() const { return m_size; }
		void SetSize(const Vector3& size);

	protected:
		virtual void Start();
		virtual void Update();
		virtual void OnTranformChanged();

	private:
		Vector3 m_center;
		Vector3 m_size;
	};
}
