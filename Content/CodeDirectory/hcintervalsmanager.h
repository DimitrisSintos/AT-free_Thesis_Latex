#ifndef HCINTERVALSMANAGER_H
#define HCINTERVALSMANAGER_H

#include <list>

class Interval;


class HCIntervalsManager
{
public:
    HCIntervalsManager();
    HCIntervalsManager(int num_of_intervals);
    ~HCIntervalsManager();

    /* Input: intervals sorted based on their right endpoint.
     * number of intervals.
     * the path cover of G(I) in indexes corresponding to the input interval array
     * Output: a hamiltonian cycle in idexes corresponding to the input interval array */
    bool runAlgorithm(Interval *intervalArray, int num_of_intervals, const std::list<std::list<int>> &pathCover, std::list<int> &cycle_out);

    void setNumOfIntervals(int n) {m_num_of_intervals = n;}
    void resetManager();

private:
    void init();
    void cleanup();
    bool checkInput(Interval *intervalArray, int num_of_intervals, const std::list<std::list<int>> &pathCover);

    int m_num_of_intervals;
    bool m_initialized;
};

#endif // HCINTERVALSMANAGER_H
