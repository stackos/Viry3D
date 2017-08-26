#pragma once

#include "string/String.h"
#include "container/Vector.h"
#include "math/Vector3.h"
#include "math/Vector2.h"
#include "math/Vector4.h"
#include "graphics/Color.h"

namespace Viry3D
{
	struct XMLPass
	{
		String name;
		String vs;
		String ps;
		String rs;
	};

	struct XMLUniform
	{
		String name;
		int offset;
		int size;

		XMLUniform():
			offset(0),
			size(0)
		{
		}
	};

	struct XMLUniformBuffer
	{
		String name;
		int binding;
		Vector<XMLUniform> uniforms;
		int size;
		int offset;

		XMLUniformBuffer():
			binding(-1),
			size(0),
			offset(0)
		{
		}
	};

	struct XMLSampler
	{
		String name;
		int binding;
		String default_tex;

		XMLSampler():
			binding(-1),
			default_tex("white")
		{
		}
	};

	struct VertexAttributeType
	{
		enum Enum
		{
			None = -1,

			Vertex = 0,
			Color,
			Texcoord,
			Texcoord2,
			Normal,
			Tangent,
			BlendWeight,
			BlendIndices,

			Count
		};
	};

	struct Vertex
	{
		Vector3 vertex;
		Color color;
		Vector2 uv;
		Vector2 uv2;
		Vector3 normal;
		Vector4 tangent;
		Vector4 bone_weight;
		Vector4 bone_indices;
	};

	extern const char* VERTEX_ATTR_TYPES[VertexAttributeType::Count];
	extern const int VERTEX_ATTR_SIZES[VertexAttributeType::Count];
	extern const int VERTEX_ATTR_OFFSETS[VertexAttributeType::Count];
	extern const int VERTEX_STRIDE;

	struct XMLVertexAttribute
	{
		String name;
		VertexAttributeType::Enum type;
		int location;

		int size;
		int offset;

		XMLVertexAttribute():
			type(VertexAttributeType::None),
			location(-1),
			size(0),
			offset(0)
		{
		}
	};

	struct XMLVertexShader
	{
		String name;
		Vector<String> includes;
		String src;
		XMLUniformBuffer uniform_buffer;
		Vector<XMLVertexAttribute> attrs;
		int stride;

		XMLVertexShader():
			stride(0)
		{
		}
	};

	struct XMLPixelShader
	{
		String name;
		Vector<String> includes;
		String src;
		XMLUniformBuffer uniform_buffer;
		Vector<XMLSampler> samplers;
	};

	struct XMLBlend
	{
		bool enable;
		String src;
		String dst;
		String src_a;
		String dst_a;

		XMLBlend():
			enable(false)
		{
		}
	};

	struct XMLOffset
	{
		bool enable;
		float factor;
		float units;

		XMLOffset():
			enable(false),
			factor(0),
			units(0)
		{
		}
	};

	struct XMLStencil
	{
		bool enable;
		int RefValue;
		int ReadMask;
		int WriteMask;
		String Comp;
		String Pass;
		String Fail;
		String ZFail;

		XMLStencil():
			enable(false),
			RefValue(0),
			ReadMask(255),
			WriteMask(255),
			Comp("Always"),
			Pass("Keep"),
			Fail("Keep"),
			ZFail("Keep")
		{
		}
	};

	struct XMLRenderState
	{
		String name;
		String Cull;
		String ZTest;
		String ZWrite;
		String AlphaTest;	//alpha test暂时没用，需要该功能可在fragment shader使用discard/clip实现
		XMLBlend Blend;
		String ColorMask;
		XMLOffset Offset;
		XMLStencil Stencil;

		XMLRenderState():
			Cull("Back"),
			ZTest("LEqual"),
			ZWrite("On"),
			AlphaTest("Off"),
			ColorMask("RGBA")
		{
		}
	};

	struct XMLShader
	{
		String name;
		int queue;

		Vector<XMLPass> passes;
		Vector<XMLVertexShader> vss;
		Vector<XMLPixelShader> pss;
		Vector<XMLRenderState> rss;

		void Load(String path);
		void Clear();
	};
}