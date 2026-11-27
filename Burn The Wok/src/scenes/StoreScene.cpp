#include "scenes/StoreScene.h"

#include "art/FieldIcons.h"
#include "art/PixelText.h"
#include "progress/SaveData.h"
#include "scenes/LoadingScene.h"
#include "scenes/MenuScene.h"

#include <engine/core/Application.h>
#include <engine/core/Log.h>
#include <engine/graphics/Camera2D.h>
#include <algorithm>
#include <cmath>
#include <exception>

namespace game {
using engine::Color;
using engine::Key;
using art::FieldIcon;
namespace {
struct CounterLayout {
    float s, x, y;
    explicit CounterLayout(engine::Vec2 size) {
        s = std::max(0.01f, std::min(size.x/1280, size.y/720));
        x = (size.x-1100*s)*0.5f;
        y = (size.y-596*s)*0.5f;
    }
    engine::Rect Box(float bx, float by, float w, float h) const {
        return {std::round(x+bx*s), std::round(y+by*s), w*s, h*s};
    }
    engine::Rect Row(int id) const { return Box(28, 115+id*60.0f, 563, 55); }
    engine::Rect Buy() const { return Box(639, 467, 402, 55); }
    engine::Rect Back() const { return Box(28, 554, 170, 28); }
};
constexpr FieldIcon Icons[] = {FieldIcon::Tank, FieldIcon::Nozzle, FieldIcon::Pump,
    FieldIcon::Boot, FieldIcon::Crew, FieldIcon::Megaphone, FieldIcon::Megaphone};
struct Offer {
    int level;
    int price;
    const char* unavailable;
};
Offer GetOffer(int id, const SaveData& save) {
    const int level = id < Local ? save.upgrades[id] : 0;
    const auto& supply = Supplies[id];
    if (id == National && !save.cleared[1]) return {level, supply.prices[0], "TUNTASKAN STAGE 2"};
    if (level >= supply.maxLevel) return {level, 0, "LEVEL MAKSIMAL"};
    const int price = supply.prices[level];
    if (id >= Local && save.campaignSeconds >= 2*CampaignSeconds)
        return {level, price, "DURASI SUDAH MAKSIMAL"};
    if (save.points < price) return {level, price, "POIN BELUM CUKUP"};
    return {level, price, nullptr};
}
std::string UpgradeEffect(int id, int level) {
    switch (id) {
    case Tank: return std::to_string(static_cast<int>(TankSizes[level])) + " UNIT";
    case Nozzle: return "+" + std::to_string(level*15) + "% DAYA";
    case Pump: return "+" + std::to_string(level*40) + "% ISI ULANG";
    case Boots: return "+" + std::to_string(level*10) + "% LAJU";
    case Crew: return std::to_string(level) + " RELAWAN";
    default: return "";
    }
}
}

void StoreScene::OnEnter() {
    App().GetWindow().SetTitle("Burn The Wo - Toko Perlengkapan");
    App().GetWindow().SetClearColor(Color::FromHex(0x0C1C18));
    auto& save = SaveData::Current();
    if (save.storeNew) {
        auto next = save;
        next.storeNew = false;
        save.Commit(next);
    }
    m_sfx = std::make_unique<SoundBank>(App().GetAudio());
    m_sfx->Load({"ui_hover", "purchase_ok", "purchase_fail", "upgrade_level", "campaign_launch"});
    try {
        m_music = App().GetAudio().Play(App().GetAudio().LoadSound("assets/audio/music/menu.wav"), 0.45f, true);
    } catch (const std::exception& error) {
        engine::LogError("Store music unavailable: %s", error.what());
    }
}

void StoreScene::OnExit() {
    App().GetAudio().Stop(m_music);
}

void StoreScene::Purchase() {
    auto& save = SaveData::Current();
    const auto offer = GetOffer(m_selected, save);
    m_notice = 3;
    m_error = true;
    if (offer.unavailable) {
        m_message = offer.unavailable;
        m_sfx->Play("purchase_fail", 0.55f);
        return;
    }
    auto next = save;
    next.points -= offer.price;
    if (m_selected < Local) {
        ++next.upgrades[m_selected];
    } else {
        const int tier = m_selected == National ? 2 : 1;
        next.campaignTier = next.campaignSeconds > 0 ? std::max(next.campaignTier, tier) : tier;
        next.campaignSeconds = std::min(2.0f*CampaignSeconds, next.campaignSeconds+CampaignSeconds);
    }
    if (!save.Commit(next)) {
        m_message = "GAGAL MENYIMPAN PEMBELIAN";
        m_sfx->Play("purchase_fail", 0.55f);
        return;
    }
    m_error = false;
    m_message = m_selected < Local ? "SIAP UNTUK PATROLI BERIKUTNYA" : "SIAP DIPANGGIL DENGAN Q";
    m_sfx->Play("purchase_ok", 0.45f);
    if (m_selected >= Local) {
        m_poster = 2.65f;
        m_sfx->Play("campaign_launch", 0.55f);
    } else if (offer.level > 0) {
        m_sfx->Play("upgrade_level", 0.42f);
    }
}

void StoreScene::Update(float dt) {
    const auto& input = App().GetInput();
    const auto mouse = input.GetMousePosition();
    const bool click = input.IsMousePressed(engine::MouseButton::Left);
    const bool enter = input.IsKeyPressed(Key::Enter) || input.IsKeyPressed(Key::NumpadEnter);
    const bool escape = input.IsKeyPressed(Key::Escape);
    const CounterLayout layout(App().GetWindow().GetSize());
    if (m_poster > 0) {
        m_poster = std::max(0.0f, m_poster-dt);
        if (click || enter || escape || input.IsKeyPressed(Key::Space)) m_poster = 0;
        return;
    }
    m_notice = std::max(0.0f, m_notice-dt);
    if (escape || (click && layout.Back().Contains(mouse))) {
        App().GetScenes().SwitchTo(std::make_unique<LoadingScene>(
            [] { return std::make_unique<MenuScene>(); }, "KEMBALI KE POS JAGA"));
        return;
    }
    const int previous = m_selected;
    if (input.IsKeyPressed(Key::Up) || input.IsKeyPressed(Key::Left))
        m_selected = (m_selected+UpgradeCount-1) % UpgradeCount;
    if (input.IsKeyPressed(Key::Down) || input.IsKeyPressed(Key::Right))
        m_selected = (m_selected+1) % UpgradeCount;
    if (click) {
        for (int i = 0; i < UpgradeCount; ++i)
            if (layout.Row(i).Contains(mouse)) m_selected = i;
    }
    if (previous != m_selected) {
        m_sfx->Play("ui_hover", 0.35f);
        m_notice = 0;
    }
    if (enter || (click && layout.Buy().Contains(mouse))) Purchase();
}

void StoreScene::Draw(float /*alpha*/) {
    const auto size = App().GetWindow().GetSize();
    const CounterLayout layout(size);
    const auto& save = SaveData::Current();
    auto& batch = App().GetSpriteBatch();
    engine::Camera2D camera;
    camera.SetViewportSize(size);
    camera.SetPosition(size*0.5f);
    batch.Begin(camera.GetViewProjection());
    const Color ink = Color::FromHex(0x23372C), paper = Color::FromHex(0xE8D9AF);
    const Color cream = Color::FromHex(0xFFF0CB), brass = Color::FromHex(0xE9BE70);
    const Color dim = Color::FromHex(0x676956), moss = Color::FromHex(0xA8B67E);
    const Color rule = Color::FromHex(0xBAAC84), coal = Color::FromHex(0x11231C);
    const auto rect = [&](float x, float y, float w, float h, Color color) {
        batch.DrawRect(layout.Box(x,y,w,h), color);
    };
    const auto text = [&](std::string_view value, float x, float y, float scale, Color color) {
        const auto box = layout.Box(x,y,0,0);
        art::DrawPixelText(batch, value, box.x, box.y, scale*layout.s, color);
    };
    const auto icon = [&](FieldIcon glyph, float x, float y, float scale, Color color) {
        const auto box = layout.Box(x,y,0,0);
        art::DrawFieldIcon(batch, glyph, box.x, box.y, scale*layout.s, color);
    };

    rect(8, 9, 1100, 596, Color::FromHex(0x06100C));
    rect(0, 0, 1100, 596, ink);
    for (int i = 0; i < 24; ++i) rect(i*47.0f, 0, 1, 596, Color::FromHex(0xB9BC82, 12));
    rect(0,0,1100,4,moss);
    text("POS PENJAGA / GUDANG", 28, 22, 1.5f, moss);
    text("TOKO PERLENGKAPAN", 28, 49, 4, cream);
    icon(FieldIcon::Leaf, 760, 35, 3, brass);
    text("SALDO " + std::to_string(save.points), 796, 42, 2.5f, brass);
    rect(28, 94, 1044, 1, moss);
    for (int i = 0; i < UpgradeCount; ++i) {
        const float y = 115+i*60.0f;
        const auto offer = GetOffer(i, save);
        const bool selected = m_selected == i;
        const Color front = offer.unavailable ? dim : ink;
        rect(28, y+3, 563, 55, coal);
        rect(28, y, 563, 55, selected ? cream : paper);
        rect(28, y, 5, 55, selected ? brass : rule);
        rect(43, y+8, 42, 39, selected ? ink : rule);
        icon(Icons[i], 52, y+15, 3, selected ? cream : front);
        text(Supplies[i].title, 101, y+10, 2.1f, front);
        if (i == National && !save.cleared[1]) icon(FieldIcon::Lock, 545, y+15, 2.6f, dim);
        else text(offer.price ? std::to_string(offer.price) + " P" : "MAKS", 478, y+17, 2.1f, front);
        const std::string detail = i < Local ? "LV " + std::to_string(offer.level) + "/" +
            std::to_string(Supplies[i].maxLevel) : "IKLANKAN KAMPANYE";
        text(detail, 101, y+35, 1.3f, dim);
    }

    rect(617, 115, 455, 415, coal);
    rect(611, 110, 455, 420, paper);
    rect(623, 124, 431, 146, Color::FromHex(0xD7CAA2));
    for (int i = 0; i < 13; ++i) rect(623, 129+i*11.0f, 431, 1, Color::FromHex(0x857D61, 29));
    for (int i = 0; i < 18; ++i) rect(627+i*24.0f, 124, 1, 146, Color::FromHex(0x857D61, 29));
    icon(Icons[m_selected], 793, 151, 11, ink);
    rect(1005, 130, 43, 14, brass);
    const auto offer = GetOffer(m_selected, save);
    text(Supplies[m_selected].title, 639, 294, 2.4f, ink);
    text(Supplies[m_selected].description, 639, 327, 1.7f, dim);
    rect(639, 357, 402, 1, rule);
    if (m_selected < Local) {
        text("SEKARANG", 639, 375, 1.5f, dim);
        text(UpgradeEffect(m_selected, offer.level), 639, 394, 1.9f, ink);
        if (offer.level < Supplies[m_selected].maxLevel) {
            text("SETELAH UPGRADE", 843, 375, 1.5f, dim);
            text(UpgradeEffect(m_selected, offer.level+1), 843, 394, 1.9f, ink);
        }
    } else {
        text("+10 MENIT PATROLI / MAKS 20 MENIT", 639, 376, 1.6f, ink);
        text("SATU PANGGILAN PER STAGE", 639, 399, 1.5f, dim);
    }
    if (m_notice > 0) text(m_message, 639, 438, 1.6f, m_error ? Color::FromHex(0x953B2E) : ink);
    const bool hover = layout.Buy().Contains(App().GetInput().GetMousePosition());
    batch.DrawRect(layout.Buy(), offer.unavailable ? Color::FromHex(0x65695B) :
                   hover ? Color::FromHex(0x3F674C) : ink);
    text(offer.unavailable ? offer.unavailable : m_selected < Local ? "BELI / " + std::to_string(offer.price) + " P" :
         "IKLANKAN / " + std::to_string(offer.price) + " P", 655, 485, 2, cream);
    text("ENTER", 963, 486, 1.6f, moss);
    rect(28, 543, 1044, 1, Color::FromHex(0x667654));
    text("ESC / KEMBALI", 28, 561, 1.7f, cream);
    text("PANAH / PILIH", 234, 561, 1.7f, moss);
    if (save.campaignSeconds > 0) {
        icon(FieldIcon::Megaphone, 766, 556, 2, brass);
        text("KAMPANYE AKTIF " + CampaignClock(save.campaignSeconds), 793, 561, 1.7f, brass);
    }

    if (m_poster > 0) {
        batch.DrawRect({0,0,size.x,size.y}, Color::FromHex(0x0B1714, 198));
        const float slide = std::max(0.0f, m_poster-2.33f)*95;
        rect(235, 89+slide, 636, 435, coal);
        rect(226, 80+slide, 636, 435, brass);
        rect(244, 98+slide, 600, 399, ink);
        rect(262, 116+slide, 564, 363, paper);
        for (int i = 0; i < 6; ++i) {
            const float shift = std::fmod((2.65f-m_poster)*18+i*8, 48.0f);
            rect(330+i*73.0f, 188+slide-shift, 4, 7, i%2 ? moss : brass);
        }
        icon(FieldIcon::Megaphone, 486, 167+slide, 14, ink);
        text("SIARAN POS PENJAGA", 311, 138+slide, 2, dim);
        text("#HUTANPANGGIL", 316, 306+slide, 5.5f, ink);
        text("KABAR MENYEBAR. REGU SIAP BERANGKAT.", 316, 379+slide, 1.9f, ink);
        text("ENTER / LEWATI", 620, 445+slide, 1.6f, dim);
    }
    batch.End();
}
}
