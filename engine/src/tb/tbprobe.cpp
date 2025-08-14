/*
  Copyright (c) 2013-2015 Ronald de Man
  This file may be redistributed and/or modified without restrictions.

  tbprobe.cpp contains the Stockfish-specific routines of the
  tablebase probing code. It should be relatively easy to adapt
  this code to other chess engines.
*/

// The probing code currently expects a little-endian architecture (e.g. x86).

// Define DECOMP64 when compiling for a 64-bit platform.
// 32-bit is only supported for 5-piece tables, because tables are mmap()ed
// into memory.
#include <bit>

#include "tb/tbcore.h"
#include "tb/tbprobe.h"

#include "engine/board.hpp"
#include "engine/hash.hpp"
#include "engine/move.hpp"
#include "engine/move_gen.hpp"
#include "engine/position.hpp"
#include "engine/square.hpp"
#include "engine/types.hpp"

#ifdef IS_64BIT
#define DECOMP64
#endif

int TBlargest = 0;

#include "tbcore.c"

static const engine::Piece kPieceMap[6] = {
    engine::PAWN, engine::KNIGHT, engine::BISHOP,
    engine::ROOK, engine::QUEEN,  engine::KING,
};

// Given a position with 6 or fewer pieces, produce a text string
// of the form KQPvKRP, where "KQP" represents the white pieces if
// mirror == 0 and the black pieces if mirror == 1.
// No need to make this very efficient.
static void prt_str(Position &pos, char *str, int mirror) {
  int color;
  int i;

  color = !mirror ? engine::WHITE : engine::BLACK;

  auto pieces = pos.Pieces(static_cast<engine::Color>(color));

  for (int pt = 6; pt >= 1; pt--) {
    engine::Piece piece = kPieceMap[pt - 1];

    for (i = std::popcount(pieces[piece]); i > 0; i--) {
      *str++ = pchr[6 - pt];
    }
  }

  *str++ = 'v';
  color = OPP(color);
  pieces = pos.Pieces(static_cast<engine::Color>(color));

  for (int pt = 6; pt >= 1; pt--) {
    engine::Piece piece = kPieceMap[pt - 1];

    for (i = std::popcount(pieces[piece]); i > 0; i--) {
      *str++ = pchr[6 - pt];
    }
  }

  *str++ = 0;
}

// Given a position, produce a 64-bit material signature key.
// If the engine supports such a key, it should equal the engine's key.
// Again no need to make this very efficient.
static uint64 calc_key(Position &pos, int mirror) {
  int color;
  int i;
  uint64 key = 0;

  color = !mirror ? engine::WHITE : engine::BLACK;

  auto pieces = pos.Pieces(static_cast<engine::Color>(color));

  // INFO: pawn=1, knight=2, bishop=3, rook=4, queen=5, king=6
  for (int pt = 1; pt <= 6; pt++) {
    engine::Piece piece = kPieceMap[pt - 1];

    for (i = std::popcount(pieces[piece]); i > 0; i--) {
      key ^=
          engine::kZobrist.piece_sq[ZOBRIST_INDEX(piece, engine::WHITE, i - 1)];
    }
  }

  color = OPP(color);
  pieces = pos.Pieces(static_cast<engine::Color>(color));

  for (int pt = 1; pt <= 6; pt++) {
    engine::Piece piece = kPieceMap[pt - 1];

    for (i = std::popcount(pieces[piece]); i > 0; i--) {
      key ^=
          engine::kZobrist.piece_sq[ZOBRIST_INDEX(piece, engine::BLACK, i - 1)];
    }
  }

  return key;
}

// Produce a 64-bit material key corresponding to the material combination
// defined by pcs[16], where pcs[1], ..., pcs[6] is the number of white
// pawns, ..., kings and pcs[9], ..., pcs[14] is the number of black
// pawns, ..., kings.
// Again no need to be efficient here.
static uint64 calc_key_from_pcs(int *pcs, int mirror) {
  int color;
  uint64 key = 0;

  color = !mirror ? 0 : 8;

  // INFO: pawn=1, knight=2, bishop=3, rook=4, queen=5, king=6
  for (int pt = 1; pt <= 6; pt++) {
    for (int i = 0; i < pcs[color + pt]; i++) {
      engine::Piece piece = kPieceMap[pt - 1];

      key ^= engine::kZobrist.piece_sq[ZOBRIST_INDEX(piece, engine::WHITE, i)];
    }
  }

  color ^= 8;

  for (int pt = 1; pt <= 6; pt++) {
    for (int i = 0; i < pcs[color + pt]; i++) {
      engine::Piece piece = kPieceMap[pt - 1];

      key ^= engine::kZobrist.piece_sq[ZOBRIST_INDEX(piece, engine::BLACK, i)];
    }
  }

  return key;
}

// TODO: implement a material key concept
static uint64 material_key(Position &pos) { return calc_key(pos, 0); }

// probe_wdl_table and probe_dtz_table require similar adaptations.
static int probe_wdl_table(Position &pos, int *success) {
  struct TBEntry *ptr;
  struct TBHashEntry *ptr2;
  uint64 idx;
  uint64 key;
  int i;
  ubyte res;
  int p[TBPIECES];

  // Obtain the position's material signature key.
  key = material_key(pos);

  // Test for KvK.
  if (key == (engine::kZobrist
                  .piece_sq[ZOBRIST_INDEX(engine::KING, engine::WHITE, 0)] ^
              engine::kZobrist
                  .piece_sq[ZOBRIST_INDEX(engine::KING, engine::BLACK, 0)])) {
    return 0;
  }

  ptr2 = TB_hash[key >> (64 - TBHASHBITS)];
  for (i = 0; i < HSHMAX; i++)
    if (ptr2[i].key == key) break;
  if (i == HSHMAX) {
    *success = 0;
    return 0;
  }

  ptr = ptr2[i].ptr;
  if (!ptr->ready) {
    LOCK(TB_mutex);
    if (!ptr->ready) {
      char str[16];
      prt_str(pos, str, ptr->key != key);
      if (!init_table_wdl(ptr, str)) {
        ptr2[i].key = 0ULL;
        *success = 0;
        UNLOCK(TB_mutex);
        return 0;
      }
      // Memory barrier to ensure ptr->ready = 1 is not reordered.
      __asm__ __volatile__("" ::: "memory");
      ptr->ready = 1;
    }
    UNLOCK(TB_mutex);
  }

  int bside, mirror, cmirror;
  if (!ptr->symmetric) {
    if (key != ptr->key) {
      cmirror = 8;
      mirror = 0x38;
      bside = (pos.Turn() == engine::WHITE);
    } else {
      cmirror = mirror = 0;
      bside = !(pos.Turn() == engine::WHITE);
    }
  } else {
    cmirror = pos.Turn() == engine::WHITE ? 0 : 8;
    mirror = pos.Turn() == engine::WHITE ? 0 : 0x38;
    bside = 0;
  }

  // p[i] is to contain the square 0-63 (A1-H8) for a piece of type
  // pc[i] ^ cmirror, where 1 = white pawn, ..., 14 = black king.
  // Pieces of the same type are guaranteed to be consecutive.
  if (!ptr->has_pawns) {
    struct TBEntry_piece *entry = (struct TBEntry_piece *)ptr;
    ubyte *pc = entry->pieces[bside];
    for (i = 0; i < entry->num;) {
      const auto &pieces = pos.Pieces((engine::Color)((pc[i] ^ cmirror) >> 3));
      engine::Bitboard bb = pieces[kPieceMap[(pc[i] & 0x07) - 1]];

      BITLOOP(bb) { p[i++] = LOOP_INDEX; }
    }
    idx = encode_piece(entry, entry->norm[bside], p, entry->factor[bside]);
    res = decompress_pairs(entry->precomp[bside], idx);
  } else {
    struct TBEntry_pawn *entry = (struct TBEntry_pawn *)ptr;
    int k = entry->file[0].pieces[0][0] ^ cmirror;

    const auto &pieces = pos.Pieces((engine::Color)(k >> 3));
    engine::Bitboard bb = pieces[kPieceMap[(k & 0x07) - 1]];
    i = 0;

    BITLOOP(bb) { p[i++] = LOOP_INDEX ^ mirror; }

    int f = pawn_file(entry, p);
    ubyte *pc = entry->file[f].pieces[bside];
    for (; i < entry->num;) {
      const auto &pieces = pos.Pieces((engine::Color)((pc[i] ^ cmirror) >> 3));
      engine::Bitboard bb = pieces[kPieceMap[(pc[i] & 0x07) - 1]];

      BITLOOP(bb) { p[i++] = LOOP_INDEX ^ mirror; }
    }
    idx = encode_pawn(entry, entry->file[f].norm[bside], p,
                      entry->file[f].factor[bside]);
    res = decompress_pairs(entry->file[f].precomp[bside], idx);
  }

  return ((int)res) - 2;
}

static int probe_dtz_table(Position &pos, int wdl, int *success) {
  struct TBEntry *ptr;
  uint64 idx;
  int i, res;
  int p[TBPIECES];

  // Obtain the position's material signature key.
  uint64 key = material_key(pos);

  if (DTZ_table[0].key1 != key && DTZ_table[0].key2 != key) {
    for (i = 1; i < DTZ_ENTRIES; i++)
      if (DTZ_table[i].key1 == key || DTZ_table[i].key2 == key) break;
    if (i < DTZ_ENTRIES) {
      struct DTZTableEntry table_entry = DTZ_table[i];
      for (; i > 0; i--) DTZ_table[i] = DTZ_table[i - 1];
      DTZ_table[0] = table_entry;
    } else {
      struct TBHashEntry *ptr2 = TB_hash[key >> (64 - TBHASHBITS)];
      for (i = 0; i < HSHMAX; i++)
        if (ptr2[i].key == key) break;
      if (i == HSHMAX) {
        *success = 0;
        return 0;
      }
      ptr = ptr2[i].ptr;
      char str[16];
      int mirror = (ptr->key != key);
      prt_str(pos, str, mirror);
      if (DTZ_table[DTZ_ENTRIES - 1].entry)
        free_dtz_entry(DTZ_table[DTZ_ENTRIES - 1].entry);
      for (i = DTZ_ENTRIES - 1; i > 0; i--) DTZ_table[i] = DTZ_table[i - 1];
      load_dtz_table(str, calc_key(pos, mirror), calc_key(pos, !mirror));
    }
  }

  ptr = DTZ_table[0].entry;
  if (!ptr) {
    *success = 0;
    return 0;
  }

  int bside, mirror, cmirror;
  if (!ptr->symmetric) {
    if (key != ptr->key) {
      cmirror = 8;
      mirror = 0x38;
      bside = (pos.Turn() == engine::WHITE);
    } else {
      cmirror = mirror = 0;
      bside = !(pos.Turn() == engine::WHITE);
    }
  } else {
    cmirror = pos.Turn() == engine::WHITE ? 0 : 8;
    mirror = pos.Turn() == engine::WHITE ? 0 : 0x38;
    bside = 0;
  }

  if (!ptr->has_pawns) {
    struct DTZEntry_piece *entry = (struct DTZEntry_piece *)ptr;
    if ((entry->flags & 1) != bside && !entry->symmetric) {
      *success = -1;
      return 0;
    }
    ubyte *pc = entry->pieces;
    for (i = 0; i < entry->num;) {
      const auto &pieces = pos.Pieces((engine::Color)((pc[i] ^ cmirror) >> 3));
      engine::Bitboard bb = pieces[kPieceMap[(pc[i] & 0x07) - 1]];

      BITLOOP(bb) { p[i++] = LOOP_INDEX; }
    }
    idx = encode_piece((struct TBEntry_piece *)entry, entry->norm, p,
                       entry->factor);
    res = decompress_pairs(entry->precomp, idx);

    if (entry->flags & 2)
      res = entry->map[entry->map_idx[wdl_to_map[wdl + 2]] + res];

    if (!(entry->flags & pa_flags[wdl + 2]) || (wdl & 1)) res *= 2;
  } else {
    struct DTZEntry_pawn *entry = (struct DTZEntry_pawn *)ptr;
    int k = entry->file[0].pieces[0] ^ cmirror;

    const auto &pieces = pos.Pieces((engine::Color)(k >> 3));
    engine::Bitboard bb = pieces[kPieceMap[(k & 0x07) - 1]];
    i = 0;

    BITLOOP(bb) { p[i++] = LOOP_INDEX ^ mirror; }

    int f = pawn_file((struct TBEntry_pawn *)entry, p);
    if ((entry->flags[f] & 1) != bside) {
      *success = -1;
      return 0;
    }
    ubyte *pc = entry->file[f].pieces;
    for (; i < entry->num;) {
      const auto &pieces = pos.Pieces((engine::Color)((pc[i] ^ cmirror) >> 3));
      engine::Bitboard bb = pieces[kPieceMap[(pc[i] & 0x07) - 1]];

      BITLOOP(bb) { p[i++] = LOOP_INDEX ^ mirror; }
    }
    idx = encode_pawn((struct TBEntry_pawn *)entry, entry->file[f].norm, p,
                      entry->file[f].factor);
    res = decompress_pairs(entry->file[f].precomp, idx);

    if (entry->flags[f] & 2)
      res = entry->map[entry->map_idx[f][wdl_to_map[wdl + 2]] + res];

    if (!(entry->flags[f] & pa_flags[wdl + 2]) || (wdl & 1)) res *= 2;
  }

  return res;
}

static int probe_ab(Position &pos, int alpha, int beta, int *success) {
  int v;
  const auto move_list = pos.LegalMoves();

  for (auto const &move : move_list) {
    if (!move.Is(engine::move::CAPTURE)) {
      continue;
    }

    pos.Make(move);
    v = -probe_ab(pos, -beta, -alpha, success);
    pos.Undo(move);

    if (*success == 0) {
      return 0;
    }

    if (v > alpha) {
      if (v >= beta) {
        return v;
      }

      alpha = v;
    }
  }

  v = probe_wdl_table(pos, success);

  return alpha >= v ? alpha : v;
}

// Probe the WDL table for a particular position.
//
// If *success != 0, the probe was successful.
//
// If *success == 2, the position has a winning capture, or the position
// is a cursed win and has a cursed winning capture, or the position
// has an ep capture as only best move.
// This is used in probe_dtz().
//
// The return value is from the point of view of the side to move:
// -2 : loss
// -1 : loss, but draw under 50-move rule
//  0 : draw
//  1 : win, but draw under 50-move rule
//  2 : win
int probe_wdl(Position &pos, int *success) {
  *success = 1;

  const auto move_list = pos.LegalMoves();

  int best_cap = -3, best_ep = -3;

  // We do capture resolution, letting best_cap keep track of the best
  // capture without ep rights and letting best_ep keep track of still
  // better ep captures if they exist.

  for (const auto &move : move_list) {
    if (!move.Is(engine::move::CAPTURE)) {
      continue;
    }

    pos.Make(move);
    int v = -probe_ab(pos, -2, -best_cap, success);
    pos.Undo(move);

    if (*success == 0) return 0;
    if (v > best_cap) {
      if (v == 2) {
        *success = 2;
        return 2;
      }

      if (!move.Is(engine::move::EN_PASSANT)) {
        best_cap = v;
      } else if (v > best_ep) {
        best_ep = v;
      }
    }
  }

  int v = probe_wdl_table(pos, success);
  if (*success == 0) return 0;

  // Now max(v, best_cap) is the WDL value of the position without ep rights.
  // If the position without ep rights is not stalemate or no ep captures
  // exist, then the value of the position is max(v, best_cap, best_ep).
  // If the position without ep rights is stalemate and best_ep > -3,
  // then the value of the position is best_ep (and we will have v == 0).

  if (best_ep > best_cap) {
    if (best_ep > v) {  // ep capture (possibly cursed losing) is best.
      *success = 2;
      return best_ep;
    }
    best_cap = best_ep;
  }

  // Now max(v, best_cap) is the WDL value of the position unless
  // the position without ep rights is stalemate and best_ep > -3.

  if (best_cap >= v) {
    // No need to test for the stalemate case here: either there are
    // non-ep captures, or best_cap == best_ep >= v anyway.
    *success = 1 + (best_cap > 0);
    return best_cap;
  }

  // TODO: handle stalemate case
  // Now handle the stalemate case.
  // if (best_ep > -3 && v == 0) {
  //   // Check for stalemate in the position with ep captures.
  //   for (const auto &move : move_list) {
  //     if (move.Is(engine::move::EN_PASSANT)) {
  //       continue;
  //     }
  //     if (pos.legal(move, ci.pinned)) break;
  //   }
  //
  //   if (moves == end && !pos.checkers()) {
  //     end = generate<QUIETS>(pos, end);
  //     for (; moves < end; moves++) {
  //       Move move = moves->move;
  //       if (pos.legal(move, ci.pinned)) break;
  //     }
  //   }
  //   if (moves == end) {  // Stalemate detected.
  //     *success = 2;
  //     return best_ep;
  //   }
  // }

  // Stalemate / en passant not an issue, so v is the correct value.

  return v;
}

static int wdl_to_dtz[] = {-1, -101, 0, 101, 1};

// Probe the DTZ table for a particular position.
// If *success != 0, the probe was successful.
// The return value is from the point of view of the side to move:
//         n < -100 : loss, but draw under 50-move rule
// -100 <= n < -1   : loss in n ply (assuming 50-move counter == 0)
//         0	    : draw
//     1 < n <= 100 : win in n ply (assuming 50-move counter == 0)
//   100 < n        : win, but draw under 50-move rule
//
// If the position is mate, -1 is returned instead of 0.
//
// The return value n can be off by 1: a return value -n can mean a loss
// in n+1 ply and a return value +n can mean a win in n+1 ply. This
// cannot happen for tables with positions exactly on the "edge" of
// the 50-move rule.
//
// This means that if dtz > 0 is returned, the position is certainly
// a win if dtz + 50-move-counter <= 99. Care must be taken that the engine
// picks moves that preserve dtz + 50-move-counter <= 99.
//
// If n = 100 immediately after a capture or pawn move, then the position
// is also certainly a win, and during the whole phase until the next
// capture or pawn move, the inequality to be preserved is
// dtz + 50-movecounter <= 100.
//
// In short, if a move is available resulting in dtz + 50-move-counter <= 99,
// then do not accept moves leading to dtz + 50-move-counter == 100.
//
int probe_dtz(Position &pos, int *success) {
  int wdl = probe_wdl(pos, success);
  if (*success == 0) return 0;

  // If draw, then dtz = 0.
  if (wdl == 0) return 0;

  // Check for winning (cursed) capture or ep capture as only best move.
  if (*success == 2) return wdl_to_dtz[wdl + 2];

  engine::MoveList move_list;

  // If winning, check for a winning pawn move.
  if (wdl > 0) {
    // Generate at least all legal non-capturing pawn moves
    // including non-capturing promotions.
    // (The call to generate<>() in fact generates all moves.)
    move_list = pos.LegalMoves();

    for (const auto &move : move_list) {
      if (move.piece != engine::PAWN || move.Is(engine::move::CAPTURE)) {
        continue;
      }

      pos.Make(move);
      int v = -probe_wdl(pos, success);
      pos.Undo(move);

      if (*success == 0) {
        return 0;
      }

      if (v == wdl) {
        return wdl_to_dtz[wdl + 2];
      }
    }
  }

  // If we are here, we know that the best move is not an ep capture.
  // In other words, the value of wdl corresponds to the WDL value of
  // the position without ep rights. It is therefore safe to probe the
  // DTZ table with the current value of wdl.

  int dtz = probe_dtz_table(pos, wdl, success);
  if (*success >= 0) return wdl_to_dtz[wdl + 2] + ((wdl > 0) ? dtz : -dtz);

  // *success < 0 means we need to probe DTZ for the other side to move.
  int best;
  if (wdl > 0) {
    best = INT32_MAX;
    // If wdl > 0, we already generated all moves.
  } else {
    // If (cursed) loss, the worst case is a losing capture or pawn move
    // as the "best" move, leading to dtz of -1 or -101.
    // In case of mate, this will cause -1 to be returned.
    best = wdl_to_dtz[wdl + 2];

    move_list = pos.LegalMoves();
  }

  for (const auto &move : move_list) {
    // We can skip pawn moves and captures.
    // If wdl > 0, we already caught them. If wdl < 0, the initial value
    // of best already takes account of them.
    if (move.Is(engine::move::CAPTURE) || move.piece == engine::PAWN) {
      continue;
    }

    pos.Make(move);
    int v = -probe_dtz(pos, success);
    pos.Undo(move);

    if (*success == 0) {
      return 0;
    }

    if (wdl > 0) {
      if (v > 0 && v + 1 < best) {
        best = v + 1;
      }
    } else {
      if (v - 1 < best) {
        best = v - 1;
      }
    }
  }
  return best;
}
