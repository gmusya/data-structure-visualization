#pragma once

#include <functional>
#include <list>

namespace DSVisualization {
    template<typename T>
    class Observer;

    template<typename T>
    class Observable {
        friend Observer<T>;

    public:
        template<typename Tt>
        Observable(Tt&& data) : data_(std::forward<Tt>(data)) {
        }

        Observable(const Observable<T>&) = delete;
        Observable& operator=(const Observable<T>&) = delete;
        Observable(Observable&&) = delete;
        Observable& operator=(Observer<T>&&) = delete;
        ~Observable() {
            while (!observers_.empty()) {
                observers_.front()->Unsubscribe();
            }
        }

        void Subscribe(Observer<T>* obs) {
            if (obs->IsSubscribed()) {
                obs->Unsubscribe();
            }
            observers_.push_back(obs);
            obs->SetObservable(this);
            obs->on_subscribe_(data_());
        }

        void Notify() const {
            for (auto obs : observers_) {
                obs->on_notify_(data_());
            }
        }

        void Set(const T& data) {
            data_ = [&data]() {
                return data;
            };
            Notify();
        }

    private:
        void Detach(Observer<T>* obs) {
            obs->on_unsubscribe_(data_());
            observers_.remove(obs);
        }
        std::function<T()> data_;
        std::list<Observer<T>*> observers_;
    };
}// namespace DSVisualization
