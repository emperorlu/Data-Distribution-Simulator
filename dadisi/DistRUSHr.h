/* 
 * File:   DistRUSHr.h
 * Author: ywkang
 *
 * Created on April 7, 2011, 5:06 PM
 */

#ifndef DISTRUSHR_H
#define	DISTRUSHR_H

#include "Distributor.h"
#include <set>
#include <map>
#include <list>
#include <iostream>
#include <math.h>
#include <stdio.h>
using namespace std;

namespace VDRIVE {
    
    class RUSHrGroup {
    public:
      int32_t servers;
      int32_t cumulative_servers;
      double weight;
      double weighted_servers;
      double weighted_cumulative_servers;
    };
    
    class DistRUSHr : public Distributor{
    public:
        DistRUSHr(int argc, char **argv) ;
        DistRUSHr(const DistRUSHr& orig);
        DistRUSHr(xercesc::DOMElement* data);
        virtual ~DistRUSHr();

        virtual std::list<Disk*>* placeExtent(int64_t virtualVolumeId, int64_t position) ;
        virtual void setClusters(std::list<std::list<Disk*>*>* clusters, std::list<uint32_t>* weights);
        virtual void setConfiguration(std::list<Disk*>* disks, int64_t extentsize, int32_t copies);
        virtual void setDisks(std::list<Disk*>* disks);
        virtual std::list<Disk*>* getDisks() const;
        virtual int64_t getExtentsize() const;
        virtual int32_t getCopies() const;
        virtual xercesc::DOMElement* toXML(xercesc::DOMDocument* doc) const;
        static std::string getXMLRootType() {
            return std::string("Rushr");
        }
    private:
        uint32_t copies;
        int32_t j;   // cluster id
        map<int64_t, list<Disk*>* > clusters;
        map<int64_t, RUSHrGroup* > groups;


        //map<int32_t, int32_t > weights;
        vector<double> weights;
        vector<Disk*> alldisks;
        

        void RUSHr(int servers[], int64_t key, int32_t number_of_replicas);
        void draw_k_of_n(int *results, int k, int n, long &x, long &y, long &z);
        int i_sample(int r, double n, double N, double w, long &x, long &y, long &z);
        void init_choices(int n);
        void uninit_choices();
        
        Disk* getDisk(int32_t c, int64_t offset) const {
            map<int64_t, list<Disk*>* >::const_iterator ci = clusters.find(c);
            list<Disk*>* disks = ci->second;

            list<Disk*>::const_iterator it = (*disks).begin();
            for (int i = 0; i < offset; i++, ++it);

            return *it;
        }

        Disk* getDiskByIndex(int64_t index) const {
            
            int64_t tmp = index + 1;
            uint32_t i;
            
            for (i= 0; i < clusters.size(); i++) {
                map<int64_t, list<Disk*>* >::const_iterator ci = clusters.find(i);
                int32_t size = ci->second->size();
                if (tmp  < size) break;
                tmp -= size;
            }
            
            map<int64_t, list<Disk*>* >::const_iterator ci = clusters.find(i);
            list<Disk*>* disks = ci->second;

            list<Disk*>::const_iterator it = (*disks).begin();
            for (int i = 0; i < tmp; i++, ++it);

            Disk* d = *it;
            if (d->getId() != index) {
                cout << "disk id mismatch " << d->getId() << "," << index << endl;
            }

            return *it;
        }


        void seed(int64_t seed_to_set, long &x, long &y, long &z) const {
          x = seed_to_set % 30268 + 1;
          seed_to_set = seed_to_set / 30268;

          y = seed_to_set % 30306 + 1;
          seed_to_set = seed_to_set / 30306;

          z = seed_to_set % 30322 + 1;
          seed_to_set = seed_to_set / 30322;
        }

        double wh_random(long &x, long &y, long &z) const {
          x = (171 * x) % 30269;
          y = (172 * y) % 30307;
          z = (170 * z) % 30323;
          return fmod(((double)x)/30269.0 + ((double)y)/30307.0+
                      ((double)z)/30323.0,1.0);
        }

        void jumpahead(long jumps, long &x, long &y, long &z) const{
          x = x * expmod(171, jumps, 30269) % 30269;
          y = y * expmod(172, jumps, 30307) % 30307;
          z = z * expmod(170, jumps, 30323) % 30323;
        }

        long expmod(long a, long b, long n) const{
          long x, y, accum;

          x = b ;
          y = a % n;

          accum = 1;

        /*   printf("x = %d, y = %d, accum = %d, a = %d, b = %d, n = %d\n", */
        /* 	 x,y,accum, a,b,n); */

          while (x > 0) {
            if (x & 1) {
              accum = (accum * y) % n;
            }
            y = (y*y) %n;
            x = x >> 1;
        /*     printf("x = %d, y = %d, accum = %d, a = %d, b = %d, n = %d\n", */
        /* 	   x,y,accum, a,b,n); */
          }
          return accum;
        }
    };

}

#endif	/* DISTRUSHR_H */

