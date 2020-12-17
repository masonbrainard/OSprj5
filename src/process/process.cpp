/**
 * This file contains implementations for methods in the Process class.
 *
 * You'll need to add code here to make the corresponding tests pass.
 */

#include "process/process.h"

using namespace std;


Process* Process::read_from_input(std::istream& in) {
    size_t num_bytes = 0;

    Page* page;
    vector<Page*> pages;

    while ((page = Page::read_from_input(in)) != nullptr) {
        pages.push_back(page);
        num_bytes += page->size();
    }

    return new Process(num_bytes, pages);
}


size_t Process::size() const
{
    //TODO
    return this->num_bytes;
}


bool Process::is_valid_page(size_t index) const
{
    // TODO
    if(index < 0 || index > num_bytes)
    {
        return false;
    }
    else
    {
        return true;
    }
}


size_t Process::get_rss() const
{
    // TODO
    //return resident set size of process
    return 0;
}


double Process::get_fault_percent() const
{
    //TODO
    //is this right?
    return page_faults / (memory_accesses + page_faults);
}
