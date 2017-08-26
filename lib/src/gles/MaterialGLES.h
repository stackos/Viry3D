#pragma once

#include "Object.h"

namespace Viry3D
{
	class Texture;
	class UniformBuffer;

	class MaterialGLES : public Object
	{
	public:
		void Apply(int pass_index);
		void ApplyLightmap(int pass_index, int lightmap_index);

	protected:
		void UpdateUniformsBegin(int pass_index);
		void UpdateUniformsEnd(int pass_index) { }
		void* SetUniformBegin(int pass_index);
		void SetUniformEnd(int pass_index);
		void SetUniform(int pass_index, void* uniform_buffer, String name, void* data, int size);
		void SetUniformTexture(int pass_index, String name, const Texture* texture) { }

	private:
		Vector<Ref<UniformBuffer>> m_uniform_buffers;
	};
}
