#include "position.hpp"

#include <cstring>
#include <iostream>

#include "utils.hpp"

namespace chess {

namespace zobrist {
Key piece_keys[kPieceCount][kNumSquares];
Key en_passant_keys[kNumSquares];
Key castling_keys[16];
Key side_key;

void Init() {
  for (Piece p = kWhitePawn; p <= kBlackKing; p++) {
    for (Square s = kSquareStart; s < kNumSquares; s++) {
      piece_keys[p][s] = GetRandomNumber64();
    }
  }
  for (Square s = kSquareStart; s < kNumSquares; s++) {
    en_passant_keys[s] = GetRandomNumber64();
  }
  for (CastlingRights c = kCastlingRightsStart; c < 16; c++) {
    castling_keys[c] = GetRandomNumber64();
  }
  side_key = GetRandomNumber64();
}
}  // namespace zobrist

std::ostream& operator<<(std::ostream& os, const Position& pos) {
  os << "\n  --- --- --- --- --- --- --- --- \n";

  for (Rank r = kRank8; r >= kRank1; r--) {
    for (File f = kAFile; f <= kHFile; f++) {
      os << " | " << PieceToChar(pos.PieceOn(GetSquare(f, r)));
    }
    os << " | " << RankToChar(r) << "\n  --- --- --- --- --- --- --- --- \n";
  }

  os << "   a   b   c   d   e   f   g   h\n\n";
  os << "Fen: " << pos.ToFen();

  return os;
}

Position::Position(const PositionState& state) { state_ = state; }

void Position::Reset() {
  state_ = PositionState();
  Set(kStartingPositionFen);
}

Piece Position::PieceOn(Square square) const {
  for (Piece p = kWhitePawn; p <= kBlackKing; p++) {
    if (GetBit(state_.piece_bitboards[p], square)) return p;
  }
  return kNoPiece;
}

Key Position::GenerateKey() const {
  Key key = 0;
  for (Piece p = kWhitePawn; p <= kBlackKing; ++p) {
    Bitboard b = state_.piece_bitboards[p];
    while (b) {
      Square square = static_cast<Square>(GetLSBIndex(b));
      key ^= zobrist::piece_keys[p][square];
      ClearLSB(b);
    }
  }
  if (state_.en_passant_square != kNoSquare) {
    key ^= zobrist::en_passant_keys[state_.en_passant_square];
  }
  if (state_.side_to_move == kBlack) key ^= zobrist::side_key;
  key ^= zobrist::castling_keys[state_.castling_rights];
  return key;
}

void Position::Set(const std::string& fen) {
  int i = 0;
  Square sq = kA8;

  try {
    // Clear the repetition table.
    repetition_table_.Clear();

    // Parse the piece placement.
    memset(&state_.piece_bitboards, 0, sizeof(state_.piece_bitboards));
    memset(&state_.piece_occupancy, 0, sizeof(state_.piece_occupancy));
    while (fen[i] != ' ') {
      if (fen[i] == '/') {
        i++;
        continue;
      }
      if (fen[i] >= '1' && fen[i] <= '8') {
        sq += fen[i] - '0';
        i++;
        continue;
      }
      SetBit(state_.piece_bitboards[CharToPiece(fen[i])], sq);
      sq++;
      i++;
    }
    GenerateOccupancies();
    i++;

    // Parse the color
    if (fen[i] == 'w') {
      state_.side_to_move = kWhite;
    } else {
      state_.side_to_move = kBlack;
    }
    i += 2;

    // Parse the castling rights.
    state_.castling_rights = kNoCastling;
    while (fen[i] != ' ') {
      switch (fen[i]) {
        case 'K':
          state_.castling_rights |= kWhiteKingSide;
          break;
        case 'Q':
          state_.castling_rights |= kWhiteQueenSide;
          break;
        case 'k':
          state_.castling_rights |= kBlackKingSide;
          break;
        case 'q':
          state_.castling_rights |= kBlackQueenSide;
          break;
        case '-':
          break;
        default:
          throw std::invalid_argument("Invalid castling rights.");
      }
      i++;
    }
    i++;

    // Parse the en passant square.
    if (fen[i] == '-') {
      state_.en_passant_square = kNoSquare;
    } else {
      File file = File(fen[i] - 'a');
      Rank rank = Rank(fen[i + 1] - '1');
      state_.en_passant_square = GetSquare(file, rank);
      i++;
    }
    i += 2;

    // Parse the halfmove clock and fullmove number.
    std::string halfmove_clock_str = "";
    std::string fullmove_number_str = "";
    while (fen[i] != ' ') {
      halfmove_clock_str += fen[i];
      i++;
    }
    i++;
    while (fen[i] != ' ' && i < static_cast<int>(fen.length())) {
      fullmove_number_str += fen[i];
      i++;
    }
    state_.halfmove_clock = std::stoi(halfmove_clock_str);
    state_.ply = (2 * (std::stoi(fullmove_number_str) - 1)) +
                 (state_.side_to_move == kBlack);

    // Generate the Zobrist key.
    state_.key = GenerateKey();

  } catch (const std::exception& e) {
    throw std::invalid_argument("Invalid FEN string.");
  }
}

bool Position::MakeMove(move::Move move, bool quiescencse) {
  // If we are in quiescence search, only make captures.
  if (quiescencse) {
    if (!move::IsCapture(move)) {
      return false;
    }
  }

  PositionState state = GetState();
  Square source = move::GetSourceSquare(move);
  Square target = move::GetTargetSquare(move);
  Piece piece = move::GetPiece(move);
  Piece promoted_piece = move::GetPromotedPiece(move);
  bool capture = move::IsCapture(move);
  bool double_push = move::IsDoublePush(move);
  bool en_passant = move::IsEnPassant(move);
  bool castle = move::IsCastle(move);

  // Update the halfmove clock.
  if (capture || GetPieceType(piece) == kPawn) {
    state_.halfmove_clock = 0;
  } else {
    state_.halfmove_clock++;
  }

  // Update the ply.
  state_.ply++;

  // If the move is a capture, get the captured piece.
  Piece captured_piece = PieceOn(target);

  // Move the piece.
  ClearBit(state_.piece_bitboards[piece], source);
  SetBit(state_.piece_bitboards[piece], target);

  // Update the hash key.
  state_.key ^= zobrist::piece_keys[piece][source];
  state_.key ^= zobrist::piece_keys[piece][target];

  // Handle captures.
  if (capture) {
    ClearBit(state_.piece_bitboards[captured_piece], target);
    state_.key ^= zobrist::piece_keys[captured_piece][target];
  }

  // Handle promotions.
  if (promoted_piece != kNoPiece) {
    ClearBit(state_.piece_bitboards[piece], target);
    SetBit(state_.piece_bitboards[promoted_piece], target);
    state_.key ^= zobrist::piece_keys[piece][target];
    state_.key ^= zobrist::piece_keys[promoted_piece][target];
  }

  // Handle en passant
  if (en_passant) {
    Square en_passant_target = target;
    if (GetPieceColor(piece) == kWhite) {
      en_passant_target += kSouth;
    } else {
      en_passant_target += kNorth;
    }
    Piece captured_piece = GetPiece(kPawn, ~GetPieceColor(piece));
    ClearBit(state_.piece_bitboards[captured_piece], en_passant_target);
    state_.key ^= zobrist::piece_keys[captured_piece][en_passant_target];
  }
  // hash en passant square
  if (state_.en_passant_square != kNoSquare) {
    state_.key ^= zobrist::en_passant_keys[state_.en_passant_square];
  }

  state_.en_passant_square = kNoSquare;

  // Handle double pawn pushes.
  if (double_push) {
    if (GetPieceColor(piece) == kWhite) {
      state_.en_passant_square = target + kSouth;
    } else {
      state_.en_passant_square = target + kNorth;
    }
    state_.key ^= zobrist::en_passant_keys[state_.en_passant_square];
  }

  // Handle castling.
  if (castle) {
    Square rook_source = kNoSquare;
    Square rook_target = kNoSquare;
    if (target == kG1) {
      rook_source = kH1;
      rook_target = kF1;
    } else if (target == kC1) {
      rook_source = kA1;
      rook_target = kD1;
    } else if (target == kG8) {
      rook_source = kH8;
      rook_target = kF8;
    } else if (target == kC8) {
      rook_source = kA8;
      rook_target = kD8;
    }
    ClearBit(state_.piece_bitboards[GetPiece(kRook, GetPieceColor(piece))],
             rook_source);
    SetBit(state_.piece_bitboards[GetPiece(kRook, GetPieceColor(piece))],
           rook_target);
    state_.key ^=
        zobrist::piece_keys[GetPiece(kRook, GetPieceColor(piece))][rook_source];
    state_.key ^=
        zobrist::piece_keys[GetPiece(kRook, GetPieceColor(piece))][rook_target];
  }

  // Hash castling rights.
  state_.key ^= zobrist::castling_keys[state_.castling_rights];

  // Update castling rights.
  state_.castling_rights &=
      static_cast<CastlingRights>(kCastlingUpdates[source]);
  state_.castling_rights &=
      static_cast<CastlingRights>(kCastlingUpdates[target]);

  // Hash castling rights.
  state_.key ^= zobrist::castling_keys[state_.castling_rights];

  // Update occupancies
  GenerateOccupancies();

  // Update the side to move.
  state_.side_to_move = ~state_.side_to_move;

  // Hash side to move.
  state_.key ^= zobrist::side_key;

  // Check if the king is in check.
  Piece king = GetPiece(kKing, GetPieceColor(piece));
  Square king_square =
      static_cast<Square>(GetLSBIndex(state_.piece_bitboards[king]));
  if (IsSquareAttacked(*this, king_square, ~GetPieceColor(piece))) {
    state_ = state;
    return false;
  }

  return true;
}

int Position::GetNumNonPawnKingPieces(Color side) const {
  Bitboard bitboard = state_.piece_occupancy[side];
  bitboard &= ~state_.piece_bitboards[GetPiece(kPawn, side)];
  bitboard &= ~state_.piece_bitboards[GetPiece(kKing, side)];

  return CountBits(bitboard);
}

std::string Position::ToFen() const {
  std::string fen = "";

  // Piece placement
  for (Rank r = kRank8; r >= kRank1; r--) {
    int empty_squares = kEmptyBitboard;
    for (File f = kAFile; f <= kHFile; f++) {
      Square sq = GetSquare(f, r);
      Piece p = PieceOn(sq);
      if (p == kNoPiece) {
        empty_squares++;
      } else {
        if (empty_squares > 0) {
          fen += std::to_string(empty_squares);
          empty_squares = 0;
        }
        fen += PieceToChar(p);
      }
    }
    if (empty_squares > 0) {
      fen += std::to_string(empty_squares);
    }
    if (r > kRank1) {
      fen += "/";
    }
  }
  fen += " ";

  // Side to move
  if (state_.side_to_move == kWhite) {
    fen += "w";
  } else {
    fen += "b";
  }
  fen += " ";

  // Castling rights
  if (state_.castling_rights == kNoCastling) {
    fen += "-";
  } else {
    if (state_.castling_rights & kWhiteKingSide) {
      fen += "K";
    }
    if (state_.castling_rights & kWhiteQueenSide) {
      fen += "Q";
    }
    if (state_.castling_rights & kBlackKingSide) {
      fen += "k";
    }
    if (state_.castling_rights & kBlackQueenSide) {
      fen += "q";
    }
  }
  fen += " ";

  // En passant square
  if (state_.en_passant_square == kNoSquare) {
    fen += "-";
  } else {
    fen += FileToChar(GetFile(state_.en_passant_square));
    fen += RankToChar(GetRank(state_.en_passant_square));
  }
  fen += " ";

  // Halfmove clock
  fen += std::to_string(state_.halfmove_clock);
  fen += " ";

  // Fullmove number
  fen += std::to_string((state_.ply / 2) + 1);

  return fen;
}

}  // namespace chess