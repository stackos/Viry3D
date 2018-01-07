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

//#define HDR

using UnityEngine;
using UnityEditor;

public class PBRGen : MonoBehaviour
{
#if HDR
	const RenderTextureFormat render_texture_format = RenderTextureFormat.ARGBHalf;
	const TextureFormat texture_format = TextureFormat.RGBAHalf;
#else
	const RenderTextureFormat render_texture_format = RenderTextureFormat.ARGB32;
	const TextureFormat texture_format = TextureFormat.RGBA32;
#endif

	public Cubemap m_cube_map;

	void Start()
	{
		genIrradiance();
		genBRDF();
		genPrefilter();
	}

	void genIrradiance()
	{
		var camera = new GameObject().AddComponent<Camera>();
		camera.clearFlags = CameraClearFlags.Color;
		camera.backgroundColor = Color.black;
		camera.fieldOfView = 90;
		camera.nearClipPlane = 0.1f;
		camera.farClipPlane = 10;
		camera.useOcclusionCulling = false;
		camera.allowMSAA = false;
		camera.allowHDR = true;

		var cube_mat = new Material(Shader.Find("irradiance"));
		cube_mat.SetTexture("_CubeMap", m_cube_map);

		var cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
		cube.transform.localScale = Vector3.one * 2;
		cube.GetComponent<MeshRenderer>().sharedMaterial = cube_mat;

		int size = 32;

		var irradiance = new Cubemap(size, texture_format, false);
		camera.RenderToCubemap(irradiance);

		GameObject.DestroyImmediate(cube);
		GameObject.DestroyImmediate(camera.gameObject);

		AssetDatabase.CreateAsset(irradiance, "Assets/AppPBR/irradiance.asset");
	}

	void genBRDF()
	{
		RenderTexture old_rt = RenderTexture.active;

		int size = 512;

		var rt = new RenderTexture(size, size, 0, render_texture_format);
		Graphics.Blit(null, rt, new Material(Shader.Find("brdf")), 0);

		var brdf = new Texture2D(size, size, texture_format, false);
		brdf.wrapMode = TextureWrapMode.Clamp;
		brdf.filterMode = FilterMode.Bilinear;

		RenderTexture.active = rt;
		brdf.ReadPixels(new Rect(0, 0, size, size), 0, 0);

		AssetDatabase.CreateAsset(brdf, "Assets/AppPBR/brdf.asset");

		RenderTexture.active = old_rt;
	}

	void genPrefilter()
	{
		RenderTexture old_rt = RenderTexture.active;

		int size = m_cube_map.width;

		var prefilter = new Cubemap(size, texture_format, true);
		prefilter.filterMode = FilterMode.Trilinear;

		int size_mip = size;

		for (int i = 0; i < m_cube_map.mipmapCount; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				var ps = m_cube_map.GetPixels((CubemapFace) j, i);
				var tex = new Texture2D(size_mip, size_mip, texture_format, false);
				tex.SetPixels(ps);
				tex.Apply();

				var rt = RenderTexture.GetTemporary(size_mip, size_mip, 0, render_texture_format);
				var mat = new Material(Shader.Find("prefilter"));
				mat.mainTexture = tex;
				Graphics.Blit(tex, rt, mat, 0);
				RenderTexture.active = rt;
				tex.ReadPixels(new Rect(0, 0, size_mip, size_mip), 0, 0);
				RenderTexture.ReleaseTemporary(rt);

				prefilter.SetPixels(tex.GetPixels(), (CubemapFace) j, i);
			}

			size_mip >>= 1;
		}

		AssetDatabase.CreateAsset(prefilter, "Assets/AppPBR/prefilter.asset");

		RenderTexture.active = old_rt;
	}
}
