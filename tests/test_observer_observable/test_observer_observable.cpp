#include "../../observable.h"
#include "../../observer.h"

#include <gtest/gtest.h>

namespace DSVisualization {

    namespace {
        class ValueSetter {
        public:
            explicit ValueSetter(int& value) : value_(value) {
            }

            void operator()(int x) {
                value_ = x;
            }

        private:
            int& value_;
        };
    }// namespace

    TEST(ObserverObservable, Notify) {
        int x = 0;
        Observable<int> observable([&x]() {
            return x;
        });
        int y = 0;
        Observer<int> observer((ValueSetter(y)));
        x = 1;
        ASSERT_TRUE(y == 0);
        observable.Subscribe(&observer);
        ASSERT_TRUE(y == 0);
        observable.Notify();
        ASSERT_TRUE(y == 1);
    }

    TEST(ObserverObservable, Subscribe) {
        int x = 0;
        Observable<int> observable([&x]() {
            return x;
        });
        int y = 0;
        Observer<int> observer(ValueSetter(y), Observer<int>::do_nothing,
                               Observer<int>::do_nothing);
        x = 1;
        ASSERT_TRUE(y == 0);
        observable.Subscribe(&observer);
        ASSERT_TRUE(y == 1);
    }

    TEST(ObserverObservable, Unsubscribe) {
        int x = 0;
        Observable<int> observable([&x]() {
            return x;
        });
        int y = 0;
        {
            Observer<int> observer(Observer<int>::do_nothing, Observer<int>::do_nothing,
                                   ValueSetter(y));
            x = 1;
            ASSERT_TRUE(y == 0);
            observable.Subscribe(&observer);
            ASSERT_TRUE(y == 0);
        }
        ASSERT_TRUE(y == 1);
    }


    TEST(ObserverObservable, ChangeSubscription) {
        int a = 0;
        int b = 0;
        int c = 0;
        int x = 0;
        int y = 10;
        {
            Observable<int> observable_x([&x]() {
                return ++x;
            });
            Observable<int> observable_y([&y]() {
                return ++y;
            });
            Observer<int> observer((ValueSetter(a)), ValueSetter(b), ValueSetter(c));
            observable_x.Subscribe(&observer);
            ASSERT_TRUE(std::vector<int>({1, 0, 0, 1, 10}) == std::vector<int>({a, b, c, x, y}));
            observable_x.Notify();
            ASSERT_TRUE(std::vector<int>({1, 2, 0, 2, 10}) == std::vector<int>({a, b, c, x, y}));
            observable_y.Subscribe(&observer);
            ASSERT_TRUE(std::vector<int>({11, 2, 3, 3, 11}) == std::vector<int>({a, b, c, x, y}));
            observable_y.Subscribe(&observer);
            ASSERT_TRUE(std::vector<int>({13, 2, 12, 3, 13}) == std::vector<int>({a, b, c, x, y}));
            observable_x.Notify();
            ASSERT_TRUE(std::vector<int>({13, 2, 12, 3, 13}) == std::vector<int>({a, b, c, x, y}));
            observable_y.Notify();
            ASSERT_TRUE(std::vector<int>({13, 14, 12, 3, 14}) == std::vector<int>({a, b, c, x, y}));
        }
        ASSERT_TRUE(std::vector<int>({13, 14, 15, 3, 15}) == std::vector<int>({a, b, c, x, y}));
    }

    TEST(ObserverObservable, MultipleSubscriptions) {
        int x = 0;
        size_t observers_count = 10;
        std::vector<int> values(observers_count);
        {
            Observable<int> observable_x([&x]() {
                return ++x;
            });
            std::vector<std::unique_ptr<Observer<int>>> observers(observers_count);
            for (size_t i = 0; i < observers_count; ++i) {
                observers[i] = std::make_unique<Observer<int>>(
                        ValueSetter(values[i]), ValueSetter(values[i]), ValueSetter(values[i]));
            }
            for (size_t i = 0; i < observers_count; ++i) {
                observable_x.Subscribe(observers[i].get());
            }
            for (size_t i = 0; i < observers_count; ++i) {
                ASSERT_TRUE(values[i] == i + 1);
            }
            observable_x.Notify();
            for (size_t i = 0; i < observers_count; ++i) {
                ASSERT_TRUE(values[i] == observers_count + i + 1);
            }
        }
        for (size_t i = 0; i < observers_count; ++i) {
            ASSERT_TRUE(values[i] == 2 * observers_count + i + 1);
        }
    }
}// namespace DSVisualization
