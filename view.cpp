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

    View::View()
        : main_layout_(new QGridLayout()), insert_button_(new QPushButton("Insert")),
          erase_button_(new QPushButton("Erase")), find_button_(new QPushButton("Find")),
          insert_line_edit_(new QLineEdit()), erase_line_edit_(new QLineEdit()),
          find_line_edit_(new QLineEdit()), tree_scene_(new QGraphicsScene()),
          tree_view_(new QGraphicsView(tree_scene_.get())), main_scene_(new QGraphicsScene()),

          observer_model_view_(
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
        setMinimumSize(default_width, default_height);
        setScene(main_scene_.get());
        AddWidgetsToLayout();
        setLayout(main_layout_.get());
        QObject::connect(insert_button_.get(), &QPushButton::clicked, this,
                         &View::OnInsertButtonPushed);
        QObject::connect(erase_button_.get(), &QPushButton::clicked, this,
                         &View::OnEraseButtonPushed);
        QObject::connect(find_button_.get(), &QPushButton::clicked, this,
                         &View::OnFindButtonPushed);
        show();
    }

    [[nodiscard]] Observer<RedBlackTree<int>::Data>* View::GetObserver() {
        PRINT_WHERE_AM_I();
        return &observer_model_view_;
    }

    void View::OnNotifyFromModel(const RedBlackTree<int>::Data& value) {
        PRINT_WHERE_AM_I();
        float counter = 0;
        tree_width_ = static_cast<float>(value.tree_size) *
                      (horizontal_space_between_nodes + default_node_diameter);
        std::unique_ptr<DrawableTree> result =
                std::make_unique<DrawableTree>(GetDrawableNode(value, value.root, 0, counter));
        ++trees_to_show_counter_;
        QTimer::singleShot(trees_to_show_counter_ * draw_delay_in_ms, this,
                           [this, result = std::move(result)]() {
                               this->DrawTree(result);
                               if (--trees_to_show_counter_ == 0) {
                                   EnableButtons();
                               }
                           });
    }

    void View::SubscribeToQuery(Observer<TreeQuery>* observer_view_controller) {
        PRINT_WHERE_AM_I();
        observable_view_controller_.Subscribe(observer_view_controller);
    }

    void View::AddWidgetsToLayout() {
        PRINT_WHERE_AM_I();
        main_layout_->addWidget(tree_view_.get(), 0, 0, -1, -1);
        main_layout_->addWidget(insert_line_edit_.get(), 1, 0);
        main_layout_->addWidget(erase_line_edit_.get(), 1, 1);
        main_layout_->addWidget(find_line_edit_.get(), 1, 2);
        main_layout_->addWidget(insert_button_.get(), 2, 0);
        main_layout_->addWidget(erase_button_.get(), 2, 1);
        main_layout_->addWidget(find_button_.get(), 2, 2);
    }

    void View::SetEnabledButtons(bool flag) {
        PRINT_WHERE_AM_I();
        insert_button_->setEnabled(flag);
        erase_button_->setEnabled(flag);
        find_button_->setEnabled(flag);
        insert_line_edit_->setEnabled(flag);
        erase_line_edit_->setEnabled(flag);
        find_line_edit_->setEnabled(flag);
    }

    void View::DisableButtons() {
        PRINT_WHERE_AM_I();
        SetEnabledButtons(false);
    }

    void View::EnableButtons() {
        PRINT_WHERE_AM_I();
        SetEnabledButtons(true);
    }

    void View::OnInsertButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(insert_line_edit_.get());
        HandlePushButton(TreeQueryType::insert, std::ref(str));
    }

    void View::OnEraseButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(erase_line_edit_.get());
        HandlePushButton(TreeQueryType::erase, str);
    }

    void View::OnFindButtonPushed() {
        PRINT_WHERE_AM_I();
        std::string str = GetTextAndClear(find_line_edit_.get());
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
        DisableButtons();
        std::variant<int, std::string> value = string_to_int(text);
        if (value.index() == 0) {
            query_ = {query_type, std::get<int>(value)};
            observable_view_controller_.Notify();
        } else {
            QMessageBox messageBox;
            QMessageBox::critical(nullptr, "Error", std::get<std::string>(value).c_str());
        }
        if (trees_to_show_counter_ == 0) {
            EnableButtons();
        }
    }

    std::unique_ptr<DrawableNode> View::GetDrawableNode(const TreeInfo<int>& tree_info,
                                                        const RedBlackTree<int>::Node* node, float depth,
                                                        float& counter) {
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
        tree_view_->scene()->clear();
        current_node_diameter_ = default_node_diameter;
        current_width_ = static_cast<float>(size().width());
        if (tree_width_ + default_node_diameter + margin >= current_width_) {
            current_node_diameter_ = (default_node_diameter * current_width_) /
                                     (tree_width_ + default_node_diameter + margin);
        }
        RecursiveDraw(tree->root);
        tree_view_->show();
    }

    void View::DrawNode(const std::unique_ptr<DrawableNode>& node) {
        PRINT_WHERE_AM_I();
        QPen pen;
        QBrush brush;
        pen.setWidth(5);
        brush.setColor(node->inside_color);
        brush.setStyle(Qt::SolidPattern);
        pen.setColor(node->outside_color);
        tree_view_->scene()->addEllipse(node->x, node->y, current_node_diameter_,
                                        current_node_diameter_, pen, brush);
        auto* text = new QGraphicsTextItem(std::to_string(node->key).c_str());
        auto rect = text->boundingRect();
        text->setPos(node->x - rect.width() / 2 + current_node_diameter_ / 2,
                     node->y - rect.height() / 2 + current_node_diameter_ / 2);
        text->setDefaultTextColor(Qt::white);
        tree_view_->scene()->addItem(text);
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
        tree_view_->scene()->addItem(horizontal_line);
        tree_view_->scene()->addItem(vertical_line);
    }

    void View::RecursiveDraw(const std::unique_ptr<DrawableNode>& node) {
        if (!node) {
            return;
        }
        if (tree_width_ + default_node_diameter + margin >= static_cast<float>(current_width_)) {
            node->x = node->x / (tree_width_ + default_node_diameter + margin) * current_width_;
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
