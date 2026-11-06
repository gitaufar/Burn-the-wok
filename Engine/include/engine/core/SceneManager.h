#pragma once

#include "engine/core/Scene.h"

#include <memory>
#include <vector>

namespace engine {

class Application;

// Stack of scenes. Requests (Push/Pop/SwitchTo) are queued and applied between
// updates, so a scene can safely ask to be replaced from inside its own Update()
// without being destroyed while that function is still running.
class SceneManager {
public:
    explicit SceneManager(Application& app) : m_app(app) {}
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void Push(std::unique_ptr<Scene> scene);     // put a scene on top (e.g. pause overlay)
    void Pop();                                  // remove the top scene
    void SwitchTo(std::unique_ptr<Scene> scene); // remove ALL scenes, then push this one

    bool IsEmpty() const { return m_stack.empty(); }
    Scene* GetTop() const { return m_stack.empty() ? nullptr : m_stack.back().get(); }

private:
    friend class Application;

    enum class RequestType { Push, Pop, SwitchTo };
    struct Request {
        RequestType type;
        std::unique_ptr<Scene> scene;
    };

    void ApplyPending();
    void Update(float dt);
    void Draw(float alpha);
    void Clear(); // immediate; used at shutdown

    void PushNow(std::unique_ptr<Scene> scene);
    void PopNow();

    Application& m_app;
    std::vector<std::unique_ptr<Scene>> m_stack;
    std::vector<Request> m_pending;
};

} // namespace engine
