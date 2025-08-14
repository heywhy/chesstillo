#ifndef TBPROBE_H
#define TBPROBE_H

extern int TBlargest;  // 5 if 5-piece tables, 6 if 6-piece tables were found.

namespace engine {
class Position;
class Value;
}

using Value = engine::Value;
using Position = engine::Position;

void init_tablebases(char* path);
int probe_wdl(Position& pos, int* success);
int probe_dtz(Position& pos, int* success);
int root_probe(Position& pos, Value& TBScore);
int root_probe_wdl(Position& pos, Value& TBScore);

#endif
