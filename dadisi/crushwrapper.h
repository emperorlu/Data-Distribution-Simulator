// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef __CRUSH_WRAPPER_H
#define __CRUSH_WRAPPER_H

#define BUG_ON(x) assert(!(x))


//extern "C" {
#include "crush.h"
#include "hash.h"
#include "mapper.h"
#include "Builder.h"
//}

#include <stdlib.h>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <iostream> //for testing, remove

#define IS_ERR(n) n == -1

using namespace std;

class CrushWrapper {
public:
  struct crush_map *crush;
  std::map<int, string> type_map; /* bucket/device type names */
  std::map<int, string> name_map; /* bucket/device names */
  std::map<int, string> rule_name_map;

  /* reverse maps */
  bool have_rmaps;
  std::map<string, int> type_rmap, name_rmap, rule_name_rmap;

private:
  void build_rmaps() {
    if (have_rmaps) return;
    build_rmap(type_map, type_rmap);
    build_rmap(name_map, name_rmap);
    build_rmap(rule_name_map, rule_name_rmap);
    have_rmaps = true;
  }
  void build_rmap(map<int, string> &f, std::map<string, int> &r) {
    r.clear();
    for (std::map<int, string>::iterator p = f.begin(); p != f.end(); p++)
      r[p->second] = p->first;
  }

public:
  CrushWrapper() : crush(0), have_rmaps(false) {}
  ~CrushWrapper() {
    if (crush) {
        crush_destroy(crush);
    }
  }

  /* building */
  void create() {
    if (crush) crush_destroy(crush);
    crush = crush_create();
  }

  // bucket types
  int get_num_type_names() {
    return type_map.size();
  }
  int get_type_id(const char *s) {
    string name(s);
    build_rmaps();
    if (type_rmap.count(name))
      return type_rmap[name];
    return 0;
  }
  const char *get_type_name(int t) {
    if (type_map.count(t))
      return type_map[t].c_str();
    return 0;
  }
  void set_type_name(int i, const char *n) {
    string name(n);
    type_map[i] = name;
    if (have_rmaps)
      type_rmap[name] = i;
  }

  // item/bucket names
  int get_item_id(const char *s) {
    string name(s);
    build_rmaps();
    if (name_rmap.count(name))
      return name_rmap[name];
    return 0;  /* hrm */
  }
  const char *get_item_name(int t) {
    if (name_map.count(t))
      return name_map[t].c_str();
    return 0;
  }
  void set_item_name(int i, const char *n) {
    string name(n);
    name_map[i] = name;
    if (have_rmaps)
      name_rmap[name] = i;
  }

  // rule names
  int get_rule_id(const char *n) {
    string name(n);
    build_rmaps();
    if (rule_name_rmap.count(name))
      return rule_name_rmap[name];
    return 0;  /* hrm */
  }
  const char *get_rule_name(int t) {
    if (rule_name_map.count(t))
      return rule_name_map[t].c_str();
    return 0;
  }
  void set_rule_name(int i, const char *n) {
    string name(n);
    rule_name_map[i] = name;
    if (have_rmaps)
      rule_name_rmap[name] = i;
  }

  /*** devices ***/
  int get_max_devices() {
    if (!crush) return 0;
    return crush->max_devices;
  }


  /*** rules ***/
private:
  //VDRIVE::Distributor *dist;
  crush_rule *get_rule(unsigned ruleno) {
    if (!crush) return (crush_rule *)(-1);
    if (ruleno >= crush->max_rules)
      return 0;
    return crush->rules[ruleno];
  }
  crush_rule_step *get_rule_step(unsigned ruleno, unsigned step) {
    crush_rule *n = get_rule(ruleno);
    if (!n) return (crush_rule_step *)(-1);
    if (step >= n->len) return (crush_rule_step *)(-1);
    return &n->steps[step];
  }

public:
  /* accessors */
  int get_max_rules() {
    if (!crush) return 0;
    return crush->max_rules;
  }
  bool rule_exists(unsigned ruleno) {
    if (!crush) return false;
    if (ruleno < crush->max_rules &&
	crush->rules[ruleno] != NULL)
      return true;
    return false;
  }
  

  /* modifiers */
  int add_rule(int len, int pool, int type, int minsize, int maxsize, int ruleno) {
    if (!crush) return -1;
    crush_rule *n = crush_make_rule(len, pool, type, minsize, maxsize);
    ruleno = crush_add_rule(crush, n, ruleno);
    return ruleno;
  }
  int set_rule_step(unsigned ruleno, unsigned step, int op, int arg1, int arg2) {
    if (!crush) return -1;
    crush_rule *n = get_rule(ruleno);
    if (!n) return -1;
    crush_rule_set_step(n, step, op, arg1, arg2);
    return 0;
  }
  int set_rule_step_take(unsigned ruleno, unsigned step, int val) {
    return set_rule_step(ruleno, step, CRUSH_RULE_TAKE, val, 0);
  }
  int set_rule_step_choose_firstn(unsigned ruleno, unsigned step, int val, int type) {
    return set_rule_step(ruleno, step, CRUSH_RULE_CHOOSE_FIRSTN, val, type);
  }
  int set_rule_step_choose_indep(unsigned ruleno, unsigned step, int val, int type) {
    return set_rule_step(ruleno, step, CRUSH_RULE_CHOOSE_INDEP, val, type);
  }
  int set_rule_step_choose_leaf_firstn(unsigned ruleno, unsigned step, int val, int type) {
    return set_rule_step(ruleno, step, CRUSH_RULE_CHOOSE_LEAF_FIRSTN, val, type);
  }
  int set_rule_step_choose_leaf_indep(unsigned ruleno, unsigned step, int val, int type) {
    return set_rule_step(ruleno, step, CRUSH_RULE_CHOOSE_LEAF_INDEP, val, type);
  }
  int set_rule_step_emit(unsigned ruleno, unsigned step) {
    return set_rule_step(ruleno, step, CRUSH_RULE_EMIT, 0, 0);
  }



  /** buckets **/
private:
  crush_bucket *get_bucket(int id) {
    if (!crush) return (crush_bucket *)(-1);
    int pos = -1 - id;
    if (pos >= crush->max_buckets) return 0;
    return crush->buckets[pos];
  }

public:
  int get_max_buckets() {
    if (!crush) return -1;
    return crush->max_buckets;
  }
  int get_next_bucket_id() {
    if (!crush) return -1;
    return crush_get_next_bucket_id(crush);
  }

  /* modifiers */
  int add_bucket(int bucketno, int alg, int hash, int type, int size,
		 int *items, int *weights) {
    crush_bucket *b = crush_make_bucket(alg, hash, type, size, items, weights);
    return crush_add_bucket(crush, bucketno, b);
  }

  void finalize() {
    crush_finalize(crush);
  }

  void set_max_devices(int m) {
    crush->max_devices = m;
  }

  int find_rule(int pool, int type, int size) {
    if (!crush) return -1;
    return crush_find_rule(crush, pool, type, size);
  }

  void do_rule(VDRIVE::Distributor *dist, int rule, int64_t x, vector<int>& out, int maxout, int forcefeed, vector<uint32_t>& weight) const {
    int rawout[maxout];
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_mutex_lock(&mutex);
    int numrep = crush_do_rule(dist, crush, rule, x, rawout, maxout, forcefeed, &weight[0]);
    pthread_mutex_unlock(&mutex);

    out.resize(numrep);
    for (int i=0; i<numrep; i++)
      out[i] = rawout[i];
  }




};


#endif
