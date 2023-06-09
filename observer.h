#pragma once

namespace DSVisualization {
    template<typename T>
    class Observable;

    template<class T>
    class Observer {
        friend Observable<T>;

    public:
        template<class Tt1, class Tt2, class Tt3>
        Observer(Tt1&& on_subscribe, Tt2&& on_notify, Tt3&& on_unsubscribe)
            : on_subscribe_(std::forward<Tt1>(on_subscribe)),
              on_notify_(std::forward<Tt2>(on_notify)),
              on_unsubscribe_(std::forward<Tt3>(on_unsubscribe)) {
        }

        template<class Tt2>
        explicit Observer(Tt2&& on_notify)
            : on_subscribe_(do_nothing), on_notify_(std::forward<Tt2>(on_notify)),
              on_unsubscribe_(do_nothing) {
        }
        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;
        Observer(Observer&&) = delete;
        Observer& operator=(Observer&&) = delete;

        ~Observer() {
            Unsubscribe();
        }

        void Unsubscribe() {
            if (!IsSubscribed()) {
                return;
            }
            observable_->Detach(this);
            observable_ = nullptr;
        }

        [[nodiscard]] bool IsSubscribed() const {
            return observable_;
        }

        static void do_nothing(T){};

    private:
        void SetObservable(Observable<T>* observable) {
            observable_ = observable;
        }

        using Action = std::function<void(T)>;

        Observable<T>* observable_ = nullptr;
        Action on_subscribe_ = do_nothing;
        Action on_notify_ = do_nothing;
        Action on_unsubscribe_ = do_nothing;
    };
}// namespace DSVisualization
