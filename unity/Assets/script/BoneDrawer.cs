using UnityEngine;
using System.Collections.Generic;

public class BoneDrawer : MonoBehaviour
{
    public Transform root;
    Material joint_material;
    Material bone_material;
    Mesh cone_mesh;

    void Start()
    {
        if (root == null)
        {
            return;
        }

        joint_material = new Material(Shader.Find("Bone"));
        joint_material.SetColor("_Color", new Color32(248, 181, 0, 255));

        bone_material = new Material(Shader.Find("Bone"));
        bone_material.SetColor("_Color", new Color32(40, 187, 239, 255));

        cone_mesh = CreateConeMesh("Cone");

        DrawBones(root);
    }

    static Mesh CreateConeMesh(string name)
    {
        Mesh mesh = new Mesh();
        mesh.name = name;

        List<Vector3> vertices = new List<Vector3>();
        List<int> triangles = new List<int>();

        vertices.Add(new Vector3(0, 0, 0));
        vertices.Add(new Vector3(0, 1, 0));

        for (int i = 0; i < 360; ++i)
        {
            float x = 0.5f * Mathf.Cos(Mathf.Deg2Rad * i);
            float z = 0.5f * Mathf.Sin(Mathf.Deg2Rad * i);
            vertices.Add(new Vector3(x, 0, z));

            if (i == 359)
            {
                triangles.Add(0);
                triangles.Add(2 + i);
                triangles.Add(2);

                triangles.Add(1);
                triangles.Add(2);
                triangles.Add(2 + i);
            }
            else
            {
                triangles.Add(0);
                triangles.Add(2 + i);
                triangles.Add(2 + i + 1);

                triangles.Add(1);
                triangles.Add(2 + i + 1);
                triangles.Add(2 + i);
            }
        }

        mesh.vertices = vertices.ToArray();
        mesh.triangles = triangles.ToArray();
        mesh.RecalculateNormals();

        return mesh;
    }

    void DrawBones(Transform node)
    {
        List<Transform> children = new List<Transform>();

        for (int i = 0; i < node.childCount; ++i)
        {
            children.Add(node.GetChild(i));
        }

        var joint = GameObject.CreatePrimitive(PrimitiveType.Sphere).transform;
        joint.gameObject.name = "Joint";
        joint.parent = node;
        joint.localPosition = new Vector3(0, 0, 0);
        joint.localRotation = Quaternion.identity;
        joint.localScale = new Vector3(1, 1, 1) * 0.04f;

        Destroy(joint.GetComponent<Collider>());
        var renderer = joint.GetComponent<MeshRenderer>();
        renderer.sharedMaterial = joint_material;

        for (int i = 0; i < children.Count; ++i)
        {
            var child = children[i];

            var bone_dir = child.position - node.position;
            var bone_len = bone_dir.magnitude;
            var bone_size = 0.03f;

            var bone = new GameObject("Bone").transform;
            bone.parent = node;
            bone.localPosition = new Vector3(0, 0, 0);
            bone.up = bone_dir;
            bone.localScale = new Vector3(bone_size, bone_len, bone_size);

            renderer = bone.gameObject.AddComponent<MeshRenderer>();
            renderer.sharedMaterial = bone_material;

            var filter = bone.gameObject.AddComponent<MeshFilter>();
            filter.sharedMesh = cone_mesh;

            DrawBones(child);
        }
    }
}
