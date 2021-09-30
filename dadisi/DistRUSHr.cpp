/* 
 * File:   DistRUSHr.cpp
 * Author: ywkang
 *
 * Created on April 7, 2011, 5:06 PM
 */

#include "DistRUSHr.h"

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

#define randrange(g,n) (int)(g()*(n))

using namespace xercesc;
using namespace VDRIVE;
using namespace std;

static int *n_choices;

DistRUSHr::DistRUSHr(int argc, char **argv) {
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

DistRUSHr::DistRUSHr(const DistRUSHr& orig) {
}

DistRUSHr::DistRUSHr(xercesc::DOMElement* data) {
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
DistRUSHr::~DistRUSHr() {
    uninit_choices();
    for (vector<Disk*>::iterator dt = alldisks.begin(); dt != alldisks.end(); ++dt) {
        Disk* disk = *dt;
        if (disk) {
            delete disk;
        }
    }
    for (uint32_t i = 0 ; i < groups.size(); i++) {
        free(groups[i]);
        groups[i] = NULL;
    }
}


void DistRUSHr::init_choices(int n) {
  int i;
  n_choices = (int *)calloc(n, sizeof(int));
  for (i = 0; i < n; i++) {
    n_choices[i] = i;
  }
}

void DistRUSHr::uninit_choices() {
  free(n_choices);
}

void DistRUSHr::draw_k_of_n(int *results, int k, int n, long &x, long &y, long &z) {
  int i, choice, tmp;

  for(i=0; i < k; i++) {
    choice = (int) (wh_random(x, y, z) * (n-i)); //randrange(g,n-i);
    tmp = n_choices[n-1-i];
    n_choices[n-1-i] = n_choices[choice];
    n_choices[choice] = tmp;
  }

  for(i=0; i < k; i++) {
    results[i]= n_choices[n-k+i];
  }

  for(i=0; i < k; i++) {
    choice = n_choices[n-k+i];
    if (choice < n-k){
      n_choices[choice] = choice;
    }
    n_choices[n-k+i] = n-k+i;
  }
}

int DistRUSHr::i_sample(int r, double n, double N, double w,
	   long &x, long &y, long &z) {

  int positives, i;
  double u;

  if (w == 0) {
    return 0;
  }

  positives = 0;

  N = N / w;
  n = n / w;

  for (i = 0; i < r; i++ ) {
    u = wh_random(x, y, z);
    if (u < (n/N)) {
      positives++;
      n--;
    }
    N--;
  }

  return positives;

}

void DistRUSHr::setDisks(std::list<Disk*>* disks) {

    map<int, int> cluster_index;
    this->weights.clear();
    this->clusters.clear();
    for (uint32_t i = 0 ; i < groups.size(); i++) {
        free(groups[i]);
        groups[i] = NULL;
    }
    this->groups.clear();

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
    /*
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
    */
    for (uint32_t i = 0; i < clusters.size(); i++) {
        if (clusters[i]->size() < this->copies) {
            cout << "configuration error : the number of disks in a cluster " << i << " is less than the number of replicas " << this->copies << endl;
            exit(1);
        }
    }
    {
      // cout << "start avg " << averageweight << ", total weight " << total_weight << "," << ", disks " << alldisks.size() << endl;
       // init group info
        uint32_t i = 0;
        int32_t number_of_servers = 0;
        double total_weight = 0, average_weight = 0;
        for (i=0; i < clusters.size(); i++) {
            number_of_servers += clusters[i]->size();
            total_weight += clusters[i]->size() * weights[i];

            RUSHrGroup *group = new RUSHrGroup();
            group->servers = clusters[i]->size();
            group->weight = weights[i];

            groups.insert(pair<int, RUSHrGroup* >(i, group));
        }
        
        average_weight = total_weight / number_of_servers;

        for (i=0; i < clusters.size(); i++) {
            groups[i]->weight /= average_weight;
            groups[i]->weighted_servers =
            groups[i]->weight * groups[i]->servers ;

            groups[i]->cumulative_servers = groups[i]->servers;
            groups[i]->weighted_cumulative_servers = groups[i]->weighted_servers;
            if ( i >0 ) {
              groups[i]->cumulative_servers += groups[i-1]->cumulative_servers;
              groups[i]->weighted_cumulative_servers +=
              groups[i-1]->weighted_cumulative_servers;
            }
        }
	cout << "end avg " << average_weight << ", total weight " << total_weight << ", disks " << number_of_servers << ", cu_server " << groups[groups.size()-1]->cumulative_servers << endl;
        init_choices(groups[groups.size()-1]->cumulative_servers);
      cout << "init choices" << endl;
    }

#ifdef printlog
    int k = 0;
    for (map<int32_t, list<Disk*>*>::iterator it = this->clusters.begin(); it != this->clusters.end(); ++it) {
        cout << "cluster " << k << ", weight = " << weights[k] << endl;

        list<Disk*>* acluster = it->second;
        for (list<Disk*>::iterator dt = (*acluster).begin(); dt != (*acluster).end(); ++dt) {
            Disk *disk = *dt;

            cout << "disk " << disk->getId() << "," << disk->getCapacity() << endl;
        }
    }
#endif
    j = this->clusters.size();
/*
    cout << "cluster size = " << j << endl;
    for (int i =0 ; i < j; i++) {
        cout << "cluster " << i << ": weight = " << getWj(i) << ", mj = " << getMj(i) << ", n'j = " << getNPrimej(i) <<endl;
    }
 */

};

void DistRUSHr::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    this->copies = copies;
    setDisks(disks);
};

void DistRUSHr::setClusters(std::list<std::list<Disk*>*>* pclusters, list<uint32_t>* weights) {
}

int64_t DistRUSHr::getExtentsize() const {
    return 0;
}

int32_t DistRUSHr::getCopies() const {
    return (int32_t)this->copies;
}

std::list<Disk*>* DistRUSHr::getDisks() const {
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

std::list<Disk*>* DistRUSHr::placeExtent(int64_t virtualVolumeId, int64_t position) {
    int64_t x = position;

    list<Disk*>* disks = new list<Disk*>();
    int *servers = new int[copies];
    memset(servers, 0, sizeof (int) * copies);

    RUSHr(servers, x, copies);

    for (uint32_t i = 0; i < copies; i++) {
       disks->push_back(new Disk(*alldisks[servers[i]]));
    }

    free(servers);

    return disks;
}
/*
 def RUSHR (x,R, j,l)
    seed the random number generator with hash(x, j,0)
    t ← max(0,R−nj)
    // H is a draw from the weighted hypergeometric distribution
    u ← t +H(R−t,n′j −t,m′j +n′j −t,wj)
    if u > 0 then
       seed the random number generator with hash(x, j,1)
       y ← choose(u,mj)
       reset()
       addnj toeachelementiny
       append y to l
       R←R−u
    end if
    if R = 0: return l:
    else: returnRUSHR (x,R,j−1,l)
*/


void DistRUSHr::RUSHr(int servers[], int64_t key, int32_t number_of_replicas) {
    int64_t replicas_remaining,  servers_in_previous_clusters;
    int64_t mandatory_number_to_assign, maximum_number_to_assign;
    int64_t number_assigned, i;

    replicas_remaining = number_of_replicas;

    for (int cluster = (int)groups.size() -1; (cluster >= 0) && (replicas_remaining > 0); cluster--) {
        long x = 0, y = 0, z = 0;
        seed(key, x, y, z);
	/*
	std::stringstream buf;
	buf << "RUSHp::" << key << "::" << cluster << "::" << 0;
	std::string s = buf.str();
	long jump =  Distributor::hashFunctionInt64(&s) % numeric_limits<long>::max();
*/
        jumpahead(/*jump*/(cluster + 2) * 7, x,y,z);

        servers_in_previous_clusters = 0;
        if (cluster > 0) {
          servers_in_previous_clusters =
            groups[cluster-1]->cumulative_servers;
        }

    
        mandatory_number_to_assign = 0;
        if (servers_in_previous_clusters < replicas_remaining) {
          mandatory_number_to_assign =
            replicas_remaining - servers_in_previous_clusters;
        }

        maximum_number_to_assign = replicas_remaining;

        
        number_assigned = mandatory_number_to_assign +
          i_sample(maximum_number_to_assign - mandatory_number_to_assign,
                   groups[cluster]->weighted_servers -
                   mandatory_number_to_assign,
                   groups[cluster]->weighted_cumulative_servers -
                   mandatory_number_to_assign,
                   groups[cluster]->weight,
                   x,y,z
                );

        if (number_assigned > 0) {
	  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
          pthread_mutex_lock(&mutex);
          draw_k_of_n(servers, number_assigned,
                      groups[cluster]->servers, x, y, z);
	  pthread_mutex_unlock(&mutex);
        }

        for (i = 0; i < number_assigned; i++) {
          servers[i] += servers_in_previous_clusters;
        }
        
        replicas_remaining -= number_assigned;
        servers+= number_assigned;
    }
}

xercesc::DOMElement* DistRUSHr::toXML(xercesc::DOMDocument* doc) const {
    DOMElement *result, *disksE;
    XMLCh *attr, *value;
    XMLCh *element_name;
    std::stringstream out;

    cout << "RUSH_r : loading configuration from XML " << endl;

    element_name = XMLString::transcode(DistRUSHr::getXMLRootType().c_str());
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

