#pragma once

namespace engine {

class Application;

// One state of the game: menu, gameplay, pause, win/lose screen, ...
// Scenes live on a stack owned by SceneManager. Only the top scene is updated;
// all scenes are drawn bottom to top, so an overlay (e.g. pause) can show the
// scene underneath it.
class Scene {
public:
    virtual ~Scene() = default;

    // Called when the scene becomes part of the stack / is removed from it.
    // App() is available from OnEnter() on, NOT in the constructor, so create
    // GPU resources (textures) here.
    virtual void OnEnter() {}
    virtual void OnExit() {}

    // dt is the fixed update step in seconds (1/60 by default).
    virtual void Update(float dt) = 0;

    // alpha in [0, 1) = how far we are between the last update and the next one.
    // Interpolate positions with it for smooth movement on high refresh-rate screens.
    virtual void Draw(float alpha) = 0;

protected:
    Application& App() const { return *m_app; }

private:
    friend class SceneManager;
    Application* m_app = nullptr;
};

} // namespace engine
