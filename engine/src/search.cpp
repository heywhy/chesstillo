#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>

#include <engine/config.hpp>
#include <engine/evaluation.hpp>
#include <engine/move.hpp>
#include <engine/move_gen.hpp>
#include <engine/search.hpp>
#include <engine/types.hpp>

namespace engine {

Search::Search(search::WorkerRegistry *workers)
    : tt(nullptr),
      position(nullptr),
      state(search::State::END),
      allow_node_splitting(false),
      workers(workers),
      height_(0),
      parent_(nullptr),
      master_(this) {}

void Search::Clone(Search *master) {
  state = search::State::END;

  tt = master->tt;
  *position = *master->position;
  allow_node_splitting = master->allow_node_splitting;
  workers = master->workers;

  depth_ = master->depth_;
  height_ = master->height_;

  master->AddChild(this);

  parent_ = master;
  master_ = master->master_;
}

void Search::Detach() {
  if (!!parent_) {
    parent_->RemoveChild(this);
  }
}

void Search::AddChild(Search *child) {
  spin_.Lock();

  children_.push_back(child);

  spin_.Unlock();
}

void Search::RemoveChild(Search *child) {
  spin_.Lock();

  std::size_t count = std::erase(children_, child);

  assert(count == 1);

  spin_.Unlock();
}

void Search::Run() {
  int score;
  state = search::State::RUNNING;

  // INFO: maybe do aspiration/widen search?
  for (depth_ = 1; depth_ <= MAX_DEPTH; depth_++) {
    score = search<NodeType::PV>(MIN_SCORE, MAX_SCORE, depth_, nullptr);

    if (!Continue()) {
      break;
    }
  }

  if (state == search::State::RUNNING) {
    state = search::State::END;
  }

  std::printf("got score %d\n", score);
}

template <enum NodeType T>
int Search::search(int alpha, int beta, int depth, search::Node *parent) {
  // assert(alpha <= beta);
  assert(depth >= 0);

  if (depth == 0) {
    return Quiesce(alpha, beta);
  }

  ++height_;
  search::Node node(this, alpha, beta, depth, parent);

  node.type = T;

  // TODO: increase depth when position king is in check
  if (tt->CutOff(*position, node.depth, node.alpha, node.beta, &node.best_move,
                 &node.best_score)) {
    return node.best_score;
  }

  MoveList moves_list = GenerateMoves(*position);

  OrderMoves(moves_list);

  constexpr NodeType NNT =
      T == NodeType::PV ? NodeType::CUT
                        : (T == NodeType::CUT ? NodeType::ALL : NodeType::CUT);

  if (moves_list.empty()) {
    node.best_score = Quiesce(node.alpha, node.beta);
  } else {
    Move *move;

    if ((move = node.FirstMove(moves_list))) {
      position->Make(*move);

      if constexpr (T == NodeType::PV) {
        move->score =
            -search<T>(-node.beta, -node.alpha, node.depth - 1, &node);
      } else {
        move->score = -NW_Search<NNT>(node.alpha, node.depth - 1, &node);
      }

      assert(MIN_SCORE <= move->score && move->score <= MAX_SCORE);

      position->Undo(*move);

      node.Update(*move);
    }

    while ((move = node.NextMove())) {
      const int alpha = node.alpha;

      // TODO: check if node is PV and multipv depth is reached and
      // split along that line too
      if (node.Split(*move)) {
        continue;
      }

      position->Make(*move);

      // INFO: null window search
      move->score = -NW_Search<NNT>(alpha, node.depth - 1, &node);

      // INFO: re-search using the [alpha,beta] window
      if (alpha < move->score && move->score < beta) {
        move->score =
            -search<NodeType::PV>(-beta, -alpha, node.depth - 1, &node);
      }

      position->Undo(*move);

      node.Update(*move);

      assert(MIN_SCORE <= node.best_score && node.best_score <= MAX_SCORE);
    }

    node.WaitSlaves();
  }

  if (state == search::State::RUNNING) {
    tt->Add(*position, node.depth, node.best_score, node.best_move, node.type);
  }

  --height_;

  return node.best_score;
}

template <enum NodeType T>
int Search::NW_Search(int alpha, int depth, search::Node *parent) {
  assert(MIN_SCORE <= alpha && alpha <= MAX_SCORE);

  if (state != search::State::RUNNING) {
    return alpha;
  }

  return search<T>(-alpha - 1, -alpha, depth, parent);
}

void Search::SetState(search::State new_state) {
  spin_.Lock();

  state = new_state;

  spin_.Unlock();
}

void Search::StopAll(search::State new_state) {
  spin_.Lock();

  state = new_state;

  for (auto child : children_) {
    child->StopAll(new_state);
  }

  spin_.Unlock();
}

int Search::Quiesce(int alpha, int beta) {
  int standing_pat = Evaluate(*position);
  int best_value = standing_pat;

  if (best_value >= beta) {
    return best_value;
  }

  if (best_value > alpha) {
    alpha = best_value;
  }

  MoveList moves_list = GenerateMoves(*position);

  OrderMoves(moves_list);

  for (const Move &move : moves_list) {
    if (!move.Is(move::CAPTURE)) {
      break;
    }

    position->Make(move);

    int score = -Quiesce(-beta, -alpha);

    position->Undo(move);

    if (score >= beta) {
      return score;
    }

    if (score > best_value) {
      best_value = score;
    }

    if (score > alpha) {
      alpha = score;
    }
  }

  return best_value;
}

void Search::OrderMoves(MoveList &move_list) {
  std::stable_sort(move_list.begin(), move_list.end(), &Search::OrderMove);
}

bool Search::OrderMove(const Move &a, const Move &b) {
  if (a.Is(move::CAPTURE) && !b.Is(move::CAPTURE)) {
    return true;
  }

  if (b.Is(move::CAPTURE) && !a.Is(move::CAPTURE)) {
    return false;
  }

  int piece_a_value = PieceValue(a.piece);
  int piece_b_value = PieceValue(b.piece);

  if (a.Is(move::CAPTURE) && b.Is(move::CAPTURE)) {
    int c = PieceValue(a.captured) - piece_a_value;
    int d = PieceValue(b.captured) - piece_b_value;

    return c >= d;
  }

  return piece_a_value > piece_b_value;
}

}  // namespace engine
