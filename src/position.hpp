#pragma once
#ifndef POSITION_HPP
#define POSITION_HPP

#include <iostream>
#include <string>

#include "move_gen.hpp"
#include "utils.hpp"
#include "repetition_table.hpp"
#include "transposition_table.hpp"

namespace chess {

// Represents the state of a chess position.
struct PositionState {
  Bitboard piece_bitboards[kPieceCount];
  Bitboard piece_occupancy[kOccupancies];
  Color side_to_move;
  Square en_passant_square;
  CastlingRights castling_rights;
  Key key;
  int halfmove_clock;
  int ply;
};

namespace zobrist {

extern Key piece_keys[kPieceCount][kNumSquares];
extern Key en_passant_keys[kNumSquares];
extern Key castling_keys[16];
extern Key side_key;

// Initializes the Zobrist keys.
void Init();

}  // namespace zobrist

// Represents a chess position.
class Position {
 public:
  Position() = default;
  Position(const Position&) = delete;
  Position& operator=(const Position&) = delete;

  PositionState state_;
  RepetitionTable repetition_table_;
  TranspositionTable transposition_table_;


  // Creates a position from the given state.
  // @param state The state to create the position from.
  Position(const PositionState& state);

  // Resets the position to the starting position.
  void Reset();

  // Returns the piece on the given square.
  // @param square The square to get the piece from.
  Piece PieceOn(Square square) const;

  // Returns a copy of the current state.
  // @return A copy of the current state.
  inline PositionState GetState() const;

  // Sets the current state to the given state.
  // @param state The state to set the current state to.
  inline void SetState(const PositionState& state);

  // Generates the Zobrist key for the current position.
  // @return The Zobrist key for the current position.
  Key GenerateKey() const;

  // Generates the occupancy bitboards for the white pieces.
  void GenerateWhiteOccupancies();

  // Generates the occupancy bitboards for the black pieces.
  void GenerateBlackOccupancies();

  // Generates the occupancy bitboards for all pieces and the shared occupancy
  void GenerateOccupancies();

  // Makes the given move on the current position. If the move is illegal, the
  // position is not modified.
  // @param move The move to make.
  // @return If the move was legal.
  bool MakeMove(move::Move move, bool quiescencse);

  // Gets the number of non pawn or king pieces on the board.
  // @return The number of non pawn or king pieces on the board.
  int GetNumNonPawnKingPieces(Color side) const;

  // Sets the position to the given FEN string.
  // This function assumes that the FEN string is valid.
  // @throws std::invalid_argument if the FEN string is invalid.
  // @param fen The FEN string.
  void Set(const std::string& fen);

  // Returns the FEN string for the current position.
  // @return The FEN string for the current position.
  std::string ToFen() const;
};

std::ostream& operator<<(std::ostream& os, const Position& pos);

inline PositionState Position::GetState() const { return state_; }

inline void Position::SetState(const PositionState& state) { state_ = state; }

inline void Position::GenerateWhiteOccupancies() {
  state_.piece_occupancy[kWhite] = 0;
  for (int p = kWhitePawn; p <= kWhiteKing; p++) {
    state_.piece_occupancy[kWhite] |= state_.piece_bitboards[p];
  }
}

inline void Position::GenerateBlackOccupancies() {
  state_.piece_occupancy[kBlack] = 0;
  for (int p = kBlackPawn; p <= kBlackKing; p++) {
    state_.piece_occupancy[kBlack] |= state_.piece_bitboards[p];
  }
}

inline void Position::GenerateOccupancies() {
  GenerateWhiteOccupancies();
  GenerateBlackOccupancies();
  state_.piece_occupancy[kBothColors] =
      state_.piece_occupancy[kWhite] | state_.piece_occupancy[kBlack];
}

}  // namespace chess

#endif  // POSITION_HPP