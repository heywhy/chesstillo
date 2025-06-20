#include <thread>

#include <gtest/gtest.h>

#include <engine/position.hpp>
#include <engine/search.hpp>

using namespace engine;

class SearchTestSuite : public testing::Test {
 protected:
  Search *search_;
  Position position_;

  void SetUp() override {
    search_ = new Search(std::thread::hardware_concurrency());

    position_ = Position::FromFen(kStartPos);
  }

  void TearDown() override {
    delete search_;
    position_.Reset();
  }
};
