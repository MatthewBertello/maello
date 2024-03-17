#pragma once
#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <cstdint>
#include <atomic>

#include "utils.hpp"
#include "position.hpp"

namespace chess {

class SearchEngine {
 public:

 struct PvLine {
    move::Move moves[kMaxSearchDepth] = {0};
    int count = 0;
 };

  uint64_t nodes = 0;
  int ply = 0;
  int score = kUnknownScore;
  
  move::Move killer_moves_[kMaxSearchDepth][kNumKillerMoves];
  int history_moves_[kPieceCount][kNumSquares];
  move::Move tt_move_ = 0;
  PvLine pv_line_;

  // Search parameters
  int search_depth_ = -1;
  int current_depth_= -1;
  uint64_t max_nodes_ = 0;
  Time start_time_ = 0;
  Time end_time_ = 0;
  int white_time_ = 0;
  int black_time_ = 0;
  int white_inc_ = 0;
  int black_inc_ = 0;
  int moves_to_go_ = 0;
  bool engine_decides_search_params_ = false;

  bool stop_search_ = false;
  std::atomic<bool> &external_stop_;

  SearchEngine(std::atomic<bool> &external_stop) : external_stop_(external_stop) {}

  // Starts the search.
  // @param position The position to search.
  void Search(Position &position);

  // Resets the search parameters.
  void ResetSearchParameters();

  // Negamax search
  // @param alpha The alpha value.
  // @param beta The beta value.
  // @param depth The depth to search.
  // @param position The position to search.
  // @param is_null Whether the current node is a null move.
  // @return The score of the position.
  int Negamax(int alpha, int beta, int depth, Position &position, PvLine *pv_line, bool is_null);

  // Quiescence search
  // @param alpha The alpha value.
  // @param beta The beta value.
  // @param position The position to search.
  // @return The score of the position.
  int Quiescence(int alpha, int beta, Position &position);

  // Returns whether the search should stop.
  // @return Whether the search should stop.
  bool ShouldStop();

  // Checks to see if the search should stop.
  void CheckStop();

  // Scores the given move for the given position.
  // @param move The move to score.
  // @param position The position to score the move for.
  // @return The score of the move.
  int ScoreMove(move::Move move, Position &position);

  // Sorts the given moves for the given position.
  // @param moves The moves to sort.
  // @param position The position to sort the moves for.
  void SortMoves(move::MoveList &moves, Position &position);

  // Returns if we can perform LMR on the given move and position.
  // @param move The move to check.
  // @param position The position to check.
  // @return Whether we can perform LMR on the given move and position.
  bool CanDoLMR(move::Move move, Position &position);

  // Prints the current search info.
  void PrintSearchInfo(PvLine *pv_line, Position &position);
};
// clang-format off
constexpr int kMvvLvaScores[12][12] = {
  105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
  104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
  103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
  102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
  101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
  100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600,

  105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
  104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
  103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
  102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
  101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
  100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
// clang-format on

}  // namespace chess

#endif  // SEARCH_HPP