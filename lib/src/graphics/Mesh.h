#pragma once

#include "Object.h"
#include "Color.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix4x4.h"

namespace Viry3D
{
	class GameObject;

	class Mesh : public Object
	{
	public:
		static Ref<Mesh> Create(bool dynamic = false);

		void Update();
		const Ref<VertexBuffer>& GetVertexBuffer() const { return m_vertex_buffer; }
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_index_buffer; }
		void GetIndexRange(int submesh_index, int& start, int& count);
		bool IsDynamic() const { return m_dynamic; }

		Vector<Vector3> vertices;
		Vector<Vector2> uv;				//Texture
		Vector<Color> colors;			//UI
		Vector<Vector2> uv2;			//Lightmap
		Vector<Vector3> normals;		//Light
		Vector<Vector4> tangents;		//NormalMap
		Vector<Vector4> bone_weights;
		Vector<Vector4> bone_indices;	//Skinned

		struct Submesh
		{
			int start;
			int count;
		};

		Vector<unsigned short> triangles;
		Vector<Submesh> submeshes;
		Vector<Matrix4x4> bind_poses;

	private:
		static void FillVertexBuffer(void* param, const ByteBuffer& buffer);
		static void FillIndexBuffer(void* param, const ByteBuffer& buffer);

		Mesh();
		void UpdateVertexBuffer();
		void UpdateIndexBuffer();
		int VertexBufferSize() const;
		int IndexBufferSize() const;

		bool m_dynamic;
		Ref<VertexBuffer> m_vertex_buffer;
		Ref<IndexBuffer> m_index_buffer;
	};
}