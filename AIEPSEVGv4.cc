#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME EPSEVGv4


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

  const int DWARF_HEALTH = 100;
  const int WIZARD_HEALTH = 50;


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
    for (int k = 0; k < 8; ++k) {
      Pos d = p + Dir(k) + Dir(k);
      if (not pos_ok(d) or cell(d).id == -1) continue;
      UnitType t = unit(cell(d).id).type;
      if (t == Troll) return true;
      if (t == Balrog) return true;
    }
    for (int k = 0; k < 8; ++k) {
      Pos d = p + Dir(k) + Dir(k) + Dir(k);
      if (not pos_ok(d) or cell(d).id == -1) continue;
      UnitType t = unit(cell(d).id).type;
      //if (t == Troll) return true;
      if (t == Balrog) return true;
    }
    return false;
  }

  bool heals_ally(Pos p) {
    for (int i = 0; i < 8; i += 2) {
        Pos np = p + Dir(i);
        if (pos_ok(np) and cell(np).id != -1 and unit(cell(np).id).player == me() and unit(cell(np).id).type == Dwarf and unit(cell(np).id).health < DWARF_HEALTH) return true;
        if (pos_ok(np) and cell(np).id != -1 and unit(cell(np).id).player == me() and unit(cell(np).id).type == Wizard and unit(cell(np).id).health < WIZARD_HEALTH) return true;
    }
    return false;
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
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
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
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
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
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
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

  bool ally_to_heal_near(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    CostsMap parent(rows(), vector<WPos>(cols()));
    queue<WPos> q;

    q.push({origin, 0});
    visited[origin.i][origin.j] = true;

    while(not q.empty()) {
      Pos p = q.front().p;
      int cost= q.front().cost; q.pop();
      if (cost > 5) continue;
      for (int k = 0; k < 8; k += 2) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and not is_reserved(np) and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
          visited[np.i][np.j] = true;
          if (cell(np).id != -1 and heals_ally(np)) {
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

  Dir heal_near_ally(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    PosMap parent(rows(), vector<Pos>(cols()));
    queue<Pos> q;

    q.push(origin);
    visited[origin.i][origin.j] = true;
    while(not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int k = 0; k < 8; k += 2) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;
          if (cell(np).id != -1 and heals_ally(np)) {
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
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
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

  Dir follow_ally(Pos origin) {
    BoolMap visited(rows(), vector<bool>(cols(), false));
    PosMap parent(rows(), vector<Pos>(cols()));
    queue<Pos> q;

    q.push(origin);
    visited[origin.i][origin.j] = true;
    while(not q.empty()) {
      Pos p = q.front(); q.pop();
      for (int k = 0; k < 8; k += 2) {
        Pos np = p + Dir(k);
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
          visited[np.i][np.j] = true;
          parent[np.i][np.j] = p;
          if (cell(np).id != -1 and unit(cell(np).id).type == Dwarf and unit(cell(np).id).player == me()) {
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
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
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
        if (pos_ok(np) and not visited[np.i][np.j] and (cell(np).type == Cave or cell(np).type == Outside) and not is_deadly(np, cell(origin).id)) {
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
      /*if (balrog_is_near(p) and cell(p).type != Outside) {
        d = find_way_outside(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
      } 
      else */
      if (dwarf_is_near(p)) {
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
      Dir d = None;
      if (ally_to_heal_near(p)) {
        d = heal_near_ally(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
      }
      else {
        d = follow_ally(p);
        if (cell(p+d).id == -1) reserved_positions[(p+d).i][(p+d).j] = true;
        command(id, d);
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
