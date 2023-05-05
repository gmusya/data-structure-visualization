#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

namespace DSVisualization {
    template<typename Data>
    class Observer;

    template<typename Data>
    class Observable {

    public:
        void Subscribe(std::shared_ptr<Observer<Data>> observer) {
            observers.push_back(observer);
        }

        void Subscribe(std::shared_ptr<Observer<Data>> observer, const Data& data) {
            observers.push_back(observer);
            observer->OnSubscribe(data);
        }

        void Notify(const Data& data) {
            for (auto& observer : observers) {
                observer->OnNotify(data);
            }
        }

        void Unsubscribe(std::shared_ptr<Observer<Data>> observer, const Data& data) {
            auto it = std::find(observers.begin(), observers.end(), observer);
            (*it)->OnUnsubscribe(data);
            observers.erase(it);
        }

        void Unsubscribe(std::shared_ptr<Observer<Data>> observer) {
            auto it = std::find(observers.begin(), observers.end(), observer);
            observers.erase(it);
        }

    private:
        std::vector<std::shared_ptr<Observer<Data>>> observers;
    };
}// namespace DSVisualization