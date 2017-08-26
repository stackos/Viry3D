#pragma once

#include "UIRect.h"
#include "renderer/Renderer.h"

namespace Viry3D
{
	class Camera;
	class Mesh;
	class UIView;

	class UICanvasRenderer : public Renderer, public UIRect
	{
		DECLARE_COM_CLASS(UICanvasRenderer, Renderer);
		
	private:
		friend class UIEventHandler;

	public:
		struct RenderType
		{
			enum Enum
			{
				BaseView,
				Sprite,
				Text
			};
		};

		static void HandleUIEvent(const List<UICanvasRenderer*>& list);

		virtual ~UICanvasRenderer();
		virtual const VertexBuffer* GetVertexBuffer();
		virtual const IndexBuffer* GetIndexBuffer();
		virtual void GetIndexRange(int material_index, int& start, int& count);
		void MarkDirty();
		const Vector<Ref<UIView>>& GetViews() const { return m_views; }

	protected:
		virtual void LateUpdate();
		virtual void OnTranformHierarchyChanged();

	private:
		UICanvasRenderer();
		void FindViews();
		void UpdateViews();

		RenderType::Enum m_type;
		Ref<Mesh> m_mesh;
		Vector<Ref<UIView>> m_views;
	};
}