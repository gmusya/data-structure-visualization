#include "view.h"

#include "observable.h"
#include "observer.h"
#include "queries.h"
#include "utility.h"

#include <sstream>
#include <utility>

#include <QLayout>
#include <QPushButton>
#include <QString>
#include <QtWidgets>

namespace DSVisualization {
    std::string GetTextAndClear(QLineEdit* button) {
        PRINT_WHERE_AM_I();
        assert(button != nullptr);
        std::string result = button->text().toStdString();
        button->clear();
        return result;
    }

    View::View()
        : observer_model_view(
                  std::make_shared<Observer<DataModelView>>([this](const DataModelView& x) {
                      OnNotifyFromModel(x);
                  })) {
        PRINT_WHERE_AM_I();
        observable_view_controller = std::make_shared<Observable<DataViewController>>();
        setScene(&scene);
    }

    void View::HandlePushButton(TreeQueryType query_type, const std::string& text) {
        PRINT_WHERE_AM_I();
        try {
            int32_t value = std::stoi(text);
            observable_view_controller->Notify({query_type, value});
        } catch (...) {
            QMessageBox messageBox;
            QMessageBox::critical(nullptr, "Error", "An error has occured!");
        }
    }

    void View::OnInsertButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(addressText1);
        HandlePushButton(TreeQueryType::INSERT, str);
    }

    void View::OnEraseButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(addressText2);
        HandlePushButton(TreeQueryType::ERASE, str);
    }

    void View::OnFindButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(addressText3);
        HandlePushButton(TreeQueryType::FIND, str);
    }

    void View::DoStuff() {
        PRINT_WHERE_AM_I();
        mainLayout = new QGridLayout;
        label = new QLabel;
        auto* button1 = new QPushButton("Insert");
        auto* button2 = new QPushButton("Erase");
        auto* button3 = new QPushButton("Find (not implemented)");
        addressText1 = new QLineEdit;
        addressText2 = new QLineEdit;
        addressText3 = new QLineEdit;
        auto scene2 = new QGraphicsScene();
        auto view2 = new QGraphicsView(scene2);
        view2->setSceneRect(0, 0, 540, 500);
        mainLayout->addWidget(label, 0, 0, -1, -1);
        mainLayout->addWidget(addressText1, 1, 0);
        mainLayout->addWidget(addressText2, 1, 1);
        mainLayout->addWidget(addressText3, 1, 2);
        mainLayout->addWidget(button1, 2, 0);
        mainLayout->addWidget(button2, 2, 1);
        mainLayout->addWidget(button3, 2, 2);
        QObject::connect(button1, &QPushButton::clicked, this, &View::OnInsertButtonPushed);
        QObject::connect(button2, &QPushButton::clicked, this, &View::OnEraseButtonPushed);
        QObject::connect(button3, &QPushButton::clicked, this, &View::OnFindButtonPushed);
        setLayout(mainLayout);
        setSceneRect(0, 0, 500, 500);
        setMinimumSize(960, 540);
        setMaximumSize(960, 540);
        show();
    }

    [[nodiscard]] ObserverModelViewPtr View::GetObserver() const {
        PRINT_WHERE_AM_I();
        return observer_model_view;
    }

    void View::OnNotifyFromModel(const DataModelView& value) {
        PRINT_WHERE_AM_I();
        label->setText(value.c_str());
    }

    void View::SubscribeFromController(ObserverViewControllerPtr observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller->Subscribe(std::move(observer_view_controller));
    }

    void View::UnsubscribeFromController(ObserverViewControllerPtr observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller->Unsubscribe(std::move(observer_view_controller));
    }

}// namespace DSVisualization
