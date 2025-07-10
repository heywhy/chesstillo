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
      parent_(nullptr),
      master_(this) {}

void Search::Clone(Search *master) {
  state = search::State::END;

  tt = master->tt;
  *position = *master->position;
  allow_node_splitting = master->allow_node_splitting;
  workers = master->workers;

  depth_ = master->depth_;

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

  for (depth_ = 1; depth_ <= MAX_DEPTH; depth_++) {
    score = search<PV>(MIN_SCORE, MAX_SCORE, depth_, nullptr);

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
  // std::printf("depth(%d) alpha(%d) beta(%d)\n", depth, alpha, beta);
  assert(alpha <= beta);
  assert(depth >= 0);

  if (depth == 0) {
    return Quiesce(alpha, beta);
  }

  search::Node node(this, alpha, beta, depth, parent);

  node.type = T;

  if constexpr (T != NodeType::PV) {
    // TODO: increase depth when position king is in check
    if (tt->CutOff(*position, node.depth, node.alpha, node.beta,
                   &node.best_move, &node.best_score)) {
      return node.best_score;
    }
  }

  MoveList moves_list = GenerateMoves(*position);

  constexpr NodeType NNT =
      T == NodeType::PV ? NodeType::CUT
                        : (T == NodeType::CUT ? NodeType::ALL : NodeType::CUT);

  if (moves_list.empty()) {
    node.best_score = Quiesce(node.alpha, node.beta);
  } else {
    Move *move;

    if ((move = node.FirstMove(moves_list))) {
      position->Make(*move);

      move->score = -search<T>(-node.beta, -node.alpha, node.depth - 1, &node);

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
      move->score = -search<NNT>(-alpha - 1, -alpha, node.depth - 1, &node);

      // INFO: re-search using the [alpha,beta] window
      if (alpha < move->score && move->score < beta) {
        move->score = -search<PV>(-beta, -alpha, node.depth - 1, &node);
      }

      position->Undo(*move);

      node.Update(*move);

      // std::printf("%d : [%d,%d]\n", node.best_score, MIN_SCORE, MAX_SCORE);
      assert(MIN_SCORE <= node.best_score && node.best_score <= MAX_SCORE);
    }

    node.WaitSlaves();
    // std::printf("done waiting slaves\n");
  }

  if (state == search::State::RUNNING) {
    tt->Add(*position, depth, node.best_score, node.best_move, node.type);
  }

  return node.best_score;
}

int Search::NWS_Search(int alpha, int depth, search::Node *parent) {
  assert(MIN_SCORE <= alpha && alpha <= MAX_SCORE);

  if (state != search::State::RUNNING) {
    return alpha;
  }

  return search<NodeType::CUT>(-alpha - 1, -alpha, depth, parent);
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

  if (standing_pat >= beta) {
    return beta;
  }

  if (standing_pat < alpha) {
    alpha = standing_pat;
  }

  MoveList moves_list = GenerateMoves(*position);

  for (const Move &move : moves_list) {
    if (!move.Is(move::CAPTURE)) {
      continue;
    }

    position->Make(move);

    int score = -Quiesce(-beta, -alpha);

    position->Undo(move);

    if (score >= beta) {
      return beta;
    }

    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}

}  // namespace engine
