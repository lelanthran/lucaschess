/*
 McCain, a UCI chess playing engine derived from Stockfish and Glaurung 2.1
 Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
 Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish Authors)
 Copyright (C) 2015-2016 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad (Stockfish Authors)
 Copyright (C) 2017-2019 Michael Byrne, Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad (McCain Authors)

 McCain is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 McCain is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <bitset>
#include <algorithm>

#include "bitboard.h"
#include "misc.h"

uint8_t PopCnt16[1 << 16];

uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

Bitboard LineBB[SQUARE_NB][SQUARE_NB];
Bitboard PseudoAttacks[PIECE_TYPE_NB][SQUARE_NB];
Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
Bitboard SquareBB[SQUARE_NB];

Bitboard KingFlank[FILE_NB] = {
  QueenSide ^ FileDBB, QueenSide, QueenSide,
  CenterFiles, CenterFiles,
  KingSide, KingSide, KingSide ^ FileEBB
};

Magic RookMagics[SQUARE_NB];
Magic BishopMagics[SQUARE_NB];

namespace {

#ifdef Maverick //Niklas Fiekas fast magics
Bitboard AttackTable[HasPext ? 107648 : 88772] = { 0 };

struct MagicInit {
    Bitboard magic;
    unsigned offset;
};

MagicInit BishopMagicInit[SQUARE_NB] = {
    { 0x007fbfbfbfbfbfffu,   5378 },
    { 0x0000a060401007fcu,   4093 },
    { 0x0001004008020000u,   4314 },
    { 0x0000806004000000u,   6587 },
    { 0x0000100400000000u,   6491 },
    { 0x000021c100b20000u,   6330 },
    { 0x0000040041008000u,   5609 },
    { 0x00000fb0203fff80u,  22236 },
    { 0x0000040100401004u,   6106 },
    { 0x0000020080200802u,   5625 },
    { 0x0000004010202000u,  16785 },
    { 0x0000008060040000u,  16817 },
    { 0x0000004402000000u,   6842 },
    { 0x0000000801008000u,   7003 },
    { 0x000007efe0bfff80u,   4197 },
    { 0x0000000820820020u,   7356 },
    { 0x0000400080808080u,   4602 },
    { 0x00021f0100400808u,   4538 },
    { 0x00018000c06f3fffu,  29531 },
    { 0x0000258200801000u,  45393 },
    { 0x0000240080840000u,  12420 },
    { 0x000018000c03fff8u,  15763 },
    { 0x00000a5840208020u,   5050 },
    { 0x0000020008208020u,   4346 },
    { 0x0000804000810100u,   6074 },
    { 0x0001011900802008u,   7866 },
    { 0x0000804000810100u,  32139 },
    { 0x000100403c0403ffu,  57673 },
    { 0x00078402a8802000u,  55365 },
    { 0x0000101000804400u,  15818 },
    { 0x0000080800104100u,   5562 },
    { 0x00004004c0082008u,   6390 },
    { 0x0001010120008020u,   7930 },
    { 0x000080809a004010u,  13329 },
    { 0x0007fefe08810010u,   7170 },
    { 0x0003ff0f833fc080u,  27267 },
    { 0x007fe08019003042u,  53787 },
    { 0x003fffefea003000u,   5097 },
    { 0x0000101010002080u,   6643 },
    { 0x0000802005080804u,   6138 },
    { 0x0000808080a80040u,   7418 },
    { 0x0000104100200040u,   7898 },
    { 0x0003ffdf7f833fc0u,  42012 },
    { 0x0000008840450020u,  57350 },
    { 0x00007ffc80180030u,  22813 },
    { 0x007fffdd80140028u,  56693 },
    { 0x00020080200a0004u,   5818 },
    { 0x0000101010100020u,   7098 },
    { 0x0007ffdfc1805000u,   4451 },
    { 0x0003ffefe0c02200u,   4709 },
    { 0x0000000820806000u,   4794 },
    { 0x0000000008403000u,  13364 },
    { 0x0000000100202000u,   4570 },
    { 0x0000004040802000u,   4282 },
    { 0x0004010040100400u,  14964 },
    { 0x00006020601803f4u,   4026 },
    { 0x0003ffdfdfc28048u,   4826 },
    { 0x0000000820820020u,   7354 },
    { 0x0000000008208060u,   4848 },
    { 0x0000000000808020u,  15946 },
    { 0x0000000001002020u,  14932 },
    { 0x0000000401002008u,  16588 },
    { 0x0000004040404040u,   6905 },
    { 0x007fff9fdf7ff813u,  16076 }
};

MagicInit RookMagicInit[SQUARE_NB] = {
    { 0x00280077ffebfffeu,  26304 },
    { 0x2004010201097fffu,  35520 },
    { 0x0010020010053fffu,  38592 },
    { 0x0040040008004002u,   8026 },
    { 0x7fd00441ffffd003u,  22196 },
    { 0x4020008887dffffeu,  80870 },
    { 0x004000888847ffffu,  76747 },
    { 0x006800fbff75fffdu,  30400 },
    { 0x000028010113ffffu,  11115 },
    { 0x0020040201fcffffu,  18205 },
    { 0x007fe80042ffffe8u,  53577 },
    { 0x00001800217fffe8u,  62724 },
    { 0x00001800073fffe8u,  34282 },
    { 0x00001800e05fffe8u,  29196 },
    { 0x00001800602fffe8u,  23806 },
    { 0x000030002fffffa0u,  49481 },
    { 0x00300018010bffffu,   2410 },
    { 0x0003000c0085fffbu,  36498 },
    { 0x0004000802010008u,  24478 },
    { 0x0004002020020004u,  10074 },
    { 0x0001002002002001u,  79315 },
    { 0x0001001000801040u,  51779 },
    { 0x0000004040008001u,  13586 },
    { 0x0000006800cdfff4u,  19323 },
    { 0x0040200010080010u,  70612 },
    { 0x0000080010040010u,  83652 },
    { 0x0004010008020008u,  63110 },
    { 0x0000040020200200u,  34496 },
    { 0x0002008010100100u,  84966 },
    { 0x0000008020010020u,  54341 },
    { 0x0000008020200040u,  60421 },
    { 0x0000820020004020u,  86402 },
    { 0x00fffd1800300030u,  50245 },
    { 0x007fff7fbfd40020u,  76622 },
    { 0x003fffbd00180018u,  84676 },
    { 0x001fffde80180018u,  78757 },
    { 0x000fffe0bfe80018u,  37346 },
    { 0x0001000080202001u,    370 },
    { 0x0003fffbff980180u,  42182 },
    { 0x0001fffdff9000e0u,  45385 },
    { 0x00fffefeebffd800u,  61659 },
    { 0x007ffff7ffc01400u,  12790 },
    { 0x003fffbfe4ffe800u,  16762 },
    { 0x001ffff01fc03000u,      0 },
    { 0x000fffe7f8bfe800u,  38380 },
    { 0x0007ffdfdf3ff808u,  11098 },
    { 0x0003fff85fffa804u,  21803 },
    { 0x0001fffd75ffa802u,  39189 },
    { 0x00ffffd7ffebffd8u,  58628 },
    { 0x007fff75ff7fbfd8u,  44116 },
    { 0x003fff863fbf7fd8u,  78357 },
    { 0x001fffbfdfd7ffd8u,  44481 },
    { 0x000ffff810280028u,  64134 },
    { 0x0007ffd7f7feffd8u,  41759 },
    { 0x0003fffc0c480048u,   1394 },
    { 0x0001ffffafd7ffd8u,  40910 },
    { 0x00ffffe4ffdfa3bau,  66516 },
    { 0x007fffef7ff3d3dau,   3897 },
    { 0x003fffbfdfeff7fau,   3930 },
    { 0x001fffeff7fbfc22u,  72934 },
    { 0x0000020408001001u,  72662 },
    { 0x0007fffeffff77fdu,  56325 },
    { 0x0003ffffbf7dfeecu,  66501 },
    { 0x0001ffff9dffa333u,  14826 }
};

Bitboard relevant_occupancies(Direction directions[], Square s);

template<PieceType Pt>
void init_magics(MagicInit init[], Magic magics[], Direction directions[]);
#else
Bitboard RookTable[0x19000];  // To store rook attacks
Bitboard BishopTable[0x1480]; // To store bishop attacks

void init_magics(Bitboard table[], Magic magics[], Direction directions[]);
#endif

}


/// Bitboards::pretty() returns an ASCII representation of a bitboard suitable
/// to be printed to standard output. Useful for debugging.

const std::string Bitboards::pretty(Bitboard b) {

  std::string s = "+---+---+---+---+---+---+---+---+\n";

  for (Rank r = RANK_8; r >= RANK_1; --r)
  {
      for (File f = FILE_A; f <= FILE_H; ++f)
          s += b & make_square(f, r) ? "| X " : "|   ";

      s += "|\n+---+---+---+---+---+---+---+---+\n";
  }

  return s;
}


/// Bitboards::init() initializes various bitboard tables. It is called at
/// startup and relies on global objects to be already zero-initialized.

void Bitboards::init() {

  for (unsigned i = 0; i < (1 << 16); ++i)
      PopCnt16[i] = std::bitset<16>(i).count();

  for (Square s = SQ_A1; s <= SQ_H8; ++s)
      SquareBB[s] = (1ULL << s);

  for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
      for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
              SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));

  int steps[][5] = { {}, { 7, 9 }, { 6, 10, 15, 17 }, {}, {}, {}, { 1, 7, 8, 9 } };

  for (Color c = WHITE; c <= BLACK; ++c)
      for (PieceType pt : { PAWN, KNIGHT, KING })
          for (Square s = SQ_A1; s <= SQ_H8; ++s)
              for (int i = 0; steps[pt][i]; ++i)
              {
                  Square to = s + Direction(c == WHITE ? steps[pt][i] : -steps[pt][i]);

                  if (is_ok(to) && distance(s, to) < 3)
                  {
                      if (pt == PAWN)
                          PawnAttacks[c][s] |= to;
                      else
                          PseudoAttacks[pt][s] |= to;
                  }
              }

  Direction RookDirections[] = { NORTH, EAST, SOUTH, WEST };
  Direction BishopDirections[] = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

#ifdef Maverick ////Niklas Fiekas fast magics
    if (HasPext)
    {
        unsigned offset = 0;
        for (Square s = SQ_A1; s <= SQ_H8; ++s)
        {
            RookMagicInit[s].offset = offset;
            offset += 1 << popcount(relevant_occupancies(RookDirections, s));

        }
        for (Square s = SQ_A1; s <= SQ_H8; ++s)
        {
            BishopMagicInit[s].offset = offset;
            offset += 1 << popcount(relevant_occupancies(BishopDirections, s));

        }

    }

    init_magics<ROOK>(RookMagicInit, RookMagics, RookDirections);
    init_magics<BISHOP>(BishopMagicInit, BishopMagics, BishopDirections);
#else
    init_magics(RookTable, RookMagics, RookDirections);
    init_magics(BishopTable, BishopMagics, BishopDirections);
#endif

  for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
  {
      PseudoAttacks[QUEEN][s1]  = PseudoAttacks[BISHOP][s1] = attacks_bb<BISHOP>(s1, 0);
      PseudoAttacks[QUEEN][s1] |= PseudoAttacks[  ROOK][s1] = attacks_bb<  ROOK>(s1, 0);

      for (PieceType pt : { BISHOP, ROOK })
          for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
              if (PseudoAttacks[pt][s1] & s2)
                  LineBB[s1][s2] = (attacks_bb(pt, s1, 0) & attacks_bb(pt, s2, 0)) | s1 | s2;
  }
}


namespace {

  Bitboard sliding_attack(Direction directions[], Square sq, Bitboard occupied) {

    Bitboard attack = 0;

    for (int i = 0; i < 4; ++i)
        for (Square s = sq + directions[i];
             is_ok(s) && distance(s, s - directions[i]) == 1;
             s += directions[i])
        {
            attack |= s;

            if (occupied & s)
                break;
        }

    return attack;
}
#ifdef Maverick //Niklas Fiekas fast magics
Bitboard relevant_occupancies(Direction directions[], Square s) {
    Bitboard edges = ((Rank1BB | Rank8BB) & ~rank_bb(s)) | ((FileABB | FileHBB) & ~file_bb(s));
    return sliding_attack(directions, s, 0) & ~edges;
}
#endif

  // init_magics() computes all rook and bishop attacks at startup. Magic
  // bitboards are used to look up attacks of sliding pieces. As a reference see
  // www.chessprogramming.org/Magic_Bitboards. In particular, here we use the so
  // called "fancy" approach.

#ifdef Maverick //Niklas Fiekas fast magics
template<PieceType Pt>
void init_magics(MagicInit init[], Magic magics[], Direction directions[])
{
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        Magic& m = magics[s];
        m.magic = init[s].magic;
        m.mask = relevant_occupancies(directions, s);
        m.attacks = AttackTable + init[s].offset;

        Bitboard b = 0;
        do
        {
            unsigned idx = m.index<Pt>(b);
            Bitboard attack = sliding_attack(directions, s, b);
            assert(!m.attacks[idx] || m.attacks[idx] == attack);
            m.attacks[idx] = attack;
            b = (b - m.mask) & m.mask;

        }
        while (b);

    }
}
}
#else
void init_magics(Bitboard table[], Magic magics[], Direction directions[]) {

    // Optimal PRNG seeds to pick the correct magics in the shortest time
    int seeds[][RANK_NB] = { { 8977, 44560, 54343, 38998,  5731, 95205, 104912, 17020 },
                             {  728, 10316, 55013, 32803, 12281, 15100,  16645,   255 } };

    Bitboard occupancy[4096], reference[4096], edges, b;
    int epoch[4096] = {}, cnt = 0, size = 0;

    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        // Board edges are not considered in the relevant occupancies
        edges = ((Rank1BB | Rank8BB) & ~rank_bb(s)) | ((FileABB | FileHBB) & ~file_bb(s));

        // Given a square 's', the mask is the bitboard of sliding attacks from
        // 's' computed on an empty board. The index must be big enough to contain
        // all the attacks for each possible subset of the mask and so is 2 power
        // the number of 1s of the mask. Hence we deduce the size of the shift to
        // apply to the 64 or 32 bits word to get the index.
        Magic& m = magics[s];
        m.mask  = sliding_attack(directions, s, 0) & ~edges;
        m.shift = (Is64Bit ? 64 : 32) - popcount(m.mask);

        // Set the offset for the attacks table of the square. We have individual
        // table sizes for each square with "Fancy Magic Bitboards".
        m.attacks = s == SQ_A1 ? table : magics[s - 1].attacks + size;

        // Use Carry-Rippler trick to enumerate all subsets of masks[s] and
        // store the corresponding sliding attack bitboard in reference[].
        b = size = 0;
        do {
            occupancy[size] = b;
            reference[size] = sliding_attack(directions, s, b);

            if (HasPext)
                m.attacks[pext(b, m.mask)] = reference[size];

            size++;
            b = (b - m.mask) & m.mask;
        } while (b);

        if (HasPext)
            continue;

        PRNG rng(seeds[Is64Bit][rank_of(s)]);

        // Find a magic for square 's' picking up an (almost) random number
        // until we find the one that passes the verification test.
        for (int i = 0; i < size; )
        {
            for (m.magic = 0; popcount((m.magic * m.mask) >> 56) < 6; )
                m.magic = rng.sparse_rand<Bitboard>();

            // A good magic must map every possible occupancy to an index that
            // looks up the correct sliding attack in the attacks[s] database.
            // Note that we build up the database for square 's' as a side
            // effect of verifying the magic. Keep track of the attempt count
            // and save it in epoch[], little speed-up trick to avoid resetting
            // m.attacks[] after every failed attempt.
            for (++cnt, i = 0; i < size; ++i)
            {
                unsigned idx = m.index(occupancy[i]);

                if (epoch[idx] < cnt)
                {
                    epoch[idx] = cnt;
                    m.attacks[idx] = reference[i];
                }
                else if (m.attacks[idx] != reference[i])
                    break;
            }
        }
    }
  }
}
#endif
