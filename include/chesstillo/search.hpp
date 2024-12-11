#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <cstddef>
#include <cstdlib>

#include "position.hpp"
#include "scheduler.hpp"
#include "transposition.hpp"
#include "types.hpp"

#define MAX_DEPTH 6 // 125
// #define INF 9999999999
#define INF INT_MAX
#define MAX_STACK_MOVES 2000000

struct Stack {
  int depth;
  int cmove;
  size_t nps;
  Bitboard check_mask;
  Move *moves;

  Stack()
      : depth(0), cmove(0), nps(0), check_mask(kEmpty),
        moves(static_cast<Move *>(std::calloc(MAX_STACK_MOVES, sizeof(Move)))) {
  }

  ~Stack() { std::free(moves); }
};

class SearchManager {
public:
  SearchManager(TT *tt) : tt_(tt), scheduler_(6) {}

  Move FindBestMove(Position &position);

private:
  TT *tt_;
  Scheduler scheduler_;

  void OrderMoves(Position &position, Moves &moves, int depth);
  int Quiesce(Position &position, int alpha, int beta);

  template <enum NodeType T>
  int Search(Position &position, int alpha, int beta, Stack &stack);
};
#endif
