#include "engine/core/SceneManager.h"

#include <utility>

namespace engine {

SceneManager::~SceneManager() {
    Clear();
}

void SceneManager::Push(std::unique_ptr<Scene> scene) {
    m_pending.push_back({RequestType::Push, std::move(scene)});
}

void SceneManager::Pop() {
    m_pending.push_back({RequestType::Pop, nullptr});
}

void SceneManager::SwitchTo(std::unique_ptr<Scene> scene) {
    m_pending.push_back({RequestType::SwitchTo, std::move(scene)});
}

void SceneManager::ApplyPending() {
    // OnEnter/OnExit may queue new requests, so keep going until the queue stays empty.
    while (!m_pending.empty()) {
        std::vector<Request> requests = std::move(m_pending);
        m_pending.clear();

        for (Request& request : requests) {
            switch (request.type) {
            case RequestType::Push:
                PushNow(std::move(request.scene));
                break;
            case RequestType::Pop:
                PopNow();
                break;
            case RequestType::SwitchTo:
                while (!m_stack.empty()) {
                    PopNow();
                }
                PushNow(std::move(request.scene));
                break;
            }
        }
    }
}

void SceneManager::Update(float dt) {
    if (Scene* top = GetTop()) {
        top->Update(dt);
    }
}

void SceneManager::Draw(float alpha) {
    for (const std::unique_ptr<Scene>& scene : m_stack) {
        scene->Draw(alpha);
    }
}

void SceneManager::Clear() {
    m_pending.clear();
    while (!m_stack.empty()) {
        PopNow();
    }
}

void SceneManager::PushNow(std::unique_ptr<Scene> scene) {
    if (!scene) {
        return;
    }
    scene->m_app = &m_app;
    m_stack.push_back(std::move(scene));
    m_stack.back()->OnEnter();
}

void SceneManager::PopNow() {
    if (m_stack.empty()) {
        return;
    }
    m_stack.back()->OnExit();
    m_stack.pop_back();
}

} // namespace engine
