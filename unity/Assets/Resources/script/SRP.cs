using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Experimental.Rendering;

public class SRP : RenderPipeline
{
    public override void Render(ScriptableRenderContext context, Camera[] cameras)
    {
        base.Render(context, cameras);

        for (int i = 0; i < cameras.Length; ++i)
        {
            Camera camera = cameras[i];

            ScriptableCullingParameters culling_params;
            if (!CullResults.GetCullingParameters(camera, out culling_params))
            {
                continue;
            }
            CullResults cull = CullResults.Cull(ref culling_params, context);

            context.SetupCameraProperties(camera);
            var cmd = new CommandBuffer();
            cmd.ClearRenderTarget(true, true, Color.black);
            context.ExecuteCommandBuffer(cmd);
            cmd.Release();

            var draw = new DrawRendererSettings(camera, new ShaderPassName("ForwardBase"));
            draw.sorting.flags = SortFlags.CommonOpaque;
            var filter = new FilterRenderersSettings(true);
            filter.renderQueueRange = RenderQueueRange.opaque;
            context.DrawRenderers(cull.visibleRenderers, ref draw, filter);

            draw = new DrawRendererSettings(camera, new ShaderPassName("ForwardBase"));
            draw.sorting.flags = SortFlags.CommonTransparent;
            filter = new FilterRenderersSettings(true);
            filter.renderQueueRange = RenderQueueRange.transparent;
            context.DrawRenderers(cull.visibleRenderers, ref draw, filter);

            context.Submit();
        }
    }
}
