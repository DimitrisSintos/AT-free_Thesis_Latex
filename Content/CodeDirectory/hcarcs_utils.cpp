#include "hcarcs_utils.h"
#include "endpoint.h"
#include "hcarcs_object.h"
#include <unordered_map>

const std::unordered_map<llevel, std::string> llevelsToStringMap = {
    {llevel::OUTPUT, "--- OUTPUT: "},
    {llevel::ERROR, "--- ERROR: "},
    {llevel::INFO, "--- INFO: "},
    {llevel::DEBUG, "DEBUG: "},
    {llevel::EXTRA_DEBUG, "EXTRA_DEBUG: "}
};

EasyLogger *EasyLogger::mp_singleton = nullptr;


void log_data(Interval *intervalArray, int arraySize, llevel level)
{
    std::string final_str;
    std::string log_str = "\n";

    for (int i = 0; i < arraySize; i++) {
        log_str.append("I[" + std::to_string(i) + "]( ");
        log_str.append(std::to_string(intervalArray[i].left()->value()) + ", ");
        log_str.append(std::to_string(intervalArray[i].right()->value()) + " )");
        log_str.append("\n");
    }

    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + log_str;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}

void log_data(Interval **intervalPtrArray, int arraySize, llevel level)
{
    std::string final_str;
    std::string log_str = "\n";

    for (int i = 0; i < arraySize; i++) {
        log_str.append("I_ptr[" + std::to_string(i) + "]( ");
        log_str.append(std::to_string(intervalPtrArray[i]->left()->value()) + ", ");
        log_str.append(std::to_string(intervalPtrArray[i]->right()->value()) + " )");
        log_str.append("\n");
    }

    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + log_str;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}

void log_data(Arc *arcArray, int arraySize, llevel level)
{
    std::string final_str;
    std::string log_str = "\n";

    for (int i = 0; i < arraySize; i++) {
        log_str.append("A[" + std::to_string(i) + "]( ");
        log_str.append(std::to_string(arcArray[i].head()->value()) + ", ");
        log_str.append(std::to_string(arcArray[i].tail()->value()) + " )");
        log_str.append("\n");
    }

    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + log_str;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}

void log_data(Arc **arcPtrArray, int arraySize, llevel level)
{
    std::string final_str;
    std::string log_str = "\n";

    for (int i = 0; i < arraySize; i++) {
        log_str.append("A_ptr[" + std::to_string(i) + "]( ");
        log_str.append(std::to_string(arcPtrArray[i]->head()->value()) + ", ");
        log_str.append(std::to_string(arcPtrArray[i]->tail()->value()) + " )");
        log_str.append("\n");
    }

    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + log_str;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}

void log_data(const std::list<int> &list, llevel level)
{
    std::string final_str;
    std::string log_str = "\n";

    log_str.append("{ ");
    for (std::list<int>::const_iterator it = list.begin(); it != list.end(); it++) {
        log_str.append(std::to_string(*it) + " ");
    }
    log_str.append("}\n");

    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + log_str;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}

void log_data(const std::list<std::list<int> > &list_of_lists, llevel level)
{
    std::string final_str;
    std::string log_str = "\n";

    log_str.append("{\n");
    for (std::list<std::list<int>>::const_iterator ll_it = list_of_lists.begin(); ll_it != list_of_lists.end(); ll_it++) {

        std::list<int> list = *ll_it;
        log_str.append("{ ");
        for (std::list<int>::const_iterator it = list.begin(); it != list.end(); it++) {
            log_str.append(std::to_string(*it) + " ");
        }
        log_str.append("}\n");

    }
    log_str.append("}\n");

    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + log_str;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}

void log_data(int *indexesArray, int arraySize, llevel level)
{
    std::string final_str;
    std::string log_str = "\n";

    for (int i = 0; i < arraySize; i++) {
        log_str.append("index[" + std::to_string(i) + "] = ");
        log_str.append(std::to_string(indexesArray[i]));
        log_str.append("\n");
    }

    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << log_str.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + log_str;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}


void log_msg(const std::string &msg, llevel level)
{
    std::string final_str;
    if (level == llevel::EXTRA_DEBUG) {
#if EXTRA_DEBUG_ON
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << msg.c_str() DEBUG_OUT_END;
#endif
    } else {
        DEBUG_OUT << llevelsToStringMap.at(level).c_str() << msg.c_str() DEBUG_OUT_END;
    }

    final_str = llevelsToStringMap.at(level) + msg;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(final_str));
}

void log_dashedLine()
{
    std::string log_str = "----------------------\n";
    DEBUG_OUT << log_str.c_str() DEBUG_OUT_END;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(log_str));
}

void log_asteriskLine()
{
    std::string log_str = "**********************\n";
    DEBUG_OUT << log_str.c_str() DEBUG_OUT_END;
    emit EasyLogger::getInstance()->logMessage(QString::fromStdString(log_str));
}

bool check_path_intersection(Interval *intervalArray, int arraySize, std::list<int> path)
{
    bool ok = true;

    std::list<int>::iterator p_it;
    std::list<int>::iterator final_iter = path.end(); // to stop early
    final_iter --;

    log_msg("check_path_intersection: start", llevel::EXTRA_DEBUG);
    log_data(intervalArray, arraySize, llevel::EXTRA_DEBUG);

    for (p_it = path.begin(); p_it != path.end(); p_it ++) {

        if (p_it == final_iter) continue; // at the end of the list, can't check "next"

        std::list<int>::iterator p_it_next = p_it;
        p_it_next ++;

        if (intervalArray[*p_it].intersects_interval(intervalArray[*p_it_next])) {
            std::string debug_msg = "correct intersection on: i_"
                    + std::to_string(*p_it) + " / i_" + std::to_string(*p_it_next);
            log_msg(debug_msg, llevel::EXTRA_DEBUG);
        } else {
            std::string err_msg = "check_path_intersection: no intersection found on: i_"
                    + std::to_string(*p_it) + " / i_" + std::to_string(*p_it_next);
            log_msg(err_msg, llevel::ERROR);
            ok = false;
            // do not "break;" to catch all wrong intersections
        }
    }
    log_msg("check_path_intersection: end", llevel::EXTRA_DEBUG);

    return ok;
}

bool check_path_intersection(Arc *arcArray, int arraySize, std::list<int> path)
{
    bool ok = true;

    std::list<int>::iterator p_it;
    std::list<int>::iterator final_iter = path.end(); // to stop early
    final_iter --;

    log_msg("check_path_intersection: start", llevel::EXTRA_DEBUG);
    log_data(arcArray, arraySize, llevel::EXTRA_DEBUG);

    for (p_it = path.begin(); p_it != path.end(); p_it ++) {

        if (p_it == final_iter) continue; // at the end of the list, can't check "next"

        std::list<int>::iterator p_it_next = p_it;
        p_it_next ++;

        if (arcArray[*p_it].intersects_arc(arcArray[*p_it_next])) {
            std::string debug_msg = "correct intersection on: a_"
                    + std::to_string(*p_it) + " / a_" + std::to_string(*p_it_next);
            log_msg(debug_msg, llevel::EXTRA_DEBUG);
        } else {
            std::string err_msg = "check_path_intersection: no intersection found on: a_"
                    + std::to_string(*p_it) + " / a_" + std::to_string(*p_it_next);
            log_msg(err_msg, llevel::ERROR);
            ok = false;
            // do not "break;" to catch all wrong intersections
        }
    }
    log_msg("check_path_intersection: end", llevel::EXTRA_DEBUG);

    return ok;
}


// -----

//EasyLogger *EasyLogger::mp_singleton = nullptr;

EasyLogger *EasyLogger::getInstance()
{
    if (mp_singleton == nullptr) {
        mp_singleton = new EasyLogger;
    }
    return mp_singleton;
}
