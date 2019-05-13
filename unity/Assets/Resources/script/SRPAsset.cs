using UnityEngine.Experimental.Rendering;

public class SRPAsset : RenderPipelineAsset
{
#if UNITY_EDITOR
    [UnityEditor.MenuItem("Viry3D/Create SRP")]
    static void CreateSRP()
    {
        UnityEditor.AssetDatabase.CreateAsset(CreateInstance<SRPAsset>(), "Assets/SRP.asset");
    }
#endif

    protected override IRenderPipeline InternalCreatePipeline()
    {
        return new SRP();
    }
}
