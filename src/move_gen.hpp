#pragma once
#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include <cstdint>
#include <vector>

#include "utils.hpp"

namespace chess {

class Position; // Forward declaration.

namespace move {

// Move representation:
// 0000 0000 0000 0000 0000 0000 0011 1111 -> source square
// 0000 0000 0000 0000 0000 1111 1100 0000 -> target square
// 0000 0000 0000 0000 1111 0000 0000 0000 -> piece
// 0000 0000 0000 1111 0000 0000 0000 0000 -> promoted piece
// 0000 0000 0001 0000 0000 0000 0000 0000 -> capture
// 0000 0000 0010 0000 0000 0000 0000 0000 -> double push
// 0000 0000 0100 0000 0000 0000 0000 0000 -> en passant
// 0000 0000 1000 0000 0000 0000 0000 0000 -> castle

using Move = uint32_t;
// using MoveList = std::vector<Move>;

class MoveList {
 public:
  int count = 0;
  Move moves[256];

  // Appends the given move to the end of the move list.
  // @param move The move to append.
  void push_back(Move move);

  // Returns the move at the given index.
  // @param index The index.
  Move& operator[](int index);

  // Returns an iterator to the beginning of the move list.
  // @return An iterator to the beginning of the move list.
  Move* begin();

  // Returns an iterator to the end of the move list.
  // @return An iterator to the end of the move list.
  Move* end();

  // Returns a const iterator to the beginning of the move list.
  // @return A const iterator to the beginning of the move list.
  const Move* begin() const;

  // Returns a const iterator to the end of the move list.
  // @return A const iterator to the end of the move list.
  const Move* end() const;
};

inline void MoveList::push_back(Move move) { moves[count++] = move; }
inline Move& MoveList::operator[](int index) { return moves[index]; }
inline Move* MoveList::begin() { return moves; }
inline Move* MoveList::end() { return moves + count; }
inline const Move* MoveList::begin() const { return moves; }
inline const Move* MoveList::end() const { return moves + count; }

// Returns a string representation of the given move.
// @param move The move.
// @return A string representation of the given move.
std::string ToString(Move move);

// Generates a move from the given parameters.
// @param source The source square.
// @param target The target square.
// @param piece The piece.
// @param promoted_piece The promoted piece if the move is a promotion.
// @param capture Whether the move is a capture.
// @param double_push Whether the move is a double pawn push.
// @param en_passant Whether the move is an en passant capture.
// @param castle Whether the move is a castle.
constexpr inline Move CreateMove(Square source, Square target, Piece piece,
                                 Piece promoted_piece, bool capture,
                                 bool double_push, bool en_passant,
                                 bool castle) {
  return (source | (target << 6) | (piece << 12) | (promoted_piece << 16) |
          (capture << 20) | (double_push << 21) | (en_passant << 22) |
          (castle << 23));
}

// Returns the source square of the given move.
// @param move The move.
// @return The source square of the given move.
constexpr inline Square GetSourceSquare(Move move) {
  return static_cast<Square>(move & 0x3F);
}

// Returns the target square of the given move.
// @param move The move.
// @return The target square of the given move.
constexpr inline Square GetTargetSquare(Move move) {
  return static_cast<Square>((move >> 6) & 0x3F);
}

// Returns the piece of the given move.
// @param move The move.
// @return The piece of the given move.
constexpr inline Piece GetPiece(Move move) {
  return static_cast<Piece>((move >> 12) & 0xF);
}

// Returns the promoted piece of the given move.
// @param move The move.
// @return The promoted piece of the given move.
constexpr inline Piece GetPromotedPiece(Move move) {
  return static_cast<Piece>((move >> 16) & 0xF);
}

// Returns whether the given move is a capture.
// @param move The move.
// @return Whether the given move is a capture.
constexpr inline bool IsCapture(Move move) { return (move >> 20) & 0x1; }

// Returns whether the given move is a double pawn push.
// @param move The move.
// @return Whether the given move is a double pawn push.
constexpr inline bool IsDoublePush(Move move) { return (move >> 21) & 0x1; }

// Returns whether the given move is an en passant capture.
// @param move The move.
// @return Whether the given move is an en passant capture.
constexpr inline bool IsEnPassant(Move move) { return (move >> 22) & 0x1; }

// Returns whether the given move is a castle.
// @param move The move.
// @return Whether the given move is a castle.
constexpr inline bool IsCastle(Move move) { return (move >> 23) & 0x1; }

}  // namespace move

// A helper function for perft.
// @param pos The position to perform the perft on.
// @param depth The depth to perform the perft to.
// @return The number of nodes at the given depth.
uint64_t PerftHelper(Position& pos, int depth);

// Performs a perft (performance test) on the given position to the given depth.
// @param pos The position to perform the perft on.
// @param depth The depth to perform the perft to.
// @return The number of nodes at the given depth.
void Perft(Position& pos, int depth);

// Generates all psuedo-legal moves for the given position and appends them to
// the given move list.
// @param pos The position to generate moves for.
// @param moveList The list to append the moves to.
void GenerateMoves(const Position& pos, move::MoveList& moveList);

// Generates all psuedo-legal pawn moves for the given position and appends them
// to the given move list.
// @param pos The position to generate moves for.
// @param moveList The list to append the moves to.
void GeneratePawnMoves(const Position& pos, move::MoveList& moveList);

// Generates all psuedo-legal knight moves for the given position and appends
// them to the given move list.
// @param pos The position to generate moves for.
// @param moveList The list to append the moves to.
void GenerateKnightMoves(const Position& pos, move::MoveList& moveList);

// Generates all psuedo-legal bishop moves for the given position and appends
// them to the given move list.
// @param pos The position to generate moves for.
// @param moveList The list to append the moves to.
void GenerateBishopMoves(const Position& pos, move::MoveList& moveList);

// Generates all psuedo-legal rook moves for the given position and appends them
// to the given move list.
// @param pos The position to generate moves for.
// @param moveList The list to append the moves to.
void GenerateRookMoves(const Position& pos, move::MoveList& moveList);

// Generates all psuedo-legal queen moves for the given position and appends
// them to the given move list.
// @param pos The position to generate moves for.
// @param moveList The list to append the moves to.
void GenerateQueenMoves(const Position& pos, move::MoveList& moveList);

// Generates all psuedo-legal king moves for the given position and appends them
// to the given move list.
// @param pos The position to generate moves for.
// @param moveList The list to append the moves to.
void GenerateKingMoves(const Position& pos, move::MoveList& moveList);

// Returns whether the given square is attacked by the given color.
// @param pos The position.
// @param square The square.
// @param color The attacking color.
bool IsSquareAttacked(const Position& pos, Square square, Color color);

// Returns whether the board is in check.
// @param pos The position.
// @return Whether the board is in check.
bool IsInCheck(const Position& pos);

}  // namespace chess

#endif  // MOVE_GEN_HPP