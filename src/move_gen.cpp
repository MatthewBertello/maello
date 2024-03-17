#include "move_gen.hpp"

#include <iostream>

#include "position.hpp"
#include "precomputed_data.hpp"
#include "utils.hpp"

namespace chess {
namespace move {
std::string ToString(Move move) {
  std::string str = "";
  str += kSquareStrings[GetSourceSquare(move)];
  str += kSquareStrings[GetTargetSquare(move)];
  char promotion_char = PieceToChar(GetPromotedPiece(move));
  promotion_char = tolower(promotion_char);
  if (promotion_char != ' ') {
    str += promotion_char;
  }
  return str;
}
}  // namespace move

uint64_t PerftHelper(Position& pos, int depth) {
  if (depth == 0) {
    return 1;
  }
  move::MoveList moveList;
  GenerateMoves(pos, moveList);
  uint64_t nodes = 0;
  PositionState state;
  for (const move::Move& move : moveList) {
    state = pos.GetState();
    if (!pos.MakeMove(move, false)) {
      continue;
    }

    nodes += PerftHelper(pos, depth - 1);
    pos.SetState(state);
  }
  return nodes;
}

void Perft(Position& pos, int depth) {
  Time start_time, end_time;
  start_time = GetTime();

  uint64_t nodes = 0;
  if (depth == 0) {
    std::cout << "Depth: " << depth << " Nodes: " << nodes << " Time: " << 0
              << std::endl;
  }

  move::MoveList moveList;
  GenerateMoves(pos, moveList);
  PositionState state;
  for (const move::Move& move : moveList) {
    state = pos.GetState();
    if (!pos.MakeMove(move, false)) {
      continue;
    }

    uint64_t new_nodes = PerftHelper(pos, depth - 1);
    pos.SetState(state);
    nodes += new_nodes;

    std::cout << move::ToString(move) << ": Nodes " << new_nodes << std::endl;
  }
  end_time = GetTime();
  std::cout << "Depth: " << depth << " Nodes: " << nodes
            << " Time: " << end_time - start_time << std::endl;
}

void GenerateMoves(const Position& pos, move::MoveList& moveList) {
  GeneratePawnMoves(pos, moveList);
  GenerateKnightMoves(pos, moveList);
  GenerateBishopMoves(pos, moveList);
  GenerateRookMoves(pos, moveList);
  GenerateQueenMoves(pos, moveList);
  GenerateKingMoves(pos, moveList);
}

void GeneratePawnMoves(const Position& pos, move::MoveList& moveList) {
  Square source_square, target_square;
  Color side_to_move = pos.state_.side_to_move;
  Color opponent_side = ~side_to_move;
  Piece current_piece;

  Bitboard current_bitboard, current_attacks;

  current_piece = GetPiece(kPawn, side_to_move);
  current_bitboard = pos.state_.piece_bitboards[current_piece];
  Direction push_direction = side_to_move == kWhite ? kNorth : kSouth;
  Rank promotion_rank = side_to_move == kWhite ? kRank8 : kRank1;
  Rank starting_rank = side_to_move == kWhite ? kRank2 : kRank7;
  while (current_bitboard) {
    source_square = Square(GetLSBIndex(current_bitboard));
    ClearLSB(current_bitboard);

    // Generate pawn pushes
    target_square = source_square + push_direction;
    if (IsValidSquare(target_square) &&
        !GetBit(pos.state_.piece_occupancy[kBothColors], target_square)) {
      if (GetRank(target_square) == promotion_rank) {
        moveList.push_back(move::CreateMove(
            source_square, target_square, current_piece,
            GetPiece(kQueen, side_to_move), false, false, false, false));
        moveList.push_back(move::CreateMove(
            source_square, target_square, current_piece,
            GetPiece(kRook, side_to_move), false, false, false, false));
        moveList.push_back(move::CreateMove(
            source_square, target_square, current_piece,
            GetPiece(kBishop, side_to_move), false, false, false, false));
        moveList.push_back(move::CreateMove(
            source_square, target_square, current_piece,
            GetPiece(kKnight, side_to_move), false, false, false, false));
      } else {
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, false,
                                            false, false, false));

        // Generate double pawn pushes
        if (GetRank(source_square) == starting_rank) {
          target_square = source_square + (2 * push_direction);
          if (IsValidSquare(target_square) &&
              !GetBit(pos.state_.piece_occupancy[kBothColors], target_square)) {
            moveList.push_back(move::CreateMove(source_square, target_square,
                                                current_piece, kNoPiece, false,
                                                true, false, false));
          }
        }
      }
    }

    // Generate pawn captures
    current_attacks =
        precomputed_data::pawn_attacks[source_square][side_to_move];
    while (current_attacks) {
      target_square = static_cast<Square>(GetLSBIndex(current_attacks));
      ClearLSB(current_attacks);

      if (GetBit(pos.state_.piece_occupancy[opponent_side], target_square)) {
        if (GetRank(target_square) == promotion_rank) {
          moveList.push_back(move::CreateMove(
              source_square, target_square, current_piece,
              GetPiece(kQueen, side_to_move), true, false, false, false));
          moveList.push_back(move::CreateMove(
              source_square, target_square, current_piece,
              GetPiece(kRook, side_to_move), true, false, false, false));
          moveList.push_back(move::CreateMove(
              source_square, target_square, current_piece,
              GetPiece(kBishop, side_to_move), true, false, false, false));
          moveList.push_back(move::CreateMove(
              source_square, target_square, current_piece,
              GetPiece(kKnight, side_to_move), true, false, false, false));
        } else {
          moveList.push_back(move::CreateMove(source_square, target_square,
                                              current_piece, kNoPiece, true,
                                              false, false, false));
        }
      } else if (target_square == pos.state_.en_passant_square) {
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, true,
                                            false, true, false));
      }
    }
  }
}

void GenerateKnightMoves(const Position& pos, move::MoveList& moveList) {
  Square source_square, target_square;
  Color side_to_move = pos.state_.side_to_move;
  Color opponent_side = ~side_to_move;
  Piece current_piece = GetPiece(kKnight, side_to_move);

  Bitboard current_bitboard, current_attacks;

  current_bitboard = pos.state_.piece_bitboards[current_piece];
  while (current_bitboard) {
    source_square = static_cast<Square>(GetLSBIndex(current_bitboard));
    ClearLSB(current_bitboard);
    current_attacks = precomputed_data::knight_attacks[source_square] &
                      ~pos.state_.piece_occupancy[side_to_move];

    while (current_attacks) {
      target_square = static_cast<Square>(GetLSBIndex(current_attacks));
      ClearLSB(current_attacks);

      // Generate knight captures
      if (GetBit(pos.state_.piece_occupancy[opponent_side], target_square)) {
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, true,
                                            false, false, false));
      } else {
        // Generate quiet knight moves
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, false,
                                            false, false, false));
      }
    }
  }
}

void GenerateBishopMoves(const Position& pos, move::MoveList& moveList) {
  Square source_square, target_square;
  Color side_to_move = pos.state_.side_to_move;
  Color opponent_side = ~side_to_move;
  Piece current_piece = GetPiece(kBishop, side_to_move);

  Bitboard current_bitboard, current_attacks;

  current_bitboard = pos.state_.piece_bitboards[current_piece];
  while (current_bitboard) {
    source_square = static_cast<Square>(GetLSBIndex(current_bitboard));
    ClearLSB(current_bitboard);
    current_attacks =
        precomputed_data::GetBishopAttacks(
            source_square, pos.state_.piece_occupancy[kBothColors]) &
        ~pos.state_.piece_occupancy[side_to_move];

    while (current_attacks) {
      target_square = static_cast<Square>(GetLSBIndex(current_attacks));
      ClearLSB(current_attacks);

      // Generate bishop captures
      if (GetBit(pos.state_.piece_occupancy[opponent_side], target_square)) {
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, true,
                                            false, false, false));
      } else {
        // Generate quiet bishop moves
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, false,
                                            false, false, false));
      }
    }
  }
}

void GenerateRookMoves(const Position& pos, move::MoveList& moveList) {
  Square source_square, target_square;
  Color side_to_move = pos.state_.side_to_move;
  Color opponent_side = ~side_to_move;
  Piece current_piece = GetPiece(kRook, side_to_move);

  Bitboard current_bitboard, current_attacks;

  current_bitboard = pos.state_.piece_bitboards[current_piece];
  while (current_bitboard) {
    source_square = static_cast<Square>(GetLSBIndex(current_bitboard));
    ClearLSB(current_bitboard);
    current_attacks =
        precomputed_data::GetRookAttacks(
            source_square, pos.state_.piece_occupancy[kBothColors]) &
        ~pos.state_.piece_occupancy[side_to_move];

    while (current_attacks) {
      target_square = static_cast<Square>(GetLSBIndex(current_attacks));
      ClearLSB(current_attacks);

      // Generate rook captures
      if (GetBit(pos.state_.piece_occupancy[opponent_side], target_square)) {
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, true,
                                            false, false, false));
      } else {
        // Generate quiet rook moves
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, false,
                                            false, false, false));
      }
    }
  }
}

void GenerateQueenMoves(const Position& pos, move::MoveList& moveList) {
  Square source_square, target_square;
  Color side_to_move = pos.state_.side_to_move;
  Color opponent_side = ~side_to_move;
  Piece current_piece = GetPiece(kQueen, side_to_move);

  Bitboard current_bitboard, current_attacks;

  current_bitboard = pos.state_.piece_bitboards[current_piece];
  while (current_bitboard) {
    source_square = static_cast<Square>(GetLSBIndex(current_bitboard));
    ClearLSB(current_bitboard);
    current_attacks =
        precomputed_data::GetQueenAttacks(
            source_square, pos.state_.piece_occupancy[kBothColors]) &
        ~pos.state_.piece_occupancy[side_to_move];

    while (current_attacks) {
      target_square = static_cast<Square>(GetLSBIndex(current_attacks));
      ClearLSB(current_attacks);

      // Generate queen captures
      if (GetBit(pos.state_.piece_occupancy[opponent_side], target_square)) {
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, true,
                                            false, false, false));
      } else {
        // Generate quiet queen moves
        moveList.push_back(move::CreateMove(source_square, target_square,
                                            current_piece, kNoPiece, false,
                                            false, false, false));
      }
    }
  }
}

void GenerateKingMoves(const Position& pos, move::MoveList& moveList) {
  Square source_square, target_square;
  Color side_to_move = pos.state_.side_to_move;
  Color opponent_side = ~side_to_move;
  Piece current_piece = GetPiece(kKing, side_to_move);

  Bitboard current_bitboard, current_attacks;

  current_bitboard = pos.state_.piece_bitboards[current_piece];
  source_square = static_cast<Square>(GetLSBIndex(current_bitboard));
  current_attacks = precomputed_data::king_attacks[source_square] &
                    ~pos.state_.piece_occupancy[side_to_move];

  while (current_attacks) {
    target_square = static_cast<Square>(GetLSBIndex(current_attacks));
    ClearLSB(current_attacks);

    // Generate king captures
    if (GetBit(pos.state_.piece_occupancy[opponent_side], target_square)) {
      moveList.push_back(move::CreateMove(source_square, target_square,
                                          current_piece, kNoPiece, true, false,
                                          false, false));
    } else {
      // Generate quiet king moves
      moveList.push_back(move::CreateMove(source_square, target_square,
                                          current_piece, kNoPiece, false, false,
                                          false, false));
    }
  }

  // Generate castling moves
  // Do not need to check if target square is attacked because that will be
  // caught after the move is made.
  if (side_to_move == kWhite) {
    if (pos.state_.castling_rights & kWhiteKingSide) {
      if (!GetBit(pos.state_.piece_occupancy[kBothColors], kF1) &&
          !GetBit(pos.state_.piece_occupancy[kBothColors], kG1) &&
          !IsSquareAttacked(pos, kE1, opponent_side) &&
          !IsSquareAttacked(pos, kF1, opponent_side)) {
        moveList.push_back(move::CreateMove(kE1, kG1, current_piece, kNoPiece,
                                            false, false, false, true));
      }
    }
    if (pos.state_.castling_rights & kWhiteQueenSide) {
      if (!GetBit(pos.state_.piece_occupancy[kBothColors], kD1) &&
          !GetBit(pos.state_.piece_occupancy[kBothColors], kC1) &&
          !GetBit(pos.state_.piece_occupancy[kBothColors], kB1) &&
          !IsSquareAttacked(pos, kE1, opponent_side) &&
          !IsSquareAttacked(pos, kD1, opponent_side)) {
        moveList.push_back(move::CreateMove(kE1, kC1, current_piece, kNoPiece,
                                            false, false, false, true));
      }
    }
  } else {
    if (pos.state_.castling_rights & kBlackKingSide) {
      if (!GetBit(pos.state_.piece_occupancy[kBothColors], kF8) &&
          !GetBit(pos.state_.piece_occupancy[kBothColors], kG8) &&
          !IsSquareAttacked(pos, kE8, opponent_side) &&
          !IsSquareAttacked(pos, kF8, opponent_side)) {
        moveList.push_back(move::CreateMove(kE8, kG8, current_piece, kNoPiece,
                                            false, false, false, true));
      }
    }
    if (pos.state_.castling_rights & kBlackQueenSide) {
      if (!GetBit(pos.state_.piece_occupancy[kBothColors], kD8) &&
          !GetBit(pos.state_.piece_occupancy[kBothColors], kC8) &&
          !GetBit(pos.state_.piece_occupancy[kBothColors], kB8) &&
          !IsSquareAttacked(pos, kE8, opponent_side) &&
          !IsSquareAttacked(pos, kD8, opponent_side)) {
        moveList.push_back(move::CreateMove(kE8, kC8, current_piece, kNoPiece,
                                            false, false, false, true));
      }
    }
  }
}

bool IsSquareAttacked(const Position& pos, Square square, Color side) {
  Color attacking_side = side;
  Color defending_side = ~attacking_side;

  if (precomputed_data::pawn_attacks[square][defending_side] &
      pos.state_.piece_bitboards[GetPiece(kPawn, attacking_side)]) {
    return true;
  }
  if (precomputed_data::knight_attacks[square] &
      pos.state_.piece_bitboards[GetPiece(kKnight, attacking_side)]) {
    return true;
  }
  if (precomputed_data::GetBishopAttacks(
          square, pos.state_.piece_occupancy[kBothColors]) &
      pos.state_.piece_bitboards[GetPiece(kBishop, attacking_side)]) {
    return true;
  }
  if (precomputed_data::GetRookAttacks(
          square, pos.state_.piece_occupancy[kBothColors]) &
      pos.state_.piece_bitboards[GetPiece(kRook, attacking_side)]) {
    return true;
  }
  if (precomputed_data::GetQueenAttacks(
          square, pos.state_.piece_occupancy[kBothColors]) &
      pos.state_.piece_bitboards[GetPiece(kQueen, attacking_side)]) {
    return true;
  }
  if (precomputed_data::king_attacks[square] &
      pos.state_.piece_bitboards[GetPiece(kKing, attacking_side)]) {
    return true;
  }
  return false;
}

bool IsInCheck(const Position& pos) {
  Color side_to_move = pos.state_.side_to_move;
  Piece king = GetPiece(kKing, side_to_move);
  Square king_square =
      static_cast<Square>(GetLSBIndex(pos.state_.piece_bitboards[king]));
  return IsSquareAttacked(pos, king_square, ~side_to_move);
}

}  // namespace chess