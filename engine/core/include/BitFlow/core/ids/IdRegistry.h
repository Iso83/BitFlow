#pragma once

#include <unordered_set>
#include <vector>

namespace BitFlow::Core::Ids {

template <typename TId> class GraphRegistry {
  protected:
    using ValueType = typename TId::ValueType;

    ValueType m_nextId{1};
    std::vector<ValueType> m_freeIds{};
    std::unordered_set<ValueType> m_alive{};

  public:
    using const_iterator = typename std::unordered_set<ValueType>::const_iterator;

    virtual ~GraphRegistry() = default;

    virtual TId createId() {
        ValueType idValue{};

        if (!m_freeIds.empty()) {
            idValue = m_freeIds.back();
            m_freeIds.pop_back();
        } else {
            idValue = m_nextId++;
        }

        m_alive.insert(idValue);
        return TId{idValue};
    }

    virtual bool remove(TId id) {
        const auto value = id.value();

        auto it = m_alive.find(value);
        if (it == m_alive.end()) {
            return false;
        }

        onBeforeRemove(id);

        m_alive.erase(it);
        m_freeIds.push_back(value);

        onAfterRemove(id);
        return true;
    }

    virtual bool contains(TId id) const {
        return m_alive.contains(id.value());
    }

    const_iterator begin() const {
        return m_alive.begin();
    }

    const_iterator end() const {
        return m_alive.end();
    }

  protected:
    virtual void onBeforeRemove(TId) {}
    virtual void onAfterRemove(TId) {}
};

} // namespace ScopeCanvas::Core::Ids