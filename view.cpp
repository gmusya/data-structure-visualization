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
#include <thread>

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

        std::string GetTextAndClear(QLineEdit* button) {
            PRINT_WHERE_AM_I();
            assert(button != nullptr);
            std::string result = button->text().toStdString();
            button->clear();
            return result;
        }
    }// namespace

    View::View()
        : observer_model_view(
                  std::make_shared<Observer<DataModelView>>([this](const DataModelView& x) {
                      OnNotifyFromModel(x);
                  })) {
        PRINT_WHERE_AM_I();
        observable_view_controller = std::make_shared<Observable<DataViewController>>();
        scene = new QGraphicsScene(this);
        setScene(scene);
        mainLayout = new QGridLayout;
        button1 = new QPushButton("Insert");
        button2 = new QPushButton("Erase");
        button3 = new QPushButton("Find (not implemented)");
        addressText1 = new QLineEdit;
        addressText2 = new QLineEdit;
        addressText3 = new QLineEdit;
        auto tree_scene = new QGraphicsScene();
        tree_view = new QGraphicsView(tree_scene);
        AddWidgetsToLayout();
        QObject::connect(button1, &QPushButton::clicked, this, &View::OnInsertButtonPushed);
        QObject::connect(button2, &QPushButton::clicked, this, &View::OnEraseButtonPushed);
        QObject::connect(button3, &QPushButton::clicked, this, &View::OnFindButtonPushed);
        setLayout(mainLayout);
        setSceneRect(0, 0, 500, 500);
        setMinimumSize(960, 540);
        show();
    }

    [[nodiscard]] ObserverModelViewPtr View::GetObserver() const {
        PRINT_WHERE_AM_I();
        return observer_model_view;
    }

    void View::OnNotifyFromModel(const DataModelView& value) {
        PRINT_WHERE_AM_I();
        int counter = 0;
        max_x = 0;
        std::shared_ptr<DrawableNode> result = GetDrawableNode(value, value.root, 0, counter);
        ++cnt;
        QTimer::singleShot(cnt * 500, this, [=]() {
            this->DrawTree(result);
            if (--cnt == 0) {
                ShowButtons();
            }
        });
    }

    void View::SubscribeFromController(ObserverViewControllerPtr observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller->Subscribe(std::move(observer_view_controller));
    }

    void View::UnsubscribeFromController(ObserverViewControllerPtr observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller->Unsubscribe(std::move(observer_view_controller));
    }

    void View::AddWidgetsToLayout() {
        PRINT_WHERE_AM_I();
        mainLayout->addWidget(tree_view, 0, 0, -1, -1);
        mainLayout->addWidget(addressText1, 1, 0);
        mainLayout->addWidget(addressText2, 1, 1);
        mainLayout->addWidget(addressText3, 1, 2);
        mainLayout->addWidget(button1, 2, 0);
        mainLayout->addWidget(button2, 2, 1);
        mainLayout->addWidget(button3, 2, 2);
    }

    void View::SetVisibleButtons(bool flag) {
        PRINT_WHERE_AM_I();
        button1->setVisible(flag);
        button2->setVisible(flag);
        button3->setVisible(flag);
        addressText1->setVisible(flag);
        addressText2->setVisible(flag);
        addressText3->setVisible(flag);
    }

    void View::HideButtons() {
        PRINT_WHERE_AM_I();
        SetVisibleButtons(false);
    }

    void View::ShowButtons() {
        PRINT_WHERE_AM_I();
        SetVisibleButtons(true);
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

    void View::HandlePushButton(TreeQueryType query_type, const std::string& text) {
        PRINT_WHERE_AM_I();
        HideButtons();
        try {
            int32_t value = std::stoi(text);
            observable_view_controller->Notify({query_type, value});
        } catch (...) {
            QMessageBox messageBox;
            QMessageBox::critical(nullptr, "Error", "An error has occurred!");
        }
        if (cnt == 0) {
            ShowButtons();
        }
    }

    std::shared_ptr<DrawableNode>
    View::GetDrawableNode(const TreeInfo<int>& tree_info,
                          const std::shared_ptr<RedBlackTree<int>::Node>& node, int depth,
                          int& counter) {
        if (!node) {
            return nullptr;
        }

        std::shared_ptr<DrawableNode> result = std::make_shared<DrawableNode>(
                DrawableNode{0, 0, 0, BLACK, static_cast<Status>(0), nullptr, nullptr});
        result->left = GetDrawableNode(tree_info, node->left, depth + 1, counter);
        result->x = static_cast<float>(counter) * (width + radius);
        max_x = std::max(result->x, max_x);
        result->y = static_cast<float>(depth) * (radius + height);
        result->key = node->value;
        result->color = node->color;
        result->status = tree_info.node_to_status.at(node);
        counter++;
        result->right = GetDrawableNode(tree_info, node->right, depth + 1, counter);
        return result;
    }

    void View::DrawTree(const std::shared_ptr<DrawableNode>& root) {
        PRINT_WHERE_AM_I();
        tree_view->scene()->clear();
        RecursiveDraw(root);
        tree_view->show();
    }

    void View::DrawNode(const std::shared_ptr<DrawableNode>& node) {
        PRINT_WHERE_AM_I();
        QPen pen;
        QBrush brush;
        pen.setWidth(5);
        brush.setColor(node->color == Color::RED ? Qt::red : Qt::black);
        brush.setStyle(Qt::SolidPattern);
        pen.setColor(FromStatusToQTColor(node->status));
        tree_view->scene()->addEllipse(node->x, node->y, radius, radius, pen, brush);
        auto* text = new QGraphicsTextItem(std::to_string(node->key).c_str());
        auto rect = text->boundingRect();
        text->setPos(node->x - rect.width() / 2 + radius / 2,
                     node->y - rect.height() / 2 + radius / 2);
        text->setDefaultTextColor(Qt::white);
        tree_view->scene()->addItem(text);
    }

    void View::DrawEdgeBetweenNodes(const std::shared_ptr<DrawableNode>& parent,
                                    bool is_child_left) {
        PRINT_WHERE_AM_I();
        float x1 = parent->x;
        float y1 = parent->y;
        float x2 = is_child_left ? parent->left->x : parent->right->x;
        float y2 = is_child_left ? parent->left->y : parent->right->y;
        auto* horizontal_line =
                new QGraphicsLineItem(x1 + (is_child_left ? 0 : radius), y1 + radius / 2,
                                      x2 + radius / 2, y1 + radius / 2);
        auto* vertical_line =
                new QGraphicsLineItem(x2 + radius / 2, y1 + radius / 2, x2 + radius / 2, y2);
        tree_view->scene()->addItem(horizontal_line);
        tree_view->scene()->addItem(vertical_line);
    }

    void View::RecursiveDraw(const std::shared_ptr<DrawableNode>& node) {
        if (!node) {
            return;
        }
        radius = default_radius;
        if (max_x >= 800) {
            node->x = node->x / max_x * 800;
            radius = default_radius / max_x * 800;
        }
        RecursiveDraw(node->left);
        if (node->left) {
            DrawEdgeBetweenNodes(node, true);
        }
        DrawNode(node);
        RecursiveDraw(node->right);
        if (node->right) {
            DrawEdgeBetweenNodes(node, false);
        }
    }
}// namespace DSVisualization
