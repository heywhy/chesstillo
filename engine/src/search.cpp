#include <cassert>
#include <cstdio>

#include <engine/constants.hpp>
#include <engine/evaluation.hpp>
#include <engine/move.hpp>
#include <engine/move_gen.hpp>
#include <engine/search.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>
#include <engine/ybwc.hpp>

namespace engine {

Search::Search() : Search(0) {}

Search::Search(int n_tasks)
    : tasks(nullptr),
      allow_node_splitting(n_tasks > 1),
      stop(STOP_END),
      nodes_(0),
      result_(new Result()),
      children_(0) {
  if (n_tasks) {
    tasks = new TaskStack(n_tasks);
  }
}

Search::~Search() {
  delete tasks;
  delete result_;
}

void Search::Clone(Search *master) {
  stop = STOP_END;
  tasks = master->tasks;
  height_ = master->height_;
  position = master->position;

  depth = master->depth;
  result_ = master->result_;
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

void Search::Run() {
  stop = RUNNING;
  height_ = 0;
  node_type_[height_] = PV;

  Iterative(-SCORE_INF, SCORE_INF);

  if (stop == RUNNING) {
    stop = STOP_END;
  }

  assert(height_ == 0);
}

void Search::Iterative(int alpha, int beta) {
  int score = -SCORE_INF;

  for (depth = 1; depth <= MAX_DEPTH; depth++) {
    std::printf("Depth %d\n", depth);
    score = Aspiration(alpha, beta, depth, score);
  }

  char str[6];

  ToString(str, *result_->best_move_);

  std::printf("Iterative %d - %s\n", result_->best_score_, str);
}

// TODO: aspiration window?
int Search::Aspiration(int alpha, int beta, int depth, int score) {
  assert(alpha < beta);
  assert(MIN_SCORE <= alpha && alpha <= MAX_SCORE);
  assert(MIN_SCORE <= beta && beta <= MAX_SCORE);
  assert(MIN_SCORE <= score && score <= MAX_SCORE);
  assert(depth >= 0);

  return PVS_Root(alpha, beta, depth);
}

int Search::PVS_Root(int alpha, int beta, int depth) {
  assert(alpha < beta);
  assert(MIN_SCORE <= alpha && alpha <= MAX_SCORE);
  assert(MIN_SCORE <= beta && beta <= MAX_SCORE);
  assert(depth > 0);

  Move *temp;
  Node node(this, alpha, beta, depth);
  MoveList move_list = GenerateMoves(*position);

  node_type_[0] = PV;
  node.pv_node = true;

  if ((temp = node.FirstMove(move_list))) {
    Move &move = *temp;

    UpdateMidgame(move);

    node_type_[height_] = PV;
    move.score = -PVS(-beta, -alpha, depth - 1, &node);

    assert(MIN_SCORE <= move.score && move.score <= MAX_SCORE);

    RestoreMidgame(move);
    node.Update(move);
  }

  while ((temp = node.NextMove())) {
    Move &move = *temp;
    UpdateMidgame(move);

    move.score = -PVS(-alpha - 1, -alpha, depth - 1, &node);

    if (alpha < move.score && move.score < beta) {
      node_type_[height_] = PV;

      move.score = -PVS(-beta, -alpha, depth - 1, &node);
    }

    assert(MIN_SCORE <= move.score && move.score <= MAX_SCORE);

    RestoreMidgame(move);
    node.Update(move);

    assert(MIN_SCORE <= node.best_score_ && node.best_score_ <= MAX_SCORE);
  }

  node.WaitSlaves();

  if (!stop) {
    assert(MIN_SCORE <= node.best_score_ && node.best_score_ <= MAX_SCORE);
  }

  return node.best_score_;
}

int Search::PVS(int alpha, int beta, int depth, Node *parent) {
  int score;

  assert(alpha < beta);
  assert(MIN_SCORE <= alpha && alpha <= MAX_SCORE);
  assert(MIN_SCORE <= beta && beta <= MAX_SCORE);
  assert(depth >= 0);

  if (depth == 0) {
    score = Quiesce(*position, alpha, beta);
  } else {
    score = PVS_Midgame(alpha, beta, depth, parent);
  }

  score = -score;

  assert(MIN_SCORE <= score && score <= MAX_SCORE);

  return score;
}

int Search::NWS_Midgame(int alpha, int depth, Node *parent) {
  assert(MIN_SCORE <= alpha && alpha <= MAX_SCORE);
  assert(parent != nullptr);

  // std::printf("rawdogging: %d - %d\n", stop, depth);
  if (stop) {
    return alpha;
  } else if (depth == 0) {
    return Quiesce(*position, alpha, parent->beta);
  }

  Move *move;
  const int beta = alpha + 1;

  Node node(this, alpha, beta, depth, parent);

  MoveList move_list = GenerateMoves(*position);

  if (move_list.empty()) {
    node.FirstMove(move_list);
    node.best_score_ = Quiesce(*position, alpha, beta);
  } else {
    for (move = node.FirstMove(move_list); move; move = node.NextMove()) {
      if (!node.Split(*move)) {
        UpdateMidgame(*move);

        move->score = NWS_Midgame(-beta, depth - 1, &node);

        RestoreMidgame(*move);
        node.Update(*move);
      }
    }

    node.WaitSlaves();
  }

  if (!stop) {
    // TODO: record best move into the transposition table(s).
    assert(MIN_SCORE <= node.best_score_ && node.best_score_ <= MAX_SCORE);
  } else {
    node.best_score_ = alpha;
  }

  return node.best_score_;
}

int Search::PVS_Midgame(int alpha, int beta, int depth, Node *parent) {
  assert((-MAX_SCORE <= alpha && alpha <= MAX_SCORE) ||
         std::printf("alpha = %d\n", alpha));
  assert((-MAX_SCORE <= beta && beta <= MAX_SCORE) ||
         std::printf("beta = %d\n", beta));
  assert(alpha <= beta);

  if (stop) {
    return alpha;
  } else if (depth == 0) {
    return Quiesce(*position, alpha, beta);
  }

  Node node(this, alpha, beta, depth, parent);
  MoveList move_list = GenerateMoves(*position);

  node.pv_node = true;

  if (move_list.empty()) {
    node.alpha = -(node.beta = +SCORE_INF);
    node.best_score_ = Quiesce(*position, alpha, beta);
    node.best_move_ = Move::NONE;
  } else {
    Move *temp;

    if ((temp = node.FirstMove(move_list))) {
      Move &move = *temp;

      UpdateMidgame(move);

      node_type_[height_] = PV;
      move.score = -PVS_Midgame(-beta, -alpha, depth - 1, &node);

      RestoreMidgame(move);
      node.Update(move);

      while ((temp = node.NextMove())) {
        Move &move = *temp;

        if (!node.Split(move)) {
          const int alpha = node.alpha;

          UpdateMidgame(move);

          move.score = -NWS_Midgame(-alpha - 1, depth - 1, &node);

          if (!stop && alpha < move.score && move.score < beta) {
            node_type_[height_] = PV;
            move.score = -PVS_Midgame(-beta, -alpha, depth - 1, &node);
          }

          RestoreMidgame(move);
          node.Update(move);
        }
      }

      node.WaitSlaves();
    }
  }

  if (!stop) {
    // TODO: record best move into the transposition table(s).
    assert(MIN_SCORE <= node.best_score_ && node.best_score_ <= MAX_SCORE);
  } else {
    node.best_score_ = alpha;
  }

  return node.best_score_;
}

int Search::Midgame(Move &move, int alpha, int depth, Node *node) {
  UpdateMidgame(move);

  int score = -NWS_Midgame(alpha, depth, node);

  if (alpha < score && score < node->beta) {
    score = -PVS_Midgame(-node->beta, -alpha, depth - 1, node);

    assert(node->pv_node == true);
  }

  RestoreMidgame(move);

  return score;
}

void Search::UpdateMidgame(Move &move) {
  static const NodeType next_node_type[] = {CUT, ALL, CUT};

  position->Make(move);

  ++height_;
  node_type_[height_] = next_node_type[node_type_[height_ - 1]];
}

void Search::RestoreMidgame(Move &move) {
  position->Undo(move);

  assert(height_ > 0);

  --height_;
}

void Search::RecordBestMove(const Move &move, const int alpha, const int beta,
                            const int depth) {
  Move copy(move);
  result_->spin_.Lock();

  result_->best_score_ = move.score;

  result_->depth_ = depth;
  result_->pv_.color = position->GetTurn();
  result_->pv_.moves.push_front(copy);

  result_->best_move_ = &result_->pv_.moves.front();

  result_->spin_.Unlock();
}

int Search::Quiesce(Position &position, int alpha, int beta) {
  int standing_pat = Evaluate(position);

  if (standing_pat >= beta) {
    return beta;
  }

  if (alpha < standing_pat) {
    alpha = standing_pat;
  }

  MoveList move_list = GenerateMoves(position);

  for (Move &move : move_list) {
    // INFO: ignore if move isn't a capture
    if (!move.Is(move::CAPTURE)) {
      continue;
    }

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

}  // namespace engine
