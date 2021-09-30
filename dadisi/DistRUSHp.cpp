/* 
 * File:   DistRUSHp.cpp
 * Author: ywkang
 * 
 * Created on February 25, 2010, 4:44 PM
 */

#include "DistRUSHp.h"
#include "Prime.h"
#include "types.h"
#include "hash.h"

#include<sstream>
#include<iostream>
#include<sys/time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits>
#include <fstream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>

using namespace xercesc;

using namespace VDRIVE;
using namespace std;

#define mix64(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>43); \
  b -= c; b -= a; b ^= (a<<9); \
  c -= a; c -= b; c ^= (b>>8); \
  a -= b; a -= c; a ^= (c>>38); \
  b -= c; b -= a; b ^= (a<<23); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>35); \
  b -= c; b -= a; b ^= (a<<49); \
  c -= a; c -= b; c ^= (b>>11); \
  a -= b; a -= c; a ^= (c>>12); \
  b -= c; b -= a; b ^= (a<<18); \
  c -= a; c -= b; c ^= (b>>22); \
}



DistRUSHp::DistRUSHp(int argc, char **argv) {

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

DistRUSHp::DistRUSHp(const DistRUSHp& orig) {
}


DistRUSHp::DistRUSHp(xercesc::DOMElement* data) {
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

    cout << this->copies << endl;

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
                        cout << "disk " << d->getId() << ", " << d->getCapacity() << endl;
                        disks->push_back(d);
                    }
                }
            }
            XMLString::release(&sysString);
        }
    }

    setDisks(disks);
}

DistRUSHp::~DistRUSHp() {
    for (vector<Disk*>::iterator dt = alldisks.begin(); dt != alldisks.end(); ++dt) {
        Disk* disk = *dt;
        if (disk) {
            delete disk;
        }
    }

}


void DistRUSHp::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    this->copies = copies;
    setDisks(disks);
};

std::list<Disk*>* DistRUSHp::getDisks() const {
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

int64_t DistRUSHp::getExtentsize() const {
    return 0;
}

int32_t DistRUSHp::getCopies() const {
    return this->copies;
}


void DistRUSHp::setDisks(std::list<Disk*>* disks) {
    
    map<int, int> cluster_index;
    this->weights.clear();
    this->clusters.clear();


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
            weights.push_back((double)cap);
            if (max_weight < cap) {
                max_weight = cap;
            }
            index++;
        }
        alldisks.push_back(new Disk(*d));
    }

    
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
            cout << "configuration error : the number of disks in a cluster " << i << " is less than the number of replicas " << this->copies << endl;
            exit(1);
        }
        total_weight += weights[i] * clusters[i]->size();
    }

    double averageweight = total_weight / alldisks.size();
    cout << "average weight " << averageweight << endl;
    for (uint32_t i = 0; i < weights.size(); i++) {
      weights[i] = weights[i] / averageweight;
	//        int64_t w = weights[i];
	//        double norm_weight = (double(w) / double(max_weight)) * 100000;
	//        weights[i] = int64_t(norm_weight);

      cout << " weight " << i << "," << weights[i] << endl;
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
       ofstream myfile;
       myfile.open ("hash.txt");    
    for (int64_t i = 0 ; i < 20000; i++) {
       double value = hash(i, 0, 0);

       myfile << i << ", " << value << "," << value * 1024 << "," << value * 1024 + i << ", " << ((uint64_t)(value * 1024 + i)) % 1024 << "," << value << "," << ((uint64_t)(value * 1024)) % 1024<<endl;
    }
       myfile.close();    

    cout << "cluster size = " << j << endl;
    for (int i =0 ; i < j; i++) {
        cout << "cluster " << i << ": weight = " << getWj(i) << ", mj = " << getMj(i) << ", n'j = " << getNPrimej(i) <<endl;
    }
 */
    
};

// all disks
void DistRUSHp::setClusters(std::list<std::list<Disk*>*>* pclusters, list<uint32_t>* weights) {
    // initialize weights and clusters
    /*
    cout << "set cluster" << endl;
    this->weights.clear();
    this->clusters.clear();
    this->weights.assign(weights->begin(), weights->end());
    j = 0;
    for (list<list<Disk*>*>::iterator it = (*pclusters).begin(); it != (*pclusters).end(); ++it) {
        list<Disk*>* acluster = *it;
        clusters.insert(pair<int, list<Disk*>* >(j, acluster));
        j++;
    }
     * */
}

// volumeId = an object's key, position n = n-th object with the same key
std::list<Disk*>* DistRUSHp::placeExtent(int64_t virtualVolumeId, int64_t position) {
    int64_t x = position;
    list<Disk*>* disks = new list<Disk*>();
    for (int64_t i = 0 ; i < copies; i++) {
        Disk* disk = RUSHp(x, i, j-1);

        if (disk) {
            disks->push_back(new Disk(*disk));
        }
    }

    return disks;
}


xercesc::DOMElement* DistRUSHp::toXML(xercesc::DOMDocument* doc) const {
    DOMElement *result, *disksE;
    XMLCh *attr, *value;
    XMLCh *element_name;
    std::stringstream out;

    cout << "crush: loading configuration from XML " << endl;

    element_name = XMLString::transcode(DistRUSHp::getXMLRootType().c_str());
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



/*
struct timeval before, after;

uint64_t timeInMS(struct timeval before, struct timeval after) {
    uint64_t beforems = before.tv_sec * 1000 + before.tv_usec / 1000;
    uint64_t afterms = after.tv_sec * 1000 + after.tv_usec / 1000;
    return afterms - beforems;
}
*/
long sum = 0;
void DistRUSHp::debug() {
    cout << "sum = " << sum << endl;
    sum = 0;
};

/*
 *  def RUSHp (x,r,j)
 *     m'j = mj * wj
 *     n'j = E m'j (i:0~j-1)
 *     z = hash(x,j,0)*(n'j + m'j)
 *     choose a prime number p >= m'j based on hash (x,j,1)
 *     v = x+z+r*p
 *     z'= (z+r*p) mod (n'j+m'j)
 *     if (mj >= R and z' < m'j)
 *      map the object to nj + (v mod mj)
 *     else if mj < R and z' < R*wj and v mod R < mj
 *      map the object to server nj + (v mod R)
 *     else
 *      RUSHp(x,r,j-1)
 */

static uint64_t round2(double x) {
  if (x - (uint64_t)x < 0.5 ) {
    return (uint64_t)x;
  } 
  return (uint64_t)x+1;
}

Disk* DistRUSHp::RUSHp(int64_t x, int64_t r, int32_t j) const {
  while (j >= 0) {
  // when 
        int64_t mj  = getMj(j);    // 8192
        double mpj = getMPrimej(j); // 1024   
        double npj = getNPrimej(j); // 0

	//cout << "z = " << hash(x, j, 0) << ", mpj = " << mpj << endl;
        double z   = hash(x, j, 0) * 819200 * round2((mpj + npj)); //
	//	cout << "z = " << z << "mpj = " << mpj << endl;
	//	cout << "hash key = " << x << ", " << j << ", " << 1 << endl;
        int64_t p   = Prime::getPrimeNumberIntHash(hashInt64(x, j, 1));

        uint64_t v   = round2(x + z + r*p);
        int64_t zp  = ((int64_t) (z+r*p)) % ((int64_t)(npj + mpj));

        if (p < mpj) {
            cerr << "prime number is too small p = " << p << ", mpj = " << mpj << endl;
        }
	if (p < 0) {
  	    cerr << "p negative" << endl;
	}

	//cout << " z " << z << endl;
	        //cout << " p " << p << endl;
	        //cout << "-- comp1 mj " << mj << "copies "<< copies <<", zp " << zp << ", mpj " << mpj << endl;
	//        cout << "-- comp2 mj " << mj << "copies "<< copies <<", zp " << zp << ", copies * getWj(j)" << copies * getWj(j) << ", v%copies = " << (v%copies) << endl;
        if (mj >= copies && zp < mpj) {
	  //	  Disk *d = getDisk(j, (((int64_t)v) % mj));
	  
	  //	  cout << "placing it to " << d->getId() << endl;
	    //	    cout << "-1. placed to cluster, offset " << j << ", " << (v % mj) << endl; 
          return getDisk(j, (((int64_t)v) % mj));
        }
        else if (mj < copies && zp < copies * getWj(j) && (((int64_t)v) % copies) < mj) {
	  //	  Disk *d = getDisk(j, (((int64_t)v) % copies));
	  
	    //	    cout << "-2. placed to cluster, offset " << j << ", " << (v % copies) << endl;
          return  getDisk(j, (((int64_t)v) % copies));
        }
	else {
	  j = j -1;
	}
        //else
        //    return RUSHp(x,r,j-1);
  }

  cerr << "couldn't place an object " << x << endl;
  return NULL;

}


/*
--------------------------------------------------------------------
 This works on all machines, is identical to hash() on little-endian 
 machines, and it is much faster than hash(), but it requires
 -- that the key be an array of uint64_t's, and
 -- that all your machines have the same endianness, and
 -- that the length be the number of uint64_t's in the key
--------------------------------------------------------------------
*/
uint64_t DistRUSHp::hash2(register uint64_t *k, register uint64_t length, register uint64_t level) const
{
  register uint64_t a,b,c,len;

  /* Set up the internal state */
  len = length;
  a = b = level;                         /* the previous hash value */
  c = 0x9e3779b97f4a7c13LL; /* the golden ratio; an arbitrary value */

  /*---------------------------------------- handle most of the key */
  while (len >= 3)
  {
    a += k[0];
    b += k[1];
    c += k[2];
    mix64(a,b,c);
    k += 3; len -= 3;
  }

  /*-------------------------------------- handle the last 2 uint64_t's */
  c += (length<<3);
  switch(len)              /* all the case statements fall through */
  {
    /* c is reserved for the length */
  case  2: b+=k[1];
  case  1: a+=k[0];
    /* case 0: nothing left to add */
  }
  mix64(a,b,c);
  /*-------------------------------------------- report the result */
  return c;
}

uint64_t DistRUSHp::hashInt64(int64_t key1, int64_t key2, int64_t key3) const {


    std::stringstream buf;
    buf << "RUSHp::" << key1 << "::" << key2 << "::" << key3;
    std::string s = buf.str();
     return Distributor::hashFunctionInt64(&s);
  /*
    uint64_t keys[3];
    keys[0] = (uint64_t)key1; keys[1] = (uint64_t)key2; keys[2] = (uint64_t)key3;
    uint64_t init = 1315423911 ^ key1 ^ key2 ^ key3;
    uint64_t hash = hash2(keys, 3, init);
    if (hash < 0) { hash = hash * -1; }
    cout << "hashInt64" << hash << endl;
    return hash;
  */   
}

/**
 *  hash function for RUSHp
 *      takes 3 parameters 
 */
double DistRUSHp::hash(int64_t key1, int64_t key2, int64_t key3) const {
   
    std::stringstream buf;
    buf << "RUSHp::" << key1 << "::" << key2 << "::" << key3;
    std::string s = buf.str();
    uint64_t hash =  Distributor::hashFunctionInt64(&s);
    if (hash < 0) { hash = hash * -1; }
    return ((double)hash) / ((double)numeric_limits<uint64_t>::max()) ;

     /*
    uint64_t keys[3];
    keys[0] = (uint64_t)key1; keys[1] = (uint64_t)key2; keys[2] = (uint64_t)key3;
    uint64_t init = 1315423911 ^ key1 ^ key2 ^ key3;
    uint64_t hash = hash2(keys, 3, init);
    


    cout << "hash " << hash << "," <<  (hash * 1.0) /  std::numeric_limits<uint64_t>::max() << endl;
    return (hash * 1.0) /  std::numeric_limits<uint64_t>::max();
     */ 
// TODO: make a 64 bit version of the Robert Jenkins' integer hash function
/*
    int32_t hash = crush_hash32_3(CRUSH_HASH_RJENKINS1, (int32_t) key1, (int32_t) key2, (int32_t) key3);

    return (hash * 1.0) / numeric_limits<int32_t>::max();
*/
}





