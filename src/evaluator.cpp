#include "evaluator.hpp"
#include "position.hpp"
#include "precomputed_data.hpp"
#include "utils.hpp"

namespace chess {
int mgEvalTables[12][64];
int egEvalTables[12][64];

void InitEvalTables() {
  for (Piece p = kWhitePawn; p <= kBlackKing; p++) {
    for (Square s = kSquareStart; s < kNumSquares; s++) {
      Square source_square = s;
      if (GetPieceColor(p) == kBlack) {
        source_square = MirrorSquare(s);
      }
      PieceType pt = GetPieceType(p);
      mgEvalTables[p][s] =
          kMgEvalTables[pt][source_square] + kMgPieceValues[pt];
      egEvalTables[p][s] =
          kEgEvalTables[pt][source_square] + kEgPieceValues[pt];
    }
  }
}

int Evaluate(const Position& pos) {
  int mg[2] = {0, 0};
  int eg[2] = {0, 0};
  int game_phase = 0;

  for (Piece p = kWhitePawn; p <= kBlackKing; p++) {
    Bitboard current_pieces = pos.state_.piece_bitboards[p];
    while (current_pieces) {
      Square square = static_cast<Square>(GetLSBIndex(current_pieces));
      ClearLSB(current_pieces);
      Color color = GetPieceColor(p);
      PieceType pt = GetPieceType(p);
      mg[color] += mgEvalTables[p][square];
      eg[color] += egEvalTables[p][square];
      game_phase += kGamePhaseInc[p];

      int double_pawns;
      Piece enemy_pawn;
      int bishop_moves;
      Bitboard friendly_pawns;
      int queen_moves;
      int king_shields;

      switch (pt) {
        case kPawn:
          // Double pawn penalty.
          double_pawns =
              CountBits(current_pieces & kFileMasks[GetFile(square)]);
          if (double_pawns > 1) {
            mg[color] += kDoubledPawnPenalty;
            eg[color] += kDoubledPawnPenalty;
          }

          // Isolated pawn penalty.
          if (!(current_pieces &
                precomputed_data::isolated_pawn_masks[square])) {
            mg[color] += kIsolatedPawnPenalty;
            eg[color] += kIsolatedPawnPenalty;
          }

          // Passed pawn bonus.
          enemy_pawn = GetPiece(kPawn, ~color);
          if (!(precomputed_data::passed_pawn_masks[square][color] &
                pos.state_.piece_bitboards[enemy_pawn])) {
            int passed_ranks = GetRank(square);
            if (color == kBlack) passed_ranks = 7 - passed_ranks;
            mg[color] += kPassedPawnBonus[passed_ranks];
            eg[color] += kPassedPawnBonus[passed_ranks];
          }
          break;
        case kKnight:
          break;
        case kBishop:
          // Bishop mobility bonus.
          bishop_moves =
          CountBits(precomputed_data::GetBishopAttacks(
                        square, pos.state_.piece_occupancy[kBothColors]) &
                    ~pos.state_.piece_occupancy[color]);
          mg[color] += bishop_moves * kBishopMobilityBonus;
          eg[color] += bishop_moves * kBishopMobilityBonus;
          break;
        case kRook:
          // Rook semi open file bonus.
          friendly_pawns = pos.state_.piece_bitboards[GetPiece(kPawn, color)];
          if (!(friendly_pawns & kFileMasks[GetFile(square)])) {
            mg[color] += kRookSemiOpenFileBonus;
            eg[color] += kRookSemiOpenFileBonus;
          }

          // Rook open file bonus.
          if (!((pos.state_.piece_bitboards[kWhitePawn] |
                 pos.state_.piece_bitboards[kBlackPawn]) &
                kFileMasks[GetFile(square)])) {
            mg[color] += kRookOpenFileBonus;
            eg[color] += kRookOpenFileBonus;
          }
          break;
        case kQueen:
          // Queen mobility bonus.
          queen_moves =
              CountBits(precomputed_data::GetQueenAttacks(
                            square, pos.state_.piece_occupancy[kBothColors])
                            &
                        ~pos.state_.piece_occupancy[color]);
          mg[color] += queen_moves * kQueenMobilityBonus;
          eg[color] += queen_moves * kQueenMobilityBonus;
          break;
        case kKing:
          // King semi open file penalty.
          friendly_pawns = pos.state_.piece_bitboards[GetPiece(kPawn, color)];
          if (!(friendly_pawns & kFileMasks[GetFile(square)])) {
            mg[color] += kKingSemiOpenFilePenalty;
            eg[color] += kKingSemiOpenFilePenalty;
          }

          // King open file penalty.
          if (!((pos.state_.piece_bitboards[kWhitePawn] |
                 pos.state_.piece_bitboards[kBlackPawn]) &
                kFileMasks[GetFile(square)])) {
            mg[color] += kKingOpenFilePenalty;
            eg[color] += kKingOpenFilePenalty;
          }

          // King shield bonus.
          king_shields = CountBits(precomputed_data::king_attacks[square] &
                                   pos.state_.piece_occupancy[color]);
          mg[color] += king_shields * kKingShieldBonus;
          eg[color] += king_shields * kKingShieldBonus;
          break;

        default:
          break;
      }
    }
  }

  // Bishop pair bonus.
  if (CountBits(pos.state_.piece_bitboards[kWhiteBishop]) >= 2) {
    mg[kWhite] += kBishopPairBonus;
    eg[kWhite] += kBishopPairBonus;
  }
  if (CountBits(pos.state_.piece_bitboards[kBlackBishop]) >= 2) {
    mg[kBlack] += kBishopPairBonus;
    eg[kBlack] += kBishopPairBonus;
  }

  // Tapered eval.
  int mg_score = mg[kWhite] - mg[kBlack];
  int eg_score = eg[kWhite] - eg[kBlack];
  int mg_phase = std::min(game_phase, 24);
  int eg_phase = 24 - mg_phase;

  int score = ((mg_score * mg_phase) + (eg_score * eg_phase)) / 24;

  // Flip score if black is to move.
  if (pos.state_.side_to_move == kBlack) score = -score;

  return score;
}

}  // namespace chess