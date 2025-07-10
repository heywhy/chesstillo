#include <cassert>
#include <cstddef>
#include <mutex>

#include <engine/config.hpp>
#include <engine/move.hpp>
#include <engine/search.hpp>

namespace engine {
namespace search {

Node::Node(Search *search, int alpha, int beta, int depth)
    : Node(search, alpha, beta, depth, nullptr) {}

Node::Node(Search *search, int alpha, int beta, int depth, Node *parent)
    : search(search),
      alpha(alpha),
      beta(beta),
      depth(depth),
      parent(parent),
      best_score(MIN_SCORE),
      best_move(kNullMove),
      helping_(false),
      waiting_(false),
      stop_point_(false),
      move_(nullptr),
      moves_done_(0),
      moves_todo_(0) {}

Move *Node::FirstMove(const MoveList &move_list) {
  Move *move = nullptr;
  std::lock_guard lock(mutex_);

  moves_done_ = 0;
  move_ = move_list.data();
  moves_todo_ = move_list.size();

  if (moves_todo_ && move_ && search->state == search::State::RUNNING) {
    assert(alpha < beta);
    move = const_cast<Move *>(move_);
  }

  return move;
}

Move *Node::NextMove() {
  std::lock_guard lock(mutex_);

  return NextMoveLockless();
}

Move *Node::NextMoveLockless() {
  Move *move = nullptr;

  if (move_ && moves_done_ < moves_todo_ && alpha < beta &&
      search->state == search::State::RUNNING) {
    ++moves_done_;
    --moves_todo_;
    move = const_cast<Move *>(++move_);
  }

  return move;
}

void Node::Update(const Move &move) {
  std::lock_guard lock(mutex_);

  if (search->state == search::State::RUNNING && move.score > best_score) {
    best_score = move.score;
    best_move = move;

    // TODO: record best move somewhere here
    if (move.score > alpha) {
      alpha = move.score;
    }
  }

  if (alpha >= beta && !slaves_.empty()) {
    for (auto slave : slaves_) {
      slave->StopAll(search::State::STOP_PARALLEL);
    }
  }

  moves_done_++;
}

void Node::AddSlave(Search *search) {
  std::lock_guard lock(mutex_);

  slaves_.push_back(search);
}

void Node::RemoveSlave(Search *search) {
  std::lock_guard lock(mutex_);

  std::size_t count = std::erase(slaves_, search);

  assert(count == 1);
  cv_.notify_all();
}

void Node::WaitSlaves() {
  std::unique_lock lock(mutex_);

  if ((alpha >= beta || search->state != search::State::RUNNING) &&
      !slaves_.empty()) {
    for (auto slave : slaves_) {
      slave->StopAll(search::State::STOP_PARALLEL);
    }
  }

  while (!slaves_.empty()) {
    waiting_ = true;

    assert(helping_ == false);

    cv_.wait(lock, [&] { return slaves_.empty() || helping_; });

    if (helping_) {
      // TODO: master thread to do some work

      helping_ = false;
    } else {
      waiting_ = false;
    }
  }

  // INFO: wake up the master thread
  if (search->state == search::State::STOP_PARALLEL && stop_point_) {
    search->state = search::State::RUNNING;
    stop_point_ = false;
  }
}

bool Node::Split(const Move &move) {
  // INFO: maybe not split on last node?
  if (search->allow_node_splitting && depth >= SPLIT_MIN_DEPTH && moves_done_ &&
      moves_todo_ > 1 && slaves_.size() < SPLIT_MAX_SLAVES) {
    Worker *worker = search->workers->GetIdleWorker();

    if (worker) {
      worker->Assign(this, const_cast<Move *>(&move));

      return true;
    }
  }

  return false;
}

}  // namespace search

}  // namespace engine
