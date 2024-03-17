#pragma once
#ifndef PRECOMPUTED_DATA_HPP
#define PRECOMPUTED_DATA_HPP

#include "utils.hpp"

namespace chess {
namespace precomputed_data {

// Precomputed attack masks
extern Bitboard pawn_attacks[kNumSquares][kNumColors];
extern Bitboard knight_attacks[kNumSquares];
extern Bitboard king_attacks[kNumSquares];
extern Bitboard rook_masks[kNumSquares];
extern Bitboard bishop_masks[kNumSquares];

// Precomputed bishop and rook attacks
extern Bitboard bishop_attacks[kNumSquares][512];
extern Bitboard rook_attacks[kNumSquares][4096];

// Precomputed relevant bit counts for bishop and rook attack masks
extern int bishop_relevant_bits[kNumSquares];
extern int rook_relevant_bits[kNumSquares];

// Magic numbers for hashing bishop and rook attacks
extern Key bishop_magic_numbers[kNumSquares];
extern Key rook_magic_numbers[kNumSquares];

extern Bitboard isolated_pawn_masks[kNumSquares];
extern Bitboard passed_pawn_masks[kNumSquares][kNumColors];

// Initializes the precomputed data.
void Init();

// Initializes the magic numbers for sliding pieces.
void InitMagicNumbers();

// Initializes the relevant bits for sliding pieces.
void InitRelevantBits();

// Initializes the sliding piece attacks.
void InitSlidingAttacks();

// Initializes the leaping piece attacks.
void InitLeapingAttacks();

// Initializes the evaluation masks.
void InitEvaluationMasks();

// Finds the magic number for a sliding piece.
// More info: https://www.chessprogramming.org/Looking_for_Magics
// @throws std::runtime_error if a magic number cannot be found.
// @param square The square the sliding piece is on.
// @param relevant_bits The number of relevant bits in the attack mask.
// @param piece The sliding piece.
// @return The magic number
Key FindMagicNumber(Square square, int relevant_bits, bool is_bishop);

// Generates a magic number candidate.
// @return A magic number candidate.
Key GenerateMagicNumberCandidate();

// Generates the attack mask for a pawn.
// @param square The square the pawn is on.
// @param color The color of the pawn.
// @return The attack mask for the pawn.
Bitboard GeneratePawnAttackMask(Square square, Color color);

// Generates the attack mask for a knight.
// @param square The square the knight is on.
// @return The attack mask for the knight.
Bitboard GenerateKnightAttackMask(Square square);

// Generates the attack mask for a bishop.
// @param square The square the bishop is on.
// @return The attack mask for the bishop.
Bitboard GenerateBishopAttackMask(Square square);

// Generates the attack mask for a rook.
// @param square The square the rook is on.
// @return The attack mask for the rook.
Bitboard GenerateRookAttackMask(Square square);

// Generates the attack mask for a king.
// @param square The square the king is on.
// @return The attack mask for the king.
Bitboard GenerateKingAttackMask(Square square);

// Generates the attacks for a bishop.
// @param square The square the bishop is on.
// @param occupancy The occupancy of the board.
Bitboard GenerateBishopAttacks(Square square, Bitboard occupancy);

// Generates the attacks for a rook.
// @param square The square the rook is on.
// @param occupancy The occupancy of the board.
Bitboard GenerateRookAttacks(Square square, Bitboard occupancy);

// Generates the occupancy key at the given index for an attack mask.
// Example:
// A 2x2 attack mask with 2 relevant bits:
// 10
// 01
// Flattened into a 1D array:
// 1001
// The number of possible occupancy keys is (relevant_bits ^ 2).
// This attack mask has 4 (2^2) possible occupancy keys at the indices of 0-3:
// 0 = 0000,
// 1 = 0001,
// 2 = 1000,
// 3 = 1001
// @param index The index of the occupancy key.
// @param attack_mask The attack mask to generate the occupancy key for.
// @return The occupancy key at the given index for the attack mask.
Key GenerateOccupancyKey(int index, Bitboard attack_mask);

// Gets the bishop attacks for the given square and occupancy.
// @param square The square the bishop is on.
// @param occupancy The occupancy of the board.
// @return The bishop attacks for the given square and occupancy.
inline Bitboard GetBishopAttacks(Square square, Bitboard occupancy) {
  occupancy &= bishop_masks[square];
  occupancy *= bishop_magic_numbers[square];
  occupancy >>= 64 - bishop_relevant_bits[square];
  return bishop_attacks[square][occupancy];
}

// Gets the rook attacks for the given square and occupancy.
// @param square The square the rook is on.
// @param occupancy The occupancy of the board.
// @return The rook attacks for the given square and occupancy.
inline Bitboard GetRookAttacks(Square square, Bitboard occupancy) {
  occupancy &= rook_masks[square];
  occupancy *= rook_magic_numbers[square];
  occupancy >>= 64 - rook_relevant_bits[square];
  return rook_attacks[square][occupancy];
}

// Gets the queen attacks for the given square and occupancy.
// @param square The square the queen is on.
// @param occupancy The occupancy of the board.
// @return The queen attacks for the given square and occupancy.
inline Bitboard GetQueenAttacks(Square square, Bitboard occupancy) {
  return GetBishopAttacks(square, occupancy) |
         GetRookAttacks(square, occupancy);
}

}  // namespace precomputed_data

}  // namespace chess

#endif  // PRECOMPUTED_DATA_HPP