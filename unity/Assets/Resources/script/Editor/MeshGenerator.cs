using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

public class MeshGenerator
{
    [MenuItem("Viry3D/Create Mesh Cone")]
    static void CreateMeshCone()
    {
        string path = EditorUtility.SaveFilePanelInProject("Save to", "cone", "asset", "Save mesh asset to");

        List<Vector3> vertices = new List<Vector3>();
        List<int> indices = new List<int>();

        vertices.Add(new Vector3(0, 0, 0));

        const int circle_point_count = 43;

        for (int i = 0; i < circle_point_count; ++i)
        {
            float rad = Mathf.PI * 2 / circle_point_count * i;
            Vector3 p0 = new Vector3(Mathf.Cos(rad) * 0.5f, 0, Mathf.Sin(rad) * 0.5f);
            
            vertices.Add(p0);
            vertices.Add(p0);
            vertices.Add(new Vector3(0, 1, 0));
        }

        for (int i = 0; i < circle_point_count; ++i)
        {
            int i0, i1;

            if (i == 0)
            {
                i0 = circle_point_count - 1;
                i1 = i;
            }
            else
            {
                i0 = i - 1;
                i1 = i;
            }

            indices.Add(0); indices.Add(1 + i0 * 3 + 0); indices.Add(1 + i1 * 3 + 0);
            indices.Add(1 + i1 * 3 + 2); indices.Add(1 + i1 * 3 + 1); indices.Add(1 + i0 * 3 + 1);
        }

        Mesh mesh = new Mesh();
        mesh.vertices = vertices.ToArray();
        mesh.triangles = indices.ToArray();
        mesh.RecalculateNormals();

        AssetDatabase.CreateAsset(mesh, path);
    }
}
