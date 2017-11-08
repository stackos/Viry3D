using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

public class PBR : MonoBehaviour {
	public Cubemap cube_map;

	void Start () {
		genIrradiance();
		genBRDF();
	}

	void genIrradiance() {
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
		cube_mat.SetTexture("_CubeMap", cube_map);

		var cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
		cube.transform.localScale = Vector3.one * 2;
		cube.GetComponent<MeshRenderer>().sharedMaterial = cube_mat;

		var irradiance = new Cubemap(32, TextureFormat.RGBAHalf, false);
		camera.RenderToCubemap(irradiance);

		GameObject.DestroyImmediate(cube);
		GameObject.DestroyImmediate(camera.gameObject);

		AssetDatabase.CreateAsset(irradiance, "Assets/AppPBR/irradiance.asset");
	}

	void genBRDF() {
		var camera = new GameObject().AddComponent<Camera>();
		camera.clearFlags = CameraClearFlags.Color;
		camera.backgroundColor = Color.black;
		camera.orthographic = true;
		camera.orthographicSize = 1;
		camera.nearClipPlane = -1;
		camera.farClipPlane = 1;
		camera.useOcclusionCulling = false;
		camera.allowMSAA = false;
		camera.allowHDR = true;

		var quad = GameObject.CreatePrimitive(PrimitiveType.Quad);
		quad.transform.localScale = Vector3.one * 2;
		quad.GetComponent<MeshRenderer>().sharedMaterial = new Material(Shader.Find("brdf"));

		var brdf_cube = new Cubemap(512, TextureFormat.RGBAHalf, false);
		camera.RenderToCubemap(brdf_cube);

		GameObject.DestroyImmediate(quad);
		GameObject.DestroyImmediate(camera.gameObject);

		var pixels = brdf_cube.GetPixels(CubemapFace.PositiveZ, 0);
		var brdf = new Texture2D(512, 512, TextureFormat.RGBAHalf, false);
		brdf.wrapMode = TextureWrapMode.Clamp;
		brdf.filterMode = FilterMode.Bilinear;
		brdf.SetPixels(pixels);

		AssetDatabase.CreateAsset(brdf, "Assets/AppPBR/brdf.asset");
	}

	void Update () {
		
	}
}
