#ifndef GUI_CHESSBOARD_HPP

#define GUI_CHESSBOARD_HPP

#include <chesstillo/board.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/node.hpp>
#include <memory>

#include "components/square.hpp"

class Chessboard : public ftxui::ComponentBase, public OnSelectSquare {
public:
  Chessboard(std::shared_ptr<Board> board);

  ~Chessboard() { delete selected_; }

  ftxui::Element Render() override;
  bool OnEvent(ftxui::Event event) override;
  void OnSelect(Square *) override;

  void FillBoard();

  Square **selected_;

private:
  ftxui::Box box_;
  ftxui::Element el_;
  std::shared_ptr<Board> board_;
};

#endif
