/* 
 * File:   DistRUSHp.h
 * Author: ywkang
 *
 * Created on February 25, 2010, 4:44 PM
 */

#ifndef _DISTRUSHP_H
#define	_DISTRUSHP_H
#include "Distributor.h"
#include <set>
#include <map>
#include <list>
#include <iostream>

using namespace std;

namespace VDRIVE {

    
    
    class DistRUSHp : public Distributor {
    public:
        DistRUSHp(int argc, char **argv) ;
        DistRUSHp(const DistRUSHp& orig);
        DistRUSHp(xercesc::DOMElement* data);
        virtual ~DistRUSHp();
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
            return std::string("Rushp");
        }
	uint64_t hashInt64(int64_t key1, int64_t key2, int64_t key3) const;
        uint64_t hash2(register uint64_t *k, register uint64_t length, register uint64_t level) const;
        Disk* RUSHp(int64_t x, int64_t r, int32_t j) const;
    private:
        uint32_t copies;
        int32_t j;   // cluster id
        map<int64_t, list<Disk*>* > clusters;
        
        //map<int32_t, int32_t > weights;
        vector<double> weights;
        vector<Disk*> alldisks;

        virtual void debug();

        Disk* getDisk(int32_t c, int64_t offset) const {
            map<int64_t, list<Disk*>* >::const_iterator ci = clusters.find(c);
            list<Disk*>* disks = ci->second;

            list<Disk*>::const_iterator it = (*disks).begin();
            for (int i = 0; i < offset; i++, ++it);

            return *it;
        }


        int64_t getMj(int32_t c) const {
            if (c < 0) {
                cout << "error" << endl;
            }
            map<int64_t, list<Disk*>* >::const_iterator ci = clusters.find(c);
            //cout << "cluster " << c  << ", mj = " << ci->second->size() << endl;
            return ci->second->size();
        }
/*
        int64_t getNj(int32_t c) const {
            int64_t sum = 0;
            for (int i =0 ;i <= c; i++) {
                sum += getMj(i);
            }
            if (sum < 0) {
                cout << "error " << endl;
            }
            cout << "nj = " << sum << endl;
            return sum;
        }
*/
	double getWj(int32_t c) const {
            return weights[c];
        }

        // return the total amount of weight in cluster j
        double getMPrimej(int32_t c) const {
            int64_t numdisks = getMj(c);

	    // cout << "m'j = " << numdisks * getWj(c) << endl;
            return numdisks * getWj(c);
        }

        // return the total amount of weight in cluster 0 ~ j-1
        double getNPrimej(int32_t c) const {
            int64_t sum = 0;
            //cout << "cluster " << c << endl;
            for (int i =0 ;i < c; i++) {
                sum += getMPrimej(i);
              //  cout << "subset sum =  " << sum << endl;
            }
            //cout <<  ", n'j = " << sum << endl;
            return sum;
        }
        
 
        //Robert Jenkins's hash function
    };

    

}

#endif	/* _DISTRUSHP_H */

