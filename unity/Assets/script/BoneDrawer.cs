using UnityEngine;
using System.Collections.Generic;

public class BoneDrawer : MonoBehaviour
{
    public Transform root;

    void Start()
    {
        if (root == null)
        {
            return;
        }

        DrawBones(root);
    }

    void DrawBones(Transform node)
    {
        List<Transform> children = new List<Transform>();

        for (int i = 0; i < node.childCount; ++i)
        {
            children.Add(node.GetChild(i));
        }

        var bone = GameObject.CreatePrimitive(PrimitiveType.Sphere).transform;
        bone.parent = node;
        bone.localPosition = new Vector3(0, 0, 0);
        bone.localRotation = Quaternion.identity;
        bone.localScale = new Vector3(1, 1, 1) * 0.04f;

        Destroy(bone.GetComponent<Collider>());
        var renderer = bone.GetComponent<MeshRenderer>();
        var material = new Material(Shader.Find("Bone"));
        material.SetColor("_Color", new Color(0, 1, 0, 1));
        renderer.sharedMaterial = material;

        for (int i = 0; i < children.Count; ++i)
        {
            var child = children[i];

            DrawBones(child);
        }
    }
}
