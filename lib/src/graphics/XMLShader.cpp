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

#include "XMLShader.h"
#include "io/File.h"
#include "xml/tinyxml2.h"
#include "RenderQueue.h"
#include "Debug.h"

namespace Viry3D
{
#define try_get_attribute(out, ele, attr_name) \
	{ \
		auto a = ele->Attribute(attr_name); \
		if(a != NULL) { \
			out = a; \
		} \
	}

#define try_get_attribute_to_type(out, ele, attr_name, type) \
	{ \
		auto a = ele->Attribute(attr_name); \
		if(a != NULL) { \
			out = String(a).To<type>(); \
		} \
	}

	const char* VERTEX_ATTR_TYPES[(int) VertexAttributeType::Count] =
	{
		"Vertex",
		"Color",
		"Texcoord",
		"Texcoord2",
		"Normal",
		"Tangent",
		"BlendWeight",
		"BlendIndices"
	};

	const int VERTEX_ATTR_SIZES[(int) VertexAttributeType::Count] = {
		12, 16, 8, 8, 12, 16, 16, 16
	};

	const int VERTEX_ATTR_OFFSETS[(int) VertexAttributeType::Count] =
	{
		0, 12, 28, 36, 44, 56, 72, 88
	};

	void XMLShader::Clear()
	{
		passes.Clear();
		vss.Clear();
		pss.Clear();
		rss.Clear();
	}

	void XMLShader::Load(const String& path)
	{
		Clear();

		auto bytes = File::ReadAllBytes(path);
		tinyxml2::XMLDocument doc;
		auto error = doc.Parse((const char*) bytes.Bytes(), bytes.Size());
		if (error == tinyxml2::XML_SUCCESS)
		{
			auto shader_ele = doc.FirstChildElement();
			String shader_queue;

			try_get_attribute(this->name, shader_ele, "name");
			try_get_attribute(shader_queue, shader_ele, "queue");

			if (shader_queue == "Background")
			{
				this->queue = (int) RenderQueue::Background;
			}
			else if (shader_queue == "Geometry")
			{
				this->queue = (int) RenderQueue::Geometry;
			}
			else if (shader_queue == "AlphaTest")
			{
				this->queue = (int) RenderQueue::AlphaTest;
			}
			else if (shader_queue == "Transparent")
			{
				this->queue = (int) RenderQueue::Transparent;
			}
			else if (shader_queue == "Overlay")
			{
				this->queue = (int) RenderQueue::Overlay;
			}

			for (auto node = shader_ele->FirstChildElement(); node; node = node->NextSiblingElement())
			{
				String type = node->Name();
				String name;

				try_get_attribute(name, node, "name");

				if (type == "Pass")
				{
					XMLPass pass;
					pass.name = name;

					try_get_attribute(pass.vs, node, "vs");
					try_get_attribute(pass.ps, node, "ps");
					try_get_attribute(pass.rs, node, "rs");

					passes.Add(pass);
				}
				else if (type == "VertexShader")
				{
					XMLVertexShader vs;
					vs.name = name;
					vs.uniform_buffer.binding = -1;
					vs.uniform_buffer.size = 0;

					for (auto vs_node = node->FirstChildElement(); vs_node; vs_node = vs_node->NextSiblingElement())
					{
						String vs_type = vs_node->Name();

						if (vs_type == "UniformBuffer")
						{
							try_get_attribute(vs.uniform_buffer.name, vs_node, "name");
							try_get_attribute_to_type(vs.uniform_buffer.binding, vs_node, "binding", int);

							int offset = 0;
							for (auto ub_node = vs_node->FirstChildElement(); ub_node; ub_node = ub_node->NextSiblingElement())
							{
								String ub_type = ub_node->Name();

								if (ub_type == "Uniform")
								{
									XMLUniform uniform;
									try_get_attribute(uniform.name, ub_node, "name");
									try_get_attribute_to_type(uniform.size, ub_node, "size", int);

									uniform.offset = offset;
									offset += uniform.size;

									vs.uniform_buffer.uniforms.Add(uniform);
								}
							}

							vs.uniform_buffer.size = offset;
						}
						else if (vs_type == "Include")
						{
							String include;
							try_get_attribute(include, vs_node, "name");
							vs.includes.Add(include);
						}
						else if (vs_type == "Source")
						{
							vs.src = vs_node->GetText();
						}
						else if (vs_type == "VertexAttribute")
						{
							XMLVertexAttribute attr;
							try_get_attribute(attr.name, vs_node, "name");
							try_get_attribute_to_type(attr.location, vs_node, "location", int);

							for (int i = 0; i < (int) VertexAttributeType::Count; i++)
							{
								if (attr.name == VERTEX_ATTR_TYPES[i])
								{
									attr.type = (VertexAttributeType) i;
									attr.size = VERTEX_ATTR_SIZES[i];
									attr.offset = VERTEX_ATTR_OFFSETS[i];
									break;
								}
							}

							vs.attrs.Add(attr);
						}
					}

					vs.stride = sizeof(Vertex);

					vss.Add(vs);
				}
				else if (type == "PixelShader")
				{
					XMLPixelShader ps;
					ps.name = name;
					ps.uniform_buffer.binding = -1;
					ps.uniform_buffer.size = 0;

					for (auto ps_node = node->FirstChildElement(); ps_node; ps_node = ps_node->NextSiblingElement())
					{
						String ps_type = ps_node->Name();

						if (ps_type == "UniformBuffer")
						{
							try_get_attribute(ps.uniform_buffer.name, ps_node, "name");
							try_get_attribute_to_type(ps.uniform_buffer.binding, ps_node, "binding", int);

							int offset = 0;
							for (auto ub_node = ps_node->FirstChildElement(); ub_node; ub_node = ub_node->NextSiblingElement())
							{
								String ub_type = ub_node->Name();

								if (ub_type == "Uniform")
								{
									XMLUniform uniform;
									try_get_attribute(uniform.name, ub_node, "name");
									try_get_attribute_to_type(uniform.size, ub_node, "size", int);

									uniform.offset = offset;
									offset += uniform.size;

									ps.uniform_buffer.uniforms.Add(uniform);
								}
							}

							ps.uniform_buffer.size = offset;
						}
						else if (ps_type == "Sampler")
						{
							XMLSampler sampler;
							try_get_attribute(sampler.name, ps_node, "name");
							try_get_attribute(sampler.type, ps_node, "type");
							try_get_attribute_to_type(sampler.binding, ps_node, "binding", int);
							try_get_attribute(sampler.default_tex, ps_node, "default");

							ps.samplers.Add(sampler);
						}
						else if (ps_type == "Include")
						{
							String include;
							try_get_attribute(include, ps_node, "name");
							ps.includes.Add(include);
						}
						else if (ps_type == "Source")
						{
							ps.src = ps_node->GetText();
						}
					}

					pss.Add(ps);
				}
				else if (type == "RenderState")
				{
					XMLRenderState rs;
					rs.name = name;

					for (auto rs_node = node->FirstChildElement(); rs_node; rs_node = rs_node->NextSiblingElement())
					{
						String rs_type = rs_node->Name();

						if (rs_type == "Cull")
						{
							try_get_attribute(rs.Cull, rs_node, "value");
						}
						else if (rs_type == "ZTest")
						{
							try_get_attribute(rs.ZTest, rs_node, "value");
						}
						else if (rs_type == "ZWrite")
						{
							try_get_attribute(rs.ZWrite, rs_node, "value");
						}
						else if (rs_type == "AlphaTest")
						{
							try_get_attribute(rs.AlphaTest, rs_node, "value");
						}
						else if (rs_type == "ColorMask")
						{
							try_get_attribute(rs.ColorMask, rs_node, "value");
						}
						else if (rs_type == "Blend")
						{
							rs.Blend.enable = true;
							String enable;
							try_get_attribute(enable, rs_node, "enable");
							if (enable == "Off")
							{
								rs.Blend.enable = false;
							}
							if (rs.Blend.enable)
							{
								try_get_attribute(rs.Blend.src, rs_node, "src");
								try_get_attribute(rs.Blend.dst, rs_node, "dst");
								try_get_attribute(rs.Blend.src_a, rs_node, "src_a");
								try_get_attribute(rs.Blend.dst_a, rs_node, "dst_a");

								if (rs.Blend.src_a.Empty())
								{
									rs.Blend.src_a = rs.Blend.src;
								}
								if (rs.Blend.dst_a.Empty())
								{
									rs.Blend.dst_a = rs.Blend.dst;
								}
							}
						}
						else if (rs_type == "Offset")
						{
							rs.Offset.enable = true;
							try_get_attribute_to_type(rs.Offset.factor, rs_node, "factor", float);
							try_get_attribute_to_type(rs.Offset.units, rs_node, "units", float);
						}
						else if (rs_type == "Stencil")
						{
							rs.Stencil.enable = true;
							try_get_attribute_to_type(rs.Stencil.RefValue, rs_node, "RefValue", int);
							try_get_attribute_to_type(rs.Stencil.ReadMask, rs_node, "ReadMask", int);
							try_get_attribute_to_type(rs.Stencil.WriteMask, rs_node, "WriteMask", int);
							try_get_attribute(rs.Stencil.Comp, rs_node, "Comp");
							try_get_attribute(rs.Stencil.Pass, rs_node, "Pass");
							try_get_attribute(rs.Stencil.Fail, rs_node, "Fail");
							try_get_attribute(rs.Stencil.ZFail, rs_node, "ZFail");
						}
					}

					rss.Add(rs);
				}
			}
		}
        else
        {
            Log("shader xml parse error, xml path:%s", path.CString());
        }
	}
}
