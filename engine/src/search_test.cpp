#include <thread>

#include <gtest/gtest.h>

#include <engine/fen.hpp>
#include <engine/position.hpp>
#include <engine/search.hpp>

using namespace engine;

class SearchTestSuite : public testing::Test {
 protected:
  Search *search_;
  Position position_;

  void SetUp() override {
    search_ = new Search(std::thread::hardware_concurrency());

    ApplyFen(position_, START_FEN);
  }

  void TearDown() override {
    delete search_;
    position_.Reset();
  }
};
