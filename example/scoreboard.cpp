#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "obj_dir/Vscoreboard.h" // Need to be modified for different verilog based modules

#define MAX_TIME 10
vluint64_t sim_time = 0;
int total_errors = 0;

// update the simulator's time
void update_sim(Vscoreboard *dut, VerilatedVcdC *m_trace)
{
    dut->eval();
    m_trace->dump(sim_time);
    sim_time++;
}

int get_rand()
{
    return rand() % 1000;
}

/*
* Function to toggle clk to reach next similar clock edge
* NOTE: Call function after relevant signals have been set
*/
// update clk
void toggle_pos_edge_clk(Vscoreboard *dut, VerilatedVcdC *m_trace)
{
    dut->clk = 0;
    update_sim(dut, m_trace);
    dut->clk = 1;
    update_sim(dut, m_trace);
}

int main(int argc, char** argv, char** env){
    srand(time(NULL));
    // initialize the top module
    Vscoreboard *dut = new Vscoreboard; 
    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC;
    
    dut->trace(m_trace, 5);
    m_trace->open("waveform.vcd");
    dut->clk = 0;
    update_sim(dut, m_trace);

    // reset
    dut->rst = 1;
    toggle_pos_edge_clk(dut, m_trace);
    dut->rst = 0;
    toggle_pos_edge_clk(dut, m_trace);

    // add data
    dut->in_idx = 0;
    dut->in_pc_vld = 1;
    dut->in_pc = 0xBEEF;
    dut->in_opcode_vld = 1;
    dut->in_opcode = 1;
    dut->in_completed_vld = 0;
    dut->in_invalidate_vld = 0;
    toggle_pos_edge_clk(dut, m_trace);

    while(sim_time < MAX_TIME){
        toggle_pos_edge_clk(dut, m_trace);
    }

    m_trace->close();
    delete dut;

    std::cout << "Completed" << std::endl;

    exit(EXIT_SUCCESS);
}