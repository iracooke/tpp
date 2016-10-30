/*
 * Copyright (c) 2003-2006 Fred Hutchinson Cancer Research Center
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
  modified December 2010 by Insilicos LLC to support object serialization for 
  Hadoop and MPI use
*/
#include "mscore.h"

#ifndef MSCORE_K_H
#define MSCORE_K_H

class miLookup
{
public:
    miLookup(void) {
        init();
    }

    miLookup(const miLookup& rhs) {
        init();
        (*this) = rhs;
    }

    void init() {
        m_start = 0;
        m_end = 0;
        m_pfI = NULL;
    }

    virtual ~miLookup(void)    {
        if (m_pfI != NULL)
            free(m_pfI);
    }

    void init(int start, int end) {
        m_start = start;
        m_end = end;
        if (m_pfI != NULL)
            free(m_pfI);
        m_pfI = (float*) calloc(end - start, sizeof(float));
    }

    void clear(void) {
        memset(m_pfI, 0, (m_end - m_start) * sizeof(float));
    }

    int m_start; // start dalton value of pfI[0]
    int m_end; // end dalton value after pfI (i.e. end - start == size)
    float* m_pfI; // intensity lookup for masses rounded to dalton int values

    float& operator[](int i) {
        return m_pfI[i - m_start];
    }

    float get(int i) {
        if(i < m_start || i >= m_end)
            return 0.0;
        return m_pfI[i - m_start];
    }

/*
 * a simple copy operation, using the = operator
 */
    miLookup& operator=(const miLookup &rhs)    {
        m_start = rhs.m_start;
        m_end = rhs.m_end;

        if (m_pfI != NULL)
            free(m_pfI);
        size_t size = (m_end - m_start) * sizeof(float);
        m_pfI = (float*) malloc(size);
        memcpy(m_pfI, rhs.m_pfI, size);

        return *this;
    }
};

class mscore_k : public mscore
{
protected:
    friend class mscorefactory_k;
    mscore_k(void);    // Should only be created through mscorefactory_tandem

public:
    ~mscore_k(void);

public:
    bool load_param(XmlParameter &_x); // allows score object to issue warnings, or set variables based on xml
    bool precondition(mspectrum &_s); // called before spectrum conditioning
    void prescore(const size_t _i); // called before scoring
    
    bool add_mi(mspectrum &_s);

    double sfactor(); // factor applied to final convolution score
    unsigned long mconvert(double _m, const long _c); // convert mass to integer ion m/z for mi vector
    void report_score(char* _buff, float _h); // format hyper score for output

    bool clear();

protected:
		bool add_A(const unsigned long _t,const long _c);
		bool add_B(const unsigned long _t,const long _c);
		bool add_C(const unsigned long _t,const long _c);
		bool add_Y(const unsigned long _t,const long _c);
		bool add_X(const unsigned long _t,const long _c);
		bool add_Z(const unsigned long _t,const long _c);
    double dot(unsigned long *_v); // this is where the real scoring happens
		float ion_check(const unsigned long _v,const size_t _d);

protected:
    unsigned long imass(double _m)
    {
        return (unsigned long)((_m/m_dIsotopeCorrection) + 0.5);
    }

protected:
    int m_maxEnd;
    miLookup m_miUsed;
    vector<vmiType> m_vmiType;
		vector<int> m_vmiUsed;

    double m_dIsotopeCorrection;
};

/*
 * mscorefactory_k implements a factory for creating mscore_k instances.
 */
class mscorefactory_k : public mpluginfactory
{
public:
    mscorefactory_k();

    virtual mplugin* create_plugin();
};

#endif // MSCORE_K_H
