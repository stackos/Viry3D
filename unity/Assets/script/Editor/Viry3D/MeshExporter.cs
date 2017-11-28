using UnityEditor;
using UnityEngine;
using System.IO;
using System.Collections.Generic;

public class MeshExporter : ExporterBase {
	protected static void WriteMeshRenderer(MeshRenderer renderer) {
		var filter = renderer.GetComponent<MeshFilter>();
		if(filter != null && filter.sharedMesh != null) {
			WriteMesh(filter.sharedMesh);
		} else {
			WriteString("");
		}

		WriteRendererBounds(renderer);
		WriteRendererMaterials(renderer);

		m_writer.Write(renderer.lightmapIndex);
		WriteVector4(renderer.lightmapScaleOffset);
	}

	protected static void WriteRendererBounds(Renderer renderer) {
		WriteVector3(renderer.bounds.min);
		WriteVector3(renderer.bounds.max);
	}

	protected static void WriteRendererMaterials(Renderer renderer) {
		var mats = renderer.sharedMaterials;

		m_writer.Write(mats.Length);

		for(int i = 0; i < mats.Length; i++) {
			if(mats[i] != null) {
				WriteMaterial(mats[i]);
			} else {
				WriteString("");
			}
		}
	}

    protected static void WriteMesh(Mesh mesh) {
        var path = AssetDatabase.GetAssetPath(mesh) + "." + mesh.name + ".mesh";
        if(!path.StartsWith("Assets/")) {
            path = "Assets/" + path;
        }

        WriteString(path);

		if(m_cache.ContainsKey(path)) {
			return;
		} else {
			m_cache.Add(path, mesh);
		}

        var save = m_writer;
        var ms = new MemoryStream();
        m_writer = new BinaryWriter(ms);

        WriteString(mesh.name);

        var vertices = mesh.vertices;
        m_writer.Write(vertices.Length);
		for(int i = 0; i < vertices.Length; i++) {
			WriteVector3(vertices[i]);
		}

        var uvs = mesh.uv;
        m_writer.Write(uvs.Length);
		for(int i = 0; i < uvs.Length; i++) {
            var uv = uvs[i];
            uv.y = 1.0f - uv.y;
            WriteVector2(uv);
		}

        var colors = mesh.colors;
        m_writer.Write(colors.Length);
		for(int i = 0; i < colors.Length; i++) {
			WriteColor(colors[i]);
		}

        var uv2s = mesh.uv2;
        m_writer.Write(uv2s.Length);
		for(int i = 0; i < uv2s.Length; i++) {
            var uv2 = uv2s[i];
            uv2.y = 1.0f - uv2.y;
            WriteVector2(uv2);
		}

        var normals = mesh.normals;
        m_writer.Write(normals.Length);
		for(int i = 0; i < normals.Length; i++) {
			WriteVector3(normals[i]);
		}

        var tangents = mesh.tangents;
        m_writer.Write(tangents.Length);
		for(int i = 0; i < tangents.Length; i++) {
			WriteVector4(tangents[i]);
		}

        var boneWeights = mesh.boneWeights;
        m_writer.Write(boneWeights.Length);
		for(int i = 0; i < boneWeights.Length; i++) {
			m_writer.Write(boneWeights[i].weight0);
			m_writer.Write(boneWeights[i].weight1);
			m_writer.Write(boneWeights[i].weight2);
			m_writer.Write(boneWeights[i].weight3);
		}

		m_writer.Write(boneWeights.Length);
		for(int i = 0; i < boneWeights.Length; i++) {
			m_writer.Write((float) boneWeights[i].boneIndex0);
			m_writer.Write((float) boneWeights[i].boneIndex1);
			m_writer.Write((float) boneWeights[i].boneIndex2);
			m_writer.Write((float) boneWeights[i].boneIndex3);
		}

        var bindposes = mesh.bindposes;
        m_writer.Write(bindposes.Length);
        for(int i = 0; i < bindposes.Length; i++) {
            var m = bindposes[i];

            for(int j = 0; j < 16; j++) {
                m_writer.Write(m[j / 4, j % 4]);
            }
        }

        if(mesh.subMeshCount == 1) {
			var triangles = mesh.GetTriangles(0);

			m_writer.Write(triangles.Length);
            int tri_count = triangles.Length / 3;
            for(int i = 0; i < tri_count; i++) {
				m_writer.Write((ushort) triangles[i * 3 + 0]);
                m_writer.Write((ushort) triangles[i * 3 + 2]);
                m_writer.Write((ushort) triangles[i * 3 + 1]);
            }

			m_writer.Write(0);
		} else {
			List<int> triangle_list = new List<int>();
			List<int> start = new List<int>();
			List<int> count = new List<int>();

			for(int i = 0; i < mesh.subMeshCount; i++) {
				start.Add(triangle_list.Count);
				
				var sub_triangles = mesh.GetTriangles(i);
				triangle_list.AddRange(sub_triangles);

				count.Add(sub_triangles.Length);
			}

			var triangles = triangle_list.ToArray();

			m_writer.Write(triangles.Length);
            int tri_count = triangles.Length / 3;
            for(int i = 0; i < tri_count; i++) {
                m_writer.Write((ushort) triangles[i * 3 + 0]);
                m_writer.Write((ushort) triangles[i * 3 + 2]);
                m_writer.Write((ushort) triangles[i * 3 + 1]);
            }

            m_writer.Write(mesh.subMeshCount);
			for(int i = 0; i < mesh.subMeshCount; i++) {
				m_writer.Write(start[i]);
				m_writer.Write(count[i]);
			}
		}

		int blend_shape_count = mesh.blendShapeCount;
		m_writer.Write(blend_shape_count);
		if (blend_shape_count > 0)
		{
			Vector3[] delta_vertices = new Vector3[mesh.vertexCount];
			Vector3[] delta_normals = new Vector3[mesh.vertexCount];
			Vector3[] delta_tangents = new Vector3[mesh.vertexCount];

			for (int i = 0; i < blend_shape_count; i++)
			{
				var name = mesh.GetBlendShapeName(i);
				WriteString(name);

				int frame_count = mesh.GetBlendShapeFrameCount(i);
				m_writer.Write(frame_count);
				for (int j = 0; j < frame_count; j++)
				{
					float weight = mesh.GetBlendShapeFrameWeight(i, j);
					m_writer.Write(weight);

					mesh.GetBlendShapeFrameVertices(i, j, delta_vertices, delta_normals, delta_tangents);
					for (int k = 0; k < mesh.vertexCount; k++)
					{
						WriteVector3(delta_vertices[k]);
						WriteVector3(delta_normals[k]);
						WriteVector3(delta_tangents[k]);
					}
				}
			}
		}

		var file_path = new FileInfo(m_out_path + "/" + path);
        if(!file_path.Directory.Exists) {
            Directory.CreateDirectory(file_path.Directory.FullName);
        }
        File.WriteAllBytes(file_path.FullName, ms.ToArray());

        m_writer.Close();
        m_writer = save;
    }
}