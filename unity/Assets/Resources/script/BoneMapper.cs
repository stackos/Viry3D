using System.Collections.Generic;
using UnityEngine;
using Newtonsoft.Json.Linq;

public class BoneMapper : MonoBehaviour
{
    public Transform rootSrc;
    public Transform rootDst;
    public TextAsset srcBasePose;
    public TextAsset dstBasePose;
    public TextAsset boneMap;
    public bool localRotationDirectMap = false;
    Dictionary<string, BoneState> src_base_pose;
    Dictionary<string, BoneState> dst_base_pose;
    JObject bone_map;
    List<Transform> src_bones;
    List<Transform> dst_bones;
    List<string> avatar_bones;

    class BoneState
    {
        public Vector3 pos;
        public Quaternion rot;
        public Vector3 sca;

        public BoneState parent;
        public Quaternion rot_world;

        public Quaternion RotWorld()
        {
            if (parent == null)
            {
                return rot;
            }
            else
            {
                return parent.RotWorld() * rot;
            }
        }
    }

    void Start()
    {
        avatar_bones = new List<string>();
        // body
        avatar_bones.Add("Hips");
        avatar_bones.Add("Spine");
        avatar_bones.Add("Chest"); // optional
        avatar_bones.Add("UpperChest"); // optional
        // head
        avatar_bones.Add("Neck"); // optional
        avatar_bones.Add("Head");
        avatar_bones.Add("LeftEye"); // optional
        avatar_bones.Add("RightEye"); // optional
        avatar_bones.Add("Jaw"); // optional
        // left arm
        avatar_bones.Add("LeftShoulder"); // optional
        avatar_bones.Add("LeftUpperArm");
        avatar_bones.Add("LeftLowerArm");
        avatar_bones.Add("LeftHand");
        // right arm
        avatar_bones.Add("RightShoulder"); // optional
        avatar_bones.Add("RightUpperArm");
        avatar_bones.Add("RightLowerArm");
        avatar_bones.Add("RightHand");
        // left leg
        avatar_bones.Add("LeftUpperLeg");
        avatar_bones.Add("LeftLowerLeg");
        avatar_bones.Add("LeftFoot");
        avatar_bones.Add("LeftToes"); // optional
        // right leg
        avatar_bones.Add("RightUpperLeg");
        avatar_bones.Add("RightLowerLeg");
        avatar_bones.Add("RightFoot");
        avatar_bones.Add("RightToes"); // optional
        // left hand
        avatar_bones.Add("LeftThumbProximal"); // optional
        avatar_bones.Add("LeftThumbIntermediate"); // optional
        avatar_bones.Add("LeftThumbDistal"); // optional
        avatar_bones.Add("LeftIndexProximal"); // optional
        avatar_bones.Add("LeftIndexIntermediate"); // optional
        avatar_bones.Add("LeftIndexDistal"); // optional
        avatar_bones.Add("LeftMiddleProximal"); // optional
        avatar_bones.Add("LeftMiddleIntermediate"); // optional
        avatar_bones.Add("LeftMiddleDistal"); // optional
        avatar_bones.Add("LeftRingProximal"); // optional
        avatar_bones.Add("LeftRingIntermediate"); // optional
        avatar_bones.Add("LeftRingDistal"); // optional
        avatar_bones.Add("LeftLittleProximal"); // optional
        avatar_bones.Add("LeftLittleIntermediate"); // optional
        avatar_bones.Add("LeftLittleDistal"); // optional
        // right hand
        avatar_bones.Add("RightThumbProximal"); // optional
        avatar_bones.Add("RightThumbIntermediate"); // optional
        avatar_bones.Add("RightThumbDistal"); // optional
        avatar_bones.Add("RightIndexProximal"); // optional
        avatar_bones.Add("RightIndexIntermediate"); // optional
        avatar_bones.Add("RightIndexDistal"); // optional
        avatar_bones.Add("RightMiddleProximal"); // optional
        avatar_bones.Add("RightMiddleIntermediate"); // optional
        avatar_bones.Add("RightMiddleDistal"); // optional
        avatar_bones.Add("RightRingProximal"); // optional
        avatar_bones.Add("RightRingIntermediate"); // optional
        avatar_bones.Add("RightRingDistal"); // optional
        avatar_bones.Add("RightLittleProximal"); // optional
        avatar_bones.Add("RightLittleIntermediate"); // optional
        avatar_bones.Add("RightLittleDistal"); // optional

        if (rootSrc)
        {
            src_bones = new List<Transform>();
            FindBones(rootSrc, src_bones);
        }

        if (rootDst)
        {
            dst_bones = new List<Transform>();
            FindBones(rootDst, dst_bones);
        }

        if (srcBasePose)
        {
            var bone = JObject.Parse(srcBasePose.text);
            src_base_pose = new Dictionary<string, BoneState>();
            LoadBoneState(bone, src_base_pose, null);
        }

        if (dstBasePose)
        {
            var bone = JObject.Parse(dstBasePose.text);
            dst_base_pose = new Dictionary<string, BoneState>();
            LoadBoneState(bone, dst_base_pose, null);
        }

        if (boneMap)
        {
            bone_map = JObject.Parse(boneMap.text);
        }
    }

    void FindBones(Transform t, List<Transform> bones)
    {
        bones.Add(t);

        for (int i = 0; i < t.childCount; ++i)
        {
            Transform child = t.GetChild(i);
            FindBones(child, bones);
        }
    }

    void LoadBoneState(JObject bone, Dictionary<string, BoneState> states, BoneState parent)
    {
        string name = (string) bone["name"];
        if (!states.ContainsKey(name))
        {
            BoneState state = new BoneState();
            JObject pos = (JObject) bone["localPosition"];
            state.pos = new Vector3((float) pos["x"], (float) pos["y"], (float) pos["z"]);
            JObject rot = (JObject) bone["localRotation"];
            state.rot = new Quaternion((float) rot["x"], (float) rot["y"], (float) rot["z"], (float) rot["w"]);
            JObject sca = (JObject) bone["localScale"];
            state.sca = new Vector3((float) sca["x"], (float) sca["y"], (float) sca["z"]);
            state.parent = parent;
            state.rot_world = state.RotWorld();

            states.Add(name, state);

            JArray children = (JArray) bone["children"];
            for (int i = 0; i < children.Count; ++i)
            {
                JObject child = (JObject) children[i];
                LoadBoneState(child, states, state);
            }
        }
    }

    void Update()
    {
        if (src_bones == null || dst_bones == null || src_base_pose == null || dst_base_pose == null || bone_map == null)
        {
            return;
        }

        for (int i = 0; i < avatar_bones.Count; ++i)
        {
            JObject pair = (JObject) bone_map.GetValue(avatar_bones[i]);
            if (pair != null)
            {
                string src = (string) pair.GetValue("src");
                string dst = (string) pair.GetValue("dst");

                if (src.Length > 0 && dst.Length > 0)
                {
                    Transform src_bone = src_bones.Find(delegate (Transform t) {
                        return src == t.name;
                    });
                    Transform dst_bone = dst_bones.Find(delegate (Transform t) {
                        return dst == t.name;
                    });

                    if (src_bone && dst_bone)
                    {
                        BoneState src_base_state = src_base_pose[src];
                        BoneState dst_base_state = dst_base_pose[dst];
 
                        Quaternion rot = Quaternion.Inverse(src_base_state.rot) * src_bone.localRotation;
                        if (localRotationDirectMap)
                        {
                            dst_bone.localRotation = dst_base_state.rot * rot;
                        }
                        else
                        {
                            Vector3 euler = rot.eulerAngles;
                            Vector3 right = Quaternion.Inverse(dst_base_state.rot_world) * new Vector3(1, 0, 0);
                            Vector3 up = Quaternion.Inverse(dst_base_state.rot_world) * new Vector3(0, 1, 0);
                            Vector3 forward = Quaternion.Inverse(dst_base_state.rot_world) * new Vector3(0, 0, 1);
                            dst_bone.localRotation = dst_base_state.rot
                                * Quaternion.AngleAxis(euler.y, up)
                                * Quaternion.AngleAxis(euler.x, right)
                                * Quaternion.AngleAxis(euler.z, forward);
                        }

                        float scale = rootDst.localPosition.y / rootSrc.localPosition.y;
                        Vector3 pos = src_bone.localPosition - src_base_state.pos;
                        dst_bone.localPosition = dst_base_state.pos + pos * scale;
                    }
                }
            }
        }
    }
}
