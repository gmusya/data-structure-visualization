#include "view.h"

#include "observable.h"
#include "observer.h"
#include "queries.h"
#include "utility.h"

#include <sstream>
#include <utility>

#include <ctime>
#include <QLayout>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <QtWidgets>
#include <thread>
#include <unistd.h>

namespace DSVisualization {

    namespace {
        Qt::GlobalColor FromStatusToQTColor(DSVisualization::Status status) {
            switch (status) {
                case DSVisualization::Status::TO_DELETE:
                    return Qt::GlobalColor::darkBlue;
                case DSVisualization::Status::TOUCHED:
                    return Qt::GlobalColor::green;
                case DSVisualization::Status::CURRENT:
                    return Qt::GlobalColor::yellow;
                default:
                    return Qt::GlobalColor::transparent;
            }
        }
    }

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
        scene = new QGraphicsScene;
        setScene(scene);
    }

    void View::HandlePushButton(TreeQueryType query_type, const std::string& text) {
        PRINT_WHERE_AM_I();
        s = new QSequentialAnimationGroup;
        try {
            int32_t value = std::stoi(text);
            observable_view_controller->Notify({query_type, value});
            s->start();
        } catch (...) {
            delete s;
            QMessageBox messageBox;
            QMessageBox::critical(nullptr, "Error", "An error has occured!");
        }
    }

    void View::OnInsertButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(addressText1);
        HandlePushButton(TreeQueryType::INSERT, std::ref(str));
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
        auto* button1 = new QPushButton("Insert");
        auto* button2 = new QPushButton("Erase");
        auto* button3 = new QPushButton("Find (not implemented)");
        addressText1 = new QLineEdit;
        addressText2 = new QLineEdit;
        addressText3 = new QLineEdit;
        auto tree_scene = new QGraphicsScene();
        q_painter = new QPainter;
        auto q_window = new QWindow;
        q_painter->setWindow(0, 0, 540, 500);
        tree_view = new QGraphicsView(tree_scene);
        // tree_view->setSceneRect(0, 0, 540, 500);
        mainLayout->addWidget(tree_view, 0, 0, -1, -1);
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


    std::shared_ptr<DrawableNode>
    View::DrawCurrentNode(const TreeInfo<int>& tree_info,
                          std::shared_ptr<RedBlackTree<int>::Node> node, int depth, int& counter) {
        if (!node) {
            return nullptr;
        }

        std::shared_ptr<DrawableNode> result = std::make_shared<DrawableNode>(
                DrawableNode{0, 0, 0, BLACK, static_cast<Status>(0), nullptr, nullptr});
        result->left = DrawCurrentNode(tree_info, node->left, depth + 1, counter);
        result->x = counter * width + counter * radius;
        max_x = std::max(result->x, max_x);
        result->y = depth * radius + height;
        result->key = node->value;
        result->color = node->color;
        result->status = tree_info.node_to_status.at(node);
        counter++;
        result->right = DrawCurrentNode(tree_info, node->right, depth + 1, counter);
        return result;
    }

    void View::OnNotifyFromModel(const DataModelView& value) {
        PRINT_WHERE_AM_I();
        int counter = 0;
        max_x = 0;
        std::shared_ptr<DrawableNode> result = DrawCurrentNode(value, value.root, 0, counter);
        ++cnt;
        QTimer::singleShot(cnt * 500, this, [=]() { this->Draw(result); --cnt; });
        // Draw(result);
        // label->setText(std::to_string(value.node_to_info.size()).c_str());
    }

    void View::SubscribeFromController(ObserverViewControllerPtr observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller->Subscribe(std::move(observer_view_controller));
    }

    void View::UnsubscribeFromController(ObserverViewControllerPtr observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller->Unsubscribe(std::move(observer_view_controller));
    }

    void View::Draw(std::shared_ptr<DrawableNode> node) {
        tree_view->scene()->clear();
        RecursiveDraw(node);
        tree_view->show();
    }

    void View::RecursiveDraw(std::shared_ptr<DrawableNode> node) {

        if (!node) {
            return;
        }
        double r = radius;
        if (max_x >= 800) {
            node->x = node->x / max_x * 800;
            r = radius / max_x * 800;
        }
        RecursiveDraw(node->left);
        if (node->left) {
            int x1 = node->x;
            int y1 = node->y;
            int x2 = node->left->x;
            int y2 = node->left->y;
            auto* it = new QGraphicsLineItem(x1 + r / 2, y1 + r / 2, x2 + r / 2, y2 + r / 2);
            // tree_view->scene()->addItem(it);
            auto* it1 = new QGraphicsLineItem(x1, y1 + r / 2, x2 + r / 2, y1 + r / 2);
            tree_view->scene()->addItem(it1);

            auto* it2 = new QGraphicsLineItem(x2 + r / 2, y1 + r / 2, x2 + r / 2, y2);
            tree_view->scene()->addItem(it2);
        }
        QPen pen;
        QBrush brush;
        pen.setWidth(5);
        brush.setColor(node->color == Color::RED ? Qt::red : Qt::black);
        brush.setStyle(Qt::SolidPattern);
        pen.setColor(FromStatusToQTColor(node->status));
        tree_view->scene()->addEllipse(node->x, node->y, r, r, pen, brush);
        auto* text = new QGraphicsTextItem(std::to_string(node->key).c_str());
        auto rect = text->boundingRect();
        std::cerr << node->x << ' ' << node->y << std::endl;
        std::cerr << rect.x() << ' ' << rect.y() << ' ' << rect.width() << ' ' << rect.height()
                  << std::endl;
        text->setPos(node->x - rect.width() / 2 + r / 2, node->y - rect.height() / 2 + r / 2);
        text->setDefaultTextColor(Qt::white);
        tree_view->scene()->addItem(text);
        RecursiveDraw(node->right);
        if (node->right) {
            int x1 = node->x;
            int y1 = node->y;
            int x2 = node->right->x;
            int y2 = node->right->y;
            auto* it = new QGraphicsLineItem(x1 + r, y1 + r / 2, x2 + r / 2, y1 + r / 2);
            tree_view->scene()->addItem(it);
            auto* it2 = new QGraphicsLineItem(x2 + r / 2, y1 + r / 2, x2 + r / 2, y2);
            tree_view->scene()->addItem(it2);
        }
    }
}// namespace DSVisualization
