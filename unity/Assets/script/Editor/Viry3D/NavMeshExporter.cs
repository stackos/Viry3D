using UnityEditor;
using UnityEngine;
using UnityEngine.AI;
using System.IO;
using System.Collections.Generic;

public class NavMeshExporter : ExporterBase {
	[MenuItem("Viry3D/Show NavMesh")]
	public static void ShowNavMesh() {
		var data = NavMesh.CalculateTriangulation();

		var mesh = new Mesh();
		mesh.vertices = data.vertices;
		mesh.triangles = data.indices;

		var obj = new GameObject("NavMesh");
		obj.AddComponent<MeshFilter>().sharedMesh = mesh;

		var renderer = obj.AddComponent<MeshRenderer>();
		renderer.sharedMaterial = new Material(Shader.Find("Unlit/Color"));
		renderer.sharedMaterial.color = Color.red;
	}
}