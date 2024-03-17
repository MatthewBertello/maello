#include "search.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

#include "evaluator.hpp"
#include "move_gen.hpp"
#include "transposition_table.hpp"

namespace chess {

void SearchEngine::Search(Position &position) {
  start_time_ = GetTime() - 1;  // Subtract 1 to prevent divide by 0 errors

  // Determine the time to search
  int time_remaining =
      position.state_.side_to_move == kWhite ? white_time_ : black_time_;
  int increment =
      position.state_.side_to_move == kWhite ? white_inc_ : black_inc_;
  if (engine_decides_search_params_) {
    if (time_remaining == 0) {
      engine_decides_search_params_ = false;
    } else if (time_remaining < kTimeBuffer) {
      engine_decides_search_params_ = false;
      search_depth_ = 1;
    } else {
      time_remaining -= kTimeBuffer;
      if (moves_to_go_ == 0) moves_to_go_ = kDefaultMovesToGo;
      end_time_ = start_time_ + (time_remaining / moves_to_go_) + increment;
    }
  };

  // Reset the search variables for a new search
  memset(killer_moves_, 0, sizeof(killer_moves_));
  memset(history_moves_, 0, sizeof(history_moves_));
  bool printed_info = false;
  nodes = 0;
  ply = 0;
  current_depth_ = 1;
  score = kUnknownScore;
  int temp_score = kUnknownScore;
  int alpha = -kInfinity;
  int beta = kInfinity;

  // Check if we should stop the search
  CheckStop();

  // Iterative deepening loop, always does at least one iteration
  do {
    PvLine new_pv_line;
    printed_info = false;

    // Perform aspiration window search
    temp_score =
        Negamax(alpha, beta, current_depth_, position, &new_pv_line, false);

    // Check if we are outside of the kAspirationWindow
    if (temp_score <= alpha || temp_score >= beta) {
      // If we are outside of the kAspirationWindow, do a full depth search
      temp_score = Negamax(-kInfinity, kInfinity, current_depth_, position,
                           &new_pv_line, false);
    }

    // If the search was stopped, check if we should use the new PV line
    if (stop_search_) {
      // If the new PV line has the same first move then it is a more accurate
      // evaluation so we should use it
      if (new_pv_line.moves[0] == pv_line_.moves[0]) {
        score = temp_score;
        for (int i = 0; i < new_pv_line.count; i++) {
          pv_line_.moves[i] = new_pv_line.moves[i];
        }
        pv_line_.count = new_pv_line.count;
      } else if (temp_score > score) {
        // If the new PV line is better than the old one then we should use it
        score = temp_score;
        for (int i = 0; i < new_pv_line.count; i++) {
          pv_line_.moves[i] = new_pv_line.moves[i];
        }
        pv_line_.count = new_pv_line.count;
      } else {
        // Otherwise we cannot trust the new PV line so we should just use the
        // old one
        current_depth_--;  // Decrement the depth because we didn't find any new
                           // info from the current depth
      }
      break;
    }

    // Update the PV line and score
    for (int i = 0; i < new_pv_line.count; i++) {
      pv_line_.moves[i] = new_pv_line.moves[i];
    }
    pv_line_.count = new_pv_line.count;
    score = temp_score;

    // Update the alpha and beta values for the aspiration window
    alpha = temp_score - kAspirationWindow;
    beta = temp_score + kAspirationWindow;

    // Print the search info
    PrintSearchInfo(&pv_line_, position);
    printed_info = true;

    // Check if we have a checkmate score, if so then we can stop searching
    if ((score > kCheckmateScore && score < kCheckmateWindow) ||
        (score > -kCheckmateWindow && score < -kCheckmateScore)) {
      stop_search_ = true;
      break;
    }

    current_depth_++;  // Increment the depth
    CheckStop();       // Check if we should stop the search
  } while (!stop_search_);

  if (!printed_info) {
    PrintSearchInfo(&pv_line_, position);
  }

  std::cout << "bestmove " << move::ToString(pv_line_.moves[0]) << std::endl;
}

void SearchEngine::ResetSearchParameters() {
  search_depth_ = -1;
  current_depth_ = -1;
  max_nodes_ = 0;
  start_time_ = 0;
  end_time_ = 0;
  white_time_ = 0;
  black_time_ = 0;
  white_inc_ = 0;
  black_inc_ = 0;
  moves_to_go_ = 0;
  engine_decides_search_params_ = false;
}

int SearchEngine::Negamax(int alpha, int beta, int depth, Position &position,
                          PvLine *pv_line, bool is_null) {
  if (nodes % kCheckupFrequency == 0) CheckStop();

  nodes++;

  // Probe the transposition table
  TTEntry tt_entry = position.transposition_table_.Get(position.state_.key);
  if (tt_entry.key == position.state_.key && tt_entry.depth >= depth) {
    if (tt_entry.flags == kAlphaHashFlag && tt_entry.score <= alpha) {
      return alpha;
    } else if (tt_entry.flags == kBetaHashFlag && tt_entry.score >= beta) {
      return beta;
    } else {
      tt_move_ = tt_entry.bestMove;
    }
  } else {
    tt_move_ = 0;
  }

  // Check for draw
  if (ply > 0 && position.repetition_table_.HasRepetition(position.state_.key))
    return kDrawScore;
  if (position.state_.halfmove_clock >= 100) return kDrawScore;

  // Extend the search if in check
  bool in_check = IsInCheck(position);
  if (in_check) depth++;

  // Check for max depth reached or 0 depth
  if (ply > kMaxSearchDepth - 1 || depth <= 0) {
    pv_line->count = 0;
    int score = Quiescence(alpha, beta, position);
    position.transposition_table_.Store(position.state_.key, depth,
                                        kExactHashFlag, score, 0);
    return score;
  }

  // Null move pruning
  bool null_move_allowed =
      (depth > (1 + kNullMoveReductionAmount)) && !in_check && !is_null;
  if (null_move_allowed) {
    null_move_allowed =
        position.GetNumNonPawnKingPieces(kBothColors) > kEndgamePieceCount;
  }
  if (null_move_allowed) {
    PositionState state = position.GetState();
    ply++;
    position.repetition_table_.Add(state.key);
    if (position.state_.en_passant_square != kNoSquare) {
      position.state_.key ^=
          zobrist::en_passant_keys[position.state_.en_passant_square];
      position.state_.en_passant_square = kNoSquare;
    }
    position.state_.side_to_move = ~position.state_.side_to_move;
    position.state_.key ^= zobrist::side_key;

    int score = -Negamax(-beta, -beta + 1, depth - 1 - kNullMoveReductionAmount,
                         position, pv_line, true);
    ply--;
    position.repetition_table_.RemoveLast();
    position.SetState(state);
    if (stop_search_) {
      return alpha;
    }
    if (score >= beta) {
      position.transposition_table_.Store(position.state_.key, depth,
                                          kBetaHashFlag, beta, 0);
      return beta;
    }
  }

  // Generate moves
  move::MoveList moves;
  GenerateMoves(position, moves);
  SortMoves(moves, position);

  PositionState state;
  PvLine new_pv_line;
  TTFlags tt_flag = kAlphaHashFlag;

  int legal_moves = 0;
  int moves_searched = 0;

  // loop through moves
  for (move::Move move : moves) {
    state = position.GetState();
    ply++;
    position.repetition_table_.Add(state.key);

    // If the move is not legal, skip it
    if (!position.MakeMove(move, false)) {
      ply--;
      position.repetition_table_.RemoveLast();
      continue;
    }

    legal_moves++;
    int score;

    // Always do a full depth search on the first few moves
    if (moves_searched < kMinimumFullDepthSearches) {
      score = -Negamax(-beta, -alpha, depth - 1, position, &new_pv_line, false);
    } else {  // Otherwise try to reduce the search
      // Check if we can use LMR
      if (moves_searched >= kLmrFullDepthMoves && depth >= kLmrReductionLimit &&
          CanDoLMR(move, position)) {
        // If we can use LMR, do a reduced depth PVS search
        score = -Negamax(-alpha - 1, -alpha, depth - 1 - kLmrReductionAmount,
                         position, &new_pv_line, false);
      } else {
        // If we can't use LMR, do a full PVS search
        score = alpha + 1;
      }
      // If we need to perform a full PVS search
      if (score > alpha) {
        // Do a full PVS search
        score = -Negamax(-alpha - 1, -alpha, depth - 1, position, &new_pv_line,
                         false);

        // If the score is in the window, do a full search
        if (score > alpha && score < beta) {
          score =
              -Negamax(-beta, -alpha, depth - 1, position, &new_pv_line, false);
        }
      }
    }
    // Now that we have the score, undo the move
    ply--;
    position.repetition_table_.RemoveLast();
    position.SetState(state);

    // Check if we should stop the search, if so then we can't use the score
    // and we should just return
    if (stop_search_) return alpha;

    moves_searched++;

    // Check if the score fails high
    if (score >= beta) {
      // If it is a quiet move, add it to the killer moves
      if (!move::IsCapture(move) && move::GetPromotedPiece(move) == kNoPiece) {
        for (int i = kNumKillerMoves - 1; i > 0; i--) {
          killer_moves_[current_depth_][i] =
              killer_moves_[current_depth_][i - 1];
        }
        killer_moves_[current_depth_][0] = move;
      }
      position.transposition_table_.Store(position.state_.key, depth,
                                          kBetaHashFlag, beta, move);
      return beta;
    }

    // Check if the score is better than alpha
    if (score > alpha) {
      // If it is a quiet move and not a promotion, add it to the history
      // moves
      if (!move::IsCapture(move) && move::GetPromotedPiece(move) == kNoPiece) {
        history_moves_[position.PieceOn(move::GetSourceSquare(move))]
                      [move::GetTargetSquare(move)] += depth * depth;
      }

      // Update the PV line
      pv_line->moves[0] = move;
      for (int i = 0; i < new_pv_line.count; i++) {
        pv_line->moves[i + 1] = new_pv_line.moves[i];
      }
      pv_line->count = new_pv_line.count + 1;

      tt_flag = kExactHashFlag;
      alpha = score;
    }
  }

  // If there are no legal moves, it is either checkmate or stalemate
  if (legal_moves == 0) {
    if (in_check) {
      return kCheckmateScore + ply;
    } else {
      return kDrawScore;
    }
  }

  position.transposition_table_.Store(position.state_.key, depth, tt_flag,
                                      alpha, pv_line->moves[0]);
  return alpha;
}

int SearchEngine::Quiescence(int alpha, int beta, Position &position) {
  if (nodes % kCheckupFrequency == 0) CheckStop();

  nodes++;

  // Check for draw
  if (ply > 0 && position.repetition_table_.HasRepetition(position.state_.key))
    return kDrawScore;
  if (position.state_.halfmove_clock >= 100) return kDrawScore;

  int evaluation = Evaluate(position);

  if (evaluation >= beta) return beta;
  if (evaluation > alpha) alpha = evaluation;

  // Generate moves
  move::MoveList moves;
  GenerateMoves(position, moves);
  SortMoves(moves, position);

  PositionState state;

  // loop through moves
  for (move::Move move : moves) {
    state = position.GetState();
    ply++;
    position.repetition_table_.Add(state.key);

    // If the move is not legal, skip it
    if (!position.MakeMove(move, true)) {
      ply--;
      position.repetition_table_.RemoveLast();
      continue;
    }

    int score = -Quiescence(-beta, -alpha, position);

    // Now that we have the score, undo the move
    ply--;
    position.repetition_table_.RemoveLast();
    position.SetState(state);

    // Check if we should stop the search, if so then we can't use the score
    // and we should just return
    if (stop_search_) return alpha;

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }

  return alpha;
}

bool SearchEngine::ShouldStop() {
  if (external_stop_) return true;
  if (max_nodes_ != 0 && nodes >= max_nodes_) return true;
  if (search_depth_ != -1 && current_depth_ > search_depth_) return true;
  if (end_time_ != 0 && GetTime() >= end_time_) return true;
  if (current_depth_ > kMaxSearchDepth) return true;
  return false;
}

void SearchEngine::CheckStop() { stop_search_ = ShouldStop(); }

int SearchEngine::ScoreMove(move::Move move, Position &position) {
  if (move == pv_line_.moves[ply]) return 100000;

  if (move == tt_move_) return 90000;

  if (move::IsCapture(move)) {
    Piece target_piece = position.PieceOn(move::GetTargetSquare(move));
    Piece piece = position.PieceOn(move::GetSourceSquare(move));
    return kMvvLvaScores[piece][target_piece] + 10000;
  }

  for (int i = 0; i < kNumKillerMoves; i++) {
    if (move == killer_moves_[current_depth_][i]) {
      return 9000 - i;
    }
  }

  if (move::GetPromotedPiece(move) != kNoPiece) {
    return 8000 + move::GetPromotedPiece(move);
  }

  return history_moves_[position.PieceOn(move::GetSourceSquare(move))]
                       [move::GetTargetSquare(move)];
}

void SearchEngine::SortMoves(move::MoveList &moves, Position &position) {
  std::vector<std::pair<int, move::Move>> scored_moves;
  scored_moves.reserve(moves.count);

  for (int i = 0; i < moves.count; i++) {
    scored_moves.push_back(
        std::make_pair(ScoreMove(moves[i], position), moves[i]));
  }

  std::sort(
      scored_moves.begin(), scored_moves.end(),
      [](const std::pair<int, move::Move> &a,
         const std::pair<int, move::Move> &b) { return a.first > b.first; });

  for (int i = 0; i < moves.count; i++) {
    moves[i] = scored_moves[i].second;
  }
}

bool SearchEngine::CanDoLMR(move::Move move, Position &position) {
  if (move::IsCapture(move)) return false;
  if (move::GetPromotedPiece(move) != kNoPiece) return false;
  if (IsInCheck(position)) return false;
  return true;
}

void SearchEngine::PrintSearchInfo(PvLine *pv_line, Position &position) {
  if (score > kCheckmateScore && score < kCheckmateWindow) {
    std::cout << "info score mate " << (score - kCheckmateScore + 1) / -2;
    std::cout << " depth " << current_depth_;
    std::cout << " nodes " << nodes;
    std::cout << " time " << (GetTime() - start_time_);
    std::cout << " nps " << (nodes * 1000) / (GetTime() - start_time_);
    std::cout << " hashfull " << position.transposition_table_.GetFullPercentage();
  } else if (score > -kCheckmateWindow && score < -kCheckmateScore) {
    std::cout << "info score mate "
              << std::abs(score + kCheckmateScore - 1) / 2;
    std::cout << " depth " << current_depth_;
    std::cout << " nodes " << nodes;
    std::cout << " time " << (GetTime() - start_time_);
    std::cout << " nps " << (nodes * 1000) / (GetTime() - start_time_);
    std::cout << " hashfull " << position.transposition_table_.GetFullPercentage();
  } else {
    std::cout << "info score cp " << score;
    std::cout << " depth " << current_depth_;
    std::cout << " nodes " << nodes;
    std::cout << " time " << (GetTime() - start_time_);
    std::cout << " nps " << (nodes * 1000) / (GetTime() - start_time_);
    std::cout << " hashfull " << position.transposition_table_.GetFullPercentage();
  }
  std::cout << " pv ";
  for (int count = 0; count < pv_line->count; count++) {
    std::cout << move::ToString(pv_line->moves[count]) << " ";
  }
  std::cout << std::endl;
}

}  // namespace chess