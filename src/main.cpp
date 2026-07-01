#include "Atmospheric.hpp"
#if defined(ANDROID) || (defined(__APPLE__) && TARGET_OS_IOS)
#include <SDL3/SDL_main.h>
#endif

#include <cmath>

// Smoothly cycles a SpriteComponent's fill color through HSV hue space.
class HueCycleComponent : public Component {
    float _speed;
    float _hueOffset;
    float _t = 0.0f;

    static glm::vec4 fromHSV(float h, float s, float v) {
        h = std::fmod(h, 1.0f);
        int   i = static_cast<int>(h * 6.0f);
        float f = h * 6.0f - static_cast<float>(i);
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);
        switch (i % 6) {
            case 0: return {v, t, p, 1.0f};
            case 1: return {q, v, p, 1.0f};
            case 2: return {p, v, t, 1.0f};
            case 3: return {p, q, v, 1.0f};
            case 4: return {t, p, v, 1.0f};
            case 5: return {v, p, q, 1.0f};
        }
        return {v, v, v, 1.0f};
    }

public:
    HueCycleComponent(GameObject* go, float speed, float hueOffset = 0.0f)
        : _speed(speed), _hueOffset(hueOffset) { gameObject = go; }

    std::string GetName() const override { return "HueCycleComponent"; }

    void OnTick(float dt) override {
        _t += dt;
        float hue = std::fmod(_t * _speed + _hueOffset, 1.0f);
        auto* s = gameObject->GetComponent<SpriteComponent>();
        if (s) s->SetColor(fromHSV(hue, 0.90f, 0.95f));
    }
};

// Four screen-space quads at the viewport corners, each cycling its hue at a
// 90-degree phase offset from its neighbours — producing a shifting 4-corner
// gradient that fills the viewport.
class GradientQuad : public Application {
    using Application::Application;

    static constexpr float kSpeed = 0.08f;  // full hue rotation in ~12.5 s

    void OnInit() override {
        GoScene("main", [this] { OnLoad(); });
    }

    void OnLoad() override {
        // Read the actual window size rather than assuming a fixed resolution,
        // so the four quads always tile the viewport exactly.
        const ImageSize windowSize = GetWindow()->GetLogicalSize();
        const float kW = static_cast<float>(windowSize.width);
        const float kH = static_cast<float>(windowSize.height);

        const glm::vec3 origins[4] = {
            {0.0f,      0.0f,      0.0f},
            {kW * 0.5f, 0.0f,      0.0f},
            {0.0f,      kH * 0.5f, 0.0f},
            {kW * 0.5f, kH * 0.5f, 0.0f},
        };
        // Corner hue offsets: top-left=red, top-right=yellow-green,
        // bottom-left=blue-violet, bottom-right=cyan.
        const float hueOffsets[4] = {0.0f, 0.25f, 0.75f, 0.5f};

        for (int i = 0; i < 4; ++i) {
            auto* q = CreateGameObject(origins[i]);
            q->AddComponent<SpriteComponent>(SpriteProps{
                .size    = glm::vec2(kW * 0.5f, kH * 0.5f),
                .pivot   = glm::vec2(0.0f, 0.0f),
                .color   = glm::vec4(1.0f),
                .texture = -1,
            });
            q->AddComponent<HueCycleComponent>(kSpeed, hueOffsets[i]);
        }
    }

    void OnUpdate(float /*dt*/, float /*time*/) override {
        if (input.IsKeyDown(Key::ESCAPE)) Quit();
    }
};

#ifdef __EMSCRIPTEN__
static void StartGame();

int main(int /*argc*/, char* /*argv*/[]) {
    FileSystem::Get().Prefetch({}, StartGame);
    return 0;
}

static void StartGame() {
    static GradientQuad game({.useDefaultShaders = true});
    game.Run();
}

#else
int main(int /*argc*/, char* /*argv*/[]) {
#if defined(__APPLE__) && TARGET_OS_IOS
    auto* game = new GradientQuad({.useDefaultShaders = true});
    game->Run();
    return 0;
#else
    GradientQuad game({.useDefaultShaders = true});
    game.Run();
    return 0;
#endif
}
#endif
