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
        values.Insert(value, observable_model_view);
    }

    void Model::Erase(int value) {
        PRINT_WHERE_AM_I();
        values.Erase(value);
        TreeInfo<int> tree_info = GetTreeInfo(values);
        observable_model_view->Notify(tree_info);
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
}// namespace DSVisualization