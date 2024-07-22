#ifndef GUI_SQUARE_HPP

#define GUI_SQUARE_HPP

#include <cstdint>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/box.hpp>
#include <string>

class Square;

class OnSelectSquare {
public:
  virtual void OnSelect(Square *square) = 0;
};

class Square : public ftxui::ComponentBase {
public:
  Square(OnSelectSquare *observer, Square **selected, std::uint8_t file,
         std::uint8_t rank)
      : observer_(observer), file_(file), rank_(rank), piece_('\0'),
        selected_(selected) {}

  std::uint8_t GetFile() const { return file_; }
  std::uint8_t GetRank() const { return rank_; }
  void SetPiece(char piece) { piece_ = piece; }
  bool IsEmpty() { return piece_ == '\0'; }

  ftxui::Element Render() override {
    ftxui::Elements elements;

    if (piece_ != '\0') {
      std::string p(1, piece_);

      elements.push_back({ftxui::text(p)});
    }

    ftxui::Element square = ftxui::dbox(elements | ftxui::center);

    square |= ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 4) |
              ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 2) | ftxui::center;

    if (!IsSelected() && (rank_ + file_) % 2 == 0) {
      square |= ftxui::bgcolor(ftxui::Color::Black) |
                ftxui::color(ftxui::Color::White);
    } else if (!IsSelected()) {
      square |= ftxui::bgcolor(ftxui::Color::White) |
                ftxui::color(ftxui::Color::Black);
    } else {
      square |= ftxui::bgcolor(ftxui::Color::Green4);
    }

    return square | ftxui::reflect(box_);
  }

  bool OnEvent(ftxui::Event event) override {
    if (event.is_mouse()) {
      ftxui::Mouse mouse = event.mouse();

      if (mouse.motion == ftxui::Mouse::Motion::Pressed &&
          mouse.button == ftxui::Mouse::Button::Left &&
          box_.Contain(mouse.x, mouse.y)) {
        observer_->OnSelect(this);

        return true;
      }
    }

    return ftxui::ComponentBase::OnEvent(event);
  }

private:
  char piece_;
  ftxui::Box box_;
  Square **selected_;
  std::uint8_t file_;
  std::uint8_t rank_;
  OnSelectSquare *observer_;

  bool IsSelected() { return *selected_ == this; }
};
#endif
