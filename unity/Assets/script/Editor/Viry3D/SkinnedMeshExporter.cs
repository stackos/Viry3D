using UnityEngine;
using UnityEditor;
using System.IO;
using System.Collections.Generic;

public partial class Exporter {
	static void WriteSkinnedMeshRenderer(SkinnedMeshRenderer renderer) {
		var mesh = renderer.sharedMesh;
        if(mesh != null) {
			WriteMesh(mesh);
		} else {
			WriteString("");
		}

		WriteRendererBounds(renderer);
		WriteRendererMaterials(renderer);

		var bones = renderer.bones;
        int bone_count = bones.Length;

        m_writer.Write(bone_count);

        for(int i = 0; i < bone_count; i++) {
            m_writer.Write(bones[i].GetInstanceID());
        }
    }

	static void WriteAnimation(Animation anim) {
        var clips = AnimationUtility.GetAnimationClips(anim.gameObject);
        int clip_count = clips.Length;
        var defaul_clip = anim.clip;

        if(defaul_clip != null) {
            WriteString(defaul_clip.name);
        } else {
            WriteString("");
        }

        m_writer.Write(clip_count);

        for(int i = 0; i < clip_count; i++) {
            var clip = clips[i];

            if(clip != null) {
                WriteAnimationClip(clip);
            } else {
                WriteString("");
            }
        }
    }

	static void WriteAnimationClip(AnimationClip clip) {
        var path = AssetDatabase.GetAssetPath(clip) + "." + clip.name + ".clip";

        WriteString(path);

        if(m_cache.ContainsKey(path)) {
            return;
        } else {
            m_cache.Add(path, clip);
        }

        var save = m_writer;
        var ms = new MemoryStream();
        m_writer = new BinaryWriter(ms);

        var bindings = AnimationUtility.GetCurveBindings(clip);
        int binding_count = bindings.Length;

        WriteString(clip.name);
        m_writer.Write(clip.frameRate);
        m_writer.Write(clip.length);
        m_writer.Write((int) clip.wrapMode);
        m_writer.Write(binding_count);

        for(int i = 0; i < binding_count; i++) {
            var binding = bindings[i];
            WriteString(binding.path);
            WriteString(binding.propertyName);

			var curve = AnimationUtility.GetEditorCurve(clip, binding);
			WriteAnimationCurve(curve);
        }

        var file_path = new FileInfo(m_out_path + "/" + path);
        if(!file_path.Directory.Exists) {
            Directory.CreateDirectory(file_path.Directory.FullName);
        }
        File.WriteAllBytes(file_path.FullName, ms.ToArray());

        m_writer.Close();
        m_writer = save;
    }

	static void WriteAnimationCurve(AnimationCurve curve) {
		var keys = curve.keys;
		int frame_count = keys.Length;
		m_writer.Write((int) curve.preWrapMode);
		m_writer.Write((int) curve.postWrapMode);
		m_writer.Write(frame_count);

		for(int j = 0; j < frame_count; j++) {
			var frame = keys[j];

			m_writer.Write(frame.time);
			m_writer.Write(frame.value);
			m_writer.Write(frame.inTangent);
			m_writer.Write(frame.outTangent);
			m_writer.Write(frame.tangentMode);
		}
	}

	static void SplitMesh(SkinnedMeshRenderer renderer, int split_index, int index_start, int index_count) {
        var mesh = renderer.sharedMesh;

        var split = new GameObject(renderer.gameObject.name + "_split_" + split_index);
        split.transform.parent = renderer.transform;
        split.transform.localPosition = Vector3.zero;
        split.transform.localRotation = Quaternion.identity;
        split.transform.localScale = Vector3.one;

        var split_skin = split.AddComponent<SkinnedMeshRenderer>();
        split_skin.sharedMaterials = renderer.sharedMaterials;

        if(mesh.subMeshCount == 1) {
            var split_mesh = new Mesh();
            split_mesh.name = mesh.name + "_split_" + split_index;
            split_mesh.vertices = mesh.vertices;
            split_mesh.uv = mesh.uv;
            split_mesh.colors = mesh.colors;
            split_mesh.uv2 = mesh.uv2;
            split_mesh.normals = mesh.normals;
            split_mesh.tangents = mesh.tangents;
            split_mesh.subMeshCount = 1;
            var boneWeights = mesh.boneWeights;
            var bone_indices = new Dictionary<int, int>();
            var triangles = mesh.GetTriangles(0);

            var split_triangles = new int[index_count];
            for(int i = 0; i < split_triangles.Length; i++) {
                split_triangles[i] = triangles[index_start + i];

                int index = split_triangles[i];
                if(!bone_indices.ContainsKey(boneWeights[index].boneIndex0) && boneWeights[index].weight0 > 0) {
                    int bone_index = bone_indices.Count;
                    bone_indices.Add(boneWeights[index].boneIndex0, bone_index);
                }
                if(!bone_indices.ContainsKey(boneWeights[index].boneIndex1) && boneWeights[index].weight1 > 0) {
                    int bone_index = bone_indices.Count;
                    bone_indices.Add(boneWeights[index].boneIndex1, bone_index);
                }
                if(!bone_indices.ContainsKey(boneWeights[index].boneIndex2) && boneWeights[index].weight2 > 0) {
                    int bone_index = bone_indices.Count;
                    bone_indices.Add(boneWeights[index].boneIndex2, bone_index);
                }
                if(!bone_indices.ContainsKey(boneWeights[index].boneIndex3) && boneWeights[index].weight3 > 0) {
                    int bone_index = bone_indices.Count;
                    bone_indices.Add(boneWeights[index].boneIndex3, bone_index);
                }
            }

            var new_weights = new BoneWeight[boneWeights.Length];
            for(int i = 0; i < new_weights.Length; i++) {
                if(bone_indices.ContainsKey(boneWeights[i].boneIndex0) && boneWeights[i].weight0 > 0) {
                    new_weights[i].boneIndex0 = bone_indices[boneWeights[i].boneIndex0];
                    new_weights[i].weight0 = boneWeights[i].weight0;
                }
                if(bone_indices.ContainsKey(boneWeights[i].boneIndex1) && boneWeights[i].weight1 > 0) {
                    new_weights[i].boneIndex1 = bone_indices[boneWeights[i].boneIndex1];
                    new_weights[i].weight1 = boneWeights[i].weight1;
                }
                if(bone_indices.ContainsKey(boneWeights[i].boneIndex2) && boneWeights[i].weight2 > 0) {
                    new_weights[i].boneIndex2 = bone_indices[boneWeights[i].boneIndex2];
                    new_weights[i].weight2 = boneWeights[i].weight2;
                }
                if(bone_indices.ContainsKey(boneWeights[i].boneIndex3) && boneWeights[i].weight3 > 0) {
                    new_weights[i].boneIndex3 = bone_indices[boneWeights[i].boneIndex3];
                    new_weights[i].weight3 = boneWeights[i].weight3;
                }
            }

            var bindposes = mesh.bindposes;
            var bones = renderer.bones;

            var split_bindposes = new Matrix4x4[bone_indices.Count];
            for(int i = 0; i < bindposes.Length; i++) {
                if(bone_indices.ContainsKey(i)) {
                    int new_index = bone_indices[i];
                    split_bindposes[new_index] = bindposes[i];
                }
            }

            var split_bones = new Transform[bone_indices.Count];
            for(int i = 0; i < bones.Length; i++) {
                if(bone_indices.ContainsKey(i)) {
                    int new_index = bone_indices[i];
                    split_bones[new_index] = bones[i];
                }
            }

            split_mesh.boneWeights = new_weights;
            split_mesh.SetTriangles(split_triangles, 0);
            split_mesh.bindposes = split_bindposes;

            split_skin.bones = split_bones;
            split_skin.sharedMesh = split_mesh;

            var path = AssetDatabase.GetAssetPath(mesh) + "." + split_mesh.name + ".asset";
            AssetDatabase.CreateAsset(split_mesh, path);
        } else {
            Debug.LogError("spliting mesh sub mesh count more than 1");
        }
    }

    static void SplitSkinnedMeshRenderer(SkinnedMeshRenderer renderer) {
        var mesh = renderer.sharedMesh;
        if(mesh == null) {
            return;
        }

        var triangles = mesh.GetTriangles(0);
        int count_0 = triangles.Length / 3 / 2 * 3;
        SplitMesh(renderer, 0, 0, count_0);
        SplitMesh(renderer, 1, count_0, triangles.Length - count_0);

        Component.DestroyImmediate(renderer);
    }
}