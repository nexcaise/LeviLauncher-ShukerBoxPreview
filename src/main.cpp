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
    auto treeFindAddr = memory::resolveSignature("? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 FD 03 00 91 F3 03 00 AA ? ? ? F8 ? ? ? B4 ? ? ? A9 F5 03 13 AA ? ? ? 14 ? ? ? 52 ? ? ? 71 ? ? ? 54 ? ? ? 91 ? ? ? F9 ? ? ? B4 ? ? ? 39 ? ? ? 36 ? ? ? F9 ? ? ? 36 ? ? ? F9 1F 03 16 EB E0 03 14 AA 02 33 96 9A ? ? ? 94 ? ? ? 34 ? ? ? 37 ? ? ? 52 ? ? ? 71 ? ? ? 54 ? ? ? 14 ? ? ? 91 ? ? ? 37 ? ? ? D3 1F 03 16 EB E0 03 14 AA 02 33 96 9A ? ? ? 94 ? ? ? 35 DF 02 18 EB ? ? ? 54 E8 03 1F 2A ? ? ? 71 ? ? ? 54 F5 03 17 AA ? ? ? F9 ? ? ? B5 ? ? ? 14 ? ? ? 54 ? ? ? 17 BF 02 13 EB ? ? ? 54 ? ? ? 39 ? ? ? A9 E0 03 14 AA ? ? ? D3 ? ? ? 72 ? ? ? 91 01 01 8B 9A 57 01 89 9A FF 02 16 EB E2 32 96 9A ? ? ? 94 DF 02 17 EB E8 27 9F 1A 1F 00 00 71 E9 A7 9F 1A 08 01 89 1A 1F 01 00 71 73 12 95 9A E0 03 13 AA ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A8 C0 03 5F D6 ? ? ? A9");
    Nbt_treeFind = reinterpret_cast<Nbt_treeFind_t>(treeFindAddr);

    auto ItemStackbaseloadItemaddr = memory::resolveSignature("? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D5 F3 03 00 AA ? ? ? ? ? ? ? 91 ? ? ? F9 F5 03 01 AA");
    ItemStackBase_loadItem = reinterpret_cast<ItemStackBase_loadItem_t>(ItemStackbaseloadItemaddr);

    auto isbgetdmgaddr = memory::resolveSignature("? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 ? ? ? D5 ? ? ? F9 ? ? ? F8 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4 ? ? ? F9 ? ? ? B4");
    ItemStackBase_getDamageValue = reinterpret_cast<ItemStackBase_getDamageValue_t>(isbgetdmgaddr);

    auto BaseActorRenderContextCtorAddr = memory::resolveSignature("? ? ? A9 ? ? ? A9 ? ? ? A9 FD 03 00 91 ? ? ? ? ? ? ? 91 ? ? ? A9 ? ? ? A9 F3 03 00 AA F4 03 02 AA");
    BaseActorRenderContext_ctor = reinterpret_cast<BaseActorRenderContext_ctor_t>(BaseActorRenderContextCtorAddr);

    auto ItemRendererRenderGuiItemNewAddr = memory::resolveSignature("FF C3 05 D1 EC 73 00 FD  EB 2B 0F 6D E9 23 10 6D FD 7B 11 A9 FC 6F 12 A9  FA 67 13 A9 F8 5F 14 A9 F6 57 15 A9 F4 4F 16 A9  FD 43 04 91 5B D0 3B D5");
    ItemRenderer_renderGuiItemNew = reinterpret_cast<ItemRenderer_renderGuiItemNew_t>(ItemRendererRenderGuiItemNewAddr);
}

__attribute__((constructor))
void Main() {
    Init();
}
