/* 
 * File:   Distcrush.cpp
 * Author: ywkang
 * 
 * Created on March 16, 2010, 7:42 PM
 */

#include "DistCRUSH.h"
#include<vector>
#include<list>
#include<string>
#include <set>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ostream>
#include <sstream>
#include <iostream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOM.hpp>
#include <algorithm>
#include <functional>
#include "crushwrapper.h"

#define CEPH_PG_TYPE_REP     1
#define BUCKETTYPES 4

using namespace VDRIVE;
using namespace xercesc;

DistCRUSH::DistCRUSH(int argc, char** argv) {
    int argInd;

    this->failuredomains = 0;
    this->copies         = 0;
    
    for (argInd = 0; argInd < argc; argInd++) {
        if (argv[argInd][0] != '-') {
            throw 1;
        } else if (strcmp(argv[argInd], "-layer") == 0) {
            argInd++;
            
            if (argInd +2 > argc) {
                throw "Each layer is 'name buckettype'";
            }
            
            while (argInd + 2 <= argc) {
                layer_t layer;
                layer.name = argv[argInd++];
                layer.buckettype = atoi(argv[argInd++]);
                layer.size = 0; //atoi(argv[argInd++]);
                layers.push_back(layer);
                if (argInd < argc && argv[argInd][0] == '-') { argInd--; break;}
           }
        } else if (strcmp(argv[argInd], "-ndom") == 0) {
            argInd++;
            if (argInd == argc) {
                throw "-ndom requires a number";
            }
            else {
                this->failuredomains = atoi(argv[argInd]);
                cout << "number of failure domains = " << this->failuredomains << endl;
            }
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

DistCRUSH::DistCRUSH(const DistCRUSH& orig) {
}


DistCRUSH::DistCRUSH(xercesc::DOMElement* data) {
    std::istringstream isst;
    xercesc::DOMNode *n, *m;
    DOMElement* layerE;
    XMLCh* xmlString;
    char* sysString;

    xmlString = XMLString::transcode("ndom");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->failuredomains;
    isst.str("");
    isst.clear();
    XMLString::release(&sysString);
    XMLString::release(&xmlString);

    xmlString = XMLString::transcode("copies");
    sysString = XMLString::transcode(data->getAttribute(xmlString));
    isst.str(sysString);
    isst >> this->copies;
    isst.str("");
    isst.clear();


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
            } else if (strcmp(sysString, "layers") == 0) {
                for (m = e->getFirstChild(); m != 0; m = m->getNextSibling()) {
                    if (m->getNodeType() == DOMNode::ELEMENT_NODE) {
                        layerE = (xercesc::DOMElement*)m;
                        {
                            layer_t layer;
                            XMLCh* xmlString;
                            char* sysString;

                            xmlString = XMLString::transcode("name");
                            sysString = XMLString::transcode(layerE->getAttribute(xmlString));
                            layer.name = sysString;
                            XMLString::release(&sysString);
                            XMLString::release(&xmlString);

                            xmlString = XMLString::transcode("type");
                            sysString = XMLString::transcode(layerE->getAttribute(xmlString));
                            isst.str(sysString);
                            isst >> layer.buckettype;
                            isst.str("");
                            isst.clear();
                            XMLString::release(&sysString);
                            XMLString::release(&xmlString);

                            xmlString = XMLString::transcode("size");
                            sysString = XMLString::transcode(layerE->getAttribute(xmlString));
                            isst.str(sysString);
                            isst >> layer.size;
                            isst.str("");
                            isst.clear();
                            
                            XMLString::release(&sysString);
                            XMLString::release(&xmlString);
                            
                            layers.push_back(layer);
                        }
                    }
                }
            }
            XMLString::release(&sysString);
        }
    }
    
    //buildCrushMap(layers, disks);
    setConfiguration(disks, 1, this->copies);
}

DistCRUSH::~DistCRUSH() {
     for (std::list<Disk*>::iterator it = alldisks.begin(); it != alldisks.end(); it++) {
        delete *it;
        it = alldisks.erase(it);
    }

    alldisks.clear();
    failuredomains = 0;
}

void DistCRUSH::buildCrushMap(vector<layer_t> layers, std::list<Disk*>* disks) {

    int num_osds = disks->size();

    if (layers.empty() || disks->size() == 0) {
      cerr << ": must specify at least one layer and one disk" << std::endl;
      exit(1);
    }

    crush.create();

    vector<int> lower_items;
    vector<int> lower_weights;

    for (list<Disk*>::iterator it = disks->begin(); it != disks->end(); ++it) {
        Disk* disk = *it;
        lower_items.push_back(disk->getId());
        lower_weights.push_back(disk->getCapacity());   // weight == capacity
        disk_weights.push_back(disk->getCapacity());
    }

    int type = 1;
    int rootid = 0;

    for (vector<layer_t>::iterator p = layers.begin(); p != layers.end(); p++, type++) {
      layer_t &l = *p;
/*
      cout << "layer " << type
	      << "  " << l.name.c_str()
	      << "  bucket type " << l.buckettype
	      << endl;
*/
      crush.set_type_name(type, l.name.c_str());

      if (l.buckettype > BUCKETTYPES) {
          cerr << "unknown bucket type " << l.buckettype << endl;
      }

      vector<int> cur_items;
      vector<int> cur_weights;
      unsigned lower_pos = 0;  // lower pos

      int i = 0;
      while (1) {
	if (lower_pos == lower_items.size())
	  break;

        int items[num_osds];
	int weights[num_osds];

	int weight = 0;
	int j;
	for (j=0; j < l.size || l.size == 0; j++) {
	  if (lower_pos == lower_items.size())
	    break;
	  items[j] = lower_items[lower_pos];
	  weights[j] = lower_weights[lower_pos];
	  weight += weights[j];
	  lower_pos++;
	  //cout << "  item " << items[j] << " weight " << weights[j] << endl;
	}

	crush_bucket *b = crush_make_bucket(l.buckettype, CRUSH_HASH_DEFAULT, type, j, items, weights);
	int id = crush_add_bucket(crush.crush, 0, b);
	rootid = id;

	crush.set_item_name(id, l.name.c_str());

	//cout << " in bucket " << id << " '" << l.name.c_str() << "' size " << j << " weight " << weight << endl;

	cur_items.push_back(id);
	cur_weights.push_back(weight);
	i++;
      }

      lower_items.swap(cur_items);
      lower_weights.swap(cur_weights);
    }

    //cout << "root id = " << rootid << endl;
     // make a generic rules
    int ruleset=1;
    crush_rule *rule = crush_make_rule(3, ruleset, 1, 2, 2);
    crush_rule_set_step(rule, 0, CRUSH_RULE_TAKE, rootid, 0);
    crush_rule_set_step(rule, 1, CRUSH_RULE_CHOOSE_LEAF_FIRSTN, CRUSH_CHOOSE_N, 1);
    crush_rule_set_step(rule, 2, CRUSH_RULE_EMIT, 0, 0);
    int rno = crush_add_rule(crush.crush, rule, -1);
    crush.set_rule_name(rno, "data");
    this->ruleno = rno;
    crush.finalize();
    
}

void DistCRUSH::setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies) {
    this->copies = copies;
    this->extentsize = extentsize;
    setDisks(disks);
}

// simple_map has one root bucket that has several failure domains inside.
// groups of disks = failure domain .

void DistCRUSH::setDisks(std::list<Disk*>* disks) {
    int num_osds= disks->size();

    if (num_osds == 0) return ;

    for (std::list<Disk*>::iterator it = disks->begin(); it != disks->end(); it++) {
        Disk *d = *it;
        alldisks.push_back(new Disk(*d));
    }
    
    if (this->failuredomains == 0) {
        this->failuredomains = 10;
    }

    int nper = ((num_osds -1) /  this->failuredomains) +1;
    if (layers.size() == 0) {
        // create default map

        layer_t layer;
        layer.buckettype = CRUSH_BUCKET_UNIFORM;
        layer.name       = "domain";
        layer.size       = nper;
        layers.push_back(layer);

        layer_t layer2;
        layer2.buckettype = CRUSH_BUCKET_STRAW;
        layer2.name       = "root";
        layer2.size       = 0;
        layers.push_back(layer2);

    } else {
        // size of the lowest level layer should be divided into failure domains
        layers[0].size = ((num_osds -1) /  this->failuredomains) +1;
    }

    buildCrushMap(layers, disks);    
}

std::list<Disk*>* DistCRUSH::getDisks() const {
    if (this->alldisks.size() == 0)
        return 0;
    std::list<Disk*> *resDisks = new std::list<Disk*>();

    for (list<Disk*>::const_iterator dt = alldisks.begin(); dt != alldisks.end(); ++dt) {
        Disk* disk = *dt;
        if (disk) {
            resDisks->push_back(new Disk(*disk));
        }
    }

    return resDisks;
}

int64_t DistCRUSH::getExtentsize() const {
    return this->extentsize;
};

int32_t DistCRUSH::getCopies() const {
    return this->copies;
}
/*
void DistCRUSH::setClusters(list<list<Disk*>*>* pclusters, list<uint32_t>* pweights) {

    throw "use setDisk and setConfiguration instead";
   alldisks.clear();
   disk_weights.clear();
   
   list<uint32_t>::iterator wt = (*pweights).begin();
   for (list<list<Disk*>*>::iterator it = (*pclusters).begin(); it != (*pclusters).end() && wt != (*pweights).end(); ++it, ++wt) {
        list<Disk*>* acluster = *it;
        int weight = *wt;
        for (list<Disk*>::iterator dt = (*acluster).begin(); dt != (*acluster).end(); ++dt) {
            alldisks.push_back(*dt);
            disk_weights.push_back(weight);
        }
    }

   crush.create();

   crush.set_type_name(1, "domain");
   crush.set_type_name(2, "pool");

   
   int num_osds= alldisks.size();

   int ndom = 10;

   // build a map
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

            crush_bucket *domain = crush_make_bucket(CRUSH_BUCKET_STRAW, CRUSH_HASH_DEFAULT, 1, j, items, weights);
            ritems[i] = crush_add_bucket(crush.crush,0,domain);
            char bname[10];
            snprintf(bname, sizeof(bname), "dom%d", i);
            crush.set_item_name(ritems[i], bname);
        }

        // root
        crush_bucket *root = crush_make_bucket(CRUSH_BUCKET_STRAW, CRUSH_HASH_DEFAULT, 2, ndom, ritems, rweights);
        int rootid = crush_add_bucket(crush.crush, 0, root);
        crush.set_item_name(rootid, "root");

        //cout << "root id = " << rootid << endl;
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

       // cout <<"root bucket type " << root->type << endl;

        int rootid = crush_add_bucket(crush.crush, 0, root);
        crush.set_item_name(rootid, "root");

        int ruleset=1;
        crush_rule *rule = crush_make_rule(3, ruleset, CEPH_PG_TYPE_REP, 2, 10);
        crush_rule_set_step(rule, 0, CRUSH_RULE_TAKE, rootid, 0);
        crush_rule_set_step(rule, 1, CRUSH_RULE_CHOOSE_LEAF_FIRSTN, CRUSH_CHOOSE_N, 1);
        crush_rule_set_step(rule, 2, CRUSH_RULE_EMIT, 0, 0);
        ruleno = crush_add_rule(crush.crush, rule, -1);
        crush.set_rule_name(ruleno, "data");
    }
    
    crush.finalize();

}
*/
std::list<Disk*>* DistCRUSH::placeExtent(int64_t virtualVolumeId, int64_t position) {
    list<Disk*>* disks = new list<Disk*>();
    vector<int> out;

    if (copies == 0) {
        cout << "warning: at least one copy is required.";
    }
    
    crush.do_rule(this, ruleno, position, out, copies, -1, disk_weights);
    
    for (uint32_t i = 0 ; i < out.size(); i++) {
        disks->push_back(new Disk(out[i], disk_weights[i], 0));
    }

    return disks;
}




xercesc::DOMElement* DistCRUSH::toXML(xercesc::DOMDocument* doc) const {
    DOMElement *result, *disksE, *layersE, *layerE;
    XMLCh *attr, *value;
    XMLCh *element_name;
    std::stringstream out;

    cout << "crush: loading configuration from XML " << endl;

    element_name = XMLString::transcode(DistCRUSH::getXMLRootType().c_str());
    result = doc->createElement(element_name);
    XMLString::release(&element_name);

    out.str(""); out.clear();
    out << this->failuredomains;
    attr  = XMLString::transcode("ndom");
    value = XMLString::transcode(out.str().c_str());
    result->setAttribute(attr, value);
    XMLString::release(&attr);
    XMLString::release(&value);

    
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

    for (list<Disk*>::const_iterator dt = alldisks.begin(); dt != alldisks.end(); ++dt) {
        Disk* d = *dt;
        disksE->appendChild(d->toXML(doc));
    }
    result->appendChild(disksE);

    element_name = XMLString::transcode("layers");
    layersE = doc->createElement(element_name);
    XMLString::release(&element_name);

    for (vector<layer_t>::const_iterator dt = layers.begin(); dt != layers.end(); ++dt) {
        layer_t d = *dt;

        element_name = XMLString::transcode("layer");
        layerE = doc->createElement(element_name);
        XMLString::release(&element_name);

        out.str(""); out.clear();
        attr  = XMLString::transcode("name");
        value = XMLString::transcode(d.name.c_str());
        layerE->setAttribute(attr, value);
        XMLString::release(&attr);
        XMLString::release(&value);

        out.str(""); out.clear();
        out << d.buckettype;
        attr  = XMLString::transcode("type");
        value = XMLString::transcode(out.str().c_str());
        layerE->setAttribute(attr, value);
        XMLString::release(&attr);
        XMLString::release(&value);

        out.str(""); out.clear();
        out << d.size;
        attr  = XMLString::transcode("size");
        value = XMLString::transcode(out.str().c_str());
        layerE->setAttribute(attr, value);
        XMLString::release(&attr);
        XMLString::release(&value);
        layersE->appendChild(layerE);

    }
    result->appendChild(layersE);

    return result;
}

