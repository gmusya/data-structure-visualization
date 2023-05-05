#pragma once

#include <functional>

namespace DSVisualization {
    template<typename Data>
    class Observable;

    template<typename Data>
    class Observer {
    public:
        explicit Observer(std::function<void(const Data& data)> on_subscribe,
                          std::function<void(const Data& data)> on_notify,
                          std::function<void(const Data& data)> on_unsubscribe) {
            this->on_notify = on_notify;
            this->on_subscribe = on_subscribe;
            this->on_unsubscribe = on_unsubscribe;
        }

        explicit Observer(std::function<void(const Data& data)> on_notify) {
            static std::function<void(const Data& data)> do_nothing = [](const Data&) {
            };
            this->on_notify = on_notify;
            this->on_subscribe = do_nothing;
            this->on_unsubscribe = do_nothing;
        }

        void OnNotify(const Data& data) {
            on_notify(data);
        }

        void OnUnsubscribe(const Data& data) {
            on_unsubscribe(data);
        }

        void OnSubscribe(const Data& data) {
            on_subscribe(data);
        }

    private:
        std::function<void(const Data& data)> on_notify;
        std::function<void(const Data& data)> on_subscribe;
        std::function<void(const Data& data)> on_unsubscribe;
    };
}// namespace DSVisualization