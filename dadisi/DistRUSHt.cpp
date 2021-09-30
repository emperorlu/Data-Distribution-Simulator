/* 
 * File:   DistRUSHt.cpp
 * Author: ywkang
 * 
 * Created on April 7, 2011, 4:55 PM
 */

#include "DistRUSHt.h"

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>

#include<sstream>
#include<iostream>
#include<sys/time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits>
#include "Prime.h"

using namespace xercesc;
using namespace VDRIVE;
using namespace std;


DistRUSHt::DistRUSHt(int argc, char **argv) {
    int argInd;
    for (argInd = 0; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            throw 1;
        } else if (strcmp(argv[argInd], "-copies") == 0) {
            argInd++;
            if (argInd == argc) {
                throw "-copies requires a number";
            }
            else {
                this->copies = atoi(argv[argInd]);
                cout << "number of copies = " << this->copies << endl;
            }
        } else {
            std::cout << "unknown option: " << argv[argInd] << "\n";
            throw 1;
        }
    }
} 

DistRUSHt::DistRUSHt(const DistRUSHt& orig) {
}

DistRUSHt::DistRUSHt(xercesc::DOMElement* data) {
    std::istringstream isst;
    xercesc::DOMNode *n, *m;
    XMLCh* xmlString;
    char* sysString;

    xmlString = XMLString::transcode("copies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->copies;
    isst.str("");
    isst.clear();

    //cout << this->copies << endl;

    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    std::list<Disk*>* disks = new std::list<Disk*>();

    for (n = data->getFirstChild(); n != 0; n = n->getNextSibling()) {
        if (n->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement* e = (xercesc::DOMElement*) n;
            sysString = XMLString::transcode(e->getTagName());
            if (strcmp(sysString, "disks") == 0) {
                for (m = e->getFirstChild(); m != 0; m = m->getNextSibling()) {
                    if (m->getNodeType() == DOMNode::ELEMENT_NODE) {
                        Disk *d = new Disk((xercesc::DOMElement*)m);
                        //cout << "disk " << d->getId() << ", " << d->getCapacity() << endl;
                        disks->push_back(d);
                    }
                }
            }
            XMLString::release(&sysString);
        }
    }

    setDisks(disks);
}
DistRUSHt::~DistRUSHt() {

}
void DistRUSHt::init_subtree_node(SubCluster *cluster, Node *to_init,
			      int node_index) {
  to_init->data.sub_cluster = *cluster;
  to_init->flavor = SUBCLUSTER;
  to_init->weight = cluster->server_count * cluster->weight;
  to_init->index = node_index;
}

void DistRUSHt::init_internal_node(Node *left, Node* right, Node *to_init,
			       int node_index) {
  to_init->data.node.left = left;
  to_init->data.node.left_weight = left->weight;
  if (right != NULL) {
    to_init->data.node.right = right;
    to_init->data.node.right_weight = right->weight;
  }
  to_init->flavor = NODE;
  to_init->weight =
    to_init->data.node.left_weight + to_init->data.node.right_weight;
  to_init->normalized_weight =
    (double)to_init->data.node.left_weight /
    ((double)to_init->data.node.left_weight +
     (double)to_init->data.node.right_weight);

  to_init->index = node_index;
}

Node * DistRUSHt::construct_tree(SubCluster **clusters, int cluster_count) {
  int i, old_node_count, new_node_count, node_index;
  Node *old_nodes, *new_nodes;

  node_index = 0;
  new_node_count = cluster_count;
  new_nodes = (Node *) calloc(new_node_count, sizeof(Node));

  for (i = 0; i < new_node_count; i++) {
    init_subtree_node(clusters[i], new_nodes+i, node_index++);
  }

  while (new_node_count > 1) {
    old_nodes = new_nodes;
    old_node_count = new_node_count;

    if (old_node_count % 2) {
      new_node_count = old_node_count / 2 + 1;
    } else {
      new_node_count = old_node_count / 2;
    }

    new_nodes = (Node *) calloc(new_node_count, sizeof(Node));

    for (i = 0; i < old_node_count; i+=2) {
      init_internal_node(old_nodes+i,
			 ((i+1 < old_node_count) ? old_nodes+i+1 : NULL),
			 new_nodes + i/2, node_index++);
    }

  }

  return new_nodes;
}

void DistRUSHt::setDisks(std::list<Disk*>* disks) {

    map<int, int> cluster_index;
    this->weights.clear();
    this->clusters.clear();
    cout << "set disks" << endl;
    if (alldisks.size() != 0) {
        for (vector<Disk*>::iterator dt = alldisks.begin(); dt != alldisks.end(); ++dt) {
            Disk* disk = *dt;
            if (disk) {
                delete disk;
            }
        }
        this->alldisks.clear();
    }

    int index = 0;
    int64_t max_weight = 0;
    for (list<Disk*>::iterator it = (*disks).begin(); it != (*disks).end(); ++it) {
        Disk* d = *it;
        int64_t cap = d->getCapacity();

        if (cluster_index.count(cap) == 0) {
            cluster_index[cap] = index;
            clusters.insert(pair<int, list<Disk*>* >(index, new list<Disk*>()));
            weights.push_back(cap);
            if (max_weight < cap) {
                max_weight = cap;
            }
            index++;
        }
        alldisks.push_back(new Disk(*d));
    }

   

    /*
    for (uint32_t i = 0; i < weights.size(); i++) {
        int64_t w = weights[i];
        double norm_weight = (double(w) / double(max_weight)) * 1000;
        weights[i] = int64_t(norm_weight);
    }
    */


    for (list<Disk*>::iterator it = (*disks).begin(); it != (*disks).end(); ++it) {
        Disk* d = *it;
        if (cluster_index.count(d->getCapacity()) == 0) {
            cout << "should not happen " << endl;
            throw "should not happen";
        }

        //cout << "cluster " << cluster_index[d->getCapacity()] << ", disk = " << d->getCapacity() << endl;

        clusters[cluster_index[d->getCapacity()]]->push_back(d);
    }

    double total_weight = 0.0;
    for (uint32_t i = 0; i < clusters.size(); i++) {
        if (clusters[i]->size() < this->copies) {
	  cout << clusters[i]->size() << "," << this->copies << endl;;
            cout << "configuration error : the number of disks in a cluster " << i << " is less than the number of replicas " << this->copies << endl;
            exit(1);
        }
        total_weight += weights[i] * clusters[i]->size();
    }

    double averageweight = total_weight / alldisks.size();
    cout << "average weight " << averageweight << endl;
    for (uint32_t i = 0; i < weights.size(); i++) {
      weights[i] = weights[i] / averageweight;
	//        int64_t w = weights[i
	//        double norm_weight = (double(w) / double(max_weight)) * 100000;
	//        weights[i] = int64_t(norm_weight);

      cout << " weight " << i << "," << weights[i] << endl;
    }
    SubCluster **subclusters = (SubCluster **)malloc(sizeof(SubCluster*) * clusters.size());
    
    for (uint32_t i = 0; i < clusters.size(); i++) {
        if (clusters[i]->size() < this->copies) {
            cout << "configuration error : the number of disks in a cluster " << i << " is less than the number of replicas " << this->copies << endl;
            exit(1);
        }

        SubCluster *s = new SubCluster();
        s->server_count = clusters[i]->size();
        s->weight = weights[i];

        subclusters[i] = s;
    }

    root = construct_tree(subclusters, clusters.size());


    cumulative_server_counts = get_cumulative_server_counts(subclusters, clusters.size());
    
    lowest_prime_index = get_lowest_prime_index(subclusters, clusters.size());

    cout << "set disks end-" << lowest_prime_index << endl;
    for (uint32_t i = 0; i < clusters.size(); i++) {
        delete subclusters[i];

    }


    free(subclusters);
    cout << "set disks end" << endl;
    // debugging..
    //cout << "total count = " << cumulative_server_counts[clusters.size()-1] + subclusters[clusters.size()-1]->server_count <<endl;
    //debug_tree(root);
};


void DistRUSHt::debug_tree(Node *root) {
  switch (root->flavor) {
  case SUBCLUSTER:
    printf("count: %d\t weight: %f\t index:%d\n",
	   root->data.sub_cluster.server_count,
	   root->weight, root->index);
    break;
  default:
    printf("\t\t\t weight: %f\t index:%d\n",
	   root->weight, root->index);
    debug_tree(root->data.node.left);
    if (root->data.node.right != NULL) {
      debug_tree(root->data.node.right);
    }
  }
}

int64_t * DistRUSHt::get_cumulative_server_counts(SubCluster **sub_clusters, int count) {
  int64_t *counts, i, total_servers;

  counts = (int64_t *) calloc (count, sizeof(int64_t));
  total_servers = 0;

  for (i = 0; i < count; i++) {
    counts[i] = total_servers;
    total_servers += sub_clusters[i]->server_count;
  }
  
  return counts;
}



int DistRUSHt::get_lowest_prime_index(SubCluster **sub_clusters, int sub_cluster_count) {
  int i, largest_sub_cluster, lowest_index;

  largest_sub_cluster = 0;
  for (i = 0; i < sub_cluster_count; i++) {
    if (largest_sub_cluster < sub_clusters[i]->server_count) {
      largest_sub_cluster = sub_clusters[i]->server_count;
    }
  }

  lowest_index = Prime::getLowestPrimeIndex(largest_sub_cluster);

  return lowest_index;

}

void DistRUSHt::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    this->copies = copies;
    setDisks(disks);
};

void DistRUSHt::setClusters(std::list<std::list<Disk*>*>* pclusters, list<uint32_t>* weights) {
}

int64_t DistRUSHt::getExtentsize() const {
    return 0;
}

int32_t DistRUSHt::getCopies() const {
    return (int32_t)this->copies;
}

std::list<Disk*>* DistRUSHt::getDisks() const {
    if (this->alldisks.size() == 0)
        return 0;

    std::list<Disk*> *resDisks = new std::list<Disk*>();

    for (vector<Disk*>::const_iterator dt = alldisks.begin(); dt != alldisks.end(); ++dt) {
        Disk* disk = *dt;
        if (disk) {
            resDisks->push_back(new Disk(*disk));
        }
    }

    return resDisks;
};

std::list<Disk*>* DistRUSHt::placeExtent(int64_t virtualVolumeId, int64_t position) {
    int64_t x = position;
    uint32_t replica_id;

    list<Disk*>* disks = new list<Disk*>();
    //cout << "place Extent" << x << endl;
    for (replica_id = 0; replica_id < copies; replica_id++) {
      int serverid = rusht(root, cumulative_server_counts,
			  x, replica_id);

       disks->push_back(new Disk(*alldisks[serverid]));
    }
    //cout << "place Extent done" << endl;
    return disks;
}


Node *DistRUSHt::get_subcluster(Node *root, int64_t key) {

  while (root->flavor != SUBCLUSTER) {

    if (hash(key, root->index, 0) < root->normalized_weight) {
      root = root->data.node.left;
    } else {
      root = root->data.node.right;
    }
  }

  return root;
}

int DistRUSHt::rusht(Node *root, int64_t *server_mapping, int64_t key, uint32_t replica_id) {

  Node * sub_cluster;
  int random_key;
  long random_prime;

  //  cout << "getsubcluster" << key << "," << replica_id << endl;
  sub_cluster = get_subcluster(root, key);
  
  //  cout << "subcluster" << sub_cluster->index << endl;
  random_key = hash(key, sub_cluster->index, 0) * 40031; /* this is just some prime number I chose... it doesn't even have to be prime. */
  //  cout << "random key " << random_key << endl;
  //  cout << hash(key, sub_cluster->index, 1) << endl;
  //  cout << Prime::getSize()-lowest_prime_index-1 << endl;
  //  cout << "getPrimebyIndex" << lowest_prime_index +
  //			round(hash(key, sub_cluster->index, 1)*
  //			      (Prime::getSize()-lowest_prime_index-1)) << endl;
  random_prime = Prime::getPrimeNumberByIndex(lowest_prime_index +
			round(hash(key, sub_cluster->index, 1)*
			      (Prime::getSize()-lowest_prime_index-1)));

  return server_mapping[sub_cluster->index] +
    ( key + random_key + replica_id * random_prime) %
    sub_cluster->data.sub_cluster.server_count;

}

xercesc::DOMElement* DistRUSHt::toXML(xercesc::DOMDocument* doc) const {
    DOMElement *result, *disksE;
    XMLCh *attr, *value;
    XMLCh *element_name;
    std::stringstream out;

    cout << "RUSH_t : loading configuration from XML " << endl;

    element_name = XMLString::transcode(DistRUSHt::getXMLRootType().c_str());
    result = doc->createElement(element_name);
    XMLString::release(&element_name);

    out.str(""); out.clear();
    out << this->copies;
    attr  = XMLString::transcode("copies");
    value = XMLString::transcode(out.str().c_str());
    result->setAttribute(attr, value);
    XMLString::release(&attr);
    XMLString::release(&value);

    element_name = XMLString::transcode("disks");
    disksE = doc->createElement(element_name);
    XMLString::release(&element_name);

    for (vector<Disk*>::const_iterator dt = alldisks.begin(); dt != alldisks.end(); ++dt) {
        Disk* d = *dt;
        disksE->appendChild(d->toXML(doc));
    }
    result->appendChild(disksE);

    return result;
}


/**
 *  hash function for RUSHp
 *      takes 3 parameters
 */
double DistRUSHt::hash(int64_t key1, int64_t key2, int64_t key3) const {

    std::stringstream buf;
    buf << "RUSHp::" << key1 << "::" << key2 << "::" << key3;
    std::string s = buf.str();
    uint64_t hash =  Distributor::hashFunctionInt64(&s);
    if (hash < 0) { hash = hash * -1; }
    return ((double)hash) / ((double)numeric_limits<uint64_t>::max()) ;
}

