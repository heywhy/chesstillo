#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <queue>
#include <vector>

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/evaluation.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/options.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/search.hpp>
#include <chesstillo/threads.hpp>
#include <chesstillo/transposition.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>
#include <chesstillo/ybwc.hpp>

using namespace std::chrono_literals;

Search::Search() : Search(0) {}

Search::Search(int n_tasks)
    : allow_node_splitting(n_tasks > 1), stop(STOP_END), nodes_(0),
      children_(0) {
  if (n_tasks) {
    tasks = new TaskStack(n_tasks);
  }
}

Search::~Search() { delete tasks; }

void Search::Clone(Search *master) {
  stop = STOP_END;
  tasks = master->tasks;
  height = master->height;

  depth = master->depth;
  allow_node_splitting = master->allow_node_splitting;

  master->spin_.Lock();
  assert(master->children_.size() < MAX_THREADS);
  master->children_.push_back(this);
  master->spin_.Unlock();

  parent_ = master;
  parent_->master_ = master->master_;
}

void Search::SetState(Stop stop) {
  spin_.Lock();

  this->stop = stop;

  spin_.Unlock();
}

void Search::StopAll(Stop stop) {
  spin_.Lock();

  this->stop = stop;

  for (Search *search : children_) {
    search->StopAll(stop);
  }

  spin_.Unlock();
}

void SplitPoint::Tackling(Slave *slave) {
  std::unique_lock lock(mutex);
  active_slaves.insert(slave);
}

void SplitPoint::Finished(Slave *slave) {
  std::unique_lock lock(mutex);
  active_slaves.erase(slave);
}

void SplitPoint::Copy(Moves::iterator begin, Moves::iterator end) {
  while (!moves.empty()) {
    moves.pop();
  }

  std::for_each(begin, end, [this](Move &move) { moves.push(move); });
}

void SplitPoint::WaitForSlaves() {
  while (true) {
    std::unique_lock lock(mutex);

    cv.wait_for(lock, 1ns, [&] { return active_slaves.empty(); });

    if (active_slaves.empty()) {
      break;
    }
  }
}

void SplitPoint::Search(Slave *slave) {
  search->SplitPointSearch(this, slave);
  Finished(slave);
}

// TODO: sort the moves
// * considering the game phase alongside other parameters
// * weighting the moves so that attacks or pins and the likes can be assessed
// first
void SearchManager::OrderMoves(Position &position, Moves &moves, int depth) {
  TTEntry entry;
  Moves ordered_moves;
  Bitboard *enemy_pieces = position.Pieces(OPP(position.turn_));
  Bitboard enemy_king = enemy_pieces[KING];
  Bitboard enemy_king_adjacents = KING_ATTACKS(enemy_king);
  Bitboard enemy_king_adjacents_x2 =
      enemy_king_adjacents | KING_ATTACKS(enemy_king_adjacents);

  std::sort(moves.begin(), moves.end(),
            [&enemy_king_adjacents_x2](Move &a, Move &b) {
              return (a.Is(CAPTURE) ||
                      (BITBOARD_FOR_SQUARE(a.to) & enemy_king_adjacents_x2)) &&
                     !b.Is(CAPTURE);
            });

  // after sorting above we then try to see if this position as been searched
  // before so that we can prioritize the best move before others
  if (tt_->Probe(position, &entry) && entry.depth >= depth) {
    Move &best_move = entry.best_move;

    std::erase(moves, best_move);

    moves.insert(moves.begin(), best_move);
  }
}

template <enum NodeType T>
int SearchManager::Search(Position &position, int alpha, int beta,
                          Stack &stack) {
  Move best_move;
  int score = MIN_SCORE;
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
      // stack.moves[0] = best_move;
      // stack.cmove = 1;

      return score;
    }
  }

  Moves moves = GenerateMoves(position);

  OrderMoves(position, moves, depth);

  size_t moves_size = moves.size();

  if constexpr (T == PV) {
    if (moves_size == 0) {
      return alpha;
    }

    Stack child_stack;
    Move &move = moves[0];

    position.Make(move);

    child_stack.depth = depth - 1;
    child_stack.check_mask = CheckMask(position);

    score = -Search<PV>(position, -beta, -alpha, child_stack);

    position.Undo(move);

    if (score > alpha) {
      alpha = score;
      stack.moves[0] = move;
      stack.cmove = child_stack.cmove + 1;

      std::copy_n(child_stack.moves, child_stack.cmove, stack.moves + 1);

      if (score >= beta) {
        tt_->Add(position, depth, beta, move, PV);

        return beta;
      }

      tt_->Add(position, depth, alpha, move, PV);
    }

    for (size_t i = 1; i < moves_size; i++) {
      Stack child_stack;
      Move &move = moves[i];

      position.Make(move);

      child_stack.depth = depth - 1;
      child_stack.check_mask = CheckMask(position);

      score = -Search<CUT>(position, -alpha - 1, -alpha, child_stack);

      if (score > alpha && score < beta) {
        child_stack.cmove = 0;

        score = -Search<PV>(position, -beta, -alpha, child_stack);
      }

      position.Undo(move);

      if (score > alpha) {
        alpha = score;
        stack.moves[0] = move;
        stack.cmove = child_stack.cmove + 1;

        std::copy_n(child_stack.moves, child_stack.cmove, stack.moves + 1);

        if (score >= beta) {
          tt_->Add(position, depth, beta, move, CUT);

          return beta;
        }

        tt_->Add(position, depth, alpha, move, CUT);
      }

      // TODO: split if there's an idle thread available
    }
  } else {
    for (size_t i = 0; i < moves_size; i++) {
      Stack child_stack;
      Move &move = moves[i];

      position.Make(move);

      child_stack.depth = depth - 1;
      child_stack.check_mask = CheckMask(position);

      if constexpr (T == CUT) {
        score = -Search<ALL>(position, -alpha - 1, -alpha, child_stack);
      } else {
        score = -Search<CUT>(position, -alpha - 1, -alpha, child_stack);
      }

      position.Undo(move);

      if (score > alpha) {
        alpha = score;
        stack.moves[0] = move;
        stack.cmove = child_stack.cmove + 1;

        std::copy_n(child_stack.moves, child_stack.cmove, stack.moves + 1);

        if (score >= beta) {
          tt_->Add(position, depth, beta, move, T);

          return beta;
        }

        tt_->Add(position, depth, alpha, move, T);
      }
    }
  }

  return alpha;
}

template <enum NodeType T>
int SearchManager::TSearch(Position &position, int alpha, int beta,
                           Stack &stack) {
  Move best_move;
  int score = MIN_SCORE;
  int depth = stack.depth;
  constexpr bool is_pv_node = T == PV;
  constexpr bool is_cut_node = T == CUT;

  if (IN_CHECK(stack.check_mask)) {
    ++depth;
  }

  if (depth <= 0) {
    return Quiesce(position, alpha, beta);
  }

  if (tt_->CutOff(position, depth, alpha, beta, &best_move, &score)) {
    if (!is_pv_node || (score > alpha && score < beta)) {
      stack.moves[0] = best_move;
      stack.cmove = 1;

      return score;
    }
  }

  int visited = 0;
  bool checked_pv = false;
  Moves moves = GenerateMoves(position);

  OrderMoves(position, moves, depth);

  for (Move &move : moves) {
    Stack child_stack;

    position.Make(move);

    child_stack.depth = depth - 1;
    child_stack.check_mask = CheckMask(position);

    if (is_pv_node && !checked_pv) {
      checked_pv = true;
      score = -TSearch<PV>(position, -beta, -alpha, child_stack);
    } else if (is_pv_node) {
      score = -TSearch<CUT>(position, -alpha - 1, -alpha, child_stack);

      if (score > alpha && score < beta) {
        child_stack.cmove = 0;

        score = -TSearch<PV>(position, -beta, -alpha, child_stack);
      }
    } else {
      if (is_cut_node) {
        score = -TSearch<ALL>(position, -alpha - 1, -alpha, child_stack);
      } else {
        score = -TSearch<CUT>(position, -alpha - 1, -alpha, child_stack);
      }
    }

    position.Undo(move);

    if (score > alpha) {
      alpha = score;
      stack.moves[0] = move;
      stack.cmove = child_stack.cmove + 1;

      std::copy_n(child_stack.moves, child_stack.cmove, stack.moves + 1);

      if (score >= beta) {
        tt_->Add(position, depth, beta, move, T);

        return beta;
      }

      tt_->Add(position, depth, alpha, move, T);
    }

    visited++;
    Slave *slave = slave_manager_.FindIdleSlave();

    if (slave != nullptr) {
      SplitPoint sp(position, depth - 1, alpha, beta, T, this);

      sp.Copy(moves.begin() + visited, moves.end());

      while (slave != nullptr && !sp.moves.empty()) {
        slave->Assign(&sp);

        slave = slave_manager_.FindIdleSlave();
      }

      size_t slaves = sp.active_slaves.size();

      std::printf(
          "waiting for slaves(%lu) dispatched for moves(%lu) @ depth(%d)\n",
          slaves, sp.moves.size(), sp.depth);

      sp.WaitForSlaves();

      std::printf("done waiting for slaves(%lu) dispatched for moves(%lu) @ "
                  "depth(%d)\n",
                  slaves, sp.moves.size(), sp.depth);

      alpha = sp.alpha;

      break;
    }
  }

  return alpha;
}

void SearchManager::SplitPointSearch(SplitPoint *sp, Slave *) {
  while (true) {
    std::unique_lock lock(sp->mutex);

    if (sp->stop_search || sp->moves.empty()) {
      break;
    }

    int score;
    Stack stack;
    int beta = sp->beta;
    int alpha = sp->alpha;
    Move move = sp->moves.front();
    Position position = sp->position;

    sp->moves.pop();

    // INFO: unlock mutex here so that we can proceed to searching and other
    // slaves can copy data off of split point
    lock.unlock();

    position.Make(move);

    stack.depth = sp->depth;
    stack.check_mask = CheckMask(position);

    switch (sp->node) {
    case PV:
      score = -TSearch<CUT>(position, -alpha - 1, -alpha, stack);

      if (score > alpha && score < beta) {
        stack.cmove = 0;

        score = -TSearch<PV>(position, -beta, -alpha, stack);
      }

      break;
    case CUT:
      score = -TSearch<ALL>(position, -alpha - 1, -alpha, stack);
      break;
    case ALL:
      score = -TSearch<CUT>(position, -alpha - 1, -alpha, stack);
      break;
    }

    // INFO: lock again so that we can synchronize split point state
    lock.lock();

    if (score > sp->alpha) {
      sp->alpha = score;

      if (sp->alpha >= sp->beta) {
        // TODO: tell all threads to stop
        std::printf("beta score reached @ %d\n", sp->depth);
        sp->stop_search = true;

        break;
      }
    }
  }

  sp->cv.notify_all();
}

// TODO: incorporate null moves, aspiration window
Move SearchManager::FindBestMove(Position &position) {
  int score;
  Move move;
  Stack stack;

  stack.check_mask = CheckMask(position);

  for (stack.depth = 1; stack.depth <= MAX_DEPTH; stack.depth++) {
    // INFO: stop searching when we get to depth 4 or highest score met
    if ((stack.depth == 5 && stack.cmove == 1) || (score >= MAX_SCORE)) {
      break;
    }

    score = Search<PV>(position, MIN_SCORE, MAX_SCORE, stack);

    // TODO: maybe dispatch in a thread to stop main thread from blocking?!
    // score = TSearch<PV>(position, MIN_SCORE, MAX_SCORE, stack);
  }

  std::printf("score is: %d:%d\n", score, stack.cmove);

  for (int i = 0; i < stack.cmove; i += 2) {
    char to[6];
    char from[6];
    int to_index = i + 1;
    int m_index = (i / 2) + 1;

    ToString(from, stack.moves[i]);

    to_index < stack.cmove &&ToString(to, stack.moves[to_index]);

    std::printf("%d. %s %s ", m_index, from, to);
  }

  std::printf("\n");

  return stack.moves[0];
}

Move SearchManager::TFindBestMove(Position &position) {
  int score;
  Move move;
  Stack stack;

  stack.check_mask = CheckMask(position);

  for (stack.depth = 1; stack.depth <= MAX_DEPTH; stack.depth++) {
    // INFO: stop searching when we get to depth 4 or highest score met
    if ((stack.depth == 5 && stack.cmove == 1) || (score >= MAX_SCORE)) {
      break;
    }

    score = TSearch<PV>(position, MIN_SCORE, MAX_SCORE, stack);
  }

  std::printf("score is: %d:%d\n", score, stack.cmove);

  for (int i = 0; i < stack.cmove; i += 2) {
    char to[6];
    char from[6];
    int to_index = i + 1;
    int m_index = (i / 2) + 1;

    ToString(from, stack.moves[i]);

    to_index < stack.cmove &&ToString(to, stack.moves[to_index]);

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
