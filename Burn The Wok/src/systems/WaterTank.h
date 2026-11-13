#pragma once

#include <algorithm>

namespace game {

// The ranger's water supply. Pure logic (no input, no drawing), so the rules can be
// tested and re-tuned without touching the sprayer or the HUD.
class WaterTank {
public:
    explicit WaterTank(float capacity) : m_capacity(capacity), m_amount(capacity) {}

    float GetAmount() const { return m_amount; }
    float GetCapacity() const { return m_capacity; }
    float GetFraction() const { return m_capacity > 0.0f ? m_amount / m_capacity : 0.0f; }
    bool IsEmpty() const { return m_amount <= 0.0f; }
    bool IsFull() const { return m_amount >= m_capacity; }

    // Takes exactly `amount` if the tank holds that much; otherwise takes nothing and
    // returns false. All-or-nothing keeps one spray drop = one fixed cost.
    bool TryUse(float amount) {
        if (amount > m_amount) return false;
        m_amount -= amount;
        return true;
    }

    void Refill(float amount) { m_amount = std::min(m_capacity, m_amount + std::max(0.0f, amount)); }

private:
    float m_capacity;
    float m_amount;
};

} // namespace game
