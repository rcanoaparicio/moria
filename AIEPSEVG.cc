#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME EPSEVG


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /**
   * Types and attributes for your player can be defined here.
   */

  struct WPos {
    Pos p;
    int cost;
    friend bool operator < (const WPos &a, const WPos &b) { return a.cost > b.cost; }
    inline friend ostream& operator<< (ostream& os, const WPos& p) { return os << "[" << p.p << ", " << p.cost <<"]"; }
  };

  typedef vector< vector<bool> > BoolMap;
  typedef vector< vector<Pos> > PosMap;
  typedef vector< vector<int> > IntMap;
  typedef vector< vector<WPos> > CostsMap;
  typedef vector<int> VI;

  BoolMap reserved_positions;

  static IntMap costs;
  map<int, int> kind;

  Dir get_direction(const Pos &d, const Pos &o) {
    if (d.i < o.i) {
      if (d.j < o.j) return TL;
      if (d.j > o.j) return RT;
      return Top;
    }
    if (d.i > o.i) {
      if (d.j < o.j) return LB;
      if (d.j > o.j) return BR;
      return Bottom;
    }
    if (d.j < o.j) return Left;
    if (d.j > o.j) return Right;
    return None;
  }

  bool is_reserved(Pos p) {
    if (pos_ok(p)) return reserved_positions[p.i][p.j];
    return false;
  }

  bool is_deadly(Pos p, int id) {
    for (int k = 0; k < 8; ++k) {
      Pos d = p + Dir(k);
      if (not pos_ok(d) or cell(d).id == -1) continue;
      UnitType t = unit(cell(d).id).type;
      if (t == Orc or (t == Dwarf and unit(cell(d).id).player != me())) return unit(id).health < 30;
      else if (t == Troll) return true;
      else if (t == Balrog) return true;
    }
    return false;
  }

  Pos min_cost(const CostsMap &m) {
    WPos min = m[0][0];
    for (unsigned int i = 0; i < m.size(); ++i) {
      for (unsigned int j = 0; j < m[i].size(); ++j) {
        if (m[i][j] < min) min = m[i][j];
      }
    }
    return min.p;
  }

  int calculate_cost(Pos p, int id) {
    if (cell(p).id == -1) return cell(p).turns + 1;
    else return 1000;
  }

  bool heals_ally(Pos p) {
    for (int k = -1; k <= 1; ++k) {
      if (k == 0) continue;
      if (pos_ok(Pos(p.i, p.j + k)) and cell(Pos(p.i, p.j + k)).owner == me() and cell(Pos(p.i, p.j + k)).id != -1 and unit(cell(Pos(p.i, p.j + k)).id).type == Dwarf and unit(cell(Pos(p.i, p.j + k)).id).health < 100 ) return true;
      if (pos_ok(Pos(p.i + k, p.j)) and cell(Pos(p.i + k, p.j)).owner == me() and cell(Pos(p.i + k, p.j)).id != -1 and unit(cell(Pos(p.i + k, p.j)).id).type == Dwarf and unit(cell(Pos(p.i + k, p.j)).id).health < 100) return true;
    }
    return false;
  }

  Pos find_nearest_enemy(int id, int &distance) {
    vector< vector<int> > dist(rows(), vector<int>(cols(), -1));
    queue< Pos> q;

    Pos p = unit(id).pos;
    dist[p.i][p.j] = 0;

    q.push(p);
    while (not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int d = 0; d < 8; ++d) {
        Pos np = p + Dir(d);
        if (not pos_ok(np)) continue;
        if (dist[np.i][np.j] == -1) {
          dist[np.i][np.j] = dist[p.i][p.j] + 1;
          if (cell(np).id != -1) {

            UnitType t = unit(cell(np).id).type;

            if (t == Orc or t == Troll or t == Balrog or (t == Dwarf and cell(np).owner != me())) {
              distance = dist[np.i][np.j];
              return np;
            }
          }
          q.push(np);
        }
      }
    }
    return p;
  }



  Dir find_safe_dir(int id) {
    UnitType t = unit(id).type;
    Pos p = unit(id).pos;
    int d = -1;
    Pos e = find_nearest_enemy(id, d);

    if (t == Wizard) {
      if (p.i < e.i and cell(p+Top).id != -1) return Top;
      if (p.i > e.i and cell(p+Bottom).id != -1) return Bottom;
      if (p.j < e.j and cell(p+Left).id != -1) return Left;
      if (p.j > e.j and cell(p+Right).id != -1) return Right;
    }

    return None;
  }

  Pos wizards_bfs(int id, bool &found) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    PosMap parent(rows(), vector<Pos>(cols()));
    queue< Pos> q;

    Pos p = unit(id).pos;
    visited[p.i][p.j] = true;

    q.push(p);
    while (not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int d = 0; d < 8; d += 2) {
        Pos np = p + Dir(d);
        if (not pos_ok(np) or not(cell(np).type == Cave or cell(np).type == Outside) ) continue;
        if (not visited[np.i][np.j]) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;

          if (heals_ally(np)) {
            found = true;
            Pos orig = unit(id).pos;
            while (parent[np.i][np.j] != orig) {
              np = parent[np.i][np.j];
            }
            return np;
          }
          q.push(np);
        }
      }
    }
    found = false;
    return p;
  }

  bool balrog_is_near(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    CostsMap parent(rows(), vector<WPos>(cols()));
    queue<WPos> q;

    q.push({origin, 0});
    visited[origin.i][origin.j] = true;
    int inc = 1;
    if (unit(cell(origin).id).type == Wizard) ++inc;
    while(not q.empty()) {
      Pos p = q.front().p;
      int cost= q.front().cost; q.pop();
      if (cost > 4) continue;
      for (int k = 0; k < 8; k += inc) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j]) {
          visited[np.i][np.j] = true;
          if (cell(np).id != -1 and unit(cell(np).id).type == Balrog) return true;
          parent[np.i][np.j] = {p, cost};
          q.push({np, cost + 1});
        }
      }
    }

    return false;
  }

  bool orc_is_near(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    CostsMap parent(rows(), vector<WPos>(cols()));
    queue<WPos> q;

    q.push({origin, 0});
    visited[origin.i][origin.j] = true;

    int inc = 1;
    if (unit(cell(origin).id).type == Wizard) ++inc;
    while(not q.empty()) {
      Pos p = q.front().p;
      int cost= q.front().cost; q.pop();
      if (cost > 3) continue;
      for (int k = 0; k < 8; k += inc) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside)) {
          visited[np.i][np.j] = true;
          if (cell(np).id != -1 and unit(cell(np).id).type == Orc) {
            return true;
          }
          if (cell(np).id == -1 and not is_reserved(np)) {
            parent[np.i][np.j] = {p, cost};
            q.push({np, cost + 1});
          }
        }
      }
    }
    return false;
  }

  bool dwarf_is_near(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    CostsMap parent(rows(), vector<WPos>(cols()));
    queue<WPos> q;

    q.push({origin, 0});
    visited[origin.i][origin.j] = true;

    int inc = 1;
    if (unit(cell(origin).id).type == Wizard) ++inc;
    while(not q.empty()) {
      Pos p = q.front().p;
      int cost= q.front().cost; q.pop();
      if (cost > 3) continue;
      for (int k = 0; k < 8; k += inc) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside)) {
          visited[np.i][np.j] = true;
          if (cell(np).id != -1 and unit(cell(np).id).type == Dwarf and unit(cell(np).id).player != me()) {
            return true;
          }
          if (cell(np).id == -1 and not is_reserved(np)) {
            parent[np.i][np.j] = {p, cost};
            q.push({np, cost + 1});
          }
        }
      }
    }
    return false;
  }

  bool wizard_is_near(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    CostsMap parent(rows(), vector<WPos>(cols()));
    queue<WPos> q;

    q.push({origin, 0});
    visited[origin.i][origin.j] = true;

    while(not q.empty()) {
      Pos p = q.front().p;
      int cost= q.front().cost; q.pop();
      if (cost > 3) continue;
      for (int k = 0; k < 8; ++k) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside)) {
          visited[np.i][np.j] = true;
          if (cell(np).id != -1 and unit(cell(np).id).type == Wizard and unit(cell(np).id).player != me()) {
            return true;
          }
          if (cell(np).id == -1 and not is_reserved(np)) {
            parent[np.i][np.j] = {p, cost};
            q.push({np, cost + 1});
          }
        }
      }
    }
    return false;
  }

  Dir find_way_outside(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    PosMap parent(rows(), vector<Pos>(cols()));
    queue<Pos> q;

    q.push(origin);
    visited[origin.i][origin.j] = true;
    int inc = 1;
    if (unit(cell(origin).id).type == Wizard) ++inc;
    while(not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int k = 0; k < 8; k += inc) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and cell(np).id == -1 and not is_reserved(np) and not is_deadly(np, cell(origin).id)) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;
          if (cell(np).type == Outside) {
            while (parent[np.i][np.j] != origin) {
              np = parent[np.i][np.j];
            }
            return get_direction(np, origin);
          }
          if (cell(np).type == Cave) {
            q.push(np);
          }
        }
      }
    }

    return None;
  }

  Dir attack_dwarf(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    PosMap parent(rows(), vector<Pos>(cols()));
    queue<Pos> q;

    q.push(origin);
    visited[origin.i][origin.j] = true;
    while(not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int k = 0; k < 8; ++k) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside)) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;
          if (cell(np).id != -1 and unit(cell(np).id).type == Dwarf and unit(cell(np).id).player != me()) {
            while (parent[np.i][np.j] != origin) {
              np = parent[np.i][np.j];
            }
            return get_direction(np, origin);
          }
          if (cell(np).id == -1 and not is_reserved(np)) {
            q.push(np);
          }
        }
      }
    }

    return None;
  }

  Dir attack_orc(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    PosMap parent(rows(), vector<Pos>(cols()));
    queue<Pos> q;
    visited[origin.i][origin.j] = true;
    q.push(origin);
    while(not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int k = 0; k < 8; ++k) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside)) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;
          if (cell(np).id != -1 and unit(cell(np).id).type == Orc) {
            while (parent[np.i][np.j] != origin) {
              np = parent[np.i][np.j];
            }
            return get_direction(np, origin);
          }
          if (cell(np).id == -1 and not is_reserved(np)) {
            q.push(np);
          }
        }
      }
    }

    return None;
  }

  Dir attack_wizard(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    PosMap parent(rows(), vector<Pos>(cols()));
    queue<Pos> q;
    visited[origin.i][origin.j] = true;
    q.push(origin);
    while(not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int k = 0; k < 8; ++k) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) ) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;
          if (cell(np).id != -1 and unit(cell(np).id).type == Wizard and unit(cell(np).id).player != me()) {
            while (parent[np.i][np.j] != origin) {
              np = parent[np.i][np.j];
            }
            return get_direction(np, origin);
          }
          if (cell(np).id == -1 and not is_reserved(np)) {
            q.push(np);
          }
        }
      }
    }

    return None;
  }

  Dir find_treasure(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    CostsMap costs = CostsMap(rows(), vector<WPos>(cols()));
    PosMap parent(rows(), vector<Pos>(cols()));
    priority_queue<WPos> pq;

    visited[origin.i][origin.j] = true;

    pq.push({origin, 0});
    while (not pq.empty()) {
      WPos wp = pq.top(); pq.pop();
      Pos p = wp.p;
      for (int d = 0; d < 8; ++d) {
        int cost = wp.cost;
        Pos np = p + Dir(d);
        if (not pos_ok(np) or not(cell(np).type == Cave or cell(np).type == Outside or cell(np).type == Rock)) continue;
        if (not visited[np.i][np.j] and cell(np).id == -1 and not is_deadly(np, cell(origin).id)) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;
          if (cell(np).treasure) {
            while (parent[np.i][np.j] != origin) {
              np = parent[np.i][np.j];
            }
            return get_direction(np, origin);
          }
          if (not is_reserved(np)) {
            costs[np.i][np.j] = {p, cost + 1 + cell(np).turns};
            pq.push({np, cost + 1 + cell(np).turns});
          }
        }
      }
    }

    return None;
  }

  void move_dwarves() {
    vector<int> D = dwarves(me());
    int count = 0;
    for (int id : D) {
      Pos p = unit(id).pos;
      Dir d = None;
      if (balrog_is_near(p) and cell(p).type != Outside) {
        d = find_way_outside(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
      } 
      else if (dwarf_is_near(p)) {
        d = attack_dwarf(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
      }
      else if (wizard_is_near(p)) {
        d = attack_wizard(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
      } 
      else if (orc_is_near(p)) {
        d = attack_orc(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
      }
      else {
        d = find_treasure(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
      }
      ++count;
    }
  }

  void move_wizards() {
    vector<int> W = wizards(me());
    for (int id : W) {
      Pos p = unit(id).pos;
      bool found;
      if (balrog_is_near(p)) command(id, find_way_outside(p));
      else if (heals_ally(p)) command(id, None);
      else {
        Pos np = wizards_bfs(id, found);
        if (found and p.i < np.i) command(id, Bottom);
        else if (found and p.i > np.i) command(id, Top);
        else if (found and p.j < np.j) command(id, Right);
        else if (found and p.j > np.j) command(id, Left);
        else command(id, find_safe_dir(id));
      }
    }
  }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
    reserved_positions.clear();
    reserved_positions = BoolMap(rows(), vector<bool>(cols(), false));
    move_dwarves();
    move_wizards();
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
