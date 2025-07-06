#include <cstddef>
#include <cstdio>

#include <engine/evaluation.hpp>
#include <engine/move.hpp>
#include <engine/move_gen.hpp>
#include <engine/search.hpp>
#include <engine/types.hpp>

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
      score(MIN_SCORE),
      best_move(kNullMove) {}

}  // namespace search

Search::Search(search::WorkerRegistry *workers)
    : tt(nullptr), position(nullptr), workers_(workers) {}

void Search::Clone(Search *) {}

void Search::Run() {
  int score;

  for (depth_ = 1; depth_ <= MAX_DEPTH; depth_++) {
    score = search<PV>(MIN_SCORE, MAX_SCORE, depth_, nullptr);
  }

  // depth_ = MAX_DEPTH;
  // score = search<PV>(MIN_SCORE, MAX_SCORE, depth_, nullptr);

  std::printf("got score %d\n", score);
}

template <enum NodeType T>
int Search::search(int alpha, int beta, int depth, search::Node *parent) {
  if (depth == 0) {
    return Quiesce(alpha, beta);
  }

  search::Node node(this, alpha, beta, depth, parent);

  node.type = T;

  if (tt->CutOff(*position, node.depth, node.alpha, node.beta, &node.best_move,
                 &node.score)) {
    return node.score;
  }

  MoveList moves_list = GenerateMoves(*position);
  auto it = moves_list.begin();
  auto end_it = moves_list.end();

  constexpr NodeType NNT =
      T == NodeType::PV ? NodeType::CUT
                        : (T == NodeType::CUT ? NodeType::ALL : NodeType::CUT);

  if constexpr (T == NodeType::PV) {
    Move &move = *it;
    position->Make(move);

    node.score = -search<PV>(-beta, -alpha, depth - 1, &node);

    position->Undo(move);

    if (node.score > alpha) {
      tt->Add(*position, node.depth, node.score, move, node.type);

      if (node.score >= beta) {
        return node.score;
      }

      alpha = node.score;
    }

    it++;
  }

  for (; it != end_it; it++) {
    Move &move = *it;

    position->Make(move);

    // INFO: null window search
    node.score = -search<NNT>(-alpha - 1, -alpha, depth - 1, &node);

    // INFO: re-search using the [alpha,beta] window
    if (node.score > alpha && node.score < beta) {
      node.score = -search<PV>(-beta, -alpha, depth - 1, &node);
    }

    position->Undo(move);

    // TODO: wait for slaves here?
    if (node.score > alpha) {
      tt->Add(*position, node.depth, node.score, move, node.type);

      if (node.score >= beta) {
        return node.score;
      }

      alpha = node.score;
    }
  }

  return node.score;
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
