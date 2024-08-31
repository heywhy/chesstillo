#ifndef TUI_MOVES_HPP
#define TUI_MOVES_HPP

#include <list>
#include <string>

#include <chesstillo/types.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>

#include "utils.hpp"

class MovesBoard : public ftxui::ComponentBase {
public:
  MovesBoard(std::list<Move> *moves) : moves_(moves) {}

  ftxui::Element Render() override {
    ftxui::Elements elements;

    elements.reserve(moves_->size() / 2);

    int index = 1;
    int count = 0;
    auto it = moves_->rbegin();

    while (it != moves_->rend()) {
      ftxui::Elements texts;
      Color turn = static_cast<Color>(count % 2);
      std::string move_number = std::to_string(index) + ". ";

      texts.push_back(ftxui::text(move_number) | ftxui::center |
                      ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 5));

      char text[5];

      if (ToString(text, *it, turn)) {
        texts.push_back(ftxui::text(text) |
                        ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10));

        it++;
        count++;
      }

      turn = static_cast<Color>(count % 2);

      if (it != moves_->rend() && ToString(text, *it, turn)) {
        texts.push_back(ftxui::text(text) |
                        ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10));

        it++;
        count++;
      }

      if (texts.size() > 0) {
        elements.push_back(ftxui::hbox(texts));
      }

      ++index;
    }

    return ftxui::vbox(elements) |
           ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 22) |
           ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 30);
  }

  bool Focusable() const override { return true; }

private:
  std::list<Move> *moves_;
};
#endif
