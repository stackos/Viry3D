local _ui_camera = nil
local _label = nil

function AppInit()
    print("AppInit")

    _ui_camera = Display.CreateCamera()
    _ui_camera:SetName("camera")

    print(_ui_camera, _ui_camera:GetName())

    local canvas = CanvasRenderer.New()
    canvas:SetName("canvas")

    print(canvas, canvas:GetName())

    _ui_camera:AddRenderer(canvas)

    local sprite = Sprite.New()
    sprite:SetName("sprite")

    print(sprite, sprite:GetName())

    canvas:AddView(sprite)

    print(Application.GetDataPath())

    local texture = Texture.LoadTexture2DFromFile(
        Application.GetDataPath() .. "/lua/back.png",
        FilterMode.Nearest,
        SamplerAddressMode.ClampToEdge)
    texture:SetName("texture")

    print(texture, texture:GetName())

    sprite:SetSize(384 * 2, 240 * 2)
    sprite:SetTexture(texture)

    _label = Label.New()
    _label:SetName("label")

    print(_label, _label:GetName())

    canvas:AddView(_label)

    _label:SetAlignment(ViewAlignment.Left + ViewAlignment.Top)
    _label:SetPivot(0, 0)
    _label:SetSize(100, 30)
    _label:SetOffset(40, 40)
    _label:SetFont(Font.GetFont(FontType.Consola))
    _label:SetFontSize(28)
    _label:SetTextAlignment(ViewAlignment.Left + ViewAlignment.Top)
end

function AppDone()
    print("AppDone")

    Display.DestroyCamera(_ui_camera)
end

function AppUpdate()
    _label:SetText(string.format("FPS:%d", Time.GetFPS()))
end
