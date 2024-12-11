#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <vector>

#include <chesstillo/constants.hpp>
#include <chesstillo/evaluation.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/search.hpp>
#include <chesstillo/transposition.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

// TODO: sort the moves considering the game phase alongside other parameters
void SearchManager::OrderMoves(Position &position, Moves &moves, int depth) {
  TTEntry entry;
  Moves ordered_moves;

  std::sort(moves.begin(), moves.end(),
            [](Move &a, Move &) { return a.Is(CAPTURE); });

  // after sorting above we then try to see if this position as be searched
  // before so that can prioritize the best move before others
  // if (tt_->Probe(position, &entry) && entry.depth >= depth) {
  //   Move &best_move = entry.best_move;
  //
  //   std::erase_if(moves, [&best_move](Move &move) {
  //     return best_move.from == move.from && best_move.to == move.to;
  //   });
  //
  //   moves.insert(moves.begin(), best_move);
  // }
}

template <enum NodeType T>
int SearchManager::Search(Position &position, int alpha, int beta,
                          Stack &stack) {
  Move best_move;
  int score = -INF;
  int depth = stack.depth;
  constexpr bool is_pv_node = T == PV;

  if (IN_CHECK(stack.check_mask)) {
    ++depth;
  }

  if (depth == 0) {
    return Quiesce(position, alpha, beta);
  }

  if (tt_->CutOff(position, depth, alpha, beta, &best_move, &score)) {
    if (!is_pv_node || (score > alpha && score < beta)) {
      return score;
    }
  }

  Moves moves = GenerateMoves(position);
  size_t size = moves.size();

  OrderMoves(position, moves, depth);

  for (size_t i = 0; i < size; i++) {
    Stack child_stack;
    Move &move = moves[i];

    position.Make(move);

    child_stack.depth = depth - 1;
    child_stack.check_mask = CheckMask(position);

    if (i == 0) {
      score = -Search<T>(position, -beta, -alpha, child_stack);
    } else {
      score = -Search<NON_PV>(position, -alpha - 1, -alpha, child_stack);

      if (score > alpha && is_pv_node) {
        score = -Search<PV>(position, -beta, -alpha, child_stack);
      }
    }

    position.Undo(move);

    if (score > alpha) {
      alpha = score;
      stack.moves[0] = move;
      std::copy_n(child_stack.moves, child_stack.cmove, stack.moves + 1);
      stack.cmove = child_stack.cmove + 1;

      if (score >= beta) {
        tt_->Add(position, depth, beta, move, CUT);

        return beta;
      }

      tt_->Add(position, depth, alpha, move, ALL);
    }
  }

  // INFO: maybe not add to pv again?!
  tt_->Add(position, depth, alpha, best_move, PV);

  return alpha;
}

// TODO: incorporate null moves, aspiration window
Move SearchManager::FindBestMove(Position &position) {
  int score;
  Move move;
  Stack stack;

  stack.check_mask = CheckMask(position);

  for (stack.depth = 1; stack.depth <= MAX_DEPTH; stack.depth++) {
    // INFO: stop searching when we get to depth 4 or highest score met
    if ((stack.depth == 5 && stack.cmove == 1) || (score >= INF)) {
      break;
    }

    score = Search<PV>(position, -INF, INF, stack);
  }

  for (int i = 0; i < stack.cmove; i += 2) {
    char to[6];
    char from[6];
    int to_index = i + 1;
    int m_index = (i / 2) + 1;

    ToString(from, stack.moves[i]);

    to_index < stack.cmove && ToString(to, stack.moves[to_index]);

    std::printf("%d. %s %s ", m_index, from, to);
  }

  std::printf("\n");

  return stack.moves[0];
}

int SearchManager::Quiesce(Position &position, int alpha, int beta) {
  int standing_pat = Evaluate(position);

  if (standing_pat >= beta) {
    return beta;
  }

  if (alpha < standing_pat) {
    alpha = standing_pat;
  }

  Moves moves = GenerateMoves(position);

  std::erase_if(moves, [](Move &move) { return !move.Is(CAPTURE); });

  for (Move &move : moves) {
    position.Make(move);

    int score = -Quiesce(position, -beta, -alpha);

    position.Undo(move);

    if (score >= beta) {
      return beta;
    }

    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}
