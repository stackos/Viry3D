using UnityEngine;
using System.Collections.Generic;
using System.Text;

public class AnimCombiner : MonoBehaviour {
	public GameObject body;
	public GameObject[] weapons;

	void Start() {
		Combine(body, weapons);

		Component.Destroy(this);
	}

	public static void Combine(GameObject body, GameObject[] weapons) {
		var anim_body = body.GetComponent<Animation>();
		var mesh_weapons = new List<MeshRenderer>();
		var anim_weapons = new List<Animation>();

		for(int i = 0; i < weapons.Length; i++) {
			var anim = weapons[i].GetComponent<Animation>();
			if(anim != null) {
				anim_weapons.Add(anim);
			}

			var mesh = weapons[i].GetComponent<MeshRenderer>();
			if(mesh != null) {
				mesh_weapons.Add(mesh);
			}
		}

		var renderer_body = body.GetComponentInChildren<SkinnedMeshRenderer>();
		if(renderer_body != null) {
			List<SkinnedMeshRenderer> anims = new List<SkinnedMeshRenderer>();
			anims.Add(renderer_body);

			Vector3 pos = anim_body.transform.position;
			Quaternion rot = anim_body.transform.rotation;

			anim_body.transform.position = Vector3.zero;
			anim_body.transform.rotation = Quaternion.identity;

			for(int i = 0; i < anim_weapons.Count; i++) {
				anim_weapons[i].transform.position = Vector3.zero;
				anim_weapons[i].transform.rotation = Quaternion.identity;

				var anim = CombineAnim(anim_body, anim_weapons[i]);
				if(anim != null) {
					anims.Add(anim);
				}
			}

			CombineMeshs(anims.ToArray(), mesh_weapons.ToArray(), anim_body);

			anim_body.transform.position = pos;
			anim_body.transform.rotation = rot;
		}

		//	let animation knows the new renderer, avoid culling
		body.SetActive(false);
		body.SetActive(true);
	}

	static int AddMat(List<Material> mats, List<int> mats_index, Material m) {
		if(mats.Count == 0) {
			mats.Add(m);
		} else {
			bool add = true;
			for(int j = 0; j < mats.Count; j++) {
				if(m.mainTexture == mats[j].mainTexture) {
					add = false;
					break;
				}
			}

			if(add) {
				if(mats.Count < 2) {
					mats.Add(m);
				}
			}
		}

		int index = mats.Count - 1;
		mats_index.Add(index);

		return index;
	}

	static Color32[] NewColors(int count, byte tex_index) {
		var colors = new Color32[count];
		for(int j = 0; j < count; j++) {
			colors[j] = new Color32(tex_index, 0, 0, 0);
		}
		return colors;
	}

	static void CombineMeshs(SkinnedMeshRenderer[] anims, MeshRenderer[] meshs, Animation root) {
		List<CombineInstance> combines = new List<CombineInstance>();
		List<Transform> bones = new List<Transform>();
		List<Material> mats = new List<Material>();
		List<int> mats_index = new List<int>();

		for(int i = 0; i < anims.Length; i++) {
			var inst = new CombineInstance();
			inst.mesh = anims[i].sharedMesh;
			inst.subMeshIndex = 0;
			inst.transform = anims[i].transform.localToWorldMatrix;

			combines.Add(inst);
			bones.AddRange(anims[i].bones);

			var tex_index = AddMat(mats, mats_index, anims[i].sharedMaterial);
			inst.mesh.colors32 = NewColors(inst.mesh.vertexCount, (byte) tex_index);

			anims[i].gameObject.SetActive(false);
		}

		for(int i = 0; i < meshs.Length; i++) {
			var m = meshs[i].GetComponent<MeshFilter>().mesh;
			var bindposes = new Matrix4x4[1];
			bindposes[0] = Matrix4x4.identity;
			m.bindposes = bindposes;
			var weights = new BoneWeight[m.vertexCount];
			for(int j = 0; j < weights.Length; j++) {
				weights[j].boneIndex0 = 0;
				weights[j].boneIndex1 = 0;
				weights[j].boneIndex2 = 0;
				weights[j].boneIndex3 = 0;
				weights[j].weight0 = 1;
				weights[j].weight1 = 0;
				weights[j].weight2 = 0;
				weights[j].weight3 = 0;
			}
			m.boneWeights = weights;

			var inst = new CombineInstance();
			inst.mesh = m;
			inst.subMeshIndex = 0;
			inst.transform = meshs[i].transform.localToWorldMatrix;

			combines.Add(inst);
			bones.Add(meshs[i].transform.parent);

			var tex_index = AddMat(mats, mats_index, meshs[i].sharedMaterial);
			inst.mesh.colors32 = NewColors(inst.mesh.vertexCount, (byte) tex_index);

			meshs[i].gameObject.SetActive(false);
		}

		Mesh mesh = new Mesh();
		mesh.name = "combined";
		mesh.CombineMeshes(combines.ToArray(), true, false);
		//CombineMeshes(mesh, combines.ToArray());

		var combined = new GameObject("combined").AddComponent<SkinnedMeshRenderer>();
		combined.sharedMesh = mesh;
		combined.transform.parent = root.transform;
		combined.transform.localPosition = Vector3.zero;
		combined.transform.localRotation = Quaternion.identity;
		combined.bones = bones.ToArray();

		Material mat = null;
		if(mats.Count == 1) {
			mat = mats[0];
		} else if(mats.Count == 2) {
			mat = new Material(mats[0]);
			mat.shader = Shader.Find("YuLongZhi/CharacterCombine");

			mat.SetTexture("_MainTex2", mats[1].GetTexture("_MainTex"));
			mat.SetTexture("_Bump2", mats[1].GetTexture("_Bump"));
			mat.SetTexture("_RefMap2", mats[1].GetTexture("_RefMap"));
		} else {
			Debug.LogError("mats count > 2");
		}

		combined.sharedMaterial = mat;
	}

	static void CombineMeshes(Mesh mesh, CombineInstance[] insts) {
		List<int> triangles = new List<int>();
		List<Vector3> vertices = new List<Vector3>();
		List<Vector2> uv = new List<Vector2>();
		List<Vector3> normals = new List<Vector3>();
		List<Color32> colors32 = new List<Color32>();
		List<Matrix4x4> bindposes = new List<Matrix4x4>();
		List<BoneWeight> boneWeights = new List<BoneWeight>();
		List<Vector4> tangents = new List<Vector4>();

		for(int i = 0; i < insts.Length; i++) {
			var m = insts[i].mesh;

			int vertex_count = vertices.Count;
			int bone_count = bindposes.Count;

			vertices.AddRange(m.vertices);
			uv.AddRange(m.uv);
			normals.AddRange(m.normals);
			colors32.AddRange(m.colors32);
			tangents.AddRange(m.tangents);
			bindposes.AddRange(m.bindposes);

			if(i == 0) {
				boneWeights.AddRange(m.boneWeights);
			} else {
				var weights = m.boneWeights;
				for(int j = 0; j < weights.Length; j++) {
					weights[j].boneIndex0 += bone_count;
				}
				boneWeights.AddRange(weights);
			}

			if(i == 0) {
				triangles.AddRange(m.triangles);
			} else {
				var indices = m.triangles;
				for(int j = 0; j < indices.Length; j++) {
					indices[j] += vertex_count;
				}

				triangles.AddRange(indices);
			}
		}

		mesh.vertices = vertices.ToArray();
		mesh.uv = uv.ToArray();
		mesh.normals = normals.ToArray();
		mesh.colors32 = colors32.ToArray();
		mesh.tangents = tangents.ToArray();
		mesh.bindposes = bindposes.ToArray();
		mesh.boneWeights = boneWeights.ToArray();
		mesh.triangles = triangles.ToArray();
	}

	static SkinnedMeshRenderer CombineAnim(Animation body, Animation weapon) {
		var renderer_body = body.GetComponentInChildren<SkinnedMeshRenderer>();
		var renderer_weapon = weapon.GetComponentInChildren<SkinnedMeshRenderer>();

		if(renderer_body != null && renderer_weapon != null) {
			var root_body = body.transform;
			var root_weapon = weapon.transform;
			var bones_weapon = renderer_weapon.bones;
			var bone_root_weapon = renderer_weapon.rootBone;

			for(int i = 0; i < bones_weapon.Length; i++) {
				var bone_path = FindBonePath(root_weapon, bones_weapon[i]);
				if(!string.IsNullOrEmpty(bone_path)) {
					var bone_body = root_body.Find(bone_path);
					if(bone_body != null) {
						if(bone_root_weapon == bones_weapon[i]) {
							renderer_weapon.rootBone = bone_body;
						}

						bones_weapon[i] = bone_body;
					} else {
						Debug.LogError("can not find bone in body");
					}
				}
			}

			renderer_weapon.bones = bones_weapon;
			renderer_weapon.transform.parent = body.transform;

			GameObject.Destroy(weapon.gameObject);
			
			return renderer_weapon;
		}

		return null;
	}

	static string FindBonePath(Transform root, Transform bone) {
		StringBuilder str = new StringBuilder(bone.name);

		Transform t = bone.parent;
		while(t != null) {
			if(t == root) {
				break;
			}

			str.Insert(0, '/');
			str.Insert(0, t.name);

			t = t.parent;
		}

		return str.ToString();
	}
}
