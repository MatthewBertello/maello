#include <cstring>

#include "precomputed_data.hpp"
namespace chess {

namespace precomputed_data {

Bitboard pawn_attacks[kNumSquares][kNumColors];
Bitboard knight_attacks[kNumSquares];
Bitboard king_attacks[kNumSquares];
Bitboard bishop_masks[kNumSquares];
Bitboard bishop_attacks[kNumSquares][512];

Bitboard rook_masks[kNumSquares];
Bitboard rook_attacks[kNumSquares][4096];

Bitboard isolated_pawn_masks[kNumSquares];
Bitboard passed_pawn_masks[kNumSquares][kNumColors];

void Init() {
  // The relevant bits are already precomputed, this only needs to be called if
  // we want to regenerate them.
  // InitRelevantBits();

  // The magic numbers are already precomputed, this only needs to be called if
  // we want to regenerate them.
  InitMagicNumbers();

  InitSlidingAttacks();
  InitLeapingAttacks();
  InitEvaluationMasks();
}

void InitMagicNumbers() {
  for (Square sq = kSquareStart; sq < kNumSquares; sq++) {
    rook_magic_numbers[sq] =
        FindMagicNumber(sq, CountBits(GenerateRookAttackMask(sq)), false);
  }
  for (Square sq = kSquareStart; sq < kNumSquares; sq++) {
    bishop_magic_numbers[sq] =
        FindMagicNumber(sq, CountBits(GenerateBishopAttackMask(sq)), true);
  }
}

void InitRelevantBits() {
  for (Square sq = kSquareStart; sq < kNumSquares; sq++) {
    bishop_relevant_bits[sq] = CountBits(GenerateBishopAttackMask(sq));
    rook_relevant_bits[sq] = CountBits(GenerateRookAttackMask(sq));
  }
}

void InitSlidingAttacks() {
  for (Square sq = kSquareStart; sq < kNumSquares; sq++) {
    bishop_masks[sq] = GenerateBishopAttackMask(sq);
    int relevant_bits = CountBits(bishop_masks[sq]);
    int num_occupancy_keys = 1 << relevant_bits;
    for (int key_index = 0; key_index < num_occupancy_keys; key_index++) {
      Bitboard occupancy = GenerateOccupancyKey(key_index, bishop_masks[sq]);
      Bitboard attacks = GenerateBishopAttacks(sq, occupancy);
      int magic_index =
          (occupancy * bishop_magic_numbers[sq]) >> (64 - relevant_bits);
      bishop_attacks[sq][magic_index] = attacks;
    }
    rook_masks[sq] = GenerateRookAttackMask(sq);
    relevant_bits = CountBits(rook_masks[sq]);
    num_occupancy_keys = 1 << relevant_bits;
    for (int key_index = 0; key_index < num_occupancy_keys; key_index++) {
      Bitboard occupancy = GenerateOccupancyKey(key_index, rook_masks[sq]);
      Bitboard attacks = GenerateRookAttacks(sq, occupancy);
      int magic_index =
          (occupancy * rook_magic_numbers[sq]) >> (64 - relevant_bits);
      rook_attacks[sq][magic_index] = attacks;
    }
  }
}

void InitLeapingAttacks() {
  for (Square sq = kSquareStart; sq < kNumSquares; sq++) {
    pawn_attacks[sq][kWhite] = GeneratePawnAttackMask(sq, kWhite);
    pawn_attacks[sq][kBlack] = GeneratePawnAttackMask(sq, kBlack);
    knight_attacks[sq] = GenerateKnightAttackMask(sq);
    king_attacks[sq] = GenerateKingAttackMask(sq);
  }
}

void InitEvaluationMasks() {
  // Init isolated pawn masks.
  for (File f = kAFile; f <= kHFile; f++) {
    for (Rank r = kRank1; r <= kRank8; r++) {
      Bitboard mask = kEmptyBitboard;
      if (f != kAFile) mask |= kFileMasks[f - 1];
      if (f != kHFile) mask |= kFileMasks[f + 1];
      isolated_pawn_masks[GetSquare(f, r)] = mask;
    }
  }

  // Init passed pawn masks.
  for (File f = kAFile; f <= kHFile; f++) {
    for (Rank r = kRank1; r <= kRank8; r++) {
      for (Color c = kWhite; c <= kBlack; c++) {
        Bitboard mask = kEmptyBitboard;
        if (f != kAFile) mask |= kFileMasks[f - 1];
        mask |= kFileMasks[f];
        if (f != kHFile) mask |= kFileMasks[f + 1];
        if (c == kWhite) {
          for (Rank passed_rank = kRank1; passed_rank <= r; passed_rank++) {
            mask &= ~kRankMasks[passed_rank];
          }
        } else {
          for (Rank passed_rank = kRank8; passed_rank >= r; passed_rank--) {
            mask &= ~kRankMasks[passed_rank];
          }
        }
        passed_pawn_masks[GetSquare(f, r)][c] = mask;
      }
    }
  }
}

Key FindMagicNumber(Square sq, int relevant_bits, bool is_bishop) {
  Bitboard occupancies[4096];
  Bitboard attacks[4096];
  Bitboard used_attacks[4096];
  Bitboard mask =
      is_bishop ? GenerateBishopAttackMask(sq) : GenerateRookAttackMask(sq);
  int num_occupancy_keys = 1 << relevant_bits;

  for (int index = 0; index < num_occupancy_keys; index++) {
    occupancies[index] = GenerateOccupancyKey(index, mask);
    attacks[index] = is_bishop ? GenerateBishopAttacks(sq, occupancies[index])
                               : GenerateRookAttacks(sq, occupancies[index]);
  }

  for (int i = 0; i < 100000000; i++) {
    Key magic_number = GenerateMagicNumberCandidate();
    if (CountBits((mask * magic_number) & 0xFF00000000000000ULL) < 6) continue;
    memset(used_attacks, 0ULL, sizeof(used_attacks));
    bool valid = true;
    for (int index = 0; index < num_occupancy_keys && valid; index++) {
      int magic_index =
          (occupancies[index] * magic_number) >> (64 - relevant_bits);
      if (used_attacks[magic_index] == 0ULL) {
        used_attacks[magic_index] = attacks[index];
      } else if (used_attacks[magic_index] != attacks[index]) {
        valid = false;
      }
    }
    if (valid) return magic_number;
  }
  throw std::runtime_error("Failed to find magic number.");
  return 0ULL;
}

Key GenerateMagicNumberCandidate() {
  return GetRandomNumber64() & GetRandomNumber64() & GetRandomNumber64();
}

Bitboard GeneratePawnAttackMask(Square sq, Color c) {
  Bitboard attacks = kEmptyBitboard;

  if (c == kWhite) {
    if (GetFile(sq) != kHFile && GetRank(sq) != kRank8) {
      SetBit(attacks, sq + kNorthEast);
    }
    if (GetFile(sq) != kAFile && GetRank(sq) != kRank8) {
      SetBit(attacks, sq + kNorthWest);
    }
  } else {
    if (GetFile(sq) != kHFile && GetRank(sq) != kRank1) {
      SetBit(attacks, sq + kSouthEast);
    }
    if (GetFile(sq) != kAFile && GetRank(sq) != kRank1) {
      SetBit(attacks, sq + kSouthWest);
    }
  }
  return attacks;
}

Bitboard GenerateKnightAttackMask(Square sq) {
  Bitboard attacks = kEmptyBitboard;

  if (GetFile(sq) <= kGFile && GetRank(sq) <= kRank6) {
    SetBit(attacks, sq + kNorth + kNorthEast);
  }
  if (GetFile(sq) <= kFFile && GetRank(sq) <= kRank7) {
    SetBit(attacks, sq + kEast + kNorthEast);
  }
  if (GetFile(sq) <= kFFile && GetRank(sq) >= kRank2) {
    SetBit(attacks, sq + kEast + kSouthEast);
  }
  if (GetFile(sq) <= kGFile && GetRank(sq) >= kRank3) {
    SetBit(attacks, sq + kSouth + kSouthEast);
  }
  if (GetFile(sq) >= kBFile && GetRank(sq) >= kRank3) {
    SetBit(attacks, sq + kSouth + kSouthWest);
  }
  if (GetFile(sq) >= kCFile && GetRank(sq) >= kRank2) {
    SetBit(attacks, sq + kWest + kSouthWest);
  }
  if (GetFile(sq) >= kCFile && GetRank(sq) <= kRank7) {
    SetBit(attacks, sq + kWest + kNorthWest);
  }
  if (GetFile(sq) >= kBFile && GetRank(sq) <= kRank6) {
    SetBit(attacks, sq + kNorth + kNorthWest);
  }
  return attacks;
}

Bitboard GenerateBishopAttackMask(Square sq) {
  Bitboard attacks = kEmptyBitboard;
  Square currSq = sq;

  while (GetFile(currSq) < kGFile && GetRank(currSq) < kRank7) {
    currSq += kNorthEast;
    SetBit(attacks, currSq);
  }
  currSq = sq;
  while (GetFile(currSq) < kGFile && GetRank(currSq) > kRank2) {
    currSq += kSouthEast;
    SetBit(attacks, currSq);
  }
  currSq = sq;
  while (GetFile(currSq) > kBFile && GetRank(currSq) > kRank2) {
    currSq += kSouthWest;
    SetBit(attacks, currSq);
  }
  currSq = sq;
  while (GetFile(currSq) > kBFile && GetRank(currSq) < kRank7) {
    currSq += kNorthWest;
    SetBit(attacks, currSq);
  }
  return attacks;
}

Bitboard GenerateRookAttackMask(Square sq) {
  Bitboard attacks = kEmptyBitboard;
  Square currSq = sq;

  while (GetRank(currSq) < kRank7) {
    currSq += kNorth;
    SetBit(attacks, currSq);
  }
  currSq = sq;
  while (GetFile(currSq) < kGFile) {
    currSq += kEast;
    SetBit(attacks, currSq);
  }
  currSq = sq;
  while (GetRank(currSq) > kRank2) {
    currSq += kSouth;
    SetBit(attacks, currSq);
  }
  currSq = sq;
  while (GetFile(currSq) > kBFile) {
    currSq += kWest;
    SetBit(attacks, currSq);
  }
  return attacks;
}

Bitboard GenerateKingAttackMask(Square sq) {
  Bitboard attacks = kEmptyBitboard;

  if (GetFile(sq) != kHFile) {
    if (GetRank(sq) != kRank8) SetBit(attacks, sq + kNorthEast);
    SetBit(attacks, sq + kEast);
    if (GetRank(sq) != kRank1) SetBit(attacks, sq + kSouthEast);
  }
  if (GetRank(sq) != kRank8) SetBit(attacks, sq + kNorth);
  if (GetRank(sq) != kRank1) SetBit(attacks, sq + kSouth);
  if (GetFile(sq) != kAFile) {
    if (GetRank(sq) != kRank8) SetBit(attacks, sq + kNorthWest);
    SetBit(attacks, sq + kWest);
    if (GetRank(sq) != kRank1) SetBit(attacks, sq + kSouthWest);
  }
  return attacks;
}

Bitboard GenerateBishopAttacks(Square sq, Bitboard occupancy) {
  Bitboard attacks = kEmptyBitboard;
  Square currSq = sq;

  while (GetFile(currSq) < kHFile && GetRank(currSq) < kRank8) {
    currSq += kNorthEast;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }
  currSq = sq;
  while (GetFile(currSq) < kHFile && GetRank(currSq) > kRank1) {
    currSq += kSouthEast;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }
  currSq = sq;
  while (GetFile(currSq) > kAFile && GetRank(currSq) > kRank1) {
    currSq += kSouthWest;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }
  currSq = sq;
  while (GetFile(currSq) > kAFile && GetRank(currSq) < kRank8) {
    currSq += kNorthWest;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }

  return attacks;
}

Bitboard GenerateRookAttacks(Square sq, Bitboard occupancy) {
  Bitboard attacks = kEmptyBitboard;
  Square currSq = sq;

  while (GetRank(currSq) < kRank8) {
    currSq += kNorth;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }
  currSq = sq;
  while (GetFile(currSq) < kHFile) {
    currSq += kEast;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }
  currSq = sq;
  while (GetRank(currSq) > kRank1) {
    currSq += kSouth;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }
  currSq = sq;
  while (GetFile(currSq) > kAFile) {
    currSq += kWest;
    SetBit(attacks, currSq);
    if (GetBit(occupancy, currSq)) break;
  }

  return attacks;
}

Bitboard GenerateOccupancyKey(int index, Bitboard attack_mask) {
  Key occupancy_key = 0ULL;
  int relevant_bits = CountBits(attack_mask);

  for (int mask_index = 0; mask_index < relevant_bits; mask_index++) {
    int key_index = GetLSBIndex(attack_mask);
    ClearLSB(attack_mask);
    if (GetBit(index, mask_index)) {
      SetBit(occupancy_key, key_index);
    }
  }

  return occupancy_key;
}

// clang-format off
int bishop_relevant_bits[kNumSquares] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6,};

int rook_relevant_bits[kNumSquares] = {
  12, 11, 11, 11, 11, 11, 11, 12, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  12, 11, 11, 11, 11, 11, 11, 12,};
// clang-format on

Key rook_magic_numbers[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL,  0x2801880a0017001ULL,
    0x100081001000420ULL,  0x200020010080420ULL,  0x3001c0002010008ULL,
    0x8480008002000100ULL, 0x2080088004402900ULL, 0x800098204000ULL,
    0x2024401000200040ULL, 0x100802000801000ULL,  0x120800800801000ULL,
    0x208808088000400ULL,  0x2802200800400ULL,    0x2200800100020080ULL,
    0x801000060821100ULL,  0x80044006422000ULL,   0x100808020004000ULL,
    0x12108a0010204200ULL, 0x140848010000802ULL,  0x481828014002800ULL,
    0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL,  0x2040002120081000ULL, 0x21200680100081ULL,
    0x20100080080080ULL,   0x2000a00200410ULL,    0x20080800400ULL,
    0x80088400100102ULL,   0x80004600042881ULL,   0x4040008040800020ULL,
    0x440003000200801ULL,  0x4200011004500ULL,    0x188020010100100ULL,
    0x14800401802800ULL,   0x2080040080800200ULL, 0x124080204001001ULL,
    0x200046502000484ULL,  0x480400080088020ULL,  0x1000422010034000ULL,
    0x30200100110040ULL,   0x100021010009ULL,     0x2002080100110004ULL,
    0x202008004008002ULL,  0x20020004010100ULL,   0x2048440040820001ULL,
    0x101002200408200ULL,  0x40802000401080ULL,   0x4008142004410100ULL,
    0x2060820c0120200ULL,  0x1001004080100ULL,    0x20c020080040080ULL,
    0x2935610830022400ULL, 0x44440041009200ULL,   0x280001040802101ULL,
    0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL,    0x12001008414402ULL,   0x2006104900a0804ULL,
    0x1004081002402ULL,
};

Key bishop_magic_numbers[64] = {
    0x40040844404084ULL,   0x2004208a004208ULL,   0x10190041080202ULL,
    0x108060845042010ULL,  0x581104180800210ULL,  0x2112080446200010ULL,
    0x1080820820060210ULL, 0x3c0808410220200ULL,  0x4050404440404ULL,
    0x21001420088ULL,      0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL,      0x4011002100800ULL,    0x401484104104005ULL,
    0x801010402020200ULL,  0x400210c3880100ULL,   0x404022024108200ULL,
    0x810018200204102ULL,  0x4002801a02003ULL,    0x85040820080400ULL,
    0x810102c808880400ULL, 0xe900410884800ULL,    0x8002020480840102ULL,
    0x220200865090201ULL,  0x2010100a02021202ULL, 0x152048408022401ULL,
    0x20080002081110ULL,   0x4001001021004000ULL, 0x800040400a011002ULL,
    0xe4004081011002ULL,   0x1c004001012080ULL,   0x8004200962a00220ULL,
    0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL,
    0x42008c0340209202ULL, 0x209188240001000ULL,  0x400408a884001800ULL,
    0x110400a6080400ULL,   0x1840060a44020800ULL, 0x90080104000041ULL,
    0x201011000808101ULL,  0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL,  0x180806108200800ULL,  0x4000020e01040044ULL,
    0x300000261044000aULL, 0x802241102020002ULL,  0x20906061210001ULL,
    0x5a84841004010310ULL, 0x4010801011c04ULL,    0xa010109502200ULL,
    0x4a02012000ULL,       0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL,   0x6000020202d0240ULL,  0x8918844842082200ULL,
    0x4010011029020020ULL,
};

}  // namespace precomputed_data

}  // namespace chess