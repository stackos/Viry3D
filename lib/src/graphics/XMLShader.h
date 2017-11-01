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

#include "string/String.h"
#include "container/Vector.h"
#include "VertexAttribute.h"

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
		String type; // 2D, Cube
		int binding;
		String default_tex;

		XMLSampler():
			type("2D"),
			binding(-1),
			default_tex("white")
		{
		}
	};

	struct XMLVertexAttribute
	{
		String name;
		VertexAttributeType type;
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

		void Load(const String& path);
		void Clear();
	};
}
