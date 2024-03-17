#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <cstdint>
#include <stdexcept>

namespace chess {

// User Additions

// User Configurable Parameters
/******************************************************************************/
inline uint32_t randomState = 1804289383;

// If there are <= this many non pawn or king pieces, then we are in the endgame
inline constexpr int kEndgamePieceCount = 6;

// The maximum search depth in plies.
inline constexpr int kMaxSearchDepth = 128;

// The amount of time that the engine tries to keep on the clock. This is
// helpful to prevent the engine from flagging. As the remaining time approaches
// this value, the engine will start moving more quickly. If the remaining time
// is less than this value, the engine will attempt to move instantly.
inline constexpr int kTimeBuffer = 1000;

// If the moves to go is not specified then the engine will assume that it will
// need to make this many more moves in the time control
inline constexpr int kDefaultMovesToGo = 60;

// The number of killer moves to store.
inline constexpr int kNumKillerMoves = 2;

// The size of the repetition table in entries. We only need to check the last
// 100 plies when checking for a draw but this table is larger because it needs
// to store extra values during the search.
inline constexpr int kRepetitionTableSize = 100 + kMaxSearchDepth;

// The size of the transposition table in megabytes
inline constexpr int kDefaultTranspositionTableSize = 128;

// The search will checkup every kCheckupFrequency nodes to see if it should
// stop.
inline constexpr int kCheckupFrequency = 2048;

// How much to reduce the search depth by when doing a null move.
inline constexpr int kNullMoveReductionAmount = 2;

// The minimum number of full depth searches to complete at each ply.
inline constexpr int kMinimumFullDepthSearches = 2;

// The number of moves to search at full depth before using LMR.
inline constexpr int kLmrFullDepthMoves = 1;

// Will not perform LMR if the depth is less than this value.
inline constexpr int kLmrReductionLimit = 3;

// The amount to reduce the search depth by when using LMR.
inline constexpr int kLmrReductionAmount = 1;

// The aspiration window size.
inline constexpr int kAspirationWindow = 50;

// A value that represents an unknown score.
inline constexpr int kUnknownScore = 100000;

// A value that represents infinity.
inline constexpr int kInfinity = 50000;

// A value that represents a checkmate score.
inline constexpr int kCheckmateScore = -49000;

// The bound of the window that contains a checkmate score.
inline constexpr int kCheckmateWindow = -48000;

// A value that represents a draw score.
inline constexpr int kDrawScore = 0;

// Types
/******************************************************************************/
using Bitboard = uint64_t;
using Key = uint64_t;
using Time = uint64_t;

// Enums
/******************************************************************************/
enum Color : int {
  kWhite = 0,
  kBlack = 1,
  kNumColors = 2,
  kBothColors = 2,
  kOccupancies = 3,
};

enum Piece {
  kWhitePawn = 0,
  kWhiteKnight = 1,
  kWhiteBishop = 2,
  kWhiteRook = 3,
  kWhiteQueen = 4,
  kWhiteKing = 5,
  kBlackPawn = 6,
  kBlackKnight = 7,
  kBlackBishop = 8,
  kBlackRook = 9,
  kBlackQueen = 10,
  kBlackKing = 11,
  kPieceStart = kWhitePawn,
  kPieceEnd = kBlackKing,
  kPieceCount = kBlackKing - kWhitePawn + 1,
  kNoPiece = 13,
};

enum PieceType : int {
  kPawn = 0,
  kKnight = 1,
  kBishop = 2,
  kRook = 3,
  kQueen = 4,
  kKing = 5,
  kPieceTypeStart = kPawn,
  kPieceTypeEnd = kKing,
  kPieceTypeCount = kKing - kPawn + 1,
  kNoPieceType = kNoPiece,
};

enum CastlingRights : int {
  kNoCastling = 0,

  kWhiteKingSide = 1 << 0,
  kWhiteQueenSide = 1 << 1,
  kWhiteCastlingRights = kWhiteKingSide | kWhiteQueenSide,

  kBlackKingSide = 1 << 2,
  kBlackQueenSide = 1 << 3,
  kBlackCastlingRights = kBlackKingSide | kBlackQueenSide,

  kCastlingRightsStart = 0,
  kCastlingRightsNB = 16,
};

enum Direction : int {
  kNorth = -8,
  kSouth = -kNorth,
  kEast = 1,
  kWest = -kEast,
  kNorthEast = kNorth + kEast,
  kSouthEast = kSouth + kEast,
  kSouthWest = kSouth + kWest,
  kNorthWest = kNorth + kWest,
};

// clang-format off
enum Square : int {
  kA8, kB8, kC8, kD8, kE8, kF8, kG8, kH8,
  kA7, kB7, kC7, kD7, kE7, kF7, kG7, kH7,
  kA6, kB6, kC6, kD6, kE6, kF6, kG6, kH6,
  kA5, kB5, kC5, kD5, kE5, kF5, kG5, kH5,
  kA4, kB4, kC4, kD4, kE4, kF4, kG4, kH4,
  kA3, kB3, kC3, kD3, kE3, kF3, kG3, kH3,
  kA2, kB2, kC2, kD2, kE2, kF2, kG2, kH2,
  kA1, kB1, kC1, kD1, kE1, kF1, kG1, kH1,
  kNoSquare = 64,
  kSquareStart = kA8,
  kSquareEnd = kH1,
  kNumSquares = kSquareEnd - kSquareStart + 1,
};
// clang-format on

enum File : int {
  kAFile = 0,
  kBFile = 1,
  kCFile = 2,
  kDFile = 3,
  kEFile = 4,
  kFFile = 5,
  kGFile = 6,
  kHFile = 7,
  kFileStart = kAFile,
  kFileEnd = kHFile,
  kNumFiles = kFileEnd - kFileStart + 1,
};

enum Rank : int {
  kRank1 = 0,
  kRank2 = 1,
  kRank3 = 2,
  kRank4 = 3,
  kRank5 = 4,
  kRank6 = 5,
  kRank7 = 6,
  kRank8 = 7,
  kRankStart = kRank1,
  kRankEnd = kRank8,
  kNumRanks = kRankEnd - kRankStart + 1,
};

// Constants
/******************************************************************************/

const std::string kStartingPositionFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const std::string kEmptyPositionFen = "8/8/8/8/8/8/8/8 w - - 0 1";

const std::string kTrickyPositionFen =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

constexpr Bitboard kEmptyBitboard = 0ULL;

constexpr Bitboard kAFileMask = 0x0101010101010101ULL;
constexpr Bitboard kBFileMask = 0x0202020202020202ULL;
constexpr Bitboard kCFileMask = 0x0404040404040404ULL;
constexpr Bitboard kDFileMask = 0x0808080808080808ULL;
constexpr Bitboard kEFileMask = 0x1010101010101010ULL;
constexpr Bitboard kFFileMask = 0x2020202020202020ULL;
constexpr Bitboard kGFileMask = 0x4040404040404040ULL;
constexpr Bitboard kHFileMask = 0x8080808080808080ULL;

constexpr Bitboard kFileMasks[] = {
    kAFileMask, kBFileMask, kCFileMask, kDFileMask,
    kEFileMask, kFFileMask, kGFileMask, kHFileMask,
};

constexpr Bitboard kRank1Mask = 0xFF00000000000000ULL;
constexpr Bitboard kRank2Mask = 0x00FF000000000000ULL;
constexpr Bitboard kRank3Mask = 0x0000FF0000000000ULL;
constexpr Bitboard kRank4Mask = 0x000000FF00000000ULL;
constexpr Bitboard kRank5Mask = 0x00000000FF000000ULL;
constexpr Bitboard kRank6Mask = 0x0000000000FF0000ULL;
constexpr Bitboard kRank7Mask = 0x000000000000FF00ULL;
constexpr Bitboard kRank8Mask = 0x00000000000000FFULL;

constexpr Bitboard kRankMasks[] = {
    kRank1Mask, kRank2Mask, kRank3Mask, kRank4Mask,
    kRank5Mask, kRank6Mask, kRank7Mask, kRank8Mask,
};

// clang-format off
constexpr const char* kSquareStrings[] = {
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a2", "b2", "c2", "d2", "e2", "f2",  "g2", "h2",
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", 
  "None"};
// clang-format on

// clang-format off
constexpr int kCastlingUpdates[] = {
  7, 15, 15, 15,  3, 15, 15, 11,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14,
};
// clang-format on

// Util Functions
/******************************************************************************/

// Toggle the color
// @param c The color
// @return The toggled color
constexpr Color operator~(Color c) {
  return static_cast<Color>(int(c) ^ kBlack);
}

// Increment the color
// @param c The color
// @return The incremented color
constexpr Color operator++(Color& c, int) {
  return c = static_cast<Color>(int(c) + 1);
}

// Defines the base operators for a type
#define ENABLE_BASE_OPERATORS_ON(T)                               \
  constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); } \
  constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); } \
  constexpr T operator-(T d) { return T(-int(d)); }               \
  inline T& operator+=(T& d1, int d2) { return d1 = d1 + d2; }    \
  inline T& operator-=(T& d1, int d2) { return d1 = d1 - d2; }

// Defines the increment operators for a type
#define ENABLE_INCR_OPERATORS_ON(T)                             \
  inline T& operator++(T& d) { return d = T(int(d) + 1); }      \
  inline T& operator--(T& d) { return d = T(int(d) - 1); }      \
  inline T& operator++(T& d, int) { return d = T(int(d) + 1); } \
  inline T& operator--(T& d, int) { return d = T(int(d) - 1); }

#define ENABLE_BIT_OPERATORS_ON(T)                                        \
  inline T& operator&=(T& d1, T d2) { return d1 = T(int(d1) & int(d2)); } \
  inline T& operator|=(T& d1, T d2) { return d1 = T(int(d1) | int(d2)); } \
  inline T& operator^=(T& d1, T d2) { return d1 = T(int(d1) ^ int(d2)); } \
  constexpr T operator&(T d1, T d2) { return T(int(d1) & int(d2)); }      \
  constexpr T operator|(T d1, T d2) { return T(int(d1) | int(d2)); }      \
  constexpr T operator^(T d1, T d2) { return T(int(d1) ^ int(d2)); }      \
  constexpr T operator~(T d) { return T(~int(d)); }

// Defines the full operators for a type
#define ENABLE_FULL_OPERATORS_ON(T)                                 \
  ENABLE_BASE_OPERATORS_ON(T)                                       \
  constexpr T operator*(int i, T d) { return T(i * int(d)); }       \
  constexpr T operator*(T d, int i) { return T(int(d) * i); }       \
  constexpr T operator/(T d, int i) { return T(int(d) / i); }       \
  constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); } \
  inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }   \
  inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

// Defines the all operators for a type
#define ENABLE_ALL_OPERATORS_ON(T) \
  ENABLE_FULL_OPERATORS_ON(T)      \
  ENABLE_INCR_OPERATORS_ON(T)      \
  ENABLE_BIT_OPERATORS_ON(T)

ENABLE_ALL_OPERATORS_ON(CastlingRights);
ENABLE_ALL_OPERATORS_ON(Piece);
ENABLE_ALL_OPERATORS_ON(Square);
ENABLE_ALL_OPERATORS_ON(File);
ENABLE_ALL_OPERATORS_ON(Rank);

#undef ENABLE_BASE_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BIT_OPERATORS_ON
#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_ALL_OPERATORS_ON

// Sets the nth bit of a number
// @param n The number
// @param i The index of the bit
template <typename NumericType>
inline void SetBit(NumericType& n, int i) {
  n |= (1ULL << i);
}

// Clears the nth bit of a number
// @param n The number
// @param i The indes of the bit
template <typename NumericType>
inline void ClearBit(NumericType& n, int i) {
  n &= ~(1ULL << i);
}

// Clears the least significant bit of a number
// @param n the number
template <typename NumericType>
inline void ClearLSB(NumericType& n) {
  n &= (n - 1);
}

// Gets the nth bit of number
// @param n The number
// @param i The index of the bit
// @return The nth bit of the number
template <typename NumericType>
constexpr inline bool GetBit(NumericType n, int i) {
  return n & (1ULL << i);
}

// Gets the nth bit of an

// Count the number of bits set in a number
// @param n The number
// @return The number of bits set
template <typename NumericType>
constexpr inline int CountBits(NumericType n) {
  int count = 0;
  while (n) {
    n &= (n - 1);
    count++;
  }
  return count;
}

// Get the index of the least significant bit in a number
// @param n The number
// @return The index of the least significant bit
template <typename NumericType>
constexpr inline int GetLSBIndex(NumericType n) {
  return CountBits((n & -n) - 1);
}

// Returns the piece with the given color
// @param pt The piece type
// @param c The color
// @return The piece
constexpr inline Piece GetPiece(PieceType pt, Color c) {
  return Piece(pt + (c * kPieceTypeCount));
}

// Returns the piece type of the given piece
// @param p The piece
// @return The piece type
constexpr inline PieceType GetPieceType(Piece p) {
  return PieceType(p % kPieceTypeCount);
}

// Returns the color of the given piece
// @param p The piece
// @return The color
constexpr inline Color GetPieceColor(Piece p) {
  return Color(p / kPieceTypeCount);
}

// Checks if a square is valid
// @param s The square
// @return True if the square is valid
constexpr inline bool IsValidSquare(Square s) {
  return s >= kSquareStart && s <= kSquareEnd;
}

// Gets the file of a square
// @param s The square
// @return The file of the square
constexpr inline File GetFile(Square s) { return File(s & 7); }

// Gets the rank of a square
// @param s The square
// @return The rank of the square
constexpr inline Rank GetRank(Square s) { return Rank(7 - (s >> 3)); }

// Gets the square from a file and rank
// @param file The file of the square
// @param rank The rank of the square
// @return The square
constexpr inline Square GetSquare(File file, Rank rank) {
  return Square(((7 - rank) << 3) | file);
}

// Get a psuedo-random 32-bit number
// @return A psuedo-random 32-bit number
inline uint32_t GetRandomNumber32() {
  randomState ^= randomState << 13;
  randomState ^= randomState >> 17;
  randomState ^= randomState << 5;
  return randomState;
}

// Get a psuedo-random 64-bit number
// @return A psuedo-random 64-bit number
inline uint64_t GetRandomNumber64() {
  uint64_t n1, n2, n3, n4;
  n1 = GetRandomNumber32() & 0xFFFF;
  n2 = GetRandomNumber32() & 0xFFFF;
  n3 = GetRandomNumber32() & 0xFFFF;
  n4 = GetRandomNumber32() & 0xFFFF;
  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// Gets the current time in milliseconds
inline uint64_t GetTime() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

// Gets the piece from a character
// @throws std::invalid_argument if the character is invalid
// @param c The character
// @return The piece
constexpr Piece CharToPiece(char c) {
  switch (c) {
    case 'P':
      return kWhitePawn;
    case 'N':
      return kWhiteKnight;
    case 'B':
      return kWhiteBishop;
    case 'R':
      return kWhiteRook;
    case 'Q':
      return kWhiteQueen;
    case 'K':
      return kWhiteKing;
    case 'p':
      return kBlackPawn;
    case 'n':
      return kBlackKnight;
    case 'b':
      return kBlackBishop;
    case 'r':
      return kBlackRook;
    case 'q':
      return kBlackQueen;
    case 'k':
      return kBlackKing;
    default:
      throw std::invalid_argument("Invalid piece character");
  }
}

// Gets the character from a piece
// @param p The piece
// @return The character
constexpr char PieceToChar(Piece p) {
  switch (p) {
    case kWhitePawn:
      return 'P';
    case kWhiteKnight:
      return 'N';
    case kWhiteBishop:
      return 'B';
    case kWhiteRook:
      return 'R';
    case kWhiteQueen:
      return 'Q';
    case kWhiteKing:
      return 'K';
    case kBlackPawn:
      return 'p';
    case kBlackKnight:
      return 'n';
    case kBlackBishop:
      return 'b';
    case kBlackRook:
      return 'r';
    case kBlackQueen:
      return 'q';
    case kBlackKing:
      return 'k';
    case kNoPiece:
      return ' ';
    default:
      return ' ';
  }
}

// Returns a string representation of a bitboard
// @param b The bitboard
// @return The string representation
inline std::string BitboardToString(Bitboard b) {
  std::string s = "";
  for (Rank r = kRank8; r >= kRank1; r--) {
    for (File f = kAFile; f <= kHFile; f++) {
      Square sq = GetSquare(f, r);
      if (GetBit(b, sq)) {
        s += "1 ";
      } else {
        s += "0 ";
      }
    }
    s += "\n";
  }
  return s;
}

// Gets the character from a file
// @param f The file
// @return The character
constexpr char FileToChar(File f) { return char(int(f) + 'a'); }

// Gets the character from a rank
// @param r The rank
// @return The character
constexpr char RankToChar(Rank r) { return char(int(r) + '1'); }

}  // namespace chess

#endif  // UTILS_HPP