/* 
 * File:   DistRUSHt.h
 * Author: ywkang
 *
 * Created on April 7, 2011, 4:55 PM
 */

#ifndef DISTRUSHT_H
#define	DISTRUSHT_H

#include "Distributor.h"
#include <set>
#include <map>
#include <list>
#include <iostream>
#include <math.h>
#include <stdio.h>
using namespace std;

namespace VDRIVE {
    typedef struct SubCluster_s {
      int server_count;
      double weight;
    } SubCluster;

    typedef struct Node_s Node;

    typedef struct InternalNode_s {
      Node *left;
      double left_weight;
      Node *right;
      double right_weight;
    } InternalNode;

    typedef enum { SUBCLUSTER, NODE } NodeFlavor;

    struct Node_s {
      union {
        SubCluster sub_cluster;
        InternalNode node;
      } data;
      NodeFlavor flavor;
      int index;
      double weight;
      double normalized_weight;
    };
    class DistRUSHt : public Distributor{
    public:
        DistRUSHt(int argc, char **argv) ;
        DistRUSHt(const DistRUSHt& orig);
        DistRUSHt(xercesc::DOMElement* data);
        virtual ~DistRUSHt();
        double hash(int64_t key1, int64_t key2, int64_t key3) const;
        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position) ;
        virtual void setClusters(std::list<std::list<Disk*>*>* clusters, std::list<uint32_t>* weights);
        virtual void setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies);
        virtual void setDisks(std::list<Disk*>* disks);
        virtual std::list<Disk*>* getDisks() const;
        virtual int64_t getExtentsize() const;
        virtual int32_t getCopies() const;
        virtual xercesc::DOMElement* toXML(xercesc::DOMDocument* doc) const;
        static std::string getXMLRootType() {
            return std::string("Rusht");
        }
    private:
        uint32_t copies;
        int32_t j;   // cluster id
        int lowest_prime_index;
        int64_t total_server_count;
        int64_t *cumulative_server_counts;
        map<int64_t, list<Disk*>* > clusters;
        Node *root;
        //map<int32_t, int32_t > weights;
        vector<double> weights;
        vector<Disk*> alldisks;

        Node * construct_tree(SubCluster **clusters, int cluster_count);
        void init_subtree_node(SubCluster *cluster, Node *to_init,
			      int node_index);
        void init_internal_node(Node *left, Node* right, Node *to_init,
			       int node_index);
        int get_lowest_prime_index(SubCluster **sub_clusters, int sub_cluster_count);
        int64_t * get_cumulative_server_counts(SubCluster **sub_clusters, int count);

        int rusht(Node *root, int64_t *server_mapping, int64_t key, uint32_t replica_id);
        Node *get_subcluster(Node *root, int64_t key);

        void debug_tree(Node* root);
    };
    
}
#endif	/* DISTRUSHT_H */

