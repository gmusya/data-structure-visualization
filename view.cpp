#include "view.h"
#include "observable.h"
#include "observer.h"
#include "queries.h"
#include "utility.h"

#include <memory>
#include <sstream>

#include <QLayout>
#include <QPushButton>
#include <QString>
#include <QtWidgets>

namespace DSVisualization {
    namespace {
        Qt::GlobalColor FromStatusToQTColor(DSVisualization::Status status) {
            switch (status) {
                case DSVisualization::Status::to_delete:
                    return Qt::GlobalColor::darkBlue;
                case DSVisualization::Status::touched:
                    return Qt::GlobalColor::green;
                case DSVisualization::Status::current:
                    return Qt::GlobalColor::yellow;
                case DSVisualization::Status::rotate:
                    return Qt::GlobalColor::magenta;
                case DSVisualization::Status::found:
                    return Qt::GlobalColor::cyan;
                default:
                    return Qt::GlobalColor::transparent;
            }
        }

        std::string GetTextAndClear(QLineEdit* line_edit) {
            PRINT_WHERE_AM_I();
            assert(line_edit != nullptr);
            std::string result = line_edit->text().toStdString();
            line_edit->clear();
            return result;
        }
    }// namespace

    View::View(MainWindow* main_window)
        : main_window_ptr_(main_window), observer_model_view_(
                                                 [this](const RedBlackTree<int>::Data& x) {
                                                     OnNotifyFromModel(x);
                                                 },
                                                 [this](const RedBlackTree<int>::Data& x) {
                                                     OnNotifyFromModel(x);
                                                 },
                                                 [this](const RedBlackTree<int>::Data& x) {
                                                     OnNotifyFromModel(x);
                                                 }),
          observable_view_controller_([this]() {
              return this->query_;
          }) {
        PRINT_WHERE_AM_I();

        QObject::connect(main_window_ptr_->insert_button_, &QPushButton::clicked, this,
                         &View::OnInsertButtonPushed);
        QObject::connect(main_window_ptr_->erase_button_, &QPushButton::clicked, this,
                         &View::OnEraseButtonPushed);
        QObject::connect(main_window_ptr_->find_button_, &QPushButton::clicked, this,
                         &View::OnFindButtonPushed);
    }

    [[nodiscard]] Observer<RedBlackTree<int>::Data>* View::GetObserver() {
        PRINT_WHERE_AM_I();
        return &observer_model_view_;
    }

    static void Delay(int milliseconds) {
        QEventLoop loop;
        QTimer t;
        QTimer::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start(milliseconds);
        loop.exec();
    }

    void View::OnNotifyFromModel(const RedBlackTree<int>::Data& value) {
        PRINT_WHERE_AM_I();
        float counter = 0;
        tree_width_ = static_cast<float>(value.tree_size) *
                      (horizontal_space_between_nodes + default_node_diameter);
        std::unique_ptr<DrawableTree> result =
                std::make_unique<DrawableTree>(GetDrawableNode(value, value.root, 0, counter));
        this->DrawTree(result);
        Delay(draw_delay_in_ms);
    }

    void View::SubscribeToQuery(Observer<TreeQuery>* observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller_.Subscribe(observer_view_controller);
    }

    void View::OnInsertButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(main_window_ptr_->insert_line_edit_);
        HandlePushButton(TreeQueryType::insert, std::ref(str));
    }

    void View::OnEraseButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(main_window_ptr_->erase_line_edit_);
        HandlePushButton(TreeQueryType::erase, str);
    }

    void View::OnFindButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(main_window_ptr_->find_line_edit_);
        HandlePushButton(TreeQueryType::find, str);
    }

    namespace {
        const int MIN_VALUE = -128;
        const int MAX_VALUE = 127;
        std::variant<int, std::string> string_to_int(const std::string& text) {
            if (text.empty()) {
                return "Empty query";
            }
            int result = 0;
            bool is_negative = false;
            size_t first_pos = 0;
            if (text[0] == '+') {
                first_pos = 1;
            } else if (text[0] == '-') {
                first_pos = 1;
                is_negative = true;
            }
            if (first_pos == text.size()) {
                return "Empty query";
            }
            for (size_t i = first_pos; i < text.size(); ++i) {
                if (!('0' <= text[i] && text[i] <= '9')) {
                    return "Value must be a number";
                }
                result = result * 10 + (text[i] - '0');
                if (result > MAX_VALUE) {
                    return "Value must be in range from " + std::to_string(MIN_VALUE) + " to " +
                           std::to_string(MAX_VALUE);
                }
            }
            if (is_negative) {
                result *= -1;
            }
            if (!(MIN_VALUE <= result && result <= MAX_VALUE)) {
                return "Value must be in range from " + std::to_string(MIN_VALUE) + " to " +
                       std::to_string(MAX_VALUE);
            }
            return result;
        }
    }// namespace

    void View::HandlePushButton(TreeQueryType query_type, const std::string& text) {
        PRINT_WHERE_AM_I();
        main_window_ptr_->DisableButtons();
        std::variant<int, std::string> value = string_to_int(text);
        if (value.index() == 0) {
            query_ = {query_type, std::get<int>(value)};
            observable_view_controller_.Notify();
        } else {
            QMessageBox messageBox;
            QMessageBox::critical(nullptr, "Error", std::get<std::string>(value).c_str());
        }
        main_window_ptr_->EnableButtons();
    }

    std::unique_ptr<DrawableNode> View::GetDrawableNode(const TreeInfo<int>& tree_info,
                                                        const RedBlackTree<int>::Node* node,
                                                        float depth, float& counter) {
        if (!node) {
            return nullptr;
        }

        std::unique_ptr<DrawableNode> result = std::make_unique<DrawableNode>(DrawableNode{
                0, 0, 0, Qt::black, FromStatusToQTColor(Status::initial), nullptr, nullptr});
        result->left = GetDrawableNode(tree_info, node->left.get(), depth + 1, counter);
        result->x = counter * (horizontal_space_between_nodes + default_node_diameter);
        result->y = depth * (default_node_diameter + vertical_space_between_nodes);
        assert(result);
        assert(node);
        result->key = node->value;
        result->inside_color = (node->color == Color::red ? Qt::red : Qt::black);
        {
            auto it = tree_info.node_to_status.find(node);
            auto status =
                    (it == tree_info.node_to_status.end() ? Status::initial
                                                          : tree_info.node_to_status.at(node));
            result->outside_color = FromStatusToQTColor(status);
        }
        counter++;
        result->right = GetDrawableNode(tree_info, node->right.get(), depth + 1, counter);
        return result;
    }

    void View::DrawTree(const std::unique_ptr<DrawableTree>& tree) {
        PRINT_WHERE_AM_I();
        main_window_ptr_->tree_view_->scene()->clear();
        current_node_diameter_ = default_node_diameter;
        main_window_ptr_->current_width_ = static_cast<float>(size().width());
        if (tree_width_ + default_node_diameter + MainWindow::margin >=
            main_window_ptr_->current_width_) {
            current_node_diameter_ = (default_node_diameter * main_window_ptr_->current_width_) /
                                     (tree_width_ + default_node_diameter + MainWindow::margin);
        }
        RecursiveDraw(tree->root);
        main_window_ptr_->tree_view_->show();
    }

    void View::DrawNode(const std::unique_ptr<DrawableNode>& node) {
        PRINT_WHERE_AM_I();
        QPen pen;
        QBrush brush;
        pen.setWidth(5);
        brush.setColor(node->inside_color);
        brush.setStyle(Qt::SolidPattern);
        pen.setColor(node->outside_color);
        main_window_ptr_->tree_view_->scene()->addEllipse(node->x, node->y, current_node_diameter_,
                                                          current_node_diameter_, pen, brush);
        auto* text = new QGraphicsTextItem(std::to_string(node->key).c_str());
        auto rect = text->boundingRect();
        text->setPos(node->x - rect.width() / 2 + current_node_diameter_ / 2,
                     node->y - rect.height() / 2 + current_node_diameter_ / 2);
        text->setDefaultTextColor(Qt::white);
        main_window_ptr_->tree_view_->scene()->addItem(text);
    }

    void View::DrawEdgeBetweenNodes(const std::unique_ptr<DrawableNode>& parent,
                                    bool is_child_left) {
        PRINT_WHERE_AM_I();
        std::cerr << "radius = " << current_node_diameter_ << std::endl;
        float x1 = parent->x;
        float y1 = parent->y;
        float x2 = is_child_left ? parent->left->x : parent->right->x;
        float y2 = is_child_left ? parent->left->y : parent->right->y;
        auto* horizontal_line = new QGraphicsLineItem(
                x1 + (is_child_left ? 0 : current_node_diameter_), y1 + current_node_diameter_ / 2,
                x2 + current_node_diameter_ / 2, y1 + current_node_diameter_ / 2);
        auto* vertical_line = new QGraphicsLineItem(x2 + current_node_diameter_ / 2,
                                                    y1 + current_node_diameter_ / 2,
                                                    x2 + current_node_diameter_ / 2, y2);
        main_window_ptr_->tree_view_->scene()->addItem(horizontal_line);
        main_window_ptr_->tree_view_->scene()->addItem(vertical_line);
    }

    void View::RecursiveDraw(const std::unique_ptr<DrawableNode>& node) {
        if (!node) {
            return;
        }
        if (tree_width_ + default_node_diameter + MainWindow::margin >=
            static_cast<float>(main_window_ptr_->current_width_)) {
            node->x = node->x / (tree_width_ + default_node_diameter + MainWindow::margin) *
                      main_window_ptr_->current_width_;
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
