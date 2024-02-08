#ifndef DOPCMANAGER_H
#define DOPCMANAGER_H

#include <list>

class Interval;
class RightInterval_Endpoint;
class UptreeNode;


class DOPCManager
{
public:
    DOPCManager();
    DOPCManager(int num_of_intervals);
    ~DOPCManager();

    /* Input: intervals sorted based on their right endpoint.
     * intervals sorted based on their left endpoint.
     * number of intervals.
     * Output: the path cover of G(I)
     * in indexes corresponding to the input interval array based on their right endpoint */
    bool runAlgorithm(Interval *intervalArray, Interval **leftIntervalPtrArray, int num_of_intervals, std::list<std::list<int>> &pathCover_out);

    void setNumOfIntervals(int n) {m_num_of_intervals = n;}
    void resetManager();
private:
    void init();
    void cleanup();
    bool checkInput(Interval *intervalArray, Interval **leftIntervalPtrArray, int num_of_intervals);

    /* uptree related functions */
    void initUpTree(Interval *&intervalArray);
    int uptree_find(int x);
    int uptree_unite(int x, int y);

    /* consecutive left groups */
    void createConsLeftGroups(Interval *&intervalArray, Interval **&leftIntervalPtrArray);
    void uniteGroupsOnX(RightInterval_Endpoint *&x, RightInterval_Endpoint *&y);

    /* L-list functions
     * parsing through the list by pointers,
     * this manager is a friend class to intervals */
    void createLlist(Interval *&intervalArray);
    bool deleteLElement(RightInterval_Endpoint *&e);

    int m_num_of_intervals;
    bool m_initialized;
    RightInterval_Endpoint *mp_dummyLstart; // L-list start
    UptreeNode *mp_uptree; // uptree root node
};


class UptreeNode
{
    friend class DOPCManager;
public:
    UptreeNode() {init();}
    UptreeNode(int id, RightInterval_Endpoint *ptr) : m_parent_id(id), m_ptr(ptr) {}
    UptreeNode(const UptreeNode &other);
    UptreeNode& operator=(const UptreeNode &other);
    ~UptreeNode() {init();}

private:
    void init() {m_parent_id = 0; m_ptr = nullptr;}

    int m_parent_id;
    /* instead of hashing the data, we use a pointer.
     * A pointer allows us to implement uptree_unite()
     * with "union-by-size" but set the new leader ourselves on uniteGroupsOnX(). */
    RightInterval_Endpoint *m_ptr;
};

#endif // DOPCMANAGER_H
