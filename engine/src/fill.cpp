#include <chesstillo/constants.hpp>
#include <chesstillo/fill.hpp>
#include <chesstillo/types.hpp>

Bitboard SouthOccluded(Bitboard bb, Bitboard pro) {
  bb |= pro & (bb >> 8);
  pro &= (pro >> 8);
  bb |= pro & (bb >> 16);
  pro &= (pro >> 16);
  bb |= pro & (bb >> 32);

  return bb;
}

Bitboard NorthOccluded(Bitboard bb, Bitboard pro) {
  bb |= pro & (bb << 8);
  pro &= (pro << 8);
  bb |= pro & (bb << 16);
  pro &= (pro << 16);
  bb |= pro & (bb << 32);

  return bb;
}

Bitboard EastOccluded(Bitboard bb, Bitboard pro) {
  pro &= ~kAFile;
  bb |= pro & (bb << 1);
  pro &= (pro << 1);
  bb |= pro & (bb << 2);
  pro &= (pro << 2);
  bb |= pro & (bb << 4);

  return bb;
}

Bitboard WestOccluded(Bitboard bb, Bitboard pro) {
  pro &= ~kHFile;
  bb |= pro & (bb >> 1);
  pro &= (pro >> 1);
  bb |= pro & (bb >> 2);
  pro &= (pro >> 2);
  bb |= pro & (bb >> 4);

  return bb;
}

Bitboard NorthEastOccluded(Bitboard bb, Bitboard pro) {
  pro &= ~kAFile;
  bb |= pro & (bb << 9);
  pro &= (pro << 9);
  bb |= pro & (bb << 18);
  pro &= (pro << 18);
  bb |= pro & (bb << 36);

  return bb;
}

Bitboard NorthWestOccluded(Bitboard bb, Bitboard pro) {
  pro &= ~kHFile;
  bb |= pro & (bb << 7);
  pro &= (pro << 7);
  bb |= pro & (bb << 14);
  pro &= (pro << 14);
  bb |= pro & (bb << 28);

  return bb;
}

Bitboard SouthWestOccluded(Bitboard bb, Bitboard pro) {
  pro &= ~kHFile;
  bb |= pro & (bb >> 9);
  pro &= (pro >> 9);
  bb |= pro & (bb >> 18);
  pro &= (pro >> 18);
  bb |= pro & (bb >> 36);

  return bb;
}

Bitboard SouthEastOccluded(Bitboard bb, Bitboard pro) {
  pro &= ~kAFile;
  bb |= pro & (bb >> 7);
  pro &= (pro >> 7);
  bb |= pro & (bb >> 14);
  pro &= (pro >> 14);
  bb |= pro & (bb >> 28);

  return bb;
}

Bitboard NorthFill(Bitboard bb) {
  bb |= (bb << 8);
  bb |= (bb << 16);
  bb |= (bb << 32);

  return bb;
}

Bitboard SouthFill(Bitboard bb) {
  bb |= (bb >> 8);
  bb |= (bb >> 16);
  bb |= (bb >> 32);

  return bb;
}
