#ifndef TREE_HPP
#define TREE_HPP

#include <cstdint>

#include "position.hpp"
#include "types.hpp"

enum NodeType : uint8_t { ALL, CUT, PV };

struct Node {
  Position position;
  Node *children;
  int value;

  Node(Position position) : position(position), value(-1) {}
};

struct Tree {
  Node root;
};

class Worker {};

Move FindBestMove(Position &position);
#endif
