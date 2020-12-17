/**
 * This file contains implementations for methods in the PageTable class.
 *
 * You'll need to add code here to make the corresponding tests pass.
 */

#include "page_table/page_table.h"

using namespace std;


size_t PageTable::get_present_page_count() const {
    // TODO: implement me
    //how many pages are present
    auto present = 0;
    for(int i = 0; i < this->rows.size(); i++)
    {
        if(this->rows[i].present)
        {
            present++;
        }
    }
    return present;
}


size_t PageTable::get_oldest_page() const {
    // TODO: implement me
    //what page is oldest in memory
    auto oldest_age = 0;
    auto oldest_num = 0;
    for(int i = 0; i < this->rows.size(); i++)
    {
        if(this->rows[i].present && this->rows[i].loaded_at < oldest_age)
        {
            oldest_age = this->rows[i].loaded_at;
            oldest_num = i;
        }
    }
    return oldest_num;
}


size_t PageTable::get_least_recently_used_page() const {
    // TODO: implement me
    //what page in mem was least recently used?
    auto least_age = 0;
    auto least_num = 0;
    for(int i = 0; i < this->rows.size(); i++)
    {
        if(this->rows[i].present && this->rows[i].last_accessed_at < least_age)
        {
            least_age = this->rows[i].last_accessed_at;
            least_num = i;
        }
    }
    return least_num;
}
