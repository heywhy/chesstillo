#include <engine/position.hpp>
#include <engine/uci.hpp>

int main() {
  engine::Position position;
  engine::UCILink uci_link(&position);

  uci_link.Loop();

  return 0;
}
