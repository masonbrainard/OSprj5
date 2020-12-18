/**
 * This file contains implementations for the methods defined in the Simulation
 * class.
 *
 * You'll probably spend a lot of your time here.
 */

#include "simulation/simulation.h"
#include <stdexcept>
#include <bitset>

Simulation::Simulation(FlagOptions& flags)
{
    this->flags = flags;
    // this->frames.reserve(this->NUM_FRAMES);
    for(int i = 0; i < NUM_FRAMES; i++)
    {
        frames.push_back(Frame());
        free_frames.push_back(i);
    }
}

void Simulation::run() {
    for (auto virtual_address : this->virtual_addresses) {
        this->perform_memory_access(virtual_address);
    }

    this->print_summary();
}

char Simulation::perform_memory_access(const VirtualAddress& virtual_address) {
    if (this->flags.verbose) {
        std::cout << virtual_address << std::endl;
    }

    if(!this->processes[virtual_address.process_id]->is_valid_page(virtual_address.page))
    {
        throw("Wow that sucks.");
    }
    
    if(this->processes[virtual_address.process_id]->page_table.rows[virtual_address.page].present)
    {
        if (this->flags.verbose) 
        {
            std::cout << "    -> IN MEMORY" << std::endl;
        }
    }
    else
    {
        if (this->flags.verbose) 
        {
            std::cout << "    -> PAGE FAULT" << std::endl;
        }        
        handle_page_fault(this->processes[virtual_address.process_id], virtual_address.page);
    }

    if (this->flags.verbose) 
    {
        //calc physical address  
        auto frame = processes[virtual_address.process_id]->page_table.rows[virtual_address.page].frame;
        auto offset = virtual_address.offset;

        auto frame_bits = std::bitset<virtual_address.PAGE_BITS>(frame);  
        auto offset_bits = std::bitset<virtual_address.OFFSET_BITS>(offset);   

        std::cout << "    -> physical address " << frame_bits << offset_bits  << " [frame: " << frame << "; offset: " << offset << "]" << std::endl;
        
        std::cout << "    -> RSS: " << processes[virtual_address.process_id]->get_rss() << std::endl << std::endl;
    } 



    return '0';
  
}

void Simulation::handle_page_fault(Process* process, size_t page) {
    
    // TODO: implement me
    this->page_faults++; //total page faults
    process->page_faults++; //per process page faults
    process->page_table.rows[page].present = true;

    //are there any free frames?
    if(free_frames.size() > 0 && process->get_rss() < this->flags.max_frames)
    {
        //grab frist frame and place process + page in
        frames[free_frames.front()].set_page(process, page);
        process->page_table.rows[page].frame = free_frames.front();
        free_frames.pop_front();
    }
    //no free frames
    else
    {        
        //find page to replace
        size_t replaced_page = 0;

        if(this->flags.strategy == ReplacementStrategy::FIFO)
        {
            //find oldest page
            replaced_page = process->page_table.get_oldest_page();
        }
        else
        {
            //lru
            replaced_page = process->page_table.get_least_recently_used_page();
        }
        //set replaced page present to false
        process->page_table.rows[replaced_page].present = false;
        process->page_table.rows[page].frame = process->page_table.rows[replaced_page].frame;
    }
}

void Simulation::print_summary() {
    if (!this->flags.csv) {
        boost::format process_fmt(
            "Process %3d:  "
            "ACCESSES: %-6lu "
            "FAULTS: %-6lu "
            "FAULT RATE: %-8.2f "
            "RSS: %-6lu\n");

        for (auto entry : this->processes) {
            std::cout << process_fmt
                % entry.first
                % entry.second->memory_accesses
                % entry.second->page_faults
                % entry.second->get_fault_percent()
                % entry.second->get_rss();
        }

        // Print statistics.
        boost::format summary_fmt(
            "\n%-25s %12lu\n"
            "%-25s %12lu\n"
            "%-25s %12lu\n");

        std::cout << summary_fmt
            % "Total memory accesses:"
            % this->virtual_addresses.size()
            % "Total page faults:"
            % this->page_faults
            % "Free frames remaining:"
            % this->free_frames.size();
    }

    if (this->flags.csv) {
        boost::format process_fmt(
            "%d,"
            "%lu,"
            "%lu,"
            "%.2f,"
            "%lu\n");

        for (auto entry : processes) {
            std::cout << process_fmt
                % entry.first
                % entry.second->memory_accesses
                % entry.second->page_faults
                % entry.second->get_fault_percent()
                % entry.second->get_rss();
        }

        // Print statistics.
        boost::format summary_fmt(
            "%lu,,,,\n"
            "%lu,,,,\n"
            "%lu,,,,\n");

        std::cout << summary_fmt
            % this->virtual_addresses.size()
            % this->page_faults
            % this->free_frames.size();
    }
}

int Simulation::read_processes(std::istream& simulation_file) {
    int num_processes;
    simulation_file >> num_processes;

    for (int i = 0; i < num_processes; ++i) {
        int pid;
        std::string process_image_path;

        simulation_file >> pid >> process_image_path;

        std::ifstream proc_img_file(process_image_path);

        if (!proc_img_file) {
            std::cerr << "Unable to read file for PID " << pid << ": " << process_image_path << std::endl;
            return 1;
        }
        this->processes[pid] = Process::read_from_input(proc_img_file);
    }
    return 0;
}

int Simulation::read_addresses(std::istream& simulation_file) {
    int pid;
    std::string virtual_address;

    try {
        while (simulation_file >> pid >> virtual_address) {
            this->virtual_addresses.push_back(VirtualAddress::from_string(pid, virtual_address));
        }
    } catch (const std::exception& except) {
        std::cerr << "Error reading virtual addresses." << std::endl;
        std::cerr << except.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error reading virtual addresses." << std::endl;
        return 1;
    }
    return 0;
}

int Simulation::read_simulation_file() {
    std::ifstream simulation_file(this->flags.filename);
    // this->simulation_file.open(this->flags.filename);

    if (!simulation_file) {
        std::cerr << "Unable to open file: " << this->flags.filename << std::endl;
        return -1;
    }
    int error = 0;
    error = this->read_processes(simulation_file);

    if (error) {
        std::cerr << "Error reading processes. Exit: " << error << std::endl;
        return error;
    }

    error = this->read_addresses(simulation_file);

    if (error) {
        std::cerr << "Error reading addresses." << std::endl;
        return error;
    }

    if (this->flags.file_verbose) {
        for (auto entry: this->processes) {
            std::cout << "Process " << entry.first << ": Size: " << entry.second->size() << std::endl;
        }

        for (auto entry : this->virtual_addresses) {
            std::cout << entry << std::endl;
        }
    }

    return 0;
}
