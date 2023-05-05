#include "model.h"
#include "observable.h"
#include "observer.h"
#include "utility.h"

#include <utility>

namespace DSVisualization {
    Model::Model() {
        PRINT_WHERE_AM_I();
        observable_model_view = std::make_shared<Observable<DataModelView>>();
    }

    void Model::Insert(int value) {
        PRINT_WHERE_AM_I();
        values.insert(value);
        observable_model_view->Notify(values);
    }

    void Model::Erase(int value) {
        PRINT_WHERE_AM_I();
        values.erase(value);
        observable_model_view->Notify(values);
    }


    Model::~Model() {
        PRINT_WHERE_AM_I();
    }

    void Model::SubscribeFromView(ObserverModelViewPtr observer) {
        PRINT_WHERE_AM_I();
        observable_model_view->Subscribe(std::move(observer), values);
    }

    void Model::UnsubscribeFromView(ObserverModelViewPtr view) {
        PRINT_WHERE_AM_I();
        observable_model_view->Unsubscribe(std::move(view), values);
    }
}// namespace DSVisualization