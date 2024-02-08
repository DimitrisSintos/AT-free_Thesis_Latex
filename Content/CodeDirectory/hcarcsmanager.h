#ifndef HCARCSMANAGER_H
#define HCARCSMANAGER_H

#include <list>
#include <string>
#include <functional>

class HCArcsOutput;
typedef std::function<void (const HCArcsOutput &output)> HCArcsOutputFunction;

class Arc;
class Interval;


class HCArcsManager
{
public:

    enum outputCase { DEF_CASE, ONLY_ARCS, ONLY_INTERVALS, CASE_7, CASE_11, CASE_13, CASE_19, CASE_21, CASE_22 };

    HCArcsManager();
    HCArcsManager(int num_of_arcs);
    ~HCArcsManager();

    /* Input: arcs sorted based on their tails.
     * arcs sorted based on their heads.
     * number of arcs.
     * Output: a hamiltonian cycle of G(F) */
    bool runAlgorithm(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs);

    // setter for callback to get the output on each of the outputCases
    void setOutputFunc(const HCArcsOutputFunction &outputFunc);
    void setInputDataToViewFunc(const HCArcsOutputFunction &dataToViewFunc);
    void setToView(bool toView) {m_toView = toView;}

    // test functions
    void testDOPC_example1();
    void testDOPC_example2();
    void testHCArcsRandExample(int num_of_arcs);
    void testHCArcsExample1();
    void testHCArcsExample2();

    // the first step of the algorithm
    int findIndexOfArcContainingNoOther(Arc *arcsArray, int num_of_arcs) const;
    // sets B_p and B_a are needed by the algorithm on multiple steps
    void compute_set_Bp(int point_p, int *&set_Bp_out, int &setSize_out) const;
    void compute_set_Ba(const Arc &arc_a, int *&set_Ba_out, int &setSize_out) const;

    void setNumOfArcs(int n) {m_num_of_arcs = n;}
    void resetManager();
private:
    void init();
    void cleanup();
    bool checkInput(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs);
    bool initializeMembers(Arc *arcsArray, Arc **leftArcsPtrArray, int num_of_arcs);
    void produceOutput(HCArcsManager::outputCase c,
                       Interval *intervalArray, int num_of_intervals,
                       Arc *arcArray, int num_of_arcs,
                       const std::list<int> &intervalIndexesPath,
                       const std::list<int> &arcIndexesPath,
                       const std::list<int> &arcIndexesCycle);

    /* sort functions are used **outside** of the algorithm to prepare the input */
    // sort by tails
    void sort_runRecursively(Arc *&arcArray, int l, int r);
    void sort_merge(Arc *&arcArray, int left, int middle, int right);
    // sort by heads
    void sort_runRecursivelyPtrs(Arc **&ptr_arcArray, int l, int r);
    void sort_mergePtrs(Arc **&ptr_arcArray, int left, int middle, int right);


    int m_num_of_arcs;
    bool m_initialized;
    Arc *mp_arcsArray; // sorted arcs based on their tails
    Arc **mp_arcsPtrArray; // sorted arcs based on their heads

    bool m_toView; // allow data to go to view
    HCArcsOutputFunction m_outputFunc; // callback function for output
    HCArcsOutputFunction m_inputDataToViewFunc; // callback function to send input data to view
};


class SimpleArc;
class SimpleInterval;
// data class for easier algorithm-to-gui transition
class HCArcsOutput
{
public:
    HCArcsOutput();
    HCArcsOutput(HCArcsManager::outputCase c,
                 Interval *intervalArray, int num_of_intervals,
                 Arc *arcArray, int num_of_arcs,
                 std::list<int> intervalIndexesPath,
                 std::list<int> arcIndexesPath,
                 std::list<int> arcIndexesCycle);
    ~HCArcsOutput();

    HCArcsManager::outputCase outputCase() const {return m_case;}
    int num_of_intervals() const {return m_num_of_intervals;}
    int num_of_arcs() const {return m_num_of_arcs;}

    SimpleInterval *intervalArray() const {return mp_intervalArray;}
    SimpleArc *arcArray() const {return mp_arcArray;}

    std::list<int> intervalIndexesPath() const {return m_intervalIndexesPath;}
    std::list<int> arcIndexesPath() const {return m_arcIndexesPath;}
    std::list<int> arcIndexesCycle() const {return m_arcIndexesCycle;}


private:
    void init();


    HCArcsManager::outputCase m_case; // line number where the algorithm produced this data

    int m_num_of_intervals;
    int m_num_of_arcs;

    SimpleInterval *mp_intervalArray;
    SimpleArc *mp_arcArray;

    std::list<int> m_intervalIndexesPath;
    std::list<int> m_arcIndexesPath;
    std::list<int> m_arcIndexesCycle;

};

#endif // HCARCSMANAGER_H
