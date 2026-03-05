#include <dlfcn.h>
#include <link.h>
#include <span>
#include <string>
#include "main.h"
#include <utility>
#include <optional>

BaseActorRenderContext_ctor_t BaseActorRenderContext_ctor = nullptr;
ItemRenderer_renderGuiItemNew_t ItemRenderer_renderGuiItemNew = nullptr;

LL_VTABLE_HOOK(
    ShulkerBoxBlockItem_appendFormattedHovertext,
    "19ShulkerBoxBlockItem",
    55,
    void,
    ShulkerBoxBlockItem* self,
    ItemStackBase* stack,
    void* level,
    std::string& out,
    bool flag
) {
    origin(self, stack, level, out, flag);
    ShulkerBoxBlockItem_appendFormattedHovertext_hook(self, stack, level, out, flag);
}

LL_VTABLE_HOOK(
    HoverRenderer_renderHoverBox,
    "17HoverTextRenderer",
    17,
    void,
    void* selfPtr,
    MinecraftUIRenderContext* ctx,
    void* client,
    void* aabb,
    float someFloat
) {
    origin(selfPtr, ctx, client, aabb, someFloat);
    HoverRenderer_renderHoverBox_hook(selfPtr, ctx, client, aabb, someFloat);
}

LL_VTABLE_HOOK(
    MinecraftUIRenderContext_drawText,
    "24MinecraftUIRenderContext",
    6,
    void,
    MinecraftUIRenderContext* self,
    Font& font,
    const RectangleArea& rect,
    const std::string& text,
    const mce::Color& color,
    ui::TextAlignment align,
    float alpha,
    const TextMeasureData& tmd,
    const CaretMeasureData& cmd
) {
    MinecraftUIRenderContext_drawText_hook(self, font, rect, text, color, align, alpha, tmd, cmd);
    origin(self, font, rect, text, color, align, alpha, tmd, cmd);
}

void Init() {
    auto treeFindAddr = memory::resolveSignature("55 41 57 41 56 41 55 41 54 53 50 48 89 FB 4C 8B 6F 08 48 83 C3 08 4D 85 ED 0F 84 B6 00 00 00 4C");
    Nbt_treeFind = reinterpret_cast<Nbt_treeFind_t>(treeFindAddr);

    auto ItemStackbaseloadItemaddr = memory::resolveSignature("55 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 49 89 F7 49 89 FC 64 48 8B 04 25 28 00 00 00 48 89 84 24 C0 00 00 00 48 8D 3D ?? ?? ?? ?? E8");
    ItemStackBase_loadItem = reinterpret_cast<ItemStackBase_loadItem_t>(ItemStackbaseloadItemaddr);

    auto isbgetdmgaddr = memory::resolveSignature("41 57 41 56 53 48 83 EC ?? 64 48 8B 04 25 ?? ?? ?? ?? 48 89 44 24 ?? 48 8B 47 ?? 48 85 C0 0F 84 ?? ?? ?? ?? ?? ?? ?? 00 0F 84 ?? ?? ?? ?? 4C 8B 77");
    ItemStackBase_getDamageValue = reinterpret_cast<ItemStackBase_getDamageValue_t>(isbgetdmgaddr);

    auto BaseActorRenderContextCtorAddr = memory::resolveSignature("41 57 41 56 41 54 53 50 49 89 D7 49 89 F4 48 89 FB 48 8D 05 ?? ?? ?? ?? ?? ?? ?? 0F 57 C0");
    BaseActorRenderContext_ctor = reinterpret_cast<BaseActorRenderContext_ctor_t>(BaseActorRenderContextCtorAddr);

    auto ItemRendererRenderGuiItemNewAddr = memory::resolveSignature("55 41 57 41 56 41 55 41 54 53 48 81 EC E8 00 00 00 4C 89 4C 24 28 F3 0F 11 64 24 18 F3 0F 11 5C");
    ItemRenderer_renderGuiItemNew = reinterpret_cast<ItemRenderer_renderGuiItemNew_t>(ItemRendererRenderGuiItemNewAddr);
}

__attribute__((constructor))
void Main() {
    Init();
}
