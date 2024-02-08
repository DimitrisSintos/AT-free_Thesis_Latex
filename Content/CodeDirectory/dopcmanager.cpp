#include "dopcmanager.h"
#include "endpoint.h"
#include "hcarcs_object.h"
#include "hcarcs_utils.h"
#include <iterator>

DOPCManager::DOPCManager()
{
    init();
}

DOPCManager::DOPCManager(int num_of_intervals)
{
    init();
    m_num_of_intervals = num_of_intervals;
}

DOPCManager::~DOPCManager()
{
    cleanup();
}


// remember to call this function after "reseting" DOPCManager
bool DOPCManager::runAlgorithm(Interval *intervalArray, Interval **leftIntervalPtrArray, int num_of_intervals, std::list<std::list<int> > &pathCover_out )
{
    bool ok = false;

    do {
        log_msg("DOPCManager::runAlgorithm(): start", llevel::INFO);

        m_initialized = checkInput(intervalArray, leftIntervalPtrArray, num_of_intervals);

        if (!m_initialized) {
            log_msg("DOPCManager::runAlgorithm(): failed to initialize manager" , llevel::ERROR);
            break;
        }

        if (!pathCover_out.empty()) {
            log_msg("DOPCManager::runAlgorithm(): pathCover is not empty" , llevel::ERROR);
            break;
        }

        log_msg("DOPCManager::runAlgorithm(): printing input", llevel::INFO);
        log_data(intervalArray, m_num_of_intervals, llevel::DEBUG);
        log_data(leftIntervalPtrArray, m_num_of_intervals, llevel::DEBUG);


        /* cleanup possible previous values */
        for (int i=0; i < m_num_of_intervals; i++) {
            intervalArray[i].right()->setUptreeID(0);
            intervalArray[i].right()->mp_Lprevious = nullptr;
            intervalArray[i].right()->mp_Lsuccessor = nullptr;

            intervalArray[i].left()->setUptreeID(0);
        }


        /* Create Consecutive Left Groups */
        log_msg("DOPCManager::runAlgorithm(): creating Consecutive Left Groups", llevel::INFO);
        createConsLeftGroups(intervalArray, leftIntervalPtrArray);

        /* Connect Right Endpoints in a doubly-linked list (L) */
        log_msg("DOPCManager::runAlgorithm(): creating L-list", llevel::INFO);
        createLlist(intervalArray);

        /* arrays for path tracking */
        int start[m_num_of_intervals] = {0};
        int next[m_num_of_intervals] = {0};


        log_msg("DOPCManager::runAlgorithm(): start main loop", llevel::INFO);

        for (int i = 0; i < m_num_of_intervals; i++) {

            RightInterval_Endpoint *b_k = nullptr;
            RightInterval_Endpoint *b_i = nullptr;
            RightInterval_Endpoint *b_h = nullptr;
            RightInterval_Endpoint *b_i_plus = nullptr;
            int k = -1, h = -1;

            int leader_id = uptree_find(intervalArray[i].left()->uptreeID());
            b_k = mp_uptree[leader_id].m_ptr;
            k = b_k->parent()->right_order_index();

            if (k == i) {
                next[i] = 0;
                start[i] = i;
            }
            if (k != i) {
                b_i = intervalArray[i].right();
                b_h = b_k->mp_Lsuccessor;
                h = b_h->parent()->right_order_index();

                if (h == i) {
                    next[k] = i;
                    start[i] = start[k];
                    uniteGroupsOnX(b_i, b_k);
                    deleteLElement(b_k);
                }
                if (h != i) {
                    next[k] = i;
                    next[i] = start[h];
                    start[h] = start[k];
                    start[i] = start[k];
                    start[start[h]] = start[k];
                    uniteGroupsOnX(b_h, b_k);
                    if (i != m_num_of_intervals - 1) {
                        b_i_plus = intervalArray[i+1].right();
                        uniteGroupsOnX(b_i_plus, b_i);
                    }
                    deleteLElement(b_k);
                    deleteLElement(b_i);
                }
            }

        }
        log_msg("DOPCManager::runAlgorithm(): end main loop", llevel::INFO);

        log_msg("DOPCManager::runAlgorithm(): printing input right endpoint array", llevel::INFO);
        log_data(intervalArray, m_num_of_intervals, llevel::DEBUG);

        log_msg("DOPCManager::runAlgorithm(): Printing start[] indexes array", llevel::INFO);
        log_data(start, m_num_of_intervals, llevel::DEBUG);

        log_msg("DOPCManager::runAlgorithm(): Printing next[] indexes array", llevel::INFO);
        log_data(next, m_num_of_intervals, llevel::DEBUG);

        /* create output */

        RightInterval_Endpoint* p_next_n = mp_dummyLstart;
        do {
            p_next_n = p_next_n->mp_Lsuccessor;

            int pathStart_index = p_next_n->parent()->right_order_index();
            int path_start = start[pathStart_index];
            int path_next = path_start;

            std::list<int> path;
            path.push_back(path_next);
            while (next[path_next] != 0 && next[path_next] != -1) {
                path_next = next[path_next];
                path.push_back(path_next);
            }

            pathCover_out.push_back(path);

        } while( p_next_n->mp_Lsuccessor != nullptr);

        log_msg("DOPCManager::runAlgorithm(): Printing output pathCover", llevel::INFO);
        log_data(pathCover_out, llevel::DEBUG);

        log_msg("DOPCManager::runAlgorithm(): checking output correctness", llevel::INFO);
        bool output_correctness_ok = true;
        for (std::list<std::list<int>>::const_iterator it = pathCover_out.begin(); it != pathCover_out.end(); it++) {

            if (!check_path_intersection(intervalArray, m_num_of_intervals, *it)) output_correctness_ok = false;
        }

        if (!output_correctness_ok) {
            log_msg("DOPCManager::runAlgorithm(): found incorrect path in output", llevel::ERROR);
            break;
        }

        log_msg("DOPCManager::runAlgorithm(): end", llevel::INFO);
        ok = true;
    } while(0);


    return ok;
}

void DOPCManager::resetManager()
{
    cleanup();
}

void DOPCManager::init()
{
    m_num_of_intervals = -1;
    m_initialized = false;
    mp_dummyLstart = nullptr;
    mp_uptree = nullptr;
}

void DOPCManager::cleanup()
{
    if (mp_dummyLstart) {
        delete mp_dummyLstart;
        mp_dummyLstart = nullptr;
    }

    if (mp_uptree) {
        delete [] mp_uptree;
        mp_uptree = nullptr;
    }
}

bool DOPCManager::checkInput(Interval *intervalArray, Interval **leftIntervalPtrArray, int num_of_intervals)
{
    bool ok = false;

    do {

        log_msg("DOPCManager::checkInput(): start", llevel::INFO);

        if (intervalArray == nullptr) {
            log_msg("DOPCManager::checkInput(): null array pointer in this function input" , llevel::ERROR);
            break;
        }
        if (leftIntervalPtrArray == nullptr) {
            log_msg("DOPCManager::checkInput(): null array pointer in this function input" , llevel::ERROR);
            break;
        }
        if (num_of_intervals != m_num_of_intervals) {
            log_msg("DOPCManager::checkInput(): size != m_num_of_intervals" , llevel::ERROR);
            break;
        }
        if (m_num_of_intervals < 3) {
            log_msg("DOPCManager::checkInput(): input size < 3. Can't find hamiltonian cycle with only 2 intervals" , llevel::ERROR);
            break;
        }
        /* check if input is in ascending rightEp order*/
        int prev_rightEp_val = -1;
        bool ascending_rightEp_ordering_ok = true;

        prev_rightEp_val = intervalArray[0].right()->value();
        for (int i = 1; i < m_num_of_intervals; i++) {

            if(prev_rightEp_val > intervalArray[i].right()->value()) {
                ascending_rightEp_ordering_ok = false;
                break;
            }
            if (intervalArray[i].right_order_index() != i) {
                ascending_rightEp_ordering_ok = false;
                break;
            }
            prev_rightEp_val = intervalArray[i].right()->value();
        }

        if (intervalArray[0].right_order_index() != 0) {
            ascending_rightEp_ordering_ok = false;
        }

        if (!ascending_rightEp_ordering_ok) {
            log_msg("DOPCManager::checkInput(): input is not ordered based on right endpoints (ascending)" , llevel::ERROR);
            break;
        }

        /* check if input is in ascending leftEp order*/

        int prev_leftEp_val = -1;
        bool ascending_leftEp_ordering_ok = true;

        prev_leftEp_val = leftIntervalPtrArray[0]->left()->value();
        for (int i = 1; i < m_num_of_intervals; i++) {

            if (prev_leftEp_val > leftIntervalPtrArray[i]->left()->value()) {
                ascending_leftEp_ordering_ok = false;
                break;
            }
            if (leftIntervalPtrArray[i]->left_order_index() != i){
                ascending_leftEp_ordering_ok = false;
                break;
            }
            prev_leftEp_val = leftIntervalPtrArray[i]->left()->value();
        }

        if (leftIntervalPtrArray[0]->left_order_index() != 0) {
            ascending_leftEp_ordering_ok = false;
        }

        if (!ascending_leftEp_ordering_ok) {
            log_msg("DOPCManager::checkInput(): input is not ordered based on left endpoints (ascending)" , llevel::ERROR);
            break;
        }

        /* check if input has left_ep value[i] < right_ep value[i] */
        bool normal_intervals_ok = true;

        for (int i=0; i < m_num_of_intervals; i++) {
            if (intervalArray[i].left()->value() > intervalArray[i].right()->value()) {
                normal_intervals_ok = false;
                break;
            }
        }
        if (!normal_intervals_ok) {
            log_msg("DOPCManager::checkInput(): found interval with left_ep->value() > right_ep->value() " , llevel::ERROR);
            break;
        }

        log_msg("DOPCManager::checkInput(): end", llevel::INFO);
        ok = true;
    } while(0);

    return ok;
}

void DOPCManager::initUpTree(Interval *&intervalArray)
{
    log_msg("DOPCManager::initUpTree(): start", llevel::INFO);

    /* there is 1 node for each endpoint */
    int num_of_eps = 2 * m_num_of_intervals;

    if (mp_uptree) {
        delete [] mp_uptree;
        mp_uptree = nullptr;
    }
    mp_uptree = new UptreeNode[num_of_eps + 1]; // we don't use mp_uptree[0]

    /* assign an ID to each endpoint from [1,2N], EP-order and EP-value does not matter */
    int count_id = 1;
    for (int i=0; i < m_num_of_intervals; i++) {
        intervalArray[i].right()->setUptreeID(count_id);
        mp_uptree[count_id] = UptreeNode(0, intervalArray[i].right());
        count_id++;
        intervalArray[i].left()->setUptreeID(count_id);
        mp_uptree[count_id] = UptreeNode(0, nullptr);
        count_id++;
    }
    log_msg("DOPCManager::initUpTree(): end", llevel::INFO);
}

int DOPCManager::uptree_find(int x)
{
    if (mp_uptree[x].m_parent_id <= 0) {
        return x; // return root = set name
    } else {
        // find parent
        return mp_uptree[x].m_parent_id = uptree_find(mp_uptree[x].m_parent_id);
    }
}

int DOPCManager::uptree_unite(int x, int y)
{
    int ret_id = 0;

    if (x == y) {
        log_msg("uptree_unite(): can't unite node with itself" , llevel::ERROR);
        return ret_id;
    }

    if (mp_uptree[x].m_parent_id > 0 || mp_uptree[y].m_parent_id > 0) {
        log_msg("uptree_unite(): called on child nodes" , llevel::ERROR);
        return ret_id; // we can only unite roots -> root values are <=0
    }

    // union by size, return id of bigger sized node

    if (-mp_uptree[x].m_parent_id > -mp_uptree[y].m_parent_id) {
        mp_uptree[x].m_parent_id +=  mp_uptree[y].m_parent_id;
        mp_uptree[y].m_parent_id = x;
        ret_id = x;
    } else {
        mp_uptree[y].m_parent_id +=  mp_uptree[x].m_parent_id;
        mp_uptree[x].m_parent_id = y;
        ret_id = y;
    }

    return ret_id;
}

void DOPCManager::createConsLeftGroups(Interval *&intervalArray, Interval **&leftIntervalPtrArray)
{
    log_msg("DOPCManager::createConsLeftGroups(): start", llevel::INFO);

    /* initialize up-tree */
    initUpTree(intervalArray);

    int i = 0, j = 0;

    RightInterval_Endpoint *head_ptr = nullptr;
    LeftInterval_Endpoint *ai_ptr = nullptr;

    head_ptr = intervalArray[j].right();

    while (i < m_num_of_intervals && j < m_num_of_intervals) {

        ai_ptr = leftIntervalPtrArray[i]->left();
        if (ai_ptr->value() < head_ptr->value()) {

            mp_uptree[ai_ptr->uptreeID()].m_parent_id = head_ptr->uptreeID();
            mp_uptree[head_ptr->uptreeID()].m_parent_id += -1;
            i ++;
        } else {
            head_ptr = intervalArray[j].right();
            j ++;
        }
    }

    while (i < m_num_of_intervals) {
        ai_ptr = leftIntervalPtrArray[i]->left();
        mp_uptree[ai_ptr->uptreeID()].m_parent_id = head_ptr->uptreeID();
        mp_uptree[head_ptr->uptreeID()].m_parent_id += -1;
        i ++;
    }

    while (j < m_num_of_intervals) {
        head_ptr = intervalArray[j].right();
        j ++;
    }

    log_msg("DOPCManager::createConsLeftGroups(): end", llevel::INFO);
}

void DOPCManager::uniteGroupsOnX(RightInterval_Endpoint *&x, RightInterval_Endpoint *&y)
{
    int bigger_id = uptree_unite(x->uptreeID(), y->uptreeID());

    if (bigger_id != x->uptreeID()) {
        // swap x and y in up-tree
        y->setUptreeID(x->uptreeID());
        x->setUptreeID(bigger_id);

        // update ptrs on nodes
        mp_uptree[x->uptreeID()].m_ptr = x;
        mp_uptree[y->m_uptreeID].m_ptr = y;
    }
}

void DOPCManager::createLlist(Interval *&intervalArray)
{
    log_msg("DOPCManager::createLlist(): start", llevel::INFO);

    mp_dummyLstart = new RightInterval_Endpoint(); // dummy endpoint -> default constructor

    for (int i=0; i < m_num_of_intervals; i++) {
        if (i == 0) {
            intervalArray[i].right()->mp_Lprevious = mp_dummyLstart;
            mp_dummyLstart->mp_Lprevious = nullptr;
            mp_dummyLstart->mp_Lsuccessor = intervalArray[i].right();
        } else {
            intervalArray[i].right()->mp_Lprevious = intervalArray[i-1].right();
        }
        if (i == m_num_of_intervals-1) {
            intervalArray[i].right()->mp_Lsuccessor = nullptr;
        } else {
            intervalArray[i].right()->mp_Lsuccessor = intervalArray[i+1].right();
        }
    }


    // because this is the only way to parse L list from start to end, leaving it here as comments.

//    RightInterval_Endpoint* p_next_n = mp_dummyLstart;
//    do {
//        p_next_n = p_next_n->mp_Lsuccessor;
//        DEBUG_OUT << "b_" << p_next_n->parent()->right_order_index() << " = " << p_next_n->value() DEBUG_OUT_END;
//    } while( p_next_n->mp_Lsuccessor != nullptr);

    log_msg("DOPCManager::createLlist(): end", llevel::INFO);
}

bool DOPCManager::deleteLElement(RightInterval_Endpoint *&e)
{
    bool ok = false;

    do {
        if (e == nullptr) {
            log_msg("deleteLElement called with null input" , llevel::ERROR);
            break;
        }
        log_msg("DOPCManager::deleteLElement called on " + std::to_string(e->value()), llevel::EXTRA_DEBUG);

        if (e->mp_Lprevious) {
            e->mp_Lprevious->mp_Lsuccessor = e->mp_Lsuccessor;
        }
        if (e->mp_Lsuccessor) {
            e->mp_Lsuccessor->mp_Lprevious = e->mp_Lprevious;
        }


        e->mp_Lprevious = nullptr;
        e->mp_Lsuccessor = nullptr;

        ok = true;
    } while(0);

    return ok;
}

// -----------------

UptreeNode::UptreeNode(const UptreeNode &other)
{
    m_parent_id = other.m_parent_id;
    m_ptr = other.m_ptr;
}

UptreeNode &UptreeNode::operator=(const UptreeNode &other)
{
    if (this != &other) {

        m_parent_id = 0;
        m_ptr = nullptr;

        m_parent_id = other.m_parent_id;
        m_ptr = other.m_ptr;
    }
    return *this;
}
