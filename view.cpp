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
                case DSVisualization::Status::ROTATE:
                    return Qt::GlobalColor::magenta;
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
        main_scene = new QGraphicsScene(this);
        setScene(main_scene);
        main_layout = new QGridLayout;
        insert_button = new QPushButton("Insert");
        erase_button = new QPushButton("Erase");
        find_button = new QPushButton("Find");
        insert_line_edit = new QLineEdit;
        erase_line_edit = new QLineEdit;
        find_line_edit = new QLineEdit;
        auto tree_scene = new QGraphicsScene();
        tree_view = new QGraphicsView(tree_scene);
        AddWidgetsToLayout();
        QObject::connect(insert_button, &QPushButton::clicked, this, &View::OnInsertButtonPushed);
        QObject::connect(erase_button, &QPushButton::clicked, this, &View::OnEraseButtonPushed);
        QObject::connect(find_button, &QPushButton::clicked, this, &View::OnFindButtonPushed);
        setLayout(main_layout);
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
        ++trees_to_show_counter;
        QTimer::singleShot(trees_to_show_counter * 500, this, [=]() {
            this->DrawTree(result);
            if (--trees_to_show_counter == 0) {
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
        main_layout->addWidget(tree_view, 0, 0, -1, -1);
        main_layout->addWidget(insert_line_edit, 1, 0);
        main_layout->addWidget(erase_line_edit, 1, 1);
        main_layout->addWidget(find_line_edit, 1, 2);
        main_layout->addWidget(insert_button, 2, 0);
        main_layout->addWidget(erase_button, 2, 1);
        main_layout->addWidget(find_button, 2, 2);
    }

    void View::SetVisibleButtons(bool flag) {
        PRINT_WHERE_AM_I();
        insert_button->setVisible(flag);
        erase_button->setVisible(flag);
        find_button->setVisible(flag);
        insert_line_edit->setVisible(flag);
        erase_line_edit->setVisible(flag);
        find_line_edit->setVisible(flag);
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
        std::string str = GetTextAndClear(insert_line_edit);
        HandlePushButton(TreeQueryType::INSERT, std::ref(str));
    }

    void View::OnEraseButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(erase_line_edit);
        HandlePushButton(TreeQueryType::ERASE, str);
    }

    void View::OnFindButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(find_line_edit);
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
        if (trees_to_show_counter == 0) {
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
        result->x = static_cast<float>(counter) * (horizontal_space_between_nodes + default_node_diameter);
        max_x = std::max(result->x, max_x);
        result->y = static_cast<float>(depth) * (default_node_diameter + vertical_space_between_nodes);
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
        node_diameter = default_node_diameter;
        current_width = static_cast<float>(size().width() - 100);
        if (max_x + default_node_diameter + 40 >= static_cast<float>(current_width)) {
            node_diameter = default_node_diameter / (max_x + default_node_diameter) * current_width;
        }
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
        tree_view->scene()->addEllipse(node->x, node->y, node_diameter, node_diameter, pen, brush);
        auto* text = new QGraphicsTextItem(std::to_string(node->key).c_str());
        auto rect = text->boundingRect();
        text->setPos(node->x - rect.width() / 2 + node_diameter / 2,
                     node->y - rect.height() / 2 + node_diameter / 2);
        text->setDefaultTextColor(Qt::white);
        tree_view->scene()->addItem(text);
    }

    void View::DrawEdgeBetweenNodes(const std::shared_ptr<DrawableNode>& parent,
                                    bool is_child_left) {
        PRINT_WHERE_AM_I();
        std::cerr << "radius = " << node_diameter << std::endl;
        float x1 = parent->x;
        float y1 = parent->y;
        float x2 = is_child_left ? parent->left->x : parent->right->x;
        float y2 = is_child_left ? parent->left->y : parent->right->y;
        auto* horizontal_line =
                new QGraphicsLineItem(x1 + (is_child_left ? 0 : node_diameter), y1 + node_diameter / 2,
                                      x2 + node_diameter / 2, y1 + node_diameter / 2);
        auto* vertical_line =
                new QGraphicsLineItem(x2 + node_diameter / 2, y1 + node_diameter / 2, x2 + node_diameter / 2, y2);
        tree_view->scene()->addItem(horizontal_line);
        tree_view->scene()->addItem(vertical_line);
    }

    void View::RecursiveDraw(const std::shared_ptr<DrawableNode>& node) {
        if (!node) {
            return;
        }
        if (max_x + default_node_diameter + 40 >= static_cast<float>(current_width)) {
            node->x = node->x / (max_x + default_node_diameter) * current_width;
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
