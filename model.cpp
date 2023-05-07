#include "model.h"
#include "observable.h"
#include "utility.h"

#include <utility>

namespace DSVisualization {
    Model::Model() {
        PRINT_WHERE_AM_I();
        observable_model_view = std::make_shared<Observable<DataModelView>>();
    }

    Model::~Model() {
        PRINT_WHERE_AM_I();
    }

    void Model::SubscribeFromView(ObserverModelViewPtr observer) {
        PRINT_WHERE_AM_I();
        observable_model_view->Subscribe(std::move(observer));
    }

    void Model::UnsubscribeFromView(ObserverModelViewPtr view) {
        PRINT_WHERE_AM_I();
        observable_model_view->Unsubscribe(std::move(view));
    }

    void Model::Insert(int value) {
        PRINT_WHERE_AM_I();
        values.Insert(value, observable_model_view);
    }

    void Model::Erase(int value) {
        PRINT_WHERE_AM_I();
        values.Erase(value, observable_model_view);
    }

    void Model::Find(int value) {
        PRINT_WHERE_AM_I();
        values.Find(value, observable_model_view);
    }
}// namespace DSVisualization