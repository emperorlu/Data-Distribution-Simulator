
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ostream>
#include <iostream>


#include<vector>
#include<list>
#include<string>
#include <set>
#include <map>
#include "Distributor.h"
#include "crushwrapper.h"

#define CEPH_PG_TYPE_REP     1
#define CEPH_PG_TYPE_RAID4   2
#define CEPH_PG_POOL_VERSION 2

using namespace std;

int main(int argc, char** argv) {
    cout << "test" << endl;
    return 0;
}
/*

int main(int argc, char** argv) {

    int num_osds = 32;
    int ruleno;
    vector<int> disk_items;
    vector<int> disk_weights;
    vector<__u32> disk_weights2;
    CrushWrapper crush;

    crush.create();

    // create layers

    crush.set_type_name(1, "domain");
    crush.set_type_name(2, "pool");

    int minrep = 1;
    int ndom   = 10;

    for (int i =0 ; i < num_osds; i++) {
        disk_items.push_back(i);
        disk_weights.push_back(0x10000);
        disk_weights2.push_back(0x10000);
    }

    if (ndom > 1 && num_osds >= ndom *3 && num_osds > 8) {
        int ritems[ndom];
        int rweights[ndom];

        int nper = ((num_osds -1) / ndom) +1;

        int o = 0;
        for (int i= 0 ; i < ndom; i++) {
            int items[nper], weights[nper];
            int j;
            rweights[i] = 0;
            for (j = 0; j < nper; j++, o++) {
                if (o == num_osds) break;
                items[j] = o;
                weights[j]= disk_weights[o];
                rweights[i] += weights[j];
            }

            crush_bucket *domain = crush_make_bucket(CRUSH_BUCKET_UNIFORM, CRUSH_HASH_DEFAULT, 1, j, items, weights);
            ritems[i] = crush_add_bucket(crush.crush,0,domain);
            char bname[10];
            snprintf(bname, sizeof(bname), "dom%d", i);
            crush.set_item_name(ritems[i], bname);
        }

        // root
        crush_bucket *root = crush_make_bucket(CRUSH_BUCKET_STRAW, CRUSH_HASH_DEFAULT, 2, ndom, ritems, rweights);
        int rootid = crush_add_bucket(crush.crush, 0, root);
        crush.set_item_name(rootid, "root");

        cout << "root id = " << rootid << endl;
        int ruleset=1;
        crush_rule *rule = crush_make_rule(3, ruleset, CEPH_PG_TYPE_REP, 2, 2);
        crush_rule_set_step(rule, 0, CRUSH_RULE_TAKE, rootid, 0);
        crush_rule_set_step(rule, 1, CRUSH_RULE_CHOOSE_LEAF_FIRSTN, CRUSH_CHOOSE_N, 1);
        crush_rule_set_step(rule, 2, CRUSH_RULE_EMIT, 0, 0);
        ruleno = crush_add_rule(crush.crush, rule, -1);
        crush.set_rule_name(ruleno, "data");
    }
    else {
        // one bucket
        int items[num_osds];
        int weights[num_osds];
        for (int i =0 ; i < num_osds; i++) {
            items[i] = i;
            weights[i] = disk_weights[i];
        }

        crush_bucket *root = crush_make_bucket(CRUSH_BUCKET_STRAW, CRUSH_HASH_DEFAULT, 1, num_osds, items, weights);
        
        int rootid = crush_add_bucket(crush.crush, 0, root);
        crush.set_item_name(rootid, "root");

    }

    

    crush.finalize();

  //  void do_rule(int rule, int x, vector<int>& out, int maxout, int forcefeed,vector<__u32>& weight) {


    int o[100];
    memset(o, 0, 100*sizeof(o[0]));
    vector<int> out;
    for (int i=0; i<32; i++) {
        crush.do_rule(ruleno, i, out, 3, -1, disk_weights2);
        cout << out[0] << " " << out[1] << " " << out[2] << endl;;
        for (int j=0; j<3; j++)
          o[out[j]]++;
    }
 
    int sum = 0;
    for (int i=0; i< num_osds; i ++) {
        sum += o[i];
        printf("%2d : %d\n", i, o[i]);
    }
    cout << "sum = " << sum << endl;



    return (EXIT_SUCCESS);
}


    int type    = 1;
    int rootid  = 0;
    for (vector<layer_t>::iterator p = layers.begin(); p != layers.end(); p++, type++) {
        layer_t &l = *p;
        cout << "layer " << type << " " << l.name << " " <<  l.buckettype << " " << l.size << endl;
        crush.set_type_name(type, l.name);

        // add items
        vector<int> cur_items;
        vector<int> cur_weights;
        unsigned lower_pos = 0;

        int i = 0;
        while (1) {
            if (lower_pos == disk_items.size()) break;  // finished
            int items[num_osds];
            int weights[num_osds];
            int weight = 0;
            int j = 0;
            for (j = 0; j < l.size || l.size == 0; j++) {
                if (lower_pos == disk_items.size()) break;
                items[j] = disk_items[lower_pos];
                weights[j] = disk_weights[lower_pos];
                weight += weights[j];
                lower_pos++;
                //cout << " item " << items[j] << ", weight " << weights[j] << endl;
            }

            crush_bucket *b = crush_make_bucket(l.buckettype, CRUSH_HASH_DEFAULT, type, j, items, weights);
            int id = crush_add_bucket(crush.crush, 0, b);
            rootid = id;

            char format[20];
            if (l.size)
              snprintf(format, sizeof(format), "%s%%d", l.name);
            else
              strcpy(format, l.name);
            char name[20];
            snprintf(name, sizeof(name), format, i);
            crush.set_item_name(id, name);

            //cout << " in bucket " << id << " '" << name << "' size " << j << " weight " << weight << endl;

            cur_items.push_back(id);
            cur_weights.push_back(weight);
            i++;

        }
        disk_items.swap(cur_items);
        disk_weights.swap(cur_weights);
    }
    


 *
 *
 *
 *
int main()
{
  int sub[10];
  int subw[10];
  int i, j;
  int d;
  int o[100];
  int root;
  int ruleno;
  int r[10];

  int uw[10] = { 1000, 1000, 500, 1000, 2000, 1000, 1000, 3000, 1000, 500 };

  struct crush_bucket *b;
  struct crush_rule *rule;

  struct crush_map *map = crush_create();

  d = 0;
  for (i=0; i<10; i++) {
    for (j=0; j<10; j++)
      o[j] = d++;
    b = (struct crush_bucket*)crush_make_uniform_bucket(1, 10, o, uw[i]);
    sub[i] = crush_add_bucket(map, b);
    subw[i] = b->weight;
    printf("make bucket %d weight %d\n", sub[i], subw[i]);
  }

  root = crush_add_bucket(map, (struct crush_bucket*)crush_make_tree_bucket(2, 10, sub, subw));

  rule = crush_make_rule(4);
  crush_rule_set_step(rule, 0, CRUSH_RULE_TAKE, root, 0);
  crush_rule_set_step(rule, 1, CRUSH_RULE_CHOOSE_FIRSTN, 3, 1);
  crush_rule_set_step(rule, 2, CRUSH_RULE_CHOOSE_FIRSTN, 1, 0);
  crush_rule_set_step(rule, 3, CRUSH_RULE_EMIT, 0, 0);
  ruleno = crush_add_rule(map, -1, rule);

  crush_finalize(map);
  printf("built\n");

  // test
  memset(o, 0, 100*sizeof(o[0]));
  for (i=0; i<1000000; i++) {
    crush_do_rule(map, ruleno, i, r, 3, -1);
    //printf("%d %d %d\n", r[0], r[1], r[2]);
    for (j=0; j<3; j++)
      o[r[j]]++;
  }

  for (i=0; i<100; i += 10)
    printf("%2d : %d\n", i, o[i]);

  return 0;
}
*/