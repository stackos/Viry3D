local _ui_camera = nil
local _label = nil

function AppInit()
    print("AppInit")

    print(Display)

    _ui_camera = Display.CreateCamera()
    _ui_camera:SetName("camera")

    print(_ui_camera, _ui_camera:GetName())

    local canvas = CanvasRenderer.New()
    canvas:SetName("canvas")

    print(canvas, canvas:GetName())

    _ui_camera:AddRenderer(canvas)

    _label = Label.New()
    _label:SetName("label")

    print(_label, _label:GetName())

    canvas:AddView(_label)
end

function AppDone()
    print("AppDone")

    Display.DestroyCamera(_ui_camera)
    _ui_camera = nil
end

function AppUpdate()

end

--[[
label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
label->SetPivot(Vector2(0, 0));
label->SetSize(Vector2i(100, 30));
label->SetOffset(Vector2i(40, 40));
label->SetFont(Font::GetFont(FontType::Consola));
label->SetFontSize(28);
label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

label->SetText(String::Format("FPS:%d", Time::GetFPS()));
]]
